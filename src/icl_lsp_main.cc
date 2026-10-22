// Copyright 2024 The ICL Format Authors.
//
// SPDX-License-Identifier: MIT
// See LICENSE in the project root for license information.
//
// icl-lsp — a Language Server for IEEE 1687-2014 (IJTAG) ICL.
//
// Formatting and navigation consume the same cached parsed document.

#include <cstdlib>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <set>
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
    if (header.size() == 8192) return false;
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
      if (length >= 0 || value.empty() || value.find_first_not_of("0123456789") != std::string::npos) return false;
      unsigned long long parsed = 0;
      for (const char c : value) {
        parsed = parsed * 10 + static_cast<unsigned>(c - '0');
        if (parsed > 64 * 1024 * 1024) return false;
      }
      length = static_cast<long>(parsed);
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

// Protocol positions are UTF-16 code units; parser locations remain byte based.
size_t Utf16Length(const std::string& text, size_t begin, size_t end) {
  size_t units = 0;
  for (size_t i = begin; i < end; ++i) {
    const auto c = static_cast<unsigned char>(text[i]);
    if ((c & 0xC0) != 0x80) units += c >= 0xF0 ? 2 : 1;
  }
  return units;
}
Json SourcePosition(const icl::ParsedDocument& doc, size_t byte) {
  byte = std::min(byte, doc.source.size());
  const auto line = static_cast<size_t>(std::upper_bound(doc.line_starts.begin(), doc.line_starts.end(), byte) - doc.line_starts.begin() - 1);
  return Position(line, Utf16Length(doc.source, doc.line_starts[line], byte));
}
Json SourceRange(const icl::ParsedDocument& doc, icl::ByteRange range) {
  return Json::Object({{"start", SourcePosition(doc, range.begin)}, {"end", SourcePosition(doc, range.end)}});
}
size_t SourceOffset(const icl::ParsedDocument& doc, const Json* position) {
  const auto line = GetInt(position, "line", 0), character = GetInt(position, "character", 0);
  if (line < 0 || character < 0) return 0;
  if (static_cast<size_t>(line) >= doc.line_starts.size()) return doc.source.size();
  size_t byte = doc.line_starts[line], units = 0;
  while (byte < doc.source.size() && doc.source[byte] != '\n' && units < static_cast<size_t>(character)) {
    const auto c = static_cast<unsigned char>(doc.source[byte]);
    const size_t width = c < 0x80 ? 1 : c < 0xE0 ? 2 : c < 0xF0 ? 3 : 4;
    const size_t count = width == 4 ? 2 : 1;
    if (units + count > static_cast<size_t>(character)) break;
    units += count; byte = std::min(doc.source.size(), byte + width);
  }
  return byte;
}
int SymbolKind(const std::string& kind) {
  if (kind == "Module") return 2;
  if (kind == "Instance") return 19;
  if (kind == "Enum") return 10;
  if (kind == "EnumMember") return 22;
  if (kind == "Parameter" || kind == "LocalParameter") return 14;
  return 13;
}
std::string FilePath(const std::string& uri) {
  if (uri.rfind("file:///", 0) != 0) return "";
  std::string path;
  for (size_t i = 7; i < uri.size(); ++i) {
    if (uri[i] == '%' && i + 2 < uri.size()) {
      const auto digit = [](char c) { return c >= '0' && c <= '9' ? c - '0' : c >= 'a' && c <= 'f' ? c - 'a' + 10 : c >= 'A' && c <= 'F' ? c - 'A' + 10 : -1; };
      const int a = digit(uri[i + 1]), b = digit(uri[i + 2]);
      if (a < 0 || b < 0 || !(a * 16 + b)) return "";
      path += static_cast<char>(a * 16 + b); i += 2;
    } else path += uri[i];
  }
  return path;
}
std::string FileUri(const std::string& path) {
  std::string uri = "file://";
  constexpr char hex[] = "0123456789ABCDEF";
  for (unsigned char c : path) {
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '/' || c == '-' || c == '_' || c == '.' || c == '~') uri += c;
    else { uri += '%'; uri += hex[c >> 4]; uri += hex[c & 15]; }
  }
  return uri;
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
  return static_cast<long long>(Utf16Length(text, start, end));
}

// The (line, character) just past the last byte of |text| — the end of the
// last line, which LineLength already knows how to measure.
//
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
      if (can_reply) {
        const auto& params = Params(message);
        const Json* capability = params.Find("capabilities");
        capability = capability ? capability->Find("workspace") : nullptr;
        capability = capability ? capability->Find("didChangeWatchedFiles") : nullptr;
        capability = capability ? capability->Find("dynamicRegistration") : nullptr;
        register_watcher_ = capability && capability->bool_value();
        ScanWorkspace(params); Reply(*id, InitializeResult());
      }
    } else if (method == "initialized") {
      if (register_watcher_) WriteMessage(Json::Object({
          {"jsonrpc", Json::String("2.0")}, {"id", Json::String("icl/watch")},
          {"method", Json::String("client/registerCapability")},
          {"params", Json::Object({{"registrations", Json::Array({Json::Object({
              {"id", Json::String("icl-files")}, {"method", Json::String("workspace/didChangeWatchedFiles")},
              {"registerOptions", Json::Object({{"watchers", Json::Array({Json::Object({{"globPattern", Json::String("**/*.icl")}})})}})}
          })})}})}
      }));
    } else if (method == "shutdown") {
      if (can_reply) Reply(*id, Json::Null());
    } else if (method == "exit") {
      exit_requested_ = true;
    } else if (method == "textDocument/didOpen") {
      const Json& params = Params(message);
      const Json* document = params.Find("textDocument");
      Open(UriOf(params), GetString(document, "text"), GetInt(document, "version", -1));
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
          Open(uri, text->string_value(), GetInt(params.Find("textDocument"), "version", -1));
        }
      }
    } else if (method == "textDocument/didClose") {
      const auto uri = UriOf(Params(message));
      open_versions_.erase(uri); workspace_.documents.erase(uri);
      ReadDisk(uri); workspace_.Reindex();
    } else if (method == "workspace/didChangeWatchedFiles") {
      WatchedFiles(Params(message));
    } else if (method == "textDocument/formatting") {
      if (can_reply) Reply(*id, Formatting(Params(message)));
    } else if (method == "textDocument/completion" || method == "textDocument/definition" ||
               method == "textDocument/declaration" || method == "textDocument/implementation" ||
               method == "textDocument/hover" || method == "textDocument/references" ||
               method == "textDocument/documentSymbol" || method == "workspace/symbol") {
      if (can_reply) Reply(*id, SemanticRequest(method, Params(message)));
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
             {"completionProvider", Json::Object({{"triggerCharacters", Json::Array({Json::String("."), Json::String("$")})}})},
             {"definitionProvider", Json::Bool(true)},
             {"declarationProvider", Json::Bool(true)},
             {"implementationProvider", Json::Bool(true)},
             {"hoverProvider", Json::Bool(true)},
             {"referencesProvider", Json::Bool(true)},
             {"documentSymbolProvider", Json::Bool(true)},
             {"workspaceSymbolProvider", Json::Bool(true)},
         })},
        {"serverInfo", Json::Object({{"name", Json::String("icl-lsp")},
                                     {"version",
                                      Json::String(ICL_FORMAT_VERSION)}})},
    });
  }

  void Open(const std::string& uri, const std::string& text, long long version) {
    const auto old = open_versions_.find(uri);
    if (old != open_versions_.end() && version >= 0 && old->second >= version) return;
    open_versions_[uri] = version;
    auto document = std::make_shared<icl::ParsedDocument>(text);
    workspace_.documents[uri] = document;
    workspace_.Reindex();
    PublishDiagnostics(uri, *document);
  }

  void PublishDiagnostics(const std::string& uri, const icl::ParsedDocument& document) {
    const auto& text = document.source;
    const auto outcome = icl::FormatParsedDocument(document, icl::FormatRequest());
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
    const auto it = workspace_.documents.find(uri);
    if (it == workspace_.documents.end() || !open_versions_.count(uri)) return Json::Array({});
    const std::string& text = it->second->source;

    // Honour the editor's tabSize: it is the same knob the CLI exposes as -n,
    // so the two front ends agree on what "indent" means. Clamping is shared
    // (see format_job.h) rather than done here, so the CLI's -n and this path
    // cannot end up with different limits.
    icl::FormatRequest request;
    request.indent_width = static_cast<int>(
        GetInt(params.Find("options"), "tabSize", 2));

    const icl::FormatOutcome outcome = icl::FormatParsedDocument(*it->second, request);
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

  void ReadDisk(const std::string& uri) {
    if (open_versions_.count(uri)) return;
    const std::string path = FilePath(uri);
    std::ifstream file(path, std::ios::binary);
    if (!file) { workspace_.documents.erase(uri); return; }
    std::string text((std::istreambuf_iterator<char>(file)), {});
    if (file.bad()) return;
    workspace_.documents[uri] = std::make_shared<icl::ParsedDocument>(std::move(text));
  }

  void ScanWorkspace(const Json& params) {
    std::vector<std::string> roots;
    const Json* folders = params.Find("workspaceFolders");
    if (folders) for (const auto& folder : folders->elements()) roots.push_back(FilePath(GetString(&folder, "uri")));
    if (roots.empty()) {
      std::string root = FilePath(GetString(&params, "rootUri"));
      if (root.empty()) root = GetString(&params, "rootPath");
      if (!root.empty()) roots.push_back(root);
    }
    for (const auto& root : roots) {
      if (root.empty()) continue;
      std::error_code error;
      std::filesystem::recursive_directory_iterator it(root, std::filesystem::directory_options::skip_permission_denied, error), end;
      while (it != end) {
        const auto path = it->path();
        const auto name = path.filename().string();
        if (it->is_directory(error) && (name.empty() || name[0] == '.' || name == "build")) it.disable_recursion_pending();
        else if (it->is_regular_file(error) && path.extension() == ".icl") ReadDisk(FileUri(path.string()));
        it.increment(error);
        if (error) break;
      }
    }
    workspace_.Reindex();
  }

  void WatchedFiles(const Json& params) {
    const Json* changes = params.Find("changes");
    if (!changes) return;
    for (const auto& change : changes->elements()) {
      const auto uri = GetString(&change, "uri");
      if (!open_versions_.count(uri)) {
        if (GetInt(&change, "type", 0) == 3) workspace_.documents.erase(uri);
        else ReadDisk(uri);
      }
    }
    workspace_.Reindex();
  }

  Json Location(icl::SymbolRef symbol, bool body = false) const {
    const auto& d = symbol.declaration();
    return Json::Object({{"uri", Json::String(workspace_.Uri(symbol.document))},
                         {"range", SourceRange(*symbol.document, body ? d.range : d.name_range)}});
  }
  Json Symbol(icl::SymbolRef symbol) const {
    const auto& d = symbol.declaration();
    std::string container;
    if (d.module >= 0) container = symbol.document->declarations[d.module].name;
    return Json::Object({{"name", Json::String(d.name)}, {"kind", Json::Int(SymbolKind(d.kind))},
                         {"location", Location(symbol)}, {"containerName", Json::String(container)}});
  }

  Json Symbols(const Json& params, bool all) const {
    const auto uri = UriOf(params), query = GetString(&params, "query");
    Json::Elements result;
    for (const auto& item : workspace_.documents) {
      if (!all && item.first != uri) continue;
      const auto* doc = item.second.get();
      for (size_t i = 0; i < doc->declarations.size(); ++i)
        if (!all || doc->declarations[i].name.find(query) != std::string::npos) result.push_back(Symbol({doc, i}));
    }
    return Json::Array(std::move(result));
  }

  Json SemanticRequest(const std::string& method, const Json& params) const {
    if (method == "workspace/symbol") return Symbols(params, true);
    if (method == "textDocument/documentSymbol") return Symbols(params, false);
    const auto found = workspace_.documents.find(UriOf(params));
    if (found == workspace_.documents.end()) return method == "textDocument/hover" ? Json::Null() : Json::Array({});
    const auto& doc = *found->second;
    const size_t byte = SourceOffset(doc, params.Find("position"));
    if (method == "textDocument/completion") {
      Json::Elements items;
      bool member_context = false;
      std::set<std::string> names;
      for (const auto symbol : workspace_.Complete(doc, byte, &member_context)) {
        const auto& d = symbol.declaration();
        if (names.insert(d.name).second)
          items.push_back(Json::Object({{"label", Json::String(d.name)}, {"kind", Json::Int(6)}, {"detail", Json::String(d.detail)}}));
      }
      if (!member_context) for (const auto& keyword : icl::IclKeywords())
        if (names.insert(keyword).second) items.push_back(Json::Object({{"label", Json::String(keyword)}, {"kind", Json::Int(14)}}));
      return Json::Array(std::move(items));
    }
    const auto* use = doc.UseAt(byte);
    auto symbol = use ? workspace_.Resolve(doc, *use) : icl::SymbolRef{};
    if (method == "textDocument/hover") {
      if (!symbol) return Json::Null();
      return Json::Object({{"contents", Json::Object({{"kind", Json::String("plaintext")}, {"value", Json::String(symbol.declaration().detail)}})},
                           {"range", SourceRange(doc, use->range)}});
    }
    if (method == "textDocument/references") {
      Json::Elements result;
      const Json* context = params.Find("context");
      const Json* include = context ? context->Find("includeDeclaration") : nullptr;
      const bool declarations = include && include->bool_value();
      if (symbol) for (const auto& item : workspace_.documents)
        for (const auto& candidate : item.second->uses)
          if ((declarations || candidate.declaration < 0) && workspace_.Resolve(*item.second, candidate) == symbol)
            result.push_back(Json::Object({{"uri", Json::String(item.first)}, {"range", SourceRange(*item.second, candidate.range)}}));
      return Json::Array(std::move(result));
    }
    const bool implementation = method == "textDocument/implementation";
    if (implementation) symbol = workspace_.Implementation(symbol);
    return symbol ? Json::Array({Location(symbol, implementation)}) : Json::Array({});
  }

  icl::SemanticWorkspace workspace_;
  std::map<std::string, long long> open_versions_;
  bool register_watcher_ = false;
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
