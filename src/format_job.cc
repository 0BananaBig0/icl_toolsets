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
  FormatOutcome outcome;

  FormatStyle style;
  style.indent_width = ClampIndentWidth(request.indent_width);

  std::string error;
  IclDriver driver;
  driver.set_style(style);

  std::string output;
  if (!driver.FormatText(text, &output, &error)) {
    // Take the structured parts, not just the pre-formatted string: the LSP
    // needs the line number on its own to build a diagnostic range, and the
    // CLI needs the whole line. One source for both.
    outcome.error_line = driver.error_line();
    outcome.error_message = driver.error_message();
    return outcome;
  }

  outcome.ok = true;
  outcome.text = std::move(output);
  if (request.with_ast_dump) outcome.ast_dump = driver.AstToString();
  return outcome;
}

}  // namespace icl
