// Copyright 2024 The ICL Format Authors.
//
// SPDX-License-Identifier: MIT
// See LICENSE in the project root for license information.
//
// One formatting job, from "an indent width was requested" to "here is the
// result, or the reason there is none".
//
// This exists so the two front ends share as much of the chain as possible:
//
//   CLI:  argv -----> -n -----> FormatDocument --+--> write file / print
//   LSP:  JSON ----> tabSize -> FormatDocument --+--> TextEdit / diagnostic
//         ^^^^^^^^^^^^^^^^^^^^                  ^
//         these genuinely differ                |
//                        everything in between is this file
//
// The split is as late as it can be. Before this existed, each front end
// built its own driver, set its own style, called FormatText and inspected
// the error — five lines apiece that could disagree, and did: the LSP clamped
// an absurd tabSize and the CLI did not.
//
// Doing no I/O here is deliberate: the LSP must not write to stdout (that
// would corrupt the protocol stream), and keeping this layer pure is what
// makes it possible.

#ifndef ICL_FORMAT_SRC_FORMAT_JOB_H_
#define ICL_FORMAT_SRC_FORMAT_JOB_H_

#include <string>
#include "icl_semantics.h"

namespace icl {

// The indent width to actually use. Shared because an absurd value is not
// merely ugly: it is applied per nesting level, so a 157-byte file with
// tabSize=99999 and six levels produced a 7.8 MB reply. Real editors send 2,
// 4 or 8; 16 leaves headroom without handing a buggy client a memory
// amplifier.
int ClampIndentWidth(int requested);

struct FormatRequest {
  int indent_width = 2;
  // Only the CLI's `--ast` asks for this; the LSP never does.
  bool with_ast_dump = false;
};

struct FormatOutcome {
  bool ok = false;

  // Formatted text. Valid when `ok`; empty otherwise, so a caller that
  // forgets to check cannot accidentally emit a half-formed document.
  std::string text;

  // The AST as text. Filled only when requested.
  std::string ast_dump;

  // Error detail, valid when `!ok`. `error_line` is 1-based, matching every
  // other line number in this code base; 0 means "no line to point at".
  int error_line = 0;
  std::string error_message;

  // "name: line 3: ..." — the one-line form both front ends need, so they
  // cannot drift into phrasing it differently.
  std::string Diagnostic(const std::string& name) const;

  // Whether formatting changed anything. Shared because it is a question
  // about the result, not about the front end: `--check` and `-w` both need
  // it, and the LSP uses the same fact to decide whether to send an edit at
  // all rather than an edit that replaces the text with itself.
  bool ChangedFrom(const std::string& original) const;
};

FormatOutcome FormatParsedDocument(const ParsedDocument& document,
                                   const FormatRequest& request);

// Runs the shared parse, semantic model, layout and render chain.
FormatOutcome FormatDocument(const std::string& text,
                             const FormatRequest& request);

}  // namespace icl

#endif  // ICL_FORMAT_SRC_FORMAT_JOB_H_
