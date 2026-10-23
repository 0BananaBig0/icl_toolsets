// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include "format_cli.h"
#include "format_job.h"

namespace {
struct Language {
  using Request = icl::FormatRequest;
  using Outcome = icl::FormatOutcome;
  static const char* Name() { return "icl-format"; }
  static const char* Description() {
    return "Formats IEEE 1687-2014 Instrument Connectivity Language files.";
  }
  static Outcome Format(const std::string& text, const Request& request) {
    return icl::FormatDocument(text, request);
  };
};
}  // namespace

int main(int argc, char** argv) {
  return toolsets::FormatCli<Language>::Run(argc, argv);
}
