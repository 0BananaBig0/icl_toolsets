#include "icl_semantics.h"
#include <algorithm>
#include <functional>
#include <optional>
#include <unordered_set>

namespace icl {
namespace {
bool Identifier(const std::string& text) {
  return !text.empty() && ((text[0] >= 'a' && text[0] <= 'z') ||
                          (text[0] >= 'A' && text[0] <= 'Z'));
}
bool ParameterKind(const std::string& kind) {
  return kind == "Parameter" || kind == "LocalParameter";
}
bool Declares(const std::string& kind) {
  static const std::unordered_set<std::string> kinds = {
    "Module", "Instance", "Parameter", "LocalParameter", "Alias", "Enum",
    "ScanRegister", "DataRegister", "LogicSignal", "ScanMux", "DataMux", "ClockMux",
    "ScanInterface", "Chain", "OneHotScanGroup", "OneHotDataGroup", "PortGroup", "AccessLink",
    "ScanInPort", "ScanOutPort", "DataInPort", "DataOutPort", "ClockPort", "ToClockPort",
    "TCKPort", "ToTCKPort", "TMSPort", "ToTMSPort", "TRSTPort", "ToTRSTPort",
    "ResetPort", "ToResetPort", "SelectPort", "ToSelectPort", "ToIRSelectPort",
    "ShiftEnPort", "ToShiftEnPort", "CaptureEnPort", "ToCaptureEnPort",
    "UpdateEnPort", "ToUpdateEnPort", "AddressPort", "WriteEnPort", "ReadEnPort"};
  return kinds.count(kind);
}
bool SignalProperty(const std::string& kind) {
  static const std::unordered_set<std::string> kinds = {
    "Source", "ScanInSource", "CaptureSource", "InputPort", "ReadDataSource", "WriteDataSource",
    "WriteEnSource", "Enable", "DifferentialInvOf", "Port", "RefEnum",
    "ActiveSignals", "ScanInterface", "AllowBroadcastOnScanInterface",
    "DefaultLoadValue", "ResetValue", "iApplyEndState"};
  return kinds.count(kind);
}
}  // namespace

const std::vector<std::string>& IclKeywords() {
  // ICL vocabulary only. Context-specific property validity remains the parser's job.
  static const std::vector<std::string> words = {
    "Module", "NameSpace", "UseNameSpace", "Parameter", "LocalParameter", "Attribute",
    "Alias", "Enum", "Instance", "Of", "ScanInPort", "ScanOutPort", "DataInPort",
    "DataOutPort", "ScanRegister", "DataRegister", "ScanMux", "DataMux", "ClockMux",
    "LogicSignal", "ScanInterface", "Chain", "OneHotScanGroup", "OneHotDataGroup",
    "AccessLink", "InputPort", "Port", "SelectedBy", "ScanInSource", "CaptureSource",
    "Source", "ResetValue", "DefaultLoadValue", "RefEnum", "iApplyEndState", "AccessTogether",
    "ClockPort", "ToClockPort", "TCKPort", "ToTCKPort", "TMSPort", "ToTMSPort",
    "TRSTPort", "ToTRSTPort", "ResetPort", "ToResetPort", "SelectPort", "ToSelectPort",
    "ToIRSelectPort", "ShiftEnPort", "ToShiftEnPort", "CaptureEnPort", "ToCaptureEnPort",
    "UpdateEnPort", "ToUpdateEnPort", "AddressPort", "ReadEnPort", "WriteEnPort",
    "Enable", "DifferentialInvOf", "LaunchEdge", "ActivePolarity", "Period",
    "FreqMultiplier", "FreqDivider", "AddressValue", "ReadCallBack", "WriteCallBack",
    "ReadDataSource", "WriteDataSource", "WriteEnSource", "AllowBroadcastOnScanInterface",
    "BSDLEntity", "ActiveSignals", "STD_1149_1_2001", "STD_1149_1_2013", "PortGroup"};
  return words;
}

ParsedDocument::ParsedDocument(std::string text) : source(std::move(text)) {
  line_starts.push_back(0);
  for (size_t i = 0; i < source.size(); ++i)
    if (source[i] == '\n') line_starts.push_back(i + 1);
  std::string error;
  ok = driver.Parse(source, &error);
  Build();
}
size_t ParsedDocument::Offset(int line, int column) const {
  if (line < 1) return 0;
  if (static_cast<size_t>(line) > line_starts.size()) return source.size();
  const size_t start = line_starts[line - 1];
  const size_t end = static_cast<size_t>(line) < line_starts.size() ? line_starts[line] - 1 : source.size();
  return std::min(end, start + static_cast<size_t>(std::max(1, column) - 1));
}
ByteRange ParsedDocument::Range(SourceSpan span) const {
  return {Offset(span.first_line, span.first_column), Offset(span.last_line, span.last_column)};
}
int ParsedDocument::ModuleAt(size_t offset) const {
  for (size_t i = 0; i < declarations.size(); ++i) {
    const auto& d = declarations[i];
    if (d.kind == "Module" && d.range.begin <= offset && offset <= d.range.end)
      return static_cast<int>(i);
  }
  return -1;
}
const NameUse* ParsedDocument::UseAt(size_t offset) const {
  auto it = std::upper_bound(uses.begin(), uses.end(), offset,
      [](size_t byte, const NameUse& use) { return byte < use.range.begin; });
  if (it != uses.begin()) {
    --it;
    if (offset < it->range.end) return &*it;
  }
  return nullptr;
}

void ParsedDocument::Build() {
  const auto& tokens = driver.unit().tokens;
  auto token_index = [&](size_t byte) {
    return static_cast<size_t>(std::lower_bound(tokens.begin(), tokens.end(), byte,
        [](const SourceToken& token, size_t value) { return token.begin < value; }) - tokens.begin());
  };
  std::string definition_ns;
  std::optional<std::string> file_use_ns;
  std::function<void(const AstNode*, int, int, std::string&, bool)> walk;
  walk = [&](const AstNode* node, int module, int parent, std::string& use_ns, bool reference_body) {
    if (dynamic_cast<const AstComment*>(node)) return;
    const auto range = Range(node->span());
    const size_t first = token_index(range.begin), last = token_index(range.end);
    if (first >= tokens.size() || first >= last) return;
    const auto* block = dynamic_cast<const AstBlock*>(node);
    size_t header_end = last;
    if (block) {
      header_end = first;
      while (header_end < last && tokens[header_end].text != "{") ++header_end;
    }
    const std::string kind = tokens[first].text;
    if (kind == "NameSpace" || kind == "UseNameSpace") {
      const std::string name = first + 1 < last && tokens[first + 1].text != ";" ? tokens[first + 1].text : "";
      if (kind == "NameSpace") definition_ns = name;
      else { use_ns = name; if (module < 0) file_use_ns = name;
        else namespace_uses[module][range.begin] = name; }
      return;
    }
    const bool in_instance = parent >= 0 && declarations[parent].kind == "Instance";
    const bool enum_item = parent >= 0 && declarations[parent].kind == "Enum";
    const bool declaration = !reference_body && (enum_item || (Declares(kind) && !(in_instance && kind == "Parameter")));
    int own = -1;
    size_t ref_start = first + 1;
    if (declaration && (enum_item || first + 1 < header_end)) {
      const auto& name = tokens[enum_item ? first : first + 1];
      Declaration d;
      d.name = name.text; d.kind = enum_item ? "EnumMember" : kind;
      d.name_range = {name.begin, name.end}; d.range = range;
      d.module = module; d.parent = parent;
      d.name_space = definition_ns;
      d.detail = source.substr(tokens[first].begin,
          (header_end < last ? tokens[header_end].begin : range.end) - tokens[first].begin);
      own = static_cast<int>(declarations.size());
      declarations.push_back(std::move(d));
      uses.push_back({{name.begin, name.end}, {name.text}, module, own});
      if (kind == "Module") { module = own; use_ns = file_use_ns.value_or(definition_ns);
        namespace_uses[module][range.begin] = use_ns; }
      else members[module][name.text].push_back(own);
      ref_start = enum_item ? first + 1 : first + 2;
      if (kind == "Instance") {
        size_t target = ref_start;
        while (target < header_end && tokens[target].text != "Of" && tokens[target].text != "of") ++target;
        ++target;
        std::string ns = use_ns;
        if (target < header_end && tokens[target].text == "::") { ns = ""; ++target; }
        else if (target + 1 < header_end && tokens[target + 1].text == "::") {
          ns = tokens[target].text; target += 2;
        }
        if (target < header_end) {
          declarations[own].target_name = tokens[target].text;
          declarations[own].target_namespace = ns;
          NameUse use{{tokens[target].begin, tokens[target].end}, {tokens[target].text}, module};
          use.module_type = true; use.name_space = ns; uses.push_back(std::move(use));
        }
        ref_start = header_end;
      }
    }
    // The parser supplies the node boundaries and syntactic category. These are
    // lexical leaves of that AST, not a second grammar or text-search parser.
    if (kind == "RefEnum" && parent >= 0 && first + 1 < header_end)
      declarations[parent].ref_enum = tokens[first + 1].text;
    const bool signals = reference_body || declaration || SignalProperty(kind) ||
        (parent >= 0 && (declarations[parent].kind == "LogicSignal" ||
                        declarations[parent].kind == "ScanMux" ||
                        declarations[parent].kind == "DataMux" || declarations[parent].kind == "ClockMux"));
    if (reference_body || (parent >= 0 && declarations[parent].kind == "LogicSignal")) ref_start = first;
    for (size_t i = ref_start; i < header_end; ++i) {
      const auto& token = tokens[i];
      const bool param = token.text[0] == '$';
      const bool override_lhs = in_instance && i == first + 1 && (kind == "InputPort" || kind == "Parameter");
      if (!(param || Identifier(token.text)) || (!signals && !param && !override_lhs)) continue;
      if (declaration && (token.text == "SelectedBy" || token.text == "Of" || token.text == "of")) continue;
      size_t name_end = token.text.size();
      if (param) {
        name_end = 1;
        while (name_end < token.text.size()) {
          const char c = token.text[name_end];
          if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_')) break;
          ++name_end;
        }
      }
      NameUse use{{token.begin + (param ? 1 : 0), token.begin + name_end},
                  {param ? token.text.substr(1, name_end - 1) : token.text}, module};
      use.parameter = param;
      if (override_lhs) use.instance = parent;
      if (kind == "DefaultLoadValue" || kind == "ResetValue" || kind == "iApplyEndState") use.enum_owner = parent;
      if (i > first + 1 && (tokens[i - 1].text == "==" || tokens[i - 1].text == "!=") && !uses.empty())
        use.enum_signal = uses.back().path;
      if (i >= 2 && tokens[i - 1].text == ".") {
        std::vector<std::string> prefix;
        size_t j = i;
        while (j >= 2 && tokens[j - 1].text == ".") { j -= 2; prefix.push_back(tokens[j].text); }
        std::reverse(prefix.begin(), prefix.end());
        prefix.push_back(token.text); use.path = std::move(prefix);
      }
      uses.push_back(std::move(use));
    }
    if (block) {
      // Generic AccessLink payload has no child AST; it remains opaque.
      const int child_parent = own >= 0 ? own : parent;
      for (const auto* child : block->body()) walk(child, module, child_parent, use_ns, kind == "ScanInterface" && own < 0);
    }
  };
  std::string use_ns;
  for (const auto* node : driver.unit().top_level()) walk(node, -1, -1, use_ns, false);
  std::stable_sort(uses.begin(), uses.end(), [](const NameUse& a, const NameUse& b) { return a.range.begin < b.range.begin; });
}

void SemanticWorkspace::Reindex() {
  modules_.clear(); uris_.clear();
  for (const auto& item : documents) {
    const auto* doc = item.second.get(); uris_[doc] = item.first;
    for (size_t i = 0; i < doc->declarations.size(); ++i) {
      const auto& d = doc->declarations[i];
      if (d.kind == "Module") modules_[d.name_space + "::" + d.name].push_back({doc, i});
    }
  }
}
std::string SemanticWorkspace::Uri(const ParsedDocument* doc) const {
  const auto it = uris_.find(doc); return it == uris_.end() ? "" : it->second;
}
SymbolRef SemanticWorkspace::Module(const std::string& ns, const std::string& name,
                                   const ParsedDocument* local) const {
  const auto it = modules_.find(ns + "::" + name);
  if (it == modules_.end()) return {};
  // Same-file redefinitions have source order; ambiguous libraries are not guessed.
  for (auto r = it->second.rbegin(); r != it->second.rend(); ++r)
    if (r->document == local) return *r;
  return it->second.size() == 1 ? it->second.front() : SymbolRef{};
}
SymbolRef SemanticWorkspace::Member(SymbolRef module, const std::string& name, bool parameter) const {
  if (!module) return {};
  const auto scope = module.document->members.find(static_cast<int>(module.index));
  if (scope == module.document->members.end()) return {};
  const auto found = scope->second.find(name);
  if (found == scope->second.end()) return {};
  SymbolRef result;
  for (size_t index : found->second) {
    if (module.document->declarations[index].kind == "EnumMember") continue;
    if (parameter && !ParameterKind(module.document->declarations[index].kind)) continue;
    if (result) return {}; // Ambiguous declarations need a diagnostic, not a guessed jump.
    result = {module.document, index};
  }
  return result;
}
SymbolRef SemanticWorkspace::EnumMember(SymbolRef owner, const std::string& name) const {
  if (!owner || owner.declaration().ref_enum.empty() || owner.declaration().module < 0) return {};
  const auto enumeration = Member({owner.document, static_cast<size_t>(owner.declaration().module)}, owner.declaration().ref_enum);
  if (!enumeration || enumeration.declaration().kind != "Enum") return {};
  const auto scope = owner.document->members.find(owner.declaration().module);
  if (scope == owner.document->members.end()) return {};
  const auto found = scope->second.find(name);
  if (found == scope->second.end()) return {};
  for (size_t index : found->second)
    if (owner.document->declarations[index].parent == static_cast<int>(enumeration.index)) return {owner.document, index};
  return {};
}
SymbolRef SemanticWorkspace::Resolve(const ParsedDocument& doc, const NameUse& use) const {
  if (use.declaration >= 0) return {&doc, static_cast<size_t>(use.declaration)};
  if (use.path.empty()) return {};
  if (use.module_type) return Module(use.name_space, use.path.back(), &doc);
  if (use.module < 0) return {};
  SymbolRef scope{&doc, static_cast<size_t>(use.module)};
  if (use.instance >= 0) scope = Implementation({&doc, static_cast<size_t>(use.instance)});
  if (use.enum_owner >= 0) return EnumMember({&doc, static_cast<size_t>(use.enum_owner)}, use.path.back());
  if (!use.enum_signal.empty()) {
    NameUse signal{{}, use.enum_signal, use.module};
    return EnumMember(Resolve(doc, signal), use.path.back());
  }
  SymbolRef result = Member(scope, use.path.front(), use.parameter);
  for (size_t i = 1; result && i < use.path.size(); ++i)
    result = Member(Implementation(result), use.path[i]);
  return result;
}
SymbolRef SemanticWorkspace::Implementation(SymbolRef symbol) const {
  if (!symbol) return {};
  const auto& d = symbol.declaration();
  if (d.kind == "Module") return symbol;
  if (d.kind == "Instance") return Module(d.target_namespace, d.target_name, symbol.document);
  return {};
}
std::vector<SymbolRef> SemanticWorkspace::Complete(const ParsedDocument& doc, size_t offset,
                                                   bool* member_context) const {
  *member_context = false;
  const int module = doc.ModuleAt(offset);
  if (module < 0) return {};
  SymbolRef scope{&doc, static_cast<size_t>(module)};
  const auto& tokens = doc.driver.unit().tokens;
  auto it = std::lower_bound(tokens.begin(), tokens.end(), offset,
      [](const SourceToken& t, size_t p) { return t.begin < p; });
  if (it != tokens.begin()) {
    auto prev = it - 1;
    if (prev->text != "." && prev->begin < offset && offset <= prev->end && prev != tokens.begin()) --prev;
    if (prev->text == "Of" || prev->text == "of" || prev->text == "::") {
      *member_context = true;
      const auto& events = doc.namespace_uses.at(module);
      auto event = events.upper_bound(offset);
      std::string ns = event == events.begin() ? "" : (--event)->second;
      if (prev->text == "::") ns = prev != tokens.begin() && (prev - 1)->text != "Of" && (prev - 1)->text != "of" ? (prev - 1)->text : "";
      std::vector<SymbolRef> result;
      for (const auto& entry : modules_) for (const auto symbol : entry.second)
        if (symbol.declaration().name_space == ns) result.push_back(symbol);
      return result;
    }
    if (prev->text == "." && prev != tokens.begin()) {
      *member_context = true;
      const auto* use = doc.UseAt((prev - 1)->begin);
      if (use) scope = Implementation(Resolve(doc, *use));
      else {
        // A trailing unfinished reference has tokens but no reduced statement yet.
        // Resolve its dot-separated prefix in the parser's current module scope.
        auto name = prev - 1;
        std::vector<std::string> path{name->text};
        while (name - tokens.begin() >= 2 && (name - 1)->text == ".") {
          name -= 2; path.push_back(name->text);
        }
        std::reverse(path.begin(), path.end());
        scope = Implementation(Resolve(doc, NameUse{{}, std::move(path), module}));
      }
    }
  }
  std::vector<SymbolRef> result;
  if (!scope) return result;
  const auto found = scope.document->members.find(static_cast<int>(scope.index));
  if (found != scope.document->members.end())
    for (const auto& entry : found->second) for (size_t index : entry.second) result.push_back({scope.document, index});
  return result;
}
}  // namespace icl
