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

#include "format_tree.h"
#include "icl_ast.h"

namespace icl {

class IclDriver {
 public:
  IclDriver() = default;

  IclDriver(const IclDriver&) = delete;
  IclDriver& operator=(const IclDriver&) = delete;

  // Runs the whole pipeline. Returns false and fills |error| when the input is
  // not valid IEEE 1687 ICL, or when it cannot be read.
  //
  // |original| receives the file as read (BOM stripped), so that callers which
  // need to know whether formatting changed anything (`--check`, `-w`) do not
  // have to read the file a second time.
  bool FormatFile(const std::string& path, std::string* original,
                  std::string* output, std::string* error);

  bool FormatText(const std::string& text, std::string* output,
                  std::string* error);

  // ---- factory helpers used by the grammar actions ----
  AstUnit& unit() { return unit_; }
  const AstUnit& unit() const { return unit_; }

  AstStatement* MakeStatement(const std::string& text, SourceSpan span) {
    return unit_.Make<AstStatement>(text, span);
  }

  AstBlock* MakeBlock(const std::string& header, SourceSpan span) {
    return unit_.Make<AstBlock>(header, span);
  }

  AstNode* AddComment(const std::string& text, SourceSpan span) {
    return unit_.Make<AstComment>(text, span);
  }

  // ---- parser diagnostics ----
  //
  // Read back by Parse() below, which owns this driver's own state; no public
  // accessors, because nothing outside reads them.
  // Called by the bison error hook. Only the *first* error is kept: later ones
  // are almost always cascades of it, and reporting them buries the cause.
  void SetParseDiagnostic(int line, const std::string& message) {
    if (has_error_) return;
    has_error_ = true;
    error_line_ = line;
    error_message_ = message;
  }

  void set_style(const FormatStyle& style) { style_ = style; }

  // Dumps the AST to stdout; used by `--ast`.
  void DumpAst() const;

 private:
  bool Parse(const std::string& text, std::string* error);

  AstUnit unit_;
  FormatStyle style_;

  bool has_error_ = false;
  int error_line_ = 0;
  std::string error_message_;
};

}  // namespace icl

#endif  // ICL_FORMAT_SRC_ICL_DRIVER_H_
