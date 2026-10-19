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

#include "format_tree.h"

#include <algorithm>
#include <string>
#include <vector>

namespace icl {
namespace {

// Blank lines are preserved by comparing source line numbers: if the next node
// starts at least two lines below where the previous one ended, the author had
// a blank line there. Cap the number kept so a wall of blank lines collapses.
int BlankLinesBetween(int previous_end_line, int next_start_line,
                      int max_blank_lines) {
  if (previous_end_line <= 0 || next_start_line <= 0) return 0;
  const int gap = next_start_line - previous_end_line - 1;
  return std::clamp(gap, 0, max_blank_lines);
}

// Splits on '\n'. A trailing newline yields no extra empty element, so a
// two-line comment produces exactly two entries.
std::vector<std::string> SplitLines(const std::string& text) {
  std::vector<std::string> lines;
  size_t start = 0;
  // `<`, not `<=`: at `start == text.size()` every character has been
  // consumed, and going round once more would append the trailing segment a
  // second time — `"a\n"` split into three entries instead of one. Stopping
  // here is also what makes the trailing-newline promise above hold.
  while (start < text.size()) {
    const size_t end = text.find('\n', start);
    if (end == std::string::npos) {
      lines.push_back(text.substr(start));
      break;
    }
    lines.push_back(text.substr(start, end - start));
    start = end + 1;
  }
  return lines;
}

// Phase 2: walks the AST and emits a flat list of renderable lines.
//
// Implemented as an |AstVisitor| so that dispatch is by static type. The
// previous version switched on |AstKind| and down-cast, which crashed:
// |Alias|, |ScanRegister|, |Reg| and |Field| each exist as both a one-line
// statement and a block, so a kind value does not identify a C++ class.
// With the visitor, adding a node class is a compile error here until it is
// handled — no runtime cast can be wrong.
//
// |depth| is visitor state rather than a parameter, because the walk is driven
// by |Accept()| and the recursion happens inside the AST classes.
class FormatTreeBuilder final : public AstVisitor {
 public:
  explicit FormatTreeBuilder(const FormatStyle& style) : style_(style) {}

  std::vector<FormatNode> Build(const AstUnit& unit) {
    unit.Accept(*this);
    return std::move(lines_);
  }

  void Visit(const AstComment& node) override {
    const std::vector<FormatNode>& nodes = lines_;
    // A single-line comment that started on the same source line as the
    // previous output line is a trailing comment. The *comment* is the node
    // that must join the previous line — marking the previous node instead
    // would pull it up onto its own predecessor.
    if (!nodes.empty() && node.span().first_line == nodes.back().source_line() &&
        !node.is_multiline()) {
      Append(node.text(), depth_, node.span().first_line,
             node.span().last_line);
      lines_.back().set_trailing(true);
      return;
    }
    // Multi-line comments keep their internal line structure. Continuation
    // lines are re-based on their common indent: the leading whitespace is an
    // artifact of wherever the author placed the comment, but the *relative*
    // indentation between lines is content and must survive, so the classic
    // " * ..." star alignment is preserved.
    const std::vector<std::string> lines = SplitLines(node.text());
    size_t common = std::string::npos;
    for (size_t i = 1; i < lines.size(); ++i) {
      const size_t indent = lines[i].find_first_not_of(" \t");
      if (indent == std::string::npos) continue;  // Blank line: ignore.
      common = (common == std::string::npos) ? indent : std::min(common, indent);
    }
    // Star-style comments get one extra column so the stars sit under the
    // opening one; anything else keeps its relative indent untouched.
    bool star_style = false;
    if (lines.size() > 1) {
      const size_t first_char = lines[1].find_first_not_of(" \t");
      star_style = first_char != std::string::npos &&
                   lines[1][first_char] == '*';
    }
    const std::string base(star_style ? 1 : 0, ' ');
    for (size_t i = 0; i < lines.size(); ++i) {
      std::string piece = lines[i];
      if (i > 0 && common != std::string::npos && piece.size() >= common) {
        // |piece.substr(common)| already carries the relative indent: cutting
        // at |common| leaves `indent - common` leading spaces in place. Adding
        // |extra| on top of that counts it twice, so the comment drifts right
        // by its own indent on every run (and is therefore not idempotent).
        const size_t indent = piece.find_first_not_of(" \t");
        (void)indent;
        piece = base + piece.substr(common);
      }
      Append(piece, depth_, node.span().first_line + static_cast<int>(i),
             node.span().first_line + static_cast<int>(i));
    }
  }

  void Visit(const AstStatement& node) override {
    // The statement may span several source lines (`CaptureSource a,` /
    // `b;`) and is emitted as one output line. |last_source_line_| must be
    // advanced to the *last* source line, not the first: otherwise the lines
    // the statement itself consumed look like gaps before the next node, and
    // a blank line appears after every wrapped statement.
    Append(node.text(), depth_, node.span().first_line, node.span().last_line);
  }

  void Visit(const AstBlock& node) override {
    EmitGroup(node.header(), node.body(), node.span());
  }

 private:
  void EmitGroup(const std::string& header,
                 const std::vector<AstNode*>& body, const SourceSpan& span) {
    Append(header + " {", depth_, span.first_line, span.first_line);
    ++depth_;
    for (const AstNode* child : body) {
      child->Accept(*this);
    }
    --depth_;
    Append("}", depth_, span.last_line, span.last_line);
  }

  // Appends one output line, inserting blank-line markers first if the source
  // had them between the previous line and this one.
  //
  // |source_line| is where the node *starts*, which is what the gap to the
  // previous node is measured from. |end_line| is where it finishes in the
  // source, which is what the next gap is measured from — the two differ for
  // anything that wraps across source lines but renders as one.
  void Append(const std::string& text, int depth, int source_line,
              int end_line) {
    const int blanks = BlankLinesBetween(last_source_line_, source_line,
                                         style_.max_blank_lines);
    for (int i = 0; i < blanks; ++i) {
      FormatNode blank(true, "", 0);
      blank.set_source_line(source_line);
      lines_.push_back(std::move(blank));
    }
    FormatNode node(false, text, depth);
    node.set_source_line(source_line);
    lines_.push_back(std::move(node));
    last_source_line_ = std::max(last_source_line_, end_line);
  }

  const FormatStyle& style_;
  std::vector<FormatNode> lines_;
  int depth_ = 0;
  int last_source_line_ = 0;
};

}  // namespace

std::vector<FormatNode> BuildFormatTree(const AstUnit& unit,
                                        const FormatStyle& style) {
  FormatTreeBuilder builder(style);
  return builder.Build(unit);
}

std::string RenderFormatTree(const std::vector<FormatNode>& lines,
                             int indent_width) {
  std::string out;
  bool at_line_start = true;
  for (const FormatNode& node : lines) {
    if (node.is_blank_line()) {
      // A blank line marker: close the current line, then emit an empty one.
      out += at_line_start ? "\n" : "\n\n";
      at_line_start = true;
      continue;
    }
    if (node.trailing() && !at_line_start) {
      out += ' ';
    } else {
      if (!at_line_start) out += '\n';
      out.append(node.indent() * indent_width, ' ');
    }
    out += node.text();
    at_line_start = false;
  }
  if (!out.empty()) out += '\n';
  return out;
}

}  // namespace icl
