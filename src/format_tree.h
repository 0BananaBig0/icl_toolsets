// Copyright 2024 The ICL Format Authors.
//
// SPDX-License-Identifier: MIT
// See LICENSE in the project root for license information.
//
// Phase 2 of the pipeline: the Format Tree (a.k.a. layout tree, doc tree).
//
// Wikipedia calls this family of structures a "document tree"; clang-format
// calls the analogous concept an "unwrapped line". Either way the job of this
// pass is to turn the AST's *semantic* structure into *visual* structure:
// decide where lines break, how much each line is indented, which nodes stay
// on one line, and where blank lines belong.
//
// Why not format straight from the AST? Because the AST answers "what is this
// construct", not "how should it look". Two examples from ICL:
//
//   * `PortGroup tap_side {si, so, en}` is a comma list, semantically close to
//     `PortGroup tap_side { Source si; ... }`, but visually it must stay on a
//     single line. That is a layout decision, not a grammar one.
//   * `} // trailing note` — the comment belongs to the closing brace's line
//     even though the AST visited it as a sibling node afterwards.
//
// Keeping this pass separate means the parser stays free of formatting policy,
// and a future style (2-space indent, Allman braces) is a change here only.

#ifndef ICL_TOOLSETS_SRC_FORMAT_TREE_H_
#define ICL_TOOLSETS_SRC_FORMAT_TREE_H_

#include <string>
#include <vector>

#include "icl_ast.h"

namespace icl {

// Style knobs. Defaults follow the Google C++ style guide: 2-space indent.
struct FormatStyle {
  // Number of spaces per indentation level.
  int indent_width = 2;

  // Maximum consecutive blank lines to keep from the input.
  int max_blank_lines = 1;
};

// One output line, or one blank-line marker.
//
// There is no `kind` field: the renderer only ever needs to distinguish "real
// text" from "blank separator", which a bool expresses directly. An earlier
// version carried a four-value FormatKind, three of which no code read.
class FormatNode {
 public:
  FormatNode(bool is_blank_line, std::string text, int indent)
      : is_blank_line_(is_blank_line),
        text_(std::move(text)),
        indent_(indent) {}

  // True for a blank-line separator, not for a line of source text. A real
  // line can also be empty (a blank line inside a multi-line comment), so an
  // empty |text| alone cannot distinguish the two.
  bool is_blank_line() const { return is_blank_line_; }

  const std::string& text() const { return text_; }
  int indent() const { return indent_; }

  // Source line the text came from (1-based). Used to decide whether a blank
  // line separates this node from the previous one.
  int source_line() const { return source_line_; }
  void set_source_line(int line) { source_line_ = line; }

  // True for a node that continues the previous output line (`} // note`).
  bool trailing() const { return trailing_; }
  void set_trailing(bool value) { trailing_ = value; }

 private:
  bool is_blank_line_;
  std::string text_;
  int indent_ = 0;
  int source_line_ = 0;
  bool trailing_ = false;
};

// Phase 2 entry point: AST -> the list of renderable lines.
//
// Decides line breaks, indentation (as absolute levels), blank-line
// preservation and trailing-comment attachment. The result is flat on purpose:
// indentation has already been resolved into every node, so printing is a
// simple walk with no state.
std::vector<FormatNode> BuildFormatTree(const AstUnit& unit,
                                        const FormatStyle& style);

// Renders a FormatTree to text. Phase 3.
//
// |indent_width| converts each node's indentation *level* into spaces; levels
// are stored in the tree so that style changes do not require a re-layout.
std::string RenderFormatTree(const std::vector<FormatNode>& lines,
                             int indent_width);

}  // namespace icl

#endif  // ICL_TOOLSETS_SRC_FORMAT_TREE_H_
