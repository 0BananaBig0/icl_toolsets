// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include "icl/format_job.h"
#include "utility/lsp_server.h"

namespace {
struct Language {
  using Document = icl::ParsedDocument;
  using Workspace = icl::SemanticWorkspace;
  using SymbolRef = icl::SymbolRef;
  using Request = icl::FormatRequest;
  static const char* Name() { return "icl-lsp"; }
  static const char* Id() { return "icl"; }
  static const char* Glob() { return "**/*.icl"; }
  static const char* Description() {
    return "ICL language server for IEEE 1687-2014.";
  }
  static bool Accepts(const std::filesystem::path& path) {
    return path.extension() == ".icl";
  }
  static std::shared_ptr<Document> Parse(std::string text,
                                         const std::string& uri) {
    return std::make_shared<Document>(std::move(text));
  }
  static const std::vector<std::string>& Keywords() {
    return icl::IclKeywords();
  }
  static auto Format(const Document& document, const Request& request) {
    return icl::FormatParsedDocument(document, request);
  }
  static std::string ErrorMessage(const Document& document) {
    return document.driver().error_message();
  }
  static icl::Json DiagnosticRange(const Document& document) {
    const auto& driver = document.driver();
    const auto& span = driver.error_span();
    const std::int64_t line =
        driver.error_line() > 0 ? driver.error_line() - 1 : 0;
    return span.first_column > 0
               ? toolsets::SourceRange(document, document.Range(span))
               : icl::Json::Object(
                     {{"start", toolsets::Position(line, 0)},
                      {"end", toolsets::Position(
                                  line, toolsets::LineLength(document.source(),
                                                             line))}});
  }
};
}  // namespace

int main(int argc, char** argv) {
  return toolsets::RunLsp<Language>(argc, argv);
}
