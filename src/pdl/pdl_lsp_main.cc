// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include "pdl/pdl_document.h"
#include "utility/lsp_server.h"

namespace {
struct Language {
  using Document = pdl::Document;
  using Workspace = pdl::Workspace;
  using SymbolRef = pdl::SymbolRef;
  using Request = pdl::FormatRequest;
  static const char* Name() { return "pdl-lsp"; }
  static const char* Id() { return "pdl"; }
  static const char* Glob() { return "**/*.{pdl,tcl,icl}"; }
  static const char* Description() {
    return "PDL language server for IEEE 1687-2014.";
  }
  static bool Accepts(const std::filesystem::path& path) {
    return path.extension() == ".pdl" || path.extension() == ".tcl" ||
           path.extension() == ".icl";
  }
  static std::shared_ptr<Document> Parse(std::string text,
                                         const std::string& uri) {
    return std::make_shared<Document>(
        std::move(text),
        std::filesystem::path(toolsets::FilePath(uri)).extension() == ".icl");
  }
  static const std::vector<std::string>& Keywords() { return pdl::Keywords(); }
  static auto Format(const Document& document, const Request& request) {
    return pdl::FormatParsedDocument(document, request);
  }
  static std::string ErrorMessage(const Document& document) {
    return document.error_message();
  }
  static icl::Json DiagnosticRange(const Document& document) {
    return toolsets::SourceRange(document, document.ErrorRange());
  }
};
}  // namespace

int main(int argc, char** argv) {
  return toolsets::RunLsp<Language>(argc, argv);
}
