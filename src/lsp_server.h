// Shared stdio LSP infrastructure; language adapters own syntax and semantics.
#ifndef ICL_TOOLSETS_SRC_LSP_SERVER_H_
#define ICL_TOOLSETS_SRC_LSP_SERVER_H_
// Copyright 2024 The ICL Format Authors.
//
// SPDX-License-Identifier: MIT
// See LICENSE in the project root for license information.
//
// Shared LSP transport and services for distinct ICL and PDL front ends.
//
// Formatting and navigation consume the same cached parsed document.

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>

#include "json.h"
#include "text_input.h"

#ifndef ICL_FORMAT_VERSION
#define ICL_FORMAT_VERSION "0.0.0"
#endif

// Everything except the driver lives in namespace icl; the server itself is
// file-local, so pull the names in rather than qualifying each use.
namespace toolsets {

using icl::Json;

// ---- LSP base protocol: Content-Length framing over stdio ------------------

// Reads one message: a header block ending in a blank line, then exactly
// Content-Length bytes. Returns false at end of stream, which is how a client
// closing the pipe shuts the server down.
inline bool ReadMessage(std::string* body) {
  std::string header;
  for (;;) {
    const int c = std::cin.get();
    if (c == EOF) return false;
    if (header.size() == 8192) return false;
    header.push_back(static_cast<char>(c));
    const size_t size = header.size();
    if (size >= 4 && header.compare(size - 4, 4, "\r\n\r\n") == 0) break;
  }
  std::int64_t length = -1;
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
      if (length >= 0 || value.empty() ||
          value.find_first_not_of("0123456789") != std::string::npos) {
        return false;
      }
      std::uint64_t parsed = 0;
      for (const char c : value) {
        parsed = parsed * 10 + static_cast<unsigned>(c - '0');
        if (parsed > 64 * 1024 * 1024) return false;
      }
      length = static_cast<std::int64_t>(parsed);
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

inline void WriteMessage(const Json& message) {
  const std::string body = message.Serialize();
  std::cout << "Content-Length: " << body.size() << "\r\n\r\n" << body;
  std::cout.flush();
}

// ---- small helpers ---------------------------------------------------------

// Object field access that tolerates a missing parent. Request shapes vary
// between clients, and a null dereference would take the whole server down.
inline std::string GetString(const Json* object, const char* key,
                             const std::string& fallback = "") {
  if (object == nullptr) return fallback;
  const Json* value = object->Find(key);
  return value == nullptr ? fallback : value->string_value(fallback);
}

inline std::int64_t GetInt(const Json* object, const char* key,
                           std::int64_t fallback) {
  if (object == nullptr) return fallback;
  const Json* value = object->Find(key);
  return value == nullptr ? fallback : value->int_value(fallback);
}

// The document uri every textDocument/* notification carries.
inline std::string UriOf(const Json& params) {
  return GetString(params.Find("textDocument"), "uri");
}

inline Json Position(std::int64_t line, std::int64_t character) {
  return Json::Object(
      {{"line", Json::Int(line)}, {"character", Json::Int(character)}});
}

// Protocol positions are UTF-16 code units; parser locations remain byte based.
inline size_t Utf16Length(const std::string& text, size_t begin, size_t end) {
  size_t units = 0;
  for (size_t i = begin; i < end; ++i) {
    const auto c = static_cast<unsigned char>(text[i]);
    if ((c & 0xC0) != 0x80) units += c >= 0xF0 ? 2 : 1;
  }
  return units;
}
template <class Document>
Json SourcePosition(const Document& doc, size_t byte) {
  byte = std::min(byte, doc.source().size());
  const auto line =
      static_cast<size_t>(std::upper_bound(doc.line_starts().begin(),
                                           doc.line_starts().end(), byte) -
                          doc.line_starts().begin() - 1);
  return Position(line,
                  Utf16Length(doc.source(), doc.line_starts()[line], byte));
}
template <class Document, class Range>
Json SourceRange(const Document& doc, Range range) {
  return Json::Object({{"start", SourcePosition(doc, range.begin)},
                       {"end", SourcePosition(doc, range.end)}});
}
template <class Document>
size_t SourceOffset(const Document& doc, const Json* position) {
  const auto line = GetInt(position, "line", 0),
             character = GetInt(position, "character", 0);
  if (line < 0 || character < 0) return 0;
  if (static_cast<size_t>(line) >= doc.line_starts().size()) {
    return doc.source().size();
  }
  size_t byte = doc.line_starts()[line], units = 0;
  while (byte < doc.source().size() && doc.source()[byte] != '\n' &&
         units < static_cast<size_t>(character)) {
    const auto c = static_cast<unsigned char>(doc.source()[byte]);
    const size_t width = c < 0x80 ? 1 : c < 0xE0 ? 2 : c < 0xF0 ? 3 : 4;
    const size_t count = width == 4 ? 2 : 1;
    if (units + count > static_cast<size_t>(character)) break;
    units += count;
    byte = std::min(doc.source().size(), byte + width);
  }
  return byte;
}
inline int SymbolKind(const std::string& kind) {
  if (kind == "Module") return 2;
  if (kind == "iProc" || kind == "proc") return 12;
  if (kind == "Instance") return 19;
  if (kind == "Enum") return 10;
  if (kind == "EnumMember") return 22;
  if (kind == "Parameter" || kind == "LocalParameter") return 14;
  return 13;
}
inline std::string FilePath(const std::string& uri) {
  if (uri.rfind("file:///", 0) != 0) return "";
  std::string path;
  for (size_t i = 7; i < uri.size(); ++i) {
    if (uri[i] == '%' && i + 2 < uri.size()) {
      const auto digit = [](char c) {
        return c >= '0' && c <= '9'   ? c - '0'
               : c >= 'a' && c <= 'f' ? c - 'a' + 10
               : c >= 'A' && c <= 'F' ? c - 'A' + 10
                                      : -1;
      };
      const int a = digit(uri[i + 1]), b = digit(uri[i + 2]);
      if (a < 0 || b < 0 || !(a * 16 + b)) return "";
      path += static_cast<char>(a * 16 + b);
      i += 2;
    } else {
      path += uri[i];
    }
  }
  return path;
}
inline std::string FileUri(const std::string& path) {
  std::string uri = "file://";
  constexpr char kHex[] = "0123456789ABCDEF";
  for (unsigned char c : path) {
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
        (c >= '0' && c <= '9') || c == '/' || c == '-' || c == '_' ||
        c == '.' || c == '~') {
      uri += c;
    } else {
      uri += '%';
      uri += kHex[c >> 4];
      uri += kHex[c & 15];
    }
  }
  return uri;
}

// UTF-16 length excluding CRLF, for diagnostics with only a line number.
inline std::int64_t LineLength(const std::string& text,
                               std::int64_t line_index) {
  size_t start = 0;
  for (std::int64_t i = 0; i < line_index; ++i) {
    const size_t newline = text.find('\n', start);
    if (newline == std::string::npos) return 0;
    start = newline + 1;
  }
  const size_t newline = text.find('\n', start);
  size_t end = (newline == std::string::npos) ? text.size() : newline;
  if (end > start && text[end - 1] == '\r') --end;
  return static_cast<std::int64_t>(Utf16Length(text, start, end));
}

// The (line, character) just past the last byte of |text| — the end of the
// last line, which LineLength already knows how to measure.
//
inline void EndOfDocument(const std::string& text, std::int64_t* line,
                          std::int64_t* character) {
  *line = 0;
  for (const char c : text) {
    if (c == '\n') ++*line;
  }
  *character = LineLength(text, *line);
}

// ---- server ----------------------------------------------------------------

template <class Language>
class LspServer {
  using Document = typename Language::Document;
  using SymbolRef = typename Language::SymbolRef;
  using Workspace = typename Language::Workspace;

 public:
  void Run() {
    std::string body;
    while (ReadMessage(&body)) {
      std::string error;
      Json message;
      if (!Json::Parse(body, &message, &error)) {
        // A malformed frame is not recoverable: there is no id to reply to.
        // Keep serving; the client will usually restart us.
        std::cerr << Language::Name() << ": malformed request: " << error
                  << "\n";
        continue;
      }
      Handle(message);
      if (exit_requested_) break;
    }
  }

 private:
  void Handle(const Json& message) {
    // Client replies to our watcher registration are responses, not requests.
    if (!message.Find("method") &&
        (message.Find("result") || message.Find("error"))) {
      return;
    }
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
        capability =
            capability ? capability->Find("didChangeWatchedFiles") : nullptr;
        capability =
            capability ? capability->Find("dynamicRegistration") : nullptr;
        register_watcher_ = capability && capability->bool_value();
        ScanWorkspace(params);
        Reply(*id, InitializeResult());
      }
    } else if (method == "initialized") {
      if (register_watcher_) {
        WriteMessage(Json::Object(
            {{"jsonrpc", Json::String("2.0")},
             {"id", Json::String(std::string(Language::Id()) + "/watch")},
             {"method", Json::String("client/registerCapability")},
             {"params",
              Json::Object(
                  {{"registrations",
                    Json::Array({Json::Object(
                        {{"id",
                          Json::String(std::string(Language::Id()) + "-files")},
                         {"method",
                          Json::String("workspace/didChangeWatchedFiles")},
                         {"registerOptions",
                          Json::Object(
                              {{"watchers",
                                Json::Array({Json::Object(
                                    {{"globPattern",
                                      Json::String(
                                          Language::Glob())}})})}})}})})}})}}));
      }
    } else if (method == "shutdown") {
      if (can_reply) Reply(*id, Json::Null());
    } else if (method == "exit") {
      exit_requested_ = true;
    } else if (method == "textDocument/didOpen") {
      const Json& params = Params(message);
      const Json* document = params.Find("textDocument");
      Open(UriOf(params), GetString(document, "text"),
           GetInt(document, "version", -1));
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
        // A ranged edit cannot replace a full-sync document safely.
        if (!last.Find("range") && text != nullptr &&
            text->type() == Json::Type::kString) {
          Open(uri, text->string_value(),
               GetInt(params.Find("textDocument"), "version", -1));
        }
      }
    } else if (method == "textDocument/didClose") {
      const auto uri = UriOf(Params(message));
      Close(uri);
      PublishDiagnostics(uri, nullptr);
    } else if (method == "workspace/didChangeWatchedFiles") {
      WatchedFiles(Params(message));
    } else if (method == "textDocument/formatting") {
      if (can_reply) Reply(*id, Formatting(Params(message)));
    } else if (method == "textDocument/completion" ||
               method == "textDocument/definition" ||
               method == "textDocument/declaration" ||
               method == "textDocument/implementation" ||
               method == "textDocument/hover" ||
               method == "textDocument/references" ||
               method == "textDocument/documentSymbol" ||
               method == "workspace/symbol") {
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
    static const Json kEmpty = Json::Object({});
    const Json* params = message.Find("params");
    return params == nullptr ? kEmpty : *params;
  }

  void Reply(const Json& id, Json result) {
    WriteMessage(Json::Object({{"jsonrpc", Json::String("2.0")},
                               {"id", id},
                               {"result", std::move(result)}}));
  }

  void ErrorReply(const Json& id, std::int64_t code,
                  const std::string& message) {
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
             {"completionProvider",
              Json::Object(
                  {{"triggerCharacters",
                    Json::Array({Json::String("."), Json::String("$")})}})},
             {"definitionProvider", Json::Bool(true)},
             {"declarationProvider", Json::Bool(true)},
             {"implementationProvider", Json::Bool(true)},
             {"hoverProvider", Json::Bool(true)},
             {"referencesProvider", Json::Bool(true)},
             {"documentSymbolProvider", Json::Bool(true)},
             {"workspaceSymbolProvider", Json::Bool(true)},
         })},
        {"serverInfo",
         Json::Object({{"name", Json::String(Language::Name())},
                       {"version", Json::String(ICL_FORMAT_VERSION)}})},
    });
  }

  void Open(const std::string& uri, const std::string& text,
            std::int64_t version) {
    const auto old = open_documents_.find(uri);
    if (old != open_documents_.end() && version >= 0 &&
        old->second.version >= version) {
      return;
    }
    const auto key = DocumentKey(uri);
    auto document = Language::Parse(text, uri);
    open_documents_[uri] = {key, version, document};
    active_uris_[key] = uri;
    workspace_.documents()[key] = document;
    workspace_.Reindex();
    PublishDiagnostics(uri, document.get());
  }

  void PublishDiagnostics(const std::string& uri, const Document* document) {
    Json::Elements diagnostics;
    if (document && !document->ok()) {
      const Json range = Language::DiagnosticRange(*document);
      diagnostics.push_back(Json::Object({
          {"range", range},
          {"severity", Json::Int(1)},
          {"source", Json::String(Language::Name())},
          {"message", Json::String(Language::ErrorMessage(*document))},
      }));
    }
    Json::Members params{{"uri", Json::String(uri)},
                         {"diagnostics", Json::Array(std::move(diagnostics))}};
    const auto version = open_documents_.find(uri);
    if (version != open_documents_.end() && version->second.version >= 0) {
      params.emplace_back("version", Json::Int(version->second.version));
    }
    WriteMessage(Json::Object(
        {{"jsonrpc", Json::String("2.0")},
         {"method", Json::String("textDocument/publishDiagnostics")},
         {"params", Json::Object(std::move(params))}}));
  }

  Json Formatting(const Json& params) {
    const Json* document = params.Find("textDocument");
    const std::string uri = GetString(document, "uri");
    const auto it = open_documents_.find(uri);
    if (it == open_documents_.end()) {
      return Json::Array({});
    }
    const std::string& text = it->second.document->source();

    // Honour the editor's tabSize: it is the same knob the CLI exposes as -n,
    // so the two front ends agree on what "indent" means. Clamping is shared
    // (see format_job.h) rather than done here, so the CLI's -n and this path
    // cannot end up with different limits.
    typename Language::Request request;
    request.indent_width =
        static_cast<int>(GetInt(params.Find("options"), "tabSize", 2));

    const auto outcome = Language::Format(*it->second.document, request);
    // A file with a syntax error is left untouched: returning no edits is the
    // only safe answer, since there is no formatted form to hand back. So is
    // a file that is already formatted — sending an edit that replaces the
    // text with itself makes the editor mark the buffer dirty for nothing.
    if (!outcome.ok || !outcome.ChangedFrom(text)) return Json::Array({});
    const std::string& output = outcome.text;

    std::int64_t end_line = 0;
    std::int64_t end_character = 0;
    EndOfDocument(text, &end_line, &end_character);
    Json::Elements edits;
    edits.push_back(Json::Object({
        {"range", Json::Object({{"start", Position(0, 0)},
                                {"end", Position(end_line, end_character)}})},
        {"newText", Json::String(output)},
    }));
    return Json::Array(std::move(edits));
  }

  static std::string FileKey(const std::string& uri) {
    const auto path = FilePath(uri);
    if (path.empty()) return uri;
    std::error_code error;
    const auto canonical = std::filesystem::weakly_canonical(path, error);
    return error ? uri : FileUri(canonical.string());
  }

  std::string DocumentKey(const std::string& uri) const {
    const auto open = open_documents_.find(uri);
    return open == open_documents_.end() ? FileKey(uri) : open->second.key;
  }

  std::string ClientUri(const std::string& key) const {
    const auto found = active_uris_.find(key);
    return found == active_uris_.end() ? key : found->second;
  }

  bool AutoIndex(const std::string& uri) const {
    const std::filesystem::path path = FilePath(uri);
    if (path.empty() || !Language::Accepts(path)) return false;
    const auto included = [&](const std::filesystem::path& root) {
      const auto relative = path.lexically_normal().lexically_relative(root);
      if (relative.empty() || *relative.begin() == "..") return false;
      auto directory = root;
      for (const auto& component : relative.parent_path()) {
        const auto name = component.string();
        if (name == "build" || (!name.empty() && name[0] == '.')) return false;
        directory /= component;
        std::error_code error;
        if (std::filesystem::is_symlink(directory, error)) return false;
      }
      return true;
    };
    return roots_.empty() ? included(path.root_path())
                          : std::any_of(roots_.begin(), roots_.end(), included);
  }

  void DropDisk(const std::string& uri) {
    const auto found = disk_keys_.find(uri);
    if (found == disk_keys_.end()) return;
    const auto key = found->second;
    disk_keys_.erase(found);
    std::error_code error;
    if (!active_uris_.count(key) &&
        (!std::filesystem::is_regular_file(FilePath(key), error) ||
         std::none_of(disk_keys_.begin(), disk_keys_.end(),
                      [&](const auto& item) { return item.second == key; }))) {
      workspace_.documents().erase(key);
    }
  }

  void Close(const std::string& uri) {
    const auto found = open_documents_.find(uri);
    if (found == open_documents_.end()) return;
    const auto key = found->second.key;
    open_documents_.erase(found);
    if (ClientUri(key) == uri) {
      active_uris_.erase(key);
      workspace_.documents().erase(key);
      for (const auto& other : open_documents_) {
        if (other.second.key == key) {
          active_uris_[key] = other.first;
          workspace_.documents()[key] = other.second.document;
          break;
        }
      }
      if (!active_uris_.count(key)) {
        // Restore the original identity even if an open symlink was retargeted.
        for (const auto& item : disk_keys_) {
          if (item.second == key && FileKey(item.first) == key) {
            ReadDisk(item.first);
            break;
          }
        }
      }
    }
    ReadDisk(uri);
    workspace_.Reindex();
  }

  void ReadDisk(const std::string& uri, bool refresh = true) {
    // The directory iterator already enforces exclusions during initial scans.
    if (refresh && !AutoIndex(uri)) return;
    const std::string path = FilePath(uri);
    std::error_code error;
    if (!std::filesystem::is_regular_file(path, error)) {
      DropDisk(uri);
      return;
    }
    const auto key = FileKey(uri);
    const auto previous = disk_keys_.find(uri);
    if (previous != disk_keys_.end() && previous->second != key) DropDisk(uri);
    disk_keys_[uri] = key;
    if (active_uris_.count(key) ||
        (!refresh && workspace_.documents().count(key))) {
      return;
    }
    std::ifstream file(path, std::ios::binary);
    std::string text;
    if (!file || !ReadTextStream(file, &text)) {
      workspace_.documents().erase(key);
      return;
    }
    workspace_.documents()[key] = Language::Parse(std::move(text), uri);
  }

  void ScanWorkspace(const Json& params) {
    std::vector<std::string> roots;
    const Json* folders = params.Find("workspaceFolders");
    if (folders) {
      for (const auto& folder : folders->elements()) {
        roots.push_back(FilePath(GetString(&folder, "uri")));
      }
    }
    if (roots.empty()) {
      std::string root = FilePath(GetString(&params, "rootUri"));
      if (root.empty()) root = GetString(&params, "rootPath");
      if (!root.empty()) roots.push_back(root);
    }
    for (const auto& root : roots) {
      if (root.empty()) continue;
      roots_.push_back(std::filesystem::path(root).lexically_normal());
    }
    for (const auto& root : roots_) {
      std::error_code error;
      std::filesystem::recursive_directory_iterator it(
          root, std::filesystem::directory_options::skip_permission_denied,
          error),
          end;
      while (it != end) {
        const auto path = it->path();
        const auto name = path.filename().string();
        if (it->is_directory(error) &&
            (name.empty() || name[0] == '.' || name == "build")) {
          it.disable_recursion_pending();
        } else if (it->is_regular_file(error) && Language::Accepts(path)) {
          ReadDisk(FileUri(path.string()), false);
        }
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
      if (GetInt(&change, "type", 0) == 3) {
        DropDisk(uri);
      } else {
        ReadDisk(uri);
      }
    }
    workspace_.Reindex();
  }

  Json Location(SymbolRef symbol, bool body = false) const {
    const auto& d = symbol.declaration();
    return Json::Object(
        {{"uri", Json::String(ClientUri(workspace_.Uri(symbol.document)))},
         {"range",
          SourceRange(*symbol.document, body ? d.range : d.name_range)}});
  }
  Json Symbol(SymbolRef symbol) const {
    const auto& d = symbol.declaration();
    std::string container;
    if (d.module >= 0) {
      container = symbol.document->declarations()[d.module].name;
    }
    return Json::Object({{"name", Json::String(d.name)},
                         {"kind", Json::Int(SymbolKind(d.kind))},
                         {"location", Location(symbol)},
                         {"containerName", Json::String(container)}});
  }

  Json Symbols(const Json& params, bool all) const {
    const auto uri = DocumentKey(UriOf(params)),
               query = GetString(&params, "query");
    Json::Elements result;
    for (const auto& item : workspace_.documents()) {
      if (!all && item.first != uri) continue;
      const auto* doc = item.second.get();
      for (size_t i = 0; i < doc->declarations().size(); ++i) {
        if (!all ||
            doc->declarations()[i].name.find(query) != std::string::npos) {
          result.push_back(Symbol({doc, i}));
        }
      }
    }
    return Json::Array(std::move(result));
  }

  Json SemanticRequest(const std::string& method, const Json& params) {
    const auto open = open_documents_.find(UriOf(params));
    if (open != open_documents_.end() &&
        ClientUri(open->second.key) != open->first) {
      // Two editor buffers may share a file but contain different unsaved text.
      active_uris_[open->second.key] = open->first;
      workspace_.documents()[open->second.key] = open->second.document;
      workspace_.Reindex();
    }
    if (method == "workspace/symbol") return Symbols(params, true);
    if (method == "textDocument/documentSymbol") return Symbols(params, false);
    const auto found = workspace_.documents().find(DocumentKey(UriOf(params)));
    if (found == workspace_.documents().end()) {
      return method == "textDocument/hover" ? Json::Null() : Json::Array({});
    }
    const auto& doc = *found->second;
    const size_t byte = SourceOffset(doc, params.Find("position"));
    if (method == "textDocument/completion") {
      Json::Elements items;
      bool member_context = false;
      std::set<std::string> names;
      for (const auto symbol :
           workspace_.Complete(doc, byte, &member_context)) {
        const auto& d = symbol.declaration();
        if (names.insert(d.name).second) {
          items.push_back(Json::Object({{"label", Json::String(d.name)},
                                        {"kind", Json::Int(6)},
                                        {"detail", Json::String(d.detail)}}));
        }
      }
      if (!member_context) {
        for (const auto& keyword : Language::Keywords()) {
          if (names.insert(keyword).second) {
            items.push_back(Json::Object(
                {{"label", Json::String(keyword)}, {"kind", Json::Int(14)}}));
          }
        }
      }
      return Json::Array(std::move(items));
    }
    const auto* use = doc.UseAt(byte);
    auto symbol = use ? workspace_.Resolve(doc, *use) : SymbolRef{};
    if (method == "textDocument/hover") {
      if (!symbol) return Json::Null();
      return Json::Object(
          {{"contents",
            Json::Object(
                {{"kind", Json::String("plaintext")},
                 {"value", Json::String(symbol.declaration().detail)}})},
           {"range", SourceRange(doc, use->range)}});
    }
    if (method == "textDocument/references") {
      Json::Elements result;
      const Json* context = params.Find("context");
      const Json* include =
          context ? context->Find("includeDeclaration") : nullptr;
      const bool declarations = include && include->bool_value();
      if (symbol) {
        for (const auto& item : workspace_.documents()) {
          for (const auto& candidate : item.second->uses()) {
            if ((declarations || candidate.declaration < 0) &&
                workspace_.Resolve(*item.second, candidate) == symbol) {
              result.push_back(Json::Object(
                  {{"uri", Json::String(ClientUri(item.first))},
                   {"range", SourceRange(*item.second, candidate.range)}}));
            }
          }
        }
      }
      return Json::Array(std::move(result));
    }
    const bool implementation = method == "textDocument/implementation";
    if (implementation) symbol = workspace_.Implementation(symbol);
    return symbol ? Json::Array({Location(symbol, implementation)})
                  : Json::Array({});
  }

  Workspace workspace_;
  struct OpenDocument {
    std::string key;
    std::int64_t version;
    std::shared_ptr<Document> document;
  };
  std::map<std::string, OpenDocument> open_documents_;
  std::map<std::string, std::string> active_uris_, disk_keys_;
  std::vector<std::filesystem::path> roots_;
  bool register_watcher_ = false;
  bool exit_requested_ = false;
};

template <class Language>
int RunLsp(int argc, char** argv) {
  if (argc == 2) {
    const std::string arg = argv[1];
    if (arg == "-h" || arg == "--help") {
      std::cout << "usage: " << Language::Name()
                << " [--help | --version]\n"
                   "\n"
                << Language::Description()
                << "\n"
                   "With no arguments, serves LSP on stdin/stdout.\n"
                   "\n"
                   "options:\n"
                   "  -h, --help    print this message and exit\n"
                   "      --version print version and exit\n";
      return 0;
    }
    if (arg == "--version") {
      std::cout << Language::Name() << " " << ICL_FORMAT_VERSION << "\n";
      return 0;
    }
  }
  if (argc > 1) {
    std::cerr << Language::Name()
              << ": invalid arguments; use --help for usage\n";
    return 2;
  }
  // The protocol stream is stdout; diagnostics about our own
  // operation must not land in it.
  std::ios::sync_with_stdio(false);
  LspServer<Language> server;
  server.Run();
  return 0;
}

}  // namespace toolsets
#endif  // ICL_TOOLSETS_SRC_LSP_SERVER_H_
