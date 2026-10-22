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
//
// The driver ties the three passes together and owns the resulting AST:
//
//   input text --[bison+flex]--> AST --[layout]--> lines --[render]--> text
//
// It also exposes the small factory helpers the grammar actions call, so the
// .yy file stays readable, and it owns the parser's diagnostic state (the
// first syntax error), which used to live in a file-scope global.

#ifndef ICL_FORMAT_SRC_ICL_DRIVER_H_
#define ICL_FORMAT_SRC_ICL_DRIVER_H_

#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "format_tree.h"
#include "icl_ast.h"

namespace icl {


class IclDriver {
 public:
  IclDriver() = default;

  IclDriver(const IclDriver&) = delete;
  IclDriver& operator=(const IclDriver&) = delete;

  // Runs the whole pipeline. Returns false and fills |error| when the input is
  // not valid IEEE 1687 ICL.
  bool FormatText(const std::string& text, std::string* output,
                  std::string* error);

  // ---- factory helpers used by the grammar actions ----
  AstUnit& unit() { return unit_; }
  const AstUnit& unit() const { return unit_; }

  AstStatement* MakeStatement(const std::string& text, SourceSpan span) {
    return unit_.Make<AstStatement>(text, span);
  }

  // Takes |body| too: every grammar action assigned the body on the very next
  // line, so keeping them apart only added a named temporary and two lines.
  // A `PortGroup { a, b }` with its elements in source order.
  //
  // One line when the list holds only names, several when it also holds a
  // comment: a `//` comment cannot stay inline, because it would comment out
  // the closing brace.
  AstNode* MakePortGroup(const std::string& name,
                         std::vector<PortGroupItem> items, SourceSpan span) {
    // Fold each comma onto the name that precedes it, so a comma is never a
    // line of its own and the text round-trips byte for byte.
    std::vector<PortGroupItem> folded;
    for (PortGroupItem& item : items) {
      if (item.is_comma && !folded.empty() && folded.back().is_name) {
        folded.back().text += ",";
        continue;
      }
      if (item.is_comma) continue;
      folded.push_back(std::move(item));
    }

    bool has_comment = false;
    for (const PortGroupItem& item : folded) {
      if (!item.is_name) has_comment = true;
    }

    if (!has_comment) {
      std::string joined;
      for (const PortGroupItem& item : folded) {
        if (!joined.empty()) joined += " ";
        joined += item.text;
      }
      return unit_.Make<AstStatement>("PortGroup " + name + " {" + joined + "}",
                                      span);
    }

    std::vector<AstNode*> body;
    for (const PortGroupItem& item : folded) {
      if (item.is_name) {
        body.push_back(unit_.Make<AstStatement>(item.text, item.span));
      } else {
        body.push_back(item.node);
      }
    }
    AstBlock* block = unit_.Make<AstBlock>("PortGroup " + name, span);
    block->body() = std::move(body);
    return block;
  }

  AstBlock* MakeBlock(const std::string& header, std::vector<AstNode*> body,
                      SourceSpan span) {
    AstBlock* block = unit_.Make<AstBlock>(header, span);
    block->body() = std::move(body);
    return block;
  }

  AstNode* AddComment(const std::string& text, SourceSpan span) {
    return unit_.Make<AstComment>(text, span);
  }

  // ---- parser diagnostics ----
  NumericValue NumberLiteral(const std::string& text) const;

  void CheckEnumMember(const std::string& name, const NumericValue& value, int line) {
    if (!enum_members_.insert(name).second)
      SetParseDiagnostic(line, "duplicate Enum member: " + name);
    if (value.width && enum_width_ && value.width != enum_width_)
      SetParseDiagnostic(line, "Enum member widths must match");
    if (value.width) enum_width_ = value.width;
  }

  void CheckEnum(const std::string& name, int line) {
    if (!enum_names_.insert(name).second)
      SetParseDiagnostic(line, "duplicate Enum name: " + name);
    enum_members_.clear();
    enum_width_ = 0;
  }

  void EndModule() { enum_names_.clear(); }
  //
  // Called by the bison error hook. Only the *first* error is kept: later ones
  // are almost always cascades of it, and reporting them buries the cause.
  void SetParseDiagnostic(int line, const std::string& message) {
    if (has_error_) return;
    has_error_ = true;
    error_line_ = line;
    error_message_ = message;
  }

  // Structured form, for callers that need the parts rather than the
  // pre-formatted CLI string: the LSP server turns these into a diagnostic
  // range plus message. |error_line| is 1-based, like every other line number
  // in this code base.
  // No `has_error()` accessor: nothing calls it. `format_job.cc` takes the
  // outcome from `FormatText`'s return value and reads the line and message,
  // so a third accessor would be a third thing that has to agree with
  // `has_error_` — and an unused one is worse than none, because a reader
  // cannot tell whether it is load-bearing.
  int error_line() const { return error_line_; }
  const std::string& error_message() const { return error_message_; }

  void set_style(const FormatStyle& style) { style_ = style; }

  // The AST as text (`--ast`). Returned rather than printed, so that the
  // shared job layer can hand it to either front end without doing I/O.
  std::string AstToString() const;

 private:
  bool Parse(const std::string& text, std::string* error);

  AstUnit unit_;
  std::unordered_set<std::string> enum_members_;
  std::unordered_set<std::string> enum_names_;
  size_t enum_width_ = 0;
  FormatStyle style_;

  bool has_error_ = false;
  int error_line_ = 0;
  std::string error_message_;
};

}  // namespace icl

#endif  // ICL_FORMAT_SRC_ICL_DRIVER_H_
