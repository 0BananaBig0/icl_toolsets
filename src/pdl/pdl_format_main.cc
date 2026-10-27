// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include "pdl/pdl_document.h"
#include "utility/format_cli.h"

namespace {
struct Language {
  using Request = pdl::FormatRequest;
  using Outcome = pdl::FormatOutcome;
  static const char* Name() { return "pdl-format"; }
  static const char* Description() {
    return "Formats IEEE 1687-2014 PDL and Tcl without executing source.";
  }
  static Outcome Format(const std::string& text, const Request& request) {
    return pdl::FormatDocument(text, request);
  };
};
}  // namespace

int main(int argc, char** argv) {
  return toolsets::FormatCli<Language>::Run(argc, argv);
}
