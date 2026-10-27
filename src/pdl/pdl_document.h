// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#ifndef ICL_TOOLSETS_SRC_PDL_PDL_DOCUMENT_H_
#define ICL_TOOLSETS_SRC_PDL_PDL_DOCUMENT_H_

#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "utility/text_position.h"

namespace pdl {
// Tcl list syntax is shared by static validation and generator execution.
std::vector<std::string> SplitList(const std::string& text, bool* valid);

struct CommandSpec {
  int minimum, maximum, level;
};
const CommandSpec* FindCommandSpec(const std::string& name);

struct ParseContext {
  int level = 1;
  std::string binding;
  std::string procedure_namespace;
  bool analyze = true;
};

struct ByteRange {
  size_t begin = 0, end = 0;
};
struct Command;
struct Word {
  ByteRange range, content;
  std::string value;
  bool literal = false;
  bool body = false;
  bool command_substitution = false;
  // A vector suffix is static PDL syntax but remains substitution in Tcl.
  bool vector_substitution = false;
  bool expanded = false;
  std::vector<ByteRange> vector_ranges;
  std::vector<Command> script;
  std::vector<Command> substitutions;
  std::vector<std::pair<ByteRange, std::string>> variables;
};
struct Command {
  ByteRange range;
  std::string comment;
  std::vector<Word> words;
};
struct Declaration {
  std::string name, kind, detail, binding, name_space;
  ByteRange name_range, range;
  int module = -1;
  std::string target = "";
  bool static_context = true;
};
struct NameUse {
  ByteRange range;
  std::string name, kind, binding, name_space;
  int module = -1;
  int declaration = -1;
  std::string prefix = "";
  bool static_context = true;
};

// PDL syntax and semantic data are independent of the ICL AST and tokens.
class Document {
 public:
  explicit Document(std::string source, bool is_icl = false,
                    const ParseContext& initial = {});
  const std::string& source() const { return source_; }
  bool ok() const { return error_message_.empty(); }
  bool CanFormat() const { return !is_icl_; }
  const std::string& error_message() const { return error_message_; }
  ByteRange ErrorRange() const { return error_range_; }
  int error_line() const;
  const std::vector<size_t>& line_starts() const { return line_starts_; }
  size_t Utf16Length(size_t begin, size_t end) const {
    return utf16_index_.Length(source_, begin, end);
  }
  const std::vector<Declaration>& declarations() const { return declarations_; }
  const std::vector<NameUse>& uses() const { return uses_; }
  const std::vector<Command>& commands() const { return commands_; }
  const NameUse* UseAt(size_t byte) const;
  std::string Format(int indent_width) const;
  std::string Dump() const;

 private:
  friend class Workspace;
  struct Context {
    int level = 1;
    int owner = -1;
    bool has_binding = false;
    bool namespace_known = true, call_namespace_known = true,
         prefix_known = true, tcl_namespace_known = true;
    std::string binding, name_space, call_namespace, tcl_namespace, prefix;
  };
  using VariableKey = std::tuple<int, std::string, std::string>;
  struct ContextSpan {
    ByteRange range;
    Context context;
  };
  const Context& ContextAt(size_t byte) const;
  const char* ParserData() const;
  std::vector<Command> Parse(size_t begin, size_t end, int depth);
  void ParseExpression(Word* word, int depth);
  void Analyze(std::vector<Command>* commands, Context context,
               ByteRange scope);
  void AnalyzeWords(std::vector<Word>* words, const Context& context,
                    const Context& child);
  void Error(ByteRange range, std::string message);
  void Declare(Declaration declaration);
  std::string source_;
  toolsets::Utf16Index utf16_index_;
  bool is_icl_ = false;
  std::string parser_source_;
  std::vector<size_t> line_starts_{0};
  std::vector<Command> commands_;
  std::vector<Declaration> declarations_;
  std::map<VariableKey, int> variables_;
  std::map<VariableKey, VariableKey> variable_bindings_;
  std::vector<ContextSpan> contexts_;
  std::vector<NameUse> uses_;
  ByteRange error_range_;
  std::string error_message_;
};

struct SymbolRef {
  const Document* document = nullptr;
  size_t index = 0;
  explicit operator bool() const { return document != nullptr; }
  const Declaration& declaration() const {
    return document->declarations()[index];
  }
  bool operator==(const SymbolRef& other) const {
    return document == other.document && index == other.index;
  }
};

class Workspace {
 public:
  auto& documents() { return documents_; }
  const auto& documents() const { return documents_; }
  void Reindex();
  SymbolRef Resolve(const Document& document, const NameUse& use) const;
  SymbolRef Implementation(SymbolRef symbol) const { return symbol; }
  std::vector<SymbolRef> Complete(const Document& document, size_t byte,
                                  bool* member_context) const;
  std::string Uri(const Document* document) const;

 private:
  std::map<std::string, std::shared_ptr<Document>> documents_;
  using ProcedureKey = std::tuple<std::string, std::string, std::string>;
  std::map<ProcedureKey, std::vector<SymbolRef>> procedures_;
  std::map<const Document*, std::string> uris_;
  SymbolRef IclModule(const std::string& name) const;
  SymbolRef IclObject(const std::string& binding,
                      const std::string& path) const;
  std::map<std::string, std::vector<SymbolRef>> modules_;
  using MemberKey = std::pair<int, std::string>;
  std::map<const Document*, std::map<MemberKey, std::vector<SymbolRef>>>
      members_;
};

struct FormatRequest {
  int indent_width = 2;
  bool with_ast_dump = false;
};
struct FormatOutcome {
  bool ok = false;
  std::string text, ast_dump, error_message;
  int error_line = 0;
  bool ChangedFrom(const std::string& original) const {
    return ok && text != original;
  }
  std::string Diagnostic(const std::string& name) const;
};
FormatOutcome FormatParsedDocument(const Document& document,
                                   const FormatRequest& request);
FormatOutcome FormatDocument(const std::string& source,
                             const FormatRequest& request);
const std::vector<std::string>& Keywords();
int ClampIndentWidth(int requested);
// Literal PDL numeric syntax; excludes argument/variable references.
bool IsNumberLiteral(const std::string& text);

}  // namespace pdl
#endif  // ICL_TOOLSETS_SRC_PDL_PDL_DOCUMENT_H_
