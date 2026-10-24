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
  static const std::unordered_set<std::string> kKinds = {
      "Module",         "Instance",      "Parameter",       "LocalParameter",
      "Alias",          "Enum",          "ScanRegister",    "DataRegister",
      "LogicSignal",    "ScanMux",       "DataMux",         "ClockMux",
      "ScanInterface",  "Chain",         "OneHotScanGroup", "OneHotDataGroup",
      "PortGroup",      "AccessLink",    "ScanInPort",      "ScanOutPort",
      "DataInPort",     "DataOutPort",   "ClockPort",       "ToClockPort",
      "TCKPort",        "ToTCKPort",     "TMSPort",         "ToTMSPort",
      "TRSTPort",       "ToTRSTPort",    "ResetPort",       "ToResetPort",
      "SelectPort",     "ToSelectPort",  "ToIRSelectPort",  "ShiftEnPort",
      "ToShiftEnPort",  "CaptureEnPort", "ToCaptureEnPort", "UpdateEnPort",
      "ToUpdateEnPort", "AddressPort",   "WriteEnPort",     "ReadEnPort"};
  return kKinds.count(kind);
}
bool SignalProperty(const std::string& kind) {
  static const std::unordered_set<std::string> kKinds = {
      "Source",
      "ScanInSource",
      "CaptureSource",
      "InputPort",
      "ReadDataSource",
      "WriteDataSource",
      "WriteEnSource",
      "Enable",
      "DifferentialInvOf",
      "Port",
      "RefEnum",
      "ActiveSignals",
      "ScanInterface",
      "AllowBroadcastOnScanInterface",
      "DefaultLoadValue",
      "ResetValue",
      "iApplyEndState"};
  return kKinds.count(kind);
}
}  // namespace

const std::vector<std::string>& IclKeywords() {
  // ICL vocabulary only. Context-specific property validity remains the
  // parser's job.
  static const std::vector<std::string> kWords = {
      "Module",
      "NameSpace",
      "UseNameSpace",
      "Parameter",
      "LocalParameter",
      "Attribute",
      "Alias",
      "Enum",
      "Instance",
      "Of",
      "ScanInPort",
      "ScanOutPort",
      "DataInPort",
      "DataOutPort",
      "ScanRegister",
      "DataRegister",
      "ScanMux",
      "DataMux",
      "ClockMux",
      "LogicSignal",
      "ScanInterface",
      "Chain",
      "OneHotScanGroup",
      "OneHotDataGroup",
      "AccessLink",
      "InputPort",
      "Port",
      "SelectedBy",
      "ScanInSource",
      "CaptureSource",
      "Source",
      "ResetValue",
      "DefaultLoadValue",
      "RefEnum",
      "iApplyEndState",
      "AccessTogether",
      "ClockPort",
      "ToClockPort",
      "TCKPort",
      "ToTCKPort",
      "TMSPort",
      "ToTMSPort",
      "TRSTPort",
      "ToTRSTPort",
      "ResetPort",
      "ToResetPort",
      "SelectPort",
      "ToSelectPort",
      "ToIRSelectPort",
      "ShiftEnPort",
      "ToShiftEnPort",
      "CaptureEnPort",
      "ToCaptureEnPort",
      "UpdateEnPort",
      "ToUpdateEnPort",
      "AddressPort",
      "ReadEnPort",
      "WriteEnPort",
      "Enable",
      "DifferentialInvOf",
      "LaunchEdge",
      "ActivePolarity",
      "Period",
      "FreqMultiplier",
      "FreqDivider",
      "AddressValue",
      "ReadCallBack",
      "WriteCallBack",
      "ReadDataSource",
      "WriteDataSource",
      "WriteEnSource",
      "AllowBroadcastOnScanInterface",
      "BSDLEntity",
      "ActiveSignals",
      "STD_1149_1_2001",
      "STD_1149_1_2013",
      "PortGroup"};
  return kWords;
}

ParsedDocument::ParsedDocument(std::string text) : source_(std::move(text)) {
  line_starts_.push_back(0);
  for (size_t i = 0; i < source_.size(); ++i) {
    if (source_[i] == '\n') line_starts_.push_back(i + 1);
  }
  std::string error;
  ok_ = driver_.Parse(source_, &error);
  Build();
}
size_t ParsedDocument::Offset(int line, int column) const {
  if (line < 1) return 0;
  if (static_cast<size_t>(line) > line_starts_.size()) return source_.size();
  const size_t start = line_starts_[line - 1];
  const size_t end = static_cast<size_t>(line) < line_starts_.size()
                         ? line_starts_[line] - 1
                         : source_.size();
  return std::min(end, start + static_cast<size_t>(std::max(1, column) - 1));
}
ByteRange ParsedDocument::Range(SourceSpan span) const {
  return {Offset(span.first_line, span.first_column),
          Offset(span.last_line, span.last_column)};
}
int ParsedDocument::ModuleAt(size_t offset) const {
  for (size_t i = 0; i < declarations_.size(); ++i) {
    const auto& d = declarations_[i];
    if (d.kind == "Module" && d.range.begin <= offset &&
        offset <= d.range.end) {
      return static_cast<int>(i);
    }
  }
  return -1;
}
const NameUse* ParsedDocument::UseAt(size_t offset) const {
  auto it = std::upper_bound(
      uses_.begin(), uses_.end(), offset,
      [](size_t byte, const NameUse& use) { return byte < use.range.begin; });
  if (it != uses_.begin()) {
    --it;
    if (offset < it->range.end) return &*it;
  }
  return nullptr;
}

void ParsedDocument::Build() {
  const auto& tokens = driver_.unit().tokens();
  std::unordered_map<size_t, size_t> enum_signals;
  for (const auto& comparison : driver_.unit().EnumComparisons()) {
    const auto signal = Range(comparison.first);
    enum_signals[Range(comparison.second).begin] =
        signal.end > signal.begin ? signal.end - 1 : 0;
  }
  auto token_index = [&](size_t byte) {
    return static_cast<size_t>(
        std::lower_bound(tokens.begin(), tokens.end(), byte,
                         [](const SourceToken& token, size_t value) {
                           return token.begin < value;
                         }) -
        tokens.begin());
  };
  std::string definition_ns;
  std::optional<std::string> file_use_ns;
  std::function<void(const AstNode*, int, int, std::string&,
                     const std::string&)>
      walk;
  walk = [&](const AstNode* node, int module, int parent, std::string& use_ns,
             const std::string& context) {
    if (dynamic_cast<const AstComment*>(node)) return;
    const auto range = Range(node->span());
    const size_t first = token_index(range.begin),
                 last = token_index(range.end);
    if (first >= tokens.size() || first >= last) return;
    const auto* block = dynamic_cast<const AstBlock*>(node);
    size_t header_end = last;
    if (block) {
      header_end = first;
      while (header_end < last && tokens[header_end].text != "{") ++header_end;
    }
    const std::string parent_kind =
        parent >= 0 ? declarations_[parent].kind : "";
    const bool enum_item = context == "Enum";
    const bool reference_body = context == "ReferenceBody";
    const bool expression = reference_body || context == "LogicSignal";
    const std::string kind = enum_item    ? "EnumMember"
                             : expression ? ""
                             : context == "AccessLink" && block
                                 ? "Instruction"
                                 : tokens[first].text;
    if ((context.empty() || context == "Module") &&
        (kind == "NameSpace" || kind == "UseNameSpace")) {
      const std::string name = first + 1 < last && tokens[first + 1].text != ";"
                                   ? tokens[first + 1].text
                                   : "";
      if (kind == "NameSpace") {
        definition_ns = name;
      } else {
        use_ns = name;
        if (module < 0) {
          file_use_ns = name;
        } else {
          namespace_uses_[module][range.begin] = name;
        }
      }
      return;
    }
    const bool in_instance = parent_kind == "Instance";
    const bool declaration =
        !expression && (enum_item || (context.empty() && kind == "Module") ||
                        (context == "Module" && Declares(kind)) ||
                        (context == "OneHotDataGroup" &&
                         (kind == "Instance" || kind == "DataRegister")) ||
                        (context == "ScanInterface" && kind == "Chain"));
    int own = -1;
    size_t ref_start = first + 1;
    if (declaration && (enum_item || first + 1 < header_end)) {
      const auto& name = tokens[enum_item ? first : first + 1];
      Declaration d;
      d.name = name.text;
      d.kind = kind;
      d.name_range = {name.begin, name.end};
      d.range = range;
      d.module = module;
      d.parent = parent;
      d.name_space = definition_ns;
      d.detail = source_.substr(
          tokens[first].begin,
          (header_end < last ? tokens[header_end].begin : range.end) -
              tokens[first].begin);
      own = static_cast<int>(declarations_.size());
      declarations_.push_back(std::move(d));
      uses_.push_back({{name.begin, name.end}, {name.text}, module, own});
      if (kind == "Module") {
        module = own;
        use_ns = file_use_ns.value_or(definition_ns);
        namespace_uses_[module][range.begin] = use_ns;
      } else {
        members_[module][name.text].push_back(own);
      }
      ref_start = enum_item ? first + 1 : first + 2;
      if (kind == "Instance") {
        size_t target = ref_start;
        while (target < header_end && tokens[target].text != "Of" &&
               tokens[target].text != "of") {
          ++target;
        }
        ++target;
        std::string ns = use_ns;
        if (target < header_end && tokens[target].text == "::") {
          ns = "";
          ++target;
        } else if (target + 1 < header_end && tokens[target + 1].text == "::") {
          ns = tokens[target].text;
          target += 2;
        }
        if (target < header_end) {
          declarations_[own].target_name = tokens[target].text;
          declarations_[own].target_namespace = ns;
          NameUse use{{tokens[target].begin, tokens[target].end},
                      {tokens[target].text},
                      module};
          use.module_type = true;
          use.name_space = ns;
          uses_.push_back(std::move(use));
        }
        ref_start = header_end;
      }
      if (kind == "AccessLink") ref_start = header_end;
    }
    // The parser supplies the node boundaries and syntactic category. These are
    // lexical leaves of that AST, not a second grammar or text-search parser.
    if (kind == "RefEnum" && parent >= 0 && first + 1 < header_end) {
      declarations_[parent].ref_enum = tokens[first + 1].text;
    }
    const bool signals =
        reference_body || declaration || SignalProperty(kind) ||
        (parent >= 0 && (declarations_[parent].kind == "LogicSignal" ||
                         declarations_[parent].kind == "ScanMux" ||
                         declarations_[parent].kind == "DataMux" ||
                         declarations_[parent].kind == "ClockMux"));
    if (reference_body ||
        (parent >= 0 && declarations_[parent].kind == "LogicSignal")) {
      ref_start = first;
    }
    bool selection_keyword =
        declaration &&
        (kind == "ScanMux" || kind == "DataMux" || kind == "ClockMux");
    for (size_t i = ref_start; i < header_end; ++i) {
      const auto& token = tokens[i];
      const bool param = token.text[0] == '$';
      const bool override_lhs =
          in_instance &&
          ((i == first + 1 && (kind == "InputPort" || kind == "Parameter")) ||
           kind == "AllowBroadcastOnScanInterface");
      if (!(param || Identifier(token.text)) ||
          (!signals && !param && !override_lhs)) {
        continue;
      }
      if (selection_keyword && token.text == "SelectedBy") {
        selection_keyword = false;
        continue;
      }
      size_t name_end = token.text.size();
      if (param) {
        name_end = 1;
        while (name_end < token.text.size()) {
          const char c = token.text[name_end];
          if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                (c >= '0' && c <= '9') || c == '_')) {
            break;
          }
          ++name_end;
        }
      }
      NameUse use{{token.begin + (param ? 1 : 0), token.begin + name_end},
                  {param ? token.text.substr(1, name_end - 1) : token.text},
                  module};
      use.parameter = param;
      if (override_lhs) use.instance = parent;
      if (kind == "DefaultLoadValue" || kind == "ResetValue" ||
          kind == "iApplyEndState") {
        use.enum_owner = parent;
      }
      // 6.4.8(t): a bare CaptureSource name prefers RefEnum, then a signal.
      if (kind == "CaptureSource" && i == first + 1 &&
          header_end == first + 3) {
        use.enum_owner = parent;
        use.enum_or_signal = true;
      }
      const auto signal = enum_signals.find(token.begin);
      if (signal != enum_signals.end()) use.enum_signal = signal->second;
      use.path_tokens = {i, i + 1};
      if (i >= 2 && tokens[i - 1].text == "." && !uses_.empty()) {
        use.path_tokens.begin = uses_.back().path_tokens.begin;
      }
      uses_.push_back(std::move(use));
    }
    if (block) {
      // Generic AccessLink payload has no child AST; it remains opaque.
      const int child_parent = own >= 0 ? own : parent;
      const std::string child_context =
          ((kind == "ScanInterface" && own < 0) || kind == "ActiveSignals" ||
           kind == "PortGroup")
              ? "ReferenceBody"
              : kind;
      for (const auto* child : block->body()) {
        walk(child, module, child_parent, use_ns, child_context);
      }
    }
  };
  std::string use_ns;
  for (const auto* node : driver_.unit().top_level()) {
    walk(node, -1, -1, use_ns, "");
  }
  std::stable_sort(uses_.begin(), uses_.end(),
                   [](const NameUse& a, const NameUse& b) {
                     return a.range.begin < b.range.begin;
                   });
}

void SemanticWorkspace::Reindex() {
  modules_.clear();
  uris_.clear();
  for (const auto& item : documents_) {
    const auto* doc = item.second.get();
    uris_[doc] = item.first;
    for (size_t i = 0; i < doc->declarations().size(); ++i) {
      const auto& d = doc->declarations()[i];
      if (d.kind == "Module") {
        modules_[d.name_space + "::" + d.name].push_back({doc, i});
      }
    }
  }
}
std::string SemanticWorkspace::Uri(const ParsedDocument* doc) const {
  const auto it = uris_.find(doc);
  return it == uris_.end() ? "" : it->second;
}
SymbolRef SemanticWorkspace::Module(const std::string& ns,
                                    const std::string& name,
                                    const ParsedDocument* local) const {
  const auto it = modules_.find(ns + "::" + name);
  if (it == modules_.end()) return {};
  // Same-file redefinitions have source order; ambiguous libraries are not
  // guessed.
  for (auto r = it->second.rbegin(); r != it->second.rend(); ++r) {
    if (r->document == local) return *r;
  }
  return it->second.size() == 1 ? it->second.front() : SymbolRef{};
}
SymbolRef SemanticWorkspace::Member(SymbolRef module, const std::string& name,
                                    bool parameter) const {
  if (!module) return {};
  const auto scope =
      module.document->members().find(static_cast<int>(module.index));
  if (scope == module.document->members().end()) return {};
  const auto found = scope->second.find(name);
  if (found == scope->second.end()) return {};
  SymbolRef result;
  for (size_t index : found->second) {
    if (module.document->declarations()[index].kind == "EnumMember") continue;
    if (parameter &&
        !ParameterKind(module.document->declarations()[index].kind)) {
      continue;
    }
    if (result) {
      // Ambiguous declarations need a diagnostic, not a guessed jump.
      return {};
    }
    result = {module.document, index};
  }
  return result;
}
SymbolRef SemanticWorkspace::EnumMember(SymbolRef owner,
                                        const std::string& name) const {
  if (!owner || owner.declaration().ref_enum.empty() ||
      owner.declaration().module < 0) {
    return {};
  }
  const auto enumeration =
      Member({owner.document, static_cast<size_t>(owner.declaration().module)},
             owner.declaration().ref_enum);
  if (!enumeration || enumeration.declaration().kind != "Enum") return {};
  const auto scope = owner.document->members().find(owner.declaration().module);
  if (scope == owner.document->members().end()) return {};
  const auto found = scope->second.find(name);
  if (found == scope->second.end()) return {};
  for (size_t index : found->second) {
    if (owner.document->declarations()[index].parent ==
        static_cast<int>(enumeration.index)) {
      return {owner.document, index};
    }
  }
  return {};
}
SymbolRef SemanticWorkspace::Resolve(const ParsedDocument& doc,
                                     const NameUse& use) const {
  if (use.declaration >= 0) return {&doc, static_cast<size_t>(use.declaration)};
  if (use.name.empty()) return {};
  if (use.module_type) return Module(use.name_space, use.name, &doc);
  if (use.module < 0) return {};
  SymbolRef scope{&doc, static_cast<size_t>(use.module)};
  if (use.instance >= 0) {
    scope = Implementation({&doc, static_cast<size_t>(use.instance)});
  }
  if (use.parameter) return Member(scope, use.name, true);
  if (use.enum_owner >= 0) {
    const auto member =
        EnumMember({&doc, static_cast<size_t>(use.enum_owner)}, use.name);
    if (member || !use.enum_or_signal) return member;
  }
  if (use.enum_signal) {
    const auto* signal = doc.UseAt(*use.enum_signal);
    return signal ? EnumMember(Resolve(doc, *signal), use.name) : SymbolRef{};
  }
  const auto& tokens = doc.driver().unit().tokens();
  const auto path = use.path_tokens;
  SymbolRef result =
      Member(scope, path.end ? tokens[path.begin].text : use.name);
  for (size_t i = path.begin + 2; result && i < path.end; i += 2) {
    result = Member(Implementation(result), tokens[i].text);
  }
  return result;
}
SymbolRef SemanticWorkspace::Implementation(SymbolRef symbol) const {
  if (!symbol) return {};
  const auto& d = symbol.declaration();
  if (d.kind == "Module") return symbol;
  if (d.kind == "Instance") {
    return Module(d.target_namespace, d.target_name, symbol.document);
  }
  return {};
}
std::vector<SymbolRef> SemanticWorkspace::Complete(const ParsedDocument& doc,
                                                   size_t offset,
                                                   bool* member_context) const {
  *member_context = false;
  const int module = doc.ModuleAt(offset);
  if (module < 0) return {};
  SymbolRef scope{&doc, static_cast<size_t>(module)};
  const auto& tokens = doc.driver().unit().tokens();
  auto it = std::lower_bound(
      tokens.begin(), tokens.end(), offset,
      [](const SourceToken& t, size_t p) { return t.begin < p; });
  if (it != tokens.begin()) {
    auto prev = it - 1;
    if (prev->text != "." && prev->text != "::" && prev->begin < offset &&
        offset <= prev->end && prev != tokens.begin()) {
      --prev;
    }
    // Keywords are identifiers too; only the Instance header introduces a
    // type.
    auto is_type_keyword = [&](auto token) {
      return token - tokens.begin() >= 3 && (token - 2)->text == "Instance" &&
             Identifier((token - 1)->text) &&
             (token->text == "Of" || token->text == "of") &&
             ((token - 3)->text == "{" || (token - 3)->text == "}" ||
              (token - 3)->text == ";");
    };
    auto type_keyword = prev;
    std::optional<std::string> explicit_ns;
    if (prev->text == "::") {
      if (prev - tokens.begin() >= 2 && is_type_keyword(prev - 2)) {
        type_keyword = prev - 2;
        explicit_ns = (prev - 1)->text;
      } else if (prev != tokens.begin() && is_type_keyword(prev - 1)) {
        type_keyword = prev - 1;
        explicit_ns = "";
      }
    }
    if (is_type_keyword(type_keyword)) {
      *member_context = true;
      const auto& events = doc.namespace_uses().at(module);
      auto event = events.upper_bound(offset);
      std::string ns = event == events.begin() ? "" : (--event)->second;
      if (explicit_ns) ns = *explicit_ns;
      std::vector<SymbolRef> result;
      for (const auto& entry : modules_) {
        for (const auto symbol : entry.second) {
          if (symbol.declaration().name_space == ns) result.push_back(symbol);
        }
      }
      return result;
    }
    if (prev->text == "." && prev != tokens.begin()) {
      *member_context = true;
      const auto* use = doc.UseAt((prev - 1)->begin);
      if (use) {
        scope = Implementation(Resolve(doc, *use));
      } else {
        // A trailing unfinished reference has tokens but no reduced
        // statement yet. Resolve its dot-separated prefix in the parser's
        // current module scope.
        auto name = prev - 1;
        while (name - tokens.begin() >= 2 && (name - 1)->text == ".") {
          name -= 2;
        }
        NameUse prefix{{}, (prev - 1)->text, module};
        prefix.path_tokens = {static_cast<size_t>(name - tokens.begin()),
                              static_cast<size_t>(prev - tokens.begin())};
        scope = Implementation(Resolve(doc, prefix));
      }
    }
  }
  std::vector<SymbolRef> result;
  if (!scope) return result;
  const auto found =
      scope.document->members().find(static_cast<int>(scope.index));
  if (found != scope.document->members().end()) {
    for (const auto& entry : found->second) {
      for (size_t index : entry.second) {
        result.push_back({scope.document, index});
      }
    }
  }
  return result;
}
}  // namespace icl
