#ifndef ICL_TOOLSETS_SRC_ICL_SEMANTICS_H_
#define ICL_TOOLSETS_SRC_ICL_SEMANTICS_H_

#include <cstddef>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "icl_driver.h"

namespace icl {
struct ByteRange {
  size_t begin = 0, end = 0;
};
struct Declaration {
  std::string name, kind, name_space, target_name, target_namespace, detail,
      ref_enum;
  ByteRange name_range, range;
  int module = -1;
  int parent = -1;
};
struct NameUse {
  NameUse(ByteRange span, std::string name, int scope, int declared = -1)
      : range(span),
        name(std::move(name)),
        module(scope),
        declaration(declared) {}
  ByteRange range;
  std::string name;
  // Half-open token span; dotted components share the document's tokens.
  ByteRange path_tokens;
  int module = -1;
  int declaration = -1;
  int instance = -1;
  bool module_type = false;
  bool parameter = false;
  int enum_owner = -1;
  bool enum_or_signal = false;
  std::optional<size_t> enum_signal;
  std::string name_space;
};

// Owns the single parse used by formatting, diagnostics and language services.
class ParsedDocument {
 public:
  explicit ParsedDocument(std::string source);
  size_t Offset(int line, int byte_column) const;
  ByteRange Range(SourceSpan span) const;
  int ModuleAt(size_t offset) const;
  const NameUse* UseAt(size_t offset) const;
  const std::string& source() const { return source_; }
  const IclDriver& driver() const { return driver_; }
  bool ok() const { return ok_; }
  const std::vector<size_t>& line_starts() const { return line_starts_; }
  const std::vector<Declaration>& declarations() const { return declarations_; }
  const std::vector<NameUse>& uses() const { return uses_; }
  const std::map<int, std::map<size_t, std::string>>& namespace_uses() const {
    return namespace_uses_;
  }
  const std::map<int, std::unordered_map<std::string, std::vector<size_t>>>&
  members() const {
    return members_;
  }

 private:
  void Build();

  std::string source_;
  IclDriver driver_;
  bool ok_ = false;
  std::vector<size_t> line_starts_;
  std::vector<Declaration> declarations_;
  std::vector<NameUse> uses_;
  std::map<int, std::map<size_t, std::string>> namespace_uses_;
  std::map<int, std::unordered_map<std::string, std::vector<size_t>>> members_;
};

struct SymbolRef {
  const ParsedDocument* document = nullptr;
  size_t index = 0;
  explicit operator bool() const { return document != nullptr; }
  const Declaration& declaration() const {
    return document->declarations()[index];
  }
  bool operator==(const SymbolRef& other) const {
    return document == other.document && index == other.index;
  }
};

class SemanticWorkspace {
 public:
  const std::map<std::string, std::shared_ptr<ParsedDocument>>& documents()
      const {
    return documents_;
  }
  std::map<std::string, std::shared_ptr<ParsedDocument>>& documents() {
    return documents_;
  }
  void Reindex();
  SymbolRef Resolve(const ParsedDocument& document, const NameUse& use) const;
  SymbolRef Implementation(SymbolRef symbol) const;
  std::vector<SymbolRef> Complete(const ParsedDocument& document, size_t offset,
                                  bool* member_context) const;
  std::string Uri(const ParsedDocument* document) const;

 private:
  SymbolRef Module(const std::string& ns, const std::string& name,
                   const ParsedDocument* local) const;
  SymbolRef Member(SymbolRef module, const std::string& name,
                   bool parameter = false) const;
  SymbolRef EnumMember(SymbolRef owner, const std::string& name) const;
  std::map<std::string, std::shared_ptr<ParsedDocument>> documents_;
  std::unordered_map<std::string, std::vector<SymbolRef>> modules_;
  std::unordered_map<const ParsedDocument*, std::string> uris_;
};
const std::vector<std::string>& IclKeywords();
}  // namespace icl
#endif  // ICL_TOOLSETS_SRC_ICL_SEMANTICS_H_
