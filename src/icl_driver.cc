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

#include "icl_driver.h"

#include <iostream>
#include <sstream>

#include "icl_parser.hh"

// Provided by the generated scanner (see icl_lexer.l).
void icl_scan_bytes(const char* text, size_t size);
void icl_scan_end();

namespace icl {
namespace {
// `--ast` printer. A visitor so that adding a node class fails to compile here
// instead of silently printing nothing.
//
// Writes to a stream rather than straight to stdout: the dump is now produced
// by the shared job layer, which has no business doing I/O of its own.
class AstPrinter final : public AstVisitor {
 public:
  AstPrinter(std::ostream* out, int depth = 0) : out_(out), depth_(depth) {}

  void Visit(const AstComment& node) override {
    std::string text = node.text();
    const size_t newline = text.find('\n');
    if (newline != std::string::npos) text = text.substr(0, newline) + "...";
    Print("Comment " + text);
  }

  AstPrinter Child() { return AstPrinter(out_, depth_ + 1); }

  void Visit(const AstStatement& node) override {
    // |text| already carries its own keyword ("Source sr;"), so a kind prefix
    // would only repeat what the line says.
    Print(node.text());
  }

  void Visit(const AstBlock& node) override {
    // The header already contains the keyword ("Module m", "Instance x Of Y"),
    // so no prefix is added.
    Print(node.header());
    VisitChildren(node.body());
  }

 private:
  void Print(const std::string& text) const {
    *out_ << std::string(depth_ * 2, ' ') << text << "\n";
  }

  void VisitChildren(const std::vector<AstNode*>& body) {
    AstPrinter child = Child();
    for (const AstNode* node : body) {
      node->Accept(child);
    }
  }

  std::ostream* out_;
  int depth_;
};

}  // namespace

bool IclDriver::Parse(const std::string& text, std::string* error) {
  // Scan straight from memory via flex's buffer API, so no temporary file is
  // ever created and the input buffer is never written to.
  icl_scan_bytes(text.data(), text.size());

  IclParser parser(*this);
#ifdef ICL_PARSER_TRACE
  // Only compiled by the `gramcov` target, which pairs a bison `-t` parser
  // with a flex `-d` scanner. The two trace these builds print are how the
  // grammar and scanner rules get coverage-measured: every reduction names
  // the line in icl_parser.yy it came from, and every accepted token names
  // the line in icl_lexer.l. Turning it on here rather than in a test file
  // is what keeps one grammar shared by both binaries — there is no second
  // parser to instrument.
  parser.set_debug_level(1);
#endif
  parser.parse();
  icl_scan_end();

  if (has_error_) {
    *error = "line " + std::to_string(error_line_) + ": " + error_message_;
    return false;
  }
  return true;
}

namespace {

// Strip a UTF-8 BOM. It used to happen on the file-reading path only, so the
// same bytes were accepted from a file and rejected from stdin — and the LSP,
// which always feeds the parser from memory, rejected them too. Doing it here
// makes every path identical.
void StripBom(std::string* text) {
  if (text->size() >= 3 && static_cast<unsigned char>((*text)[0]) == 0xEF &&
      static_cast<unsigned char>((*text)[1]) == 0xBB &&
      static_cast<unsigned char>((*text)[2]) == 0xBF) {
    text->erase(0, 3);
  }
}

}  // namespace

bool IclDriver::FormatText(const std::string& text_in, std::string* output,
                           std::string* error) {
  std::string text = text_in;
  StripBom(&text);
  if (!Parse(text, error)) return false;
  const std::vector<FormatNode> lines = BuildFormatTree(unit_, style_);
  *output = RenderFormatTree(lines, style_.indent_width);
  return true;
}

std::string IclDriver::AstToString() const {
  std::ostringstream out;
  AstPrinter printer(&out);
  unit_.Accept(printer);
  return out.str();
}

}  // namespace icl
