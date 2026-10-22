// Copyright 2024 The ICL Format Authors.
//
// SPDX-License-Identifier: MIT
// See LICENSE in the project root for license information.
//
// The driver ties the three passes together and owns the resulting AST:
//
//   input text --[bison+flex]--> AST --[layout]--> lines --[render]--> text
//
// It also exposes the small factory helpers the grammar actions call, so the
// .yy file stays readable, and it owns the parser's diagnostic state (the
// first syntax error), which used to live in a file-scope global.

#ifndef ICL_TOOLSETS_SRC_ICL_DRIVER_H_
#define ICL_TOOLSETS_SRC_ICL_DRIVER_H_

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

  // ---- factory helpers used by the grammar actions ----
  AstUnit& unit() { return unit_; }
  const AstUnit& unit() const { return unit_; }

  AstStatement* MakeStatement(const std::string& text, SourceSpan span) {
    return unit_.Make<AstStatement>(text, span);
  }

  AstStatement* MakeParameter(const std::string& keyword,
                              const std::string& name, const std::string& text,
                              ParsedValue value, SourceSpan span) {
    return unit_.Make<AstStatement>(text, span,
                                    keyword == "Parameter"
                                        ? NamedKind::kParameter
                                        : NamedKind::kLocalParameter,
                                    name, std::move(value));
  }

  void CheckNamedValues(const std::vector<AstNode*>& body, bool module);

  // A `PortGroup { a, b }` with its elements in source order.
  //
  // One line when the list holds only names, several when it also holds a
  // comment: a `//` comment cannot stay inline, because it would comment out
  // the closing brace.
  AstNode* MakePortGroup(const std::string& name,
                         std::vector<PortGroupItem> items, SourceSpan span) {
    // Validate separators before folding; comments must not hide a missing
    // name.
    bool need_name = true;
    for (const auto& item : items) {
      if (item.is_name) {
        if (!need_name) {
          SetParseDiagnostic(span.first_line,
                             "expected comma between PortGroup names");
        }
        need_name = false;
      } else if (item.is_comma) {
        if (need_name) {
          SetParseDiagnostic(span.first_line,
                             "expected PortGroup name before comma");
        }
        need_name = true;
      }
    }
    if (need_name) {
      SetParseDiagnostic(span.first_line, "expected PortGroup name");
    }
    // Fold adjacent names/commas only, preserving intervening comments.
    std::vector<PortGroupItem> folded;
    for (PortGroupItem& item : items) {
      if (item.is_comma && !folded.empty() && folded.back().is_name) {
        folded.back().text += ",";
        continue;
      }
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
      if (item.is_name || item.is_comma) {
        body.push_back(unit_.Make<AstStatement>(item.is_comma ? "," : item.text,
                                                item.span));
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
    CheckAddressContext(header, body);
    CheckNamedValues(body, header.rfind("Module ", 0) == 0);
    AstBlock* block = unit_.Make<AstBlock>(header, span);
    block->body() = std::move(body);
    return block;
  }

  AstNode* AddComment(const std::string& text, SourceSpan span) {
    return unit_.Make<AstComment>(text, span);
  }

  // ---- parser diagnostics ----
  ParsedValue NumberLiteral(const std::string& text, int line);
  void CheckBlockItems(const std::string& kind,
                       const std::vector<AstNode*>& body, int line);
  void CheckAddressContext(const std::string& header,
                           const std::vector<AstNode*>& body);

  void CheckEnumMember(const std::string& name, const ParsedValue& value,
                       int line) {
    if (!enum_members_.insert(name).second) {
      SetParseDiagnostic(line, "duplicate Enum member: " + name);
    }
    if (value.width && enum_width_ && value.width != enum_width_) {
      SetParseDiagnostic(line, "Enum member widths must match");
    }
    if (value.width) enum_width_ = value.width;
  }

  void CheckEnum(const std::string& name, int line) {
    if (!enum_names_.insert(name).second) {
      SetParseDiagnostic(line, "duplicate Enum name: " + name);
    }
    enum_members_.clear();
    enum_width_ = 0;
  }

  AstBlock* OpenModule(const std::string& header, SourceSpan span) {
    pending_module_ = unit_.Make<AstBlock>(header, span);
    unit_.top_level().push_back(pending_module_);
    return pending_module_;
  }
  void AppendModuleItem(AstNode* node) {
    pending_module_->body().push_back(node);
  }
  void EndModule() {
    pending_module_ = nullptr;
    deferred_widths_.clear();
    enum_names_.clear();
    access_link_names_.clear();
  }
  void RecordMatch(const char* text, size_t size, bool icl);
  bool Parse(const std::string& text, std::string* error);
  std::string Render(int indent_width) const;

  void CheckInstructionName(const std::string& name, int line) {
    if (!instruction_names_.insert(name).second) {
      SetParseDiagnostic(line, "duplicate AccessLink instruction: " + name);
    }
  }

  void CheckAccessLink(const std::string& name, int line) {
    if (!access_link_names_.insert(name).second) {
      SetParseDiagnostic(line, "duplicate AccessLink name: " + name);
    }
    instruction_names_.clear();
  }

  void CheckChainName(const std::string& name, int line) {
    if (!chain_names_.insert(name).second) {
      SetParseDiagnostic(line, "duplicate Chain name: " + name);
    }
  }
  //
  // Called by the bison error hook. Only the *first* error is kept: later ones
  // are almost always cascades of it, and reporting them buries the cause.
  void SetParseDiagnostic(int line, const std::string& message) {
    // Semantic checks with only a line number retain a whole-line range.
    SetParseDiagnostic(SourceSpan{line, line, 0, 0}, message);
  }
  void SetParseDiagnostic(SourceSpan span, const std::string& message) {
    if (has_error_) return;
    has_error_ = true;
    error_span_ = span;
    error_message_ = message;
  }

  // Source spans use 1-based byte columns; zero columns mean line-only errors.
  // Parse's return value indicates success; these accessors describe failure.
  int error_line() const { return error_span_.first_line; }
  const SourceSpan& error_span() const { return error_span_; }
  const std::string& error_message() const { return error_message_; }

  // The AST as text (`--ast`). Returned rather than printed, so that the
  // shared job layer can hand it to either front end without doing I/O.
  std::string AstToString() const;

 private:
  struct DeferredWidth {
    std::string parameter;
    std::string digits;
    char base;
    int line;
  };
  std::vector<DeferredWidth> deferred_widths_;
  AstBlock* pending_module_ = nullptr;
  size_t scan_offset_ = 0;
  AstUnit unit_;
  std::unordered_set<std::string> enum_members_;
  std::unordered_set<std::string> enum_names_;
  size_t enum_width_ = 0;
  std::unordered_set<std::string> chain_names_;
  std::unordered_set<std::string> instruction_names_;
  std::unordered_set<std::string> access_link_names_;

  bool has_error_ = false;
  SourceSpan error_span_;
  std::string error_message_;
};

}  // namespace icl

#endif  // ICL_TOOLSETS_SRC_ICL_DRIVER_H_
