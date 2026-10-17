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

#include <fstream>
#include <iostream>
#include <sstream>

#include "icl_parser.hh"

// Provided by the generated scanner (see icl_lexer.l).
void icl_scan_bytes(const char* text, size_t size);
void icl_scan_end();

namespace icl {
namespace {

// Reads a whole file. Returns false when the file cannot be opened or is not
// valid UTF-8-ish text (binary input must never be silently reformatted).
bool ReadFile(const std::string& path, std::string* text, std::string* error) {
  std::ifstream stream(path, std::ios::binary);
  if (!stream) {
    *error = "cannot open '" + path + "'";
    return false;
  }
  std::ostringstream buffer;
  buffer << stream.rdbuf();
  *text = buffer.str();
  // Reject files containing NUL bytes: they are not source text, and writing
  // them back would corrupt them.
  if (text->find('\0') != std::string::npos) {
    *error = "not a text file (contains NUL bytes): '" + path + "'";
    return false;
  }
  return true;
}

// `--ast` printer. A visitor rather than a switch on |AstKind| so that adding
// a node class fails to compile here instead of silently printing nothing.
class AstPrinter final : public AstVisitor {
 public:
  explicit AstPrinter(int depth = 0) : depth_(depth) {}

  void Visit(const AstComment& node) override {
    std::string text = node.text();
    const size_t newline = text.find('\n');
    if (newline != std::string::npos) text = text.substr(0, newline) + "...";
    Print("Comment " + text);
  }

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
    std::cout << std::string(depth_ * 2, ' ') << text << "\n";
  }

  void VisitChildren(const std::vector<AstNode*>& body) {
    AstPrinter child(depth_ + 1);
    for (const AstNode* node : body) {
      node->Accept(child);
    }
  }

  int depth_;
};

}  // namespace

bool IclDriver::Parse(const std::string& text, std::string* error) {
  // Scan straight from memory via flex's buffer API, so no temporary file is
  // ever created and the input buffer is never written to.
  icl_scan_bytes(text.data(), text.size());

  IclParser parser(*this);
  parser.parse();
  icl_scan_end();

  if (has_error_) {
    *error = "line " + std::to_string(error_line_) + ": " + error_message_;
    return false;
  }
  return true;
}

bool IclDriver::FormatText(const std::string& text, std::string* output,
                           std::string* error) {
  if (!Parse(text, error)) return false;
  const std::vector<FormatNode> lines = BuildFormatTree(unit_, style_);
  *output = RenderFormatTree(lines, style_.indent_width);
  return true;
}

bool IclDriver::FormatFile(const std::string& path, std::string* original,
                           std::string* output, std::string* error) {
  std::string text;
  if (!ReadFile(path, &text, error)) return false;
  // Strip a UTF-8 BOM; leaving it in would put an invisible character in the
  // middle of the output.
  if (text.size() >= 3 && static_cast<unsigned char>(text[0]) == 0xEF &&
      static_cast<unsigned char>(text[1]) == 0xBB &&
      static_cast<unsigned char>(text[2]) == 0xBF) {
    text.erase(0, 3);
  }
  *original = text;
  return FormatText(text, output, error);
}

void IclDriver::DumpAst() const {
  AstPrinter printer;
  unit_.Accept(printer);
}

}  // namespace icl
