// Copyright 2024 The ICL Format Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// icl-lsp — a Language Server for IEEE 1687-2014 (IJTAG) ICL.
//
// Reuses the formatter's front end for both features it offers: syntax
// diagnostics and whole-document formatting. There is no second parser and no
// second copy of the grammar — a diagnostic is just "FormatText failed, here
// is the line the parser stopped on".
//
// Scope is deliberately minimal (diagnostics + formatting). Anything not
// advertised in `capabilities` is never requested, and unknown requests are
// answered with -32601 rather than crashing, which is what keeps the server
// alive when a client probes for features it does not have.

#include <cstdlib>
#include <iostream>
#include <map>
#include <string>

#include "format_job.h"
#include "json.h"

#ifndef ICL_FORMAT_VERSION
#define ICL_FORMAT_VERSION "0.0.0"
#endif

// Everything except the driver lives in namespace icl; the server itself is
// file-local, so pull the names in rather than qualifying each use.
using icl::Json;

namespace {

// ---- LSP base protocol: Content-Length framing over stdio ------------------

// Reads one message: a header block ending in a blank line, then exactly
// Content-Length bytes. Returns false at end of stream, which is how a client
// closing the pipe shuts the server down.
bool ReadMessage(std::string* body) {
  std::string header;
  for (;;) {
    const int c = std::cin.get();
    if (c == EOF) return false;
    header.push_back(static_cast<char>(c));
    const size_t size = header.size();
    if (size >= 4 && header.compare(size - 4, 4, "\r\n\r\n") == 0) break;
  }
  long length = -1;
  size_t pos = 0;
  while (pos < header.size()) {
    const size_t eol = header.find("\r\n", pos);
    if (eol == std::string::npos) break;
    const std::string line = header.substr(pos, eol - pos);
    pos = eol + 2;
    const size_t colon = line.find(':');
    if (colon == std::string::npos) continue;
    std::string key = line.substr(0, colon);
    std::string value = line.substr(colon + 1);
    while (!value.empty() && (value.front() == ' ' || value.front() == '\t')) {
      value.erase(value.begin());
    }
    while (!value.empty() && (value.back() == ' ' || value.back() == '\t')) {
      value.pop_back();
    }
    // Header names are case-insensitive (the header block is HTTP-shaped).
    // Comparing exactly meant `content-length:` — which a client is entitled
    // to send — silently dropped the whole message, because a frame with no
    // length cannot be read and the loop moved on to the next one.
    std::string lower = key;
    for (char& c : lower) {
      if (c >= 'A' && c <= 'Z') c = static_cast<char>(c - 'A' + 'a');
    }
    if (lower == "content-length") {
      length = std::strtol(value.c_str(), nullptr, 10);
    }
    // Content-Type is sent by some clients; ignored on purpose.
  }
  if (length < 0) return false;
  body->resize(static_cast<size_t>(length));
  if (length > 0) {
    std::cin.read(&(*body)[0], length);
    if (std::cin.gcount() != length) return false;
  }
  return true;
}

void WriteMessage(const Json& message) {
  const std::string body = message.Serialize();
  std::cout << "Content-Length: " << body.size() << "\r\n\r\n" << body;
  std::cout.flush();
}

// ---- small helpers ---------------------------------------------------------

// Object field access that tolerates a missing parent. Request shapes vary
// between clients, and a null dereference would take the whole server down.
std::string GetString(const Json* object, const char* key,
                      const std::string& fallback = "") {
  if (object == nullptr) return fallback;
  const Json* value = object->Find(key);
  return value == nullptr ? fallback : value->string_value(fallback);
}

long long GetInt(const Json* object, const char* key, long long fallback) {
  if (object == nullptr) return fallback;
  const Json* value = object->Find(key);
  return value == nullptr ? fallback : value->int_value(fallback);
}

// The document uri every textDocument/* notification carries.
std::string UriOf(const Json& params) {
  return GetString(params.Find("textDocument"), "uri");
}

Json Position(long long line, long long character) {
  return Json::Object({{"line", Json::Int(line)},
                       {"character", Json::Int(character)}});
}

// Length of line |line_index| (0-based) in bytes, used to underline a whole
// line for a diagnostic: the parser reports a line but no column.
long long LineLength(const std::string& text, long long line_index) {
  size_t start = 0;
  for (long long i = 0; i < line_index; ++i) {
    const size_t newline = text.find('\n', start);
    if (newline == std::string::npos) return 0;
    start = newline + 1;
  }
  const size_t newline = text.find('\n', start);
  const size_t end = (newline == std::string::npos) ? text.size() : newline;
  return static_cast<long long>(end - start);
}

// The (line, character) just past the last byte of |text| — the end of the
// last line, which LineLength already knows how to measure.
//
// ponytail: counts bytes, not UTF-16 code units, which LSP actually specifies.
// Correct for ASCII ICL; a line containing non-ASCII before the reported point
// under-reports the column. Upgrade path: convert to UTF-16 units if a
// diagnostic on a non-ASCII line ever matters — it does not affect formatting,
// which replaces the whole document.
void EndOfDocument(const std::string& text, long long* line,
                   long long* character) {
  *line = 0;
  for (const char c : text) {
    if (c == '\n') ++*line;
  }
  *character = LineLength(text, *line);
}

// ---- server ----------------------------------------------------------------

class LspServer {
 public:
  void Run() {
    std::string body;
    while (ReadMessage(&body)) {
      std::string error;
      Json message;
      if (!Json::Parse(body, &message, &error)) {
        // A malformed frame is not recoverable: there is no id to reply to.
        // Keep serving; the client will usually restart us.
        std::cerr << "icl-lsp: malformed request: " << error << "\n";
        continue;
      }
      Handle(message);
      if (exit_requested_) break;
    }
  }

 private:
  void Handle(const Json& message) {
    const std::string method = GetString(&message, "method");
    const Json* id = message.Find("id");
    const bool is_request = id != nullptr && !id->is_null();

    // A request-shaped method sent without an id is a client bug; replying
    // would need an id to reply to. Ignore it rather than dereferencing null.
    const bool can_reply = is_request;

    if (method == "initialize") {
      if (can_reply) Reply(*id, InitializeResult());
    } else if (method == "shutdown") {
      if (can_reply) Reply(*id, Json::Null());
    } else if (method == "exit") {
      exit_requested_ = true;
    } else if (method == "textDocument/didOpen") {
      const Json& params = Params(message);
      const Json* document = params.Find("textDocument");
      Open(UriOf(params), GetString(document, "text"));
    } else if (method == "textDocument/didChange") {
      const Json& params = Params(message);
      const std::string uri = UriOf(params);
      // Full sync (see `textDocumentSync`): the last change carries the whole
      // document, so there is nothing to merge.
      const Json* changes = params.Find("contentChanges");
      if (changes != nullptr && !changes->elements().empty()) {
        const Json& last = changes->elements().back();
        // Reading a missing or null `text` as "" would blank the document,
        // which is far worse than ignoring the change. Full sync means the
        // field is always a string; anything else is a client we cannot trust.
        const Json* text = last.Find("text");
        if (text != nullptr && text->type() == Json::Type::kString) {
          Open(uri, text->string_value());
        }
      }
    } else if (method == "textDocument/didClose") {
      documents_.erase(UriOf(Params(message)));
    } else if (method == "textDocument/formatting") {
      if (can_reply) Reply(*id, Formatting(Params(message)));
    } else if (is_request) {
      // Answering with a result *and* an error for the same id would be two
      // responses to one request; unsupported means error only.
      ErrorReply(*id, -32601, "unsupported method: " + method);
    }
    // Notifications we do not implement (initialized, didSave,
    // workspace/didChangeConfiguration, $/setTrace, ...) are ignored: replying
    // to a notification is a protocol violation and can wedge the client.
  }

  static const Json& Params(const Json& message) {
    static const Json empty = Json::Object({});
    const Json* params = message.Find("params");
    return params == nullptr ? empty : *params;
  }

  void Reply(const Json& id, Json result) {
    WriteMessage(Json::Object({{"jsonrpc", Json::String("2.0")},
                               {"id", id},
                               {"result", std::move(result)}}));
  }

  void ErrorReply(const Json& id, long long code, const std::string& message) {
    WriteMessage(Json::Object(
        {{"jsonrpc", Json::String("2.0")},
         {"id", id},
         {"error", Json::Object({{"code", Json::Int(code)},
                                 {"message", Json::String(message)}})}}));
  }

  static Json InitializeResult() {
    return Json::Object({
        {"capabilities",
         Json::Object({
             // 1 = TextDocumentSyncKind.Full: the client resends the whole
             // document on every change, so no incremental merging is needed.
             {"textDocumentSync", Json::Int(1)},
             {"documentFormattingProvider", Json::Bool(true)},
         })},
        {"serverInfo", Json::Object({{"name", Json::String("icl-lsp")},
                                     {"version",
                                      Json::String(ICL_FORMAT_VERSION)}})},
    });
  }

  // Stores |text| and publishes diagnostics for it.
  void Open(const std::string& uri, const std::string& text) {
    documents_[uri] = text;
    PublishDiagnostics(uri, text);
  }

  void PublishDiagnostics(const std::string& uri, const std::string& text) {
    // Same pipeline the CLI runs, same diagnostics. Diagnostics do not care
    // about indentation, so the default width is fine — and the request has
    // no tabSize to honour here.
    const icl::FormatOutcome outcome =
        icl::FormatDocument(text, icl::FormatRequest());
    Json::Elements diagnostics;
    if (!outcome.ok) {
      // The job's line is 1-based; LSP positions are 0-based.
      const long long line = outcome.error_line > 0 ? outcome.error_line - 1
                                                    : 0;
      diagnostics.push_back(Json::Object({
          {"range", Json::Object({{"start", Position(line, 0)},
                                  {"end",
                                   Position(line, LineLength(text, line))}})},
          {"severity", Json::Int(1)},  // 1 = Error.
          {"source", Json::String("icl-lsp")},
          {"message", Json::String(outcome.error_message)},
      }));
    }
    WriteMessage(Json::Object(
        {{"jsonrpc", Json::String("2.0")},
         {"method", Json::String("textDocument/publishDiagnostics")},
         {"params", Json::Object({{"uri", Json::String(uri)},
                                  {"diagnostics",
                                   Json::Array(std::move(diagnostics))}})}}));
  }

  Json Formatting(const Json& params) {
    const Json* document = params.Find("textDocument");
    const std::string uri = GetString(document, "uri");
    const auto it = documents_.find(uri);
    if (it == documents_.end()) return Json::Array({});
    const std::string& text = it->second;

    // Honour the editor's tabSize: it is the same knob the CLI exposes as -n,
    // so the two front ends agree on what "indent" means. Clamping is shared
    // (see format_job.h) rather than done here, so the CLI's -n and this path
    // cannot end up with different limits.
    icl::FormatRequest request;
    request.indent_width = static_cast<int>(
        GetInt(params.Find("options"), "tabSize", 2));

    const icl::FormatOutcome outcome = icl::FormatDocument(text, request);
    // A file with a syntax error is left untouched: returning no edits is the
    // only safe answer, since there is no formatted form to hand back. So is
    // a file that is already formatted — sending an edit that replaces the
    // text with itself makes the editor mark the buffer dirty for nothing.
    if (!outcome.ok || !outcome.ChangedFrom(text)) return Json::Array({});
    const std::string& output = outcome.text;

    long long end_line = 0;
    long long end_character = 0;
    EndOfDocument(text, &end_line, &end_character);
    Json::Elements edits;
    edits.push_back(Json::Object({
        {"range", Json::Object({{"start", Position(0, 0)},
                                {"end", Position(end_line, end_character)}})},
        {"newText", Json::String(output)},
    }));
    return Json::Array(std::move(edits));
  }

  std::map<std::string, std::string> documents_;
  bool exit_requested_ = false;
};

}  // namespace

int main() {
  // The protocol stream is stdout; diagnostics about our own operation must
  // not land in it.
  std::ios::sync_with_stdio(false);
  LspServer server;
  server.Run();
  return 0;
}
