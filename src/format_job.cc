// Copyright 2024 The ICL Format Authors.
//
// SPDX-License-Identifier: MIT
// See LICENSE in the project root for license information.

#include "format_job.h"

#include "icl_driver.h"

namespace icl {
namespace {

// Largest indent width that will be honoured. See ClampIndentWidth.
constexpr int kMaxIndentWidth = 16;

}  // namespace

int ClampIndentWidth(int requested) {
  return requested < 1 ? 2
                       : (requested > kMaxIndentWidth ? kMaxIndentWidth
                                                      : requested);
}

bool FormatOutcome::ChangedFrom(const std::string& original) const {
  // Only meaningful on success: a document that failed to parse has no
  // formatted form to compare against, and answering "changed" there would
  // make `--check` flag files it could not even read.
  return ok && text != original;
}

std::string FormatOutcome::Diagnostic(const std::string& name) const {
  // No branch for a missing line number: the only way this runs is a failed
  // parse, and every parse failure comes through SetParseDiagnostic, which
  // always carries a 1-based line. Coverage confirmed the fall-back was
  // unreachable, so it was removed rather than left as a guess.
  return name + ": line " + std::to_string(error_line) + ": " + error_message;
}

FormatOutcome FormatDocument(const std::string& text,
                             const FormatRequest& request) {
  return FormatParsedDocument(ParsedDocument(text), request);
}

FormatOutcome FormatParsedDocument(const ParsedDocument& document,
                                   const FormatRequest& request) {
  FormatOutcome outcome;
  if (!document.ok) {
    outcome.error_line = document.driver.error_line();
    outcome.error_message = document.driver.error_message();
    return outcome;
  }
  outcome.ok = true;
  outcome.text = document.driver.Render(ClampIndentWidth(request.indent_width));
  if (request.with_ast_dump) outcome.ast_dump = document.driver.AstToString();
  return outcome;
}

}  // namespace icl
