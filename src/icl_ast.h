// Copyright 2024 The ICL Format Authors.
//
// SPDX-License-Identifier: MIT
// See LICENSE in the project root for license information.
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
// 1:1 map anyway — |Alias| and |ScanRegister| each have both a
// one-line form (`Alias a = b;`) and a block form (`Alias a = b { ... }`).
// Named statements also carry metadata for shared local semantic checks.

#ifndef ICL_TOOLSETS_SRC_ICL_AST_H_
#define ICL_TOOLSETS_SRC_ICL_AST_H_

#include <cstddef>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace icl {

// A zero width means context/parameter dependent, not a zero-bit object.
enum class ValueKind { kUnknown, kNumber, kString };

struct ValueAtom {
  ValueKind kind;
  bool unsized;
  std::string reference;
};

struct ParsedValue {
  ParsedValue() = default;
  explicit ParsedValue(std::string text, size_t width = 0,
                       ValueKind kind = ValueKind::kNumber)
      : text(std::move(text)), width(width), kind(kind) {}
  std::string text;
  size_t width = 0;
  // Literal magnitude only; compound values require later elaboration.
  std::optional<size_t> integer;
  bool inverted = false;
  ValueKind kind = ValueKind::kNumber;
  bool has_unknown = false;
  bool unsized = false;
  std::string reference;
  std::vector<ValueAtom> atoms;
  std::vector<std::string> dependencies;

  void Append(const std::string& gap, const ParsedValue& value) {
    integer.reset();
    if (atoms.empty()) atoms.push_back({kind, unsized, reference});
    if (value.atoms.empty()) {
      atoms.push_back({value.kind, value.unsized, value.reference});
    } else {
      atoms.insert(atoms.end(), value.atoms.begin(), value.atoms.end());
    }
    dependencies.insert(dependencies.end(), value.dependencies.begin(),
                        value.dependencies.end());
    text += ", " + gap + value.text;
    has_unknown = has_unknown || value.has_unknown;
    if (kind == ValueKind::kUnknown) kind = value.kind;
    width = width && value.width &&
                    width <= std::numeric_limits<size_t>::max() - value.width
                ? width + value.width
                : 0;
  }
};

struct ParsedReference {
  std::string text;
  bool is_scalar = true;
  size_t depth = 0;
  int end_line = 1;
  int end_column = 1;
};

// Syntactic categories needed by LogicSignal, without evaluating expressions.
struct LogicValue {
  std::string text;
  bool is_integer = false;
  bool is_number = false;
  bool is_numeric_concat = false;
  bool is_enum = false;
  bool has_unknown = false;
};

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

// Source span with 1-based lines and byte columns; the end is exclusive.
struct SourceSpan {
  int first_line = 0;
  int last_line = 0;
  int first_column = 1;
  int last_column = 1;
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

// Accept generated locations without depending on a generated header.
template <typename Location>
inline SourceSpan SpanOf(const Location& first, const Location& last) {
  return {first.begin.line, last.end.line, first.begin.column, last.end.column};
}

struct SourceToken {
  std::string text;
  size_t begin = 0;
  size_t end = 0;
};

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
  void set_span(SourceSpan span) { span_ = span; }

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

// A construct rendered on exactly one line: parameters, attributes, ScanMux
// selection entries and plain ports
// (`ScanInPort si;`).
//
// |text| is the complete line including any trailing semicolon, so rendering
// needs no knowledge of what the statement is.
enum class NamedKind { kNone, kParameter, kLocalParameter, kAttribute };

class AstStatement : public AstNode {
 public:
  AstStatement(const std::string& text, SourceSpan span,
               NamedKind kind = NamedKind::kNone, std::string name = "",
               std::optional<ParsedValue> value = std::nullopt)
      : AstNode(span),
        text_(text),
        kind_(kind),
        name_(std::move(name)),
        value_(std::move(value)) {}

  void Accept(AstVisitor& visitor) const override { visitor.Visit(*this); }

  const std::string& text() const { return text_; }

  NamedKind named_kind() const { return kind_; }
  const std::string& name() const { return name_; }
  const std::optional<ParsedValue>& value() const { return value_; }

 private:
  std::string text_;
  NamedKind kind_;
  std::string name_;
  std::optional<ParsedValue> value_;
};

// Any `HEADER { ... }` construct: Module / Enum / Alias-with-block /
// ScanInterface / ScanRegister / Instance /
// ScanMux / LogicSignal / AccessLink / nested 1149.1 blocks, and
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

// Owns every node of one parsed file and returns non-owning pointers.
// One element of a `PortGroup { ... }` list: a name, a comment, or a comma.
// The comma is kept as an item so that folding it onto the preceding name is a
// decision made once here, not a special case in the grammar actions.
struct PortGroupItem {
  bool is_name = false;
  bool is_comma = false;
  std::string text;         // name, with any comma already folded on
  AstNode* node = nullptr;  // set only for a comment
  // Each item keeps the span of *its own* token. Giving every item the span of
  // the whole group was wrong: the layout pass decides whether a comment is
  // trailing by comparing it with the previous node's end line, and with one
  // shared span a comment always looked attached to the group, so `a, // c`
  // was emitted trailing and then moved to its own line on the next pass —
  // never settling.
  SourceSpan span;

  static PortGroupItem Name(std::string t, SourceSpan span) {
    PortGroupItem item;
    item.is_name = true;
    item.text = std::move(t);
    item.span = span;
    return item;
  }
  static PortGroupItem Comment(AstNode* n, SourceSpan span) {
    PortGroupItem item;
    item.node = n;
    item.span = span;
    return item;
  }
  static PortGroupItem Comma(SourceSpan span) {
    PortGroupItem item;
    item.is_comma = true;
    item.span = span;
    return item;
  }
};

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

  std::vector<AstNode*>& top_level() { return top_level_; }
  const std::vector<AstNode*>& top_level() const { return top_level_; }
  const std::vector<SourceToken>& tokens() const { return tokens_; }
  std::vector<SourceToken>& tokens() { return tokens_; }

  // Visits every top-level node in source order.
  void Accept(AstVisitor& visitor) const {
    for (const AstNode* node : top_level_) {
      node->Accept(visitor);
    }
  }

 private:
  std::vector<SourceToken> tokens_;
  std::vector<std::unique_ptr<AstNode>> nodes_;
  std::vector<AstNode*> top_level_;
};

}  // namespace icl

#endif  // ICL_TOOLSETS_SRC_ICL_AST_H_
