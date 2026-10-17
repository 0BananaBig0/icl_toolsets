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
// Phase 1 of the pipeline: the Abstract Syntax Tree.
//
// An AST node carries *semantic* information only. It deliberately stores no
// whitespace, no indentation and no line-breaking decisions: those are decided
// later, in phase 2 (see format_tree.h). Every node keeps the source line
// span it came from, which the layout pass needs in order to preserve blank
// lines and to re-attach trailing comments.
//
// Ownership model: |AstUnit| owns every node it creates (arena style) and
// child vectors hold non-owning pointers. This keeps bison's variant values
// trivial (plain pointers) and avoids leaking nodes when a rule is discarded
// during error recovery.
//
// Traversal: nodes are visited through |AstVisitor| (see below). There is no
// kind enum and no per-construct classes: everything reduces to three shapes
// (comment / one line / header + body), and a fourth dimension would not be a
// 1:1 map anyway — |Alias|, |ScanRegister|, |Reg| and |Field| each have both a
// one-line form (`Alias a = b;`) and a block form (`Alias a = b { ... }`).
// Each node's text already carries its ICL keyword, so a separate tag would be
// metadata that nothing reads.

#ifndef ICL_FORMAT_SRC_ICL_AST_H_
#define ICL_FORMAT_SRC_ICL_AST_H_

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace icl {

class AstComment;
class AstStatement;
class AstBlock;

// Visitor over the AST node classes.
//
// One overload per concrete class (not per kind tag), so adding a node class
// is a compile-time error everywhere it is not handled — no silent fallthrough
// and no down-cast that can be wrong at runtime.
class AstVisitor {
 public:
  virtual ~AstVisitor() = default;

  virtual void Visit(const AstComment& node) = 0;
  virtual void Visit(const AstStatement& node) = 0;
  virtual void Visit(const AstBlock& node) = 0;
};

// Half-open source span [first_line, last_line] using 1-based line numbers.
//
// Formatting only needs *line* accuracy: line numbers decide whether a blank
// line separated two constructs, and whether a comment was trailing. Columns
// are deliberately not tracked.
struct SourceSpan {
  int first_line = 0;
  int last_line = 0;
};

// Builds a span from line numbers. Taking plain ints (rather than Bison's
// location type) keeps this header independent of the generated parser, so
// that the AST and the layout pass can be unit tested without bison.
inline SourceSpan SpanOf(int first_line, int last_line) {
  SourceSpan span;
  span.first_line = first_line;
  span.last_line = last_line;
  return span;
}

// Base class. Non-copyable: nodes are owned by an |AstUnit| arena and always
// referred to by pointer, so a copy would silently break that ownership (and
// slice the derived part).
class AstNode {
 public:
  explicit AstNode(SourceSpan span) : span_(span) {}
  virtual ~AstNode() = default;

  AstNode(const AstNode&) = delete;
  AstNode& operator=(const AstNode&) = delete;

  virtual void Accept(AstVisitor& visitor) const = 0;

  const SourceSpan& span() const { return span_; }

 private:
  SourceSpan span_;
};

// A `//` or `/* */` comment. Kept in the AST because comments are part of the
// document, not of the whitespace.
class AstComment : public AstNode {
 public:
  AstComment(const std::string& text, SourceSpan span)
      : AstNode(span), text_(text) {}

  void Accept(AstVisitor& visitor) const override { visitor.Visit(*this); }

  const std::string& text() const { return text_; }
  bool is_multiline() const { return text_.find('\n') != std::string::npos; }

 private:
  std::string text_;
};

// A construct rendered on exactly one line: parameters, attributes, generic
// attribute items, ScanMux selection entries, Connection edges, plain ports
// (`ScanInPort si;`) and comma lists (`PortGroup tap_side {si, so, en}`).
//
// |text| is the complete line including any trailing semicolon, so rendering
// needs no knowledge of what the statement is.
class AstStatement : public AstNode {
 public:
  AstStatement(const std::string& text, SourceSpan span)
      : AstNode(span), text_(text) {}

  void Accept(AstVisitor& visitor) const override { visitor.Visit(*this); }

  const std::string& text() const { return text_; }

 private:
  std::string text_;
};

// Any `HEADER { ... }` construct: Module / Enum / Alias-with-block /
// ScanInterface / OneHotScanInterface / ScanRegister / Reg / Field / Instance /
// ScanMux / LogicSignal / Connection / AccessLink / nested 1149.1 blocks, and
// a port declared with a body (`ScanOutPort so { Source sr; }`).
//
// There is no separate Module/Port/PortGroup class: each is just a header
// string plus a body, and nothing downstream distinguishes them. Keeping them
// apart would add three classes and three visitor overloads that behaved
// identically.
class AstBlock : public AstNode {
 public:
  AstBlock(const std::string& header, SourceSpan span)
      : AstNode(span), header_(header) {}

  void Accept(AstVisitor& visitor) const override { visitor.Visit(*this); }

  const std::string& header() const { return header_; }
  const std::vector<AstNode*>& body() const { return body_; }
  std::vector<AstNode*>& body() { return body_; }

 private:
  std::string header_;
  std::vector<AstNode*> body_;
};

// Owns every node of one parsed file, and hands out non-owning pointers.
class AstUnit {
 public:
  AstUnit() = default;

  AstUnit(const AstUnit&) = delete;
  AstUnit& operator=(const AstUnit&) = delete;

  // Creates a node of type |T| and takes ownership of it.
  template <typename T, typename... Args>
  T* Make(Args&&... args) {
    auto node = std::make_unique<T>(std::forward<Args>(args)...);
    T* raw = node.get();
    nodes_.push_back(std::move(node));
    return raw;
  }

  const std::vector<AstNode*>& top_level() const { return top_level_; }
  std::vector<AstNode*>& top_level() { return top_level_; }

  // Visits every top-level node in source order.
  void Accept(AstVisitor& visitor) const {
    for (const AstNode* node : top_level_) {
      node->Accept(visitor);
    }
  }

 private:
  std::vector<std::unique_ptr<AstNode>> nodes_;
  std::vector<AstNode*> top_level_;
};

}  // namespace icl

#endif  // ICL_FORMAT_SRC_ICL_AST_H_
