// Copyright 2024 The ICL Format Authors.
//
// SPDX-License-Identifier: MIT
// See LICENSE in the project root for license information.

#include "icl_driver.h"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <iostream>
#include <map>
#include <sstream>
#include <unordered_map>

#include "icl_parser.hh"

// Provided by the generated scanner (see icl_lexer.l).
void IclScanBytes(const char* text, size_t size);
void IclScanEnd();

namespace icl {
namespace {
// Check only literal overflow; parameter widths still require elaboration.
bool FitsWidth(const std::string& digits, char base, size_t width) {
  if (base != 'd') {
    const size_t step = base == 'b' ? 1 : 4;
    size_t bit = 0;
    for (auto it = digits.rbegin(); it != digits.rend(); ++it) {
      if (*it == '_') continue;
      const char ch =
          static_cast<char>(std::tolower(static_cast<unsigned char>(*it)));
      const unsigned digit = ch == 'x'   ? 0
                             : ch <= '9' ? ch - '0'
                                         : ch - 'a' + 10;
      if (digit &&
          (bit >= width || (width - bit < step && digit >> (width - bit)))) {
        return false;
      }
      bit += step;
    }
    return true;
  }
  // A decimal digit needs fewer than four bits; skip obviously generous widths.
  if (width / 4 >= digits.size()) return true;
  std::vector<uint32_t> limbs(1, 0);
  const size_t maximum = width / 32 + (width % 32 != 0);
  for (char ch : digits) {
    if (ch == '_') continue;
    uint64_t carry = static_cast<unsigned>(ch - '0');
    for (auto& limb : limbs) {
      carry += static_cast<uint64_t>(limb) * 10;
      limb = static_cast<uint32_t>(carry);
      carry >>= 32;
    }
    if (carry) limbs.push_back(static_cast<uint32_t>(carry));
    if (limbs.size() > maximum || (limbs.size() == maximum && width % 32 &&
                                   limbs.back() >> (width % 32))) {
      return false;
    }
  }
  return true;
}

// `--ast` printer. A visitor so that adding a node class fails to compile here
// instead of silently printing nothing.
//
// Writes to a stream rather than straight to stdout: the dump is now produced
// by the shared job layer, which has no business doing I/O of its own.
class AstPrinter final : public AstVisitor {
 public:
  explicit AstPrinter(std::ostream* out, int depth = 0)
      : out_(out), depth_(depth) {}

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

ParsedValue IclDriver::NumberLiteral(const std::string& source, int line) {
  ParsedValue value{source};
  std::string text;
  for (size_t i = 0; i < source.size();) {
    if (std::isspace(static_cast<unsigned char>(source[i]))) {
      ++i;
      continue;
    }
    if (source.compare(i, 2, "//") == 0) {
      i = source.find('\n', i) + 1;
    } else if (source.compare(i, 2, "/*") == 0) {
      const auto end = source.find("*/", i + 2);
      if (source.find("/*", i + 2) < end) {
        SetParseDiagnostic(line, "nested block comments are not allowed");
      }
      i = end + 2;
    } else {
      text += source[i++];
    }
  }
  const auto quote = text.find('\'');
  if (quote && text[0] != '$' &&
      text.substr(0, quote).find_first_not_of("0_") == std::string::npos) {
    SetParseDiagnostic(line, "number width must be nonzero");
  }
  value.has_unknown =
      text.find_first_of("xX", text.find('\'') + 2) != std::string::npos;
  value.unsized = text[0] == '\'';
  if (text[0] == '$') {
    value.dependencies.push_back(text.substr(1, text.find('\'') - 1));
  }
  // Parameter widths need elaboration; fixed widths can be checked locally.
  if (text[0] == '$' || text[0] == '\'') return value;
  for (char ch : text) {
    if (ch == '\'') break;
    if (ch == '_') continue;
    const auto digit = static_cast<size_t>(ch - '0');
    if (value.width > (std::numeric_limits<size_t>::max() - digit) / 10) {
      value.width = 0;
      return value;
    }
    value.width = value.width * 10 + digit;
  }
  if (value.width &&
      !FitsWidth(text.substr(quote + 2),
                 static_cast<char>(
                     std::tolower(static_cast<unsigned char>(text[quote + 1]))),
                 value.width)) {
    SetParseDiagnostic(line, "sized number truncates significant bits");
  }
  return value;
}

// Resolve local default types without evaluating values or instantiating
// modules. Kahn's traversal handles forward references and long dependency
// chains in O(V+E).
void IclDriver::CheckNamedValues(const std::vector<AstNode*>& body,
                                 bool module) {
  std::vector<const AstStatement*> parameters;
  std::unordered_map<std::string, size_t> names;
  std::unordered_set<std::string> attributes;
  for (const auto* node : body) {
    const auto* statement = dynamic_cast<const AstStatement*>(node);
    if (!statement) continue;
    if (statement->named_kind() == NamedKind::kAttribute &&
        !attributes.insert(statement->name()).second) {
      SetParseDiagnostic(statement->span().first_line,
                         "duplicate attribute: " + statement->name());
    }
    if (!statement->value()) continue;
    if (statement->named_kind() != NamedKind::kAttribute &&
        !names.emplace(statement->name(), parameters.size()).second) {
      SetParseDiagnostic(statement->span().first_line,
                         "duplicate parameter: " + statement->name());
      continue;
    }
    parameters.push_back(statement);
  }
  const size_t count = parameters.size();
  std::vector<size_t> pending(count);
  std::vector<std::vector<size_t>> users(count);
  std::vector<ValueAtom> types(count, {ValueKind::kUnknown, false, ""});
  std::vector<size_t> ready;
  for (size_t i = 0; i < count; ++i) {
    for (const auto& dep : parameters[i]->value()->dependencies) {
      const auto found = names.find(dep);
      if (module && found != names.end()) {
        ++pending[i];
        users[found->second].push_back(i);
      }
    }
    if (!pending[i]) ready.push_back(i);
  }
  for (size_t next = 0; next < ready.size(); ++next) {
    const size_t i = ready[next];
    const auto& value = *parameters[i]->value();
    auto atoms = value.atoms;
    if (atoms.empty()) {
      atoms.push_back({value.kind, value.unsized, value.reference});
    }
    ValueAtom result{ValueKind::kUnknown, false, ""};
    for (auto atom : atoms) {
      if (!atom.reference.empty()) {
        const auto found = names.find(atom.reference);
        if (module && found != names.end()) atom = types[found->second];
      }
      if (atoms.size() > 1 && atom.unsized) {
        SetParseDiagnostic(parameters[i]->span().first_line,
                           "unsized number in concatenation");
      }
      if (atom.kind != ValueKind::kUnknown) {
        if (result.kind != ValueKind::kUnknown && result.kind != atom.kind) {
          SetParseDiagnostic(parameters[i]->span().first_line,
                             "cannot concatenate strings and numbers");
        }
        result.kind = atom.kind;
      }
      result.unsized = result.unsized || atom.unsized;
    }
    types[i] = result;
    for (size_t user : users[i]) {
      if (!--pending[user]) ready.push_back(user);
    }
  }
  if (ready.size() != count) {
    for (size_t i = 0; i < count; ++i) {
      if (pending[i]) {
        SetParseDiagnostic(parameters[i]->span().first_line,
                           "circular parameter reference");
        break;
      }
    }
  }
}

void IclDriver::CheckBlockItems(const std::string& kind,
                                const std::vector<AstNode*>& body, int line) {
  // true means a singleton. Attribute is repeatable for every kind.
  static const std::map<std::string, std::map<std::string, bool>> kRules = {
      {"ScanInPort", {}},  // §6.4.6.1
      {"ScanOutPort",
       {{"Source", true}, {"Enable", true}, {"LaunchEdge", true}}},  // §6.4.6.2
      {"ShiftEnPort", {}},                                           // §6.4.6.3
      {"CaptureEnPort", {}},                                         // §6.4.6.4
      {"UpdateEnPort", {}},                                          // §6.4.6.5
      {"DataInPort",
       {{"RefEnum", true}, {"DefaultLoadValue", false}}},  // §6.4.6.6
      {"DataOutPort",
       {{"Source", true}, {"Enable", true}, {"RefEnum", true}}},  // §6.4.6.7
      {"ToShiftEnPort", {{"Source", true}}},                      // §6.4.6.8
      {"ToCaptureEnPort", {{"Source", true}}},                    // §6.4.6.9
      {"ToUpdateEnPort", {{"Source", true}}},                     // §6.4.6.10
      {"SelectPort", {}},                                         // §6.4.6.11
      {"ToSelectPort", {{"Source", true}}},                       // §6.4.6.12
      {"ResetPort", {{"ActivePolarity", false}}},                 // §6.4.6.13
      {"ToResetPort",
       {{"Source", true}, {"ActivePolarity", true}}},  // §6.4.6.14
      {"TMSPort", {}},                                 // §6.4.6.15
      {"ToTMSPort", {{"Source", true}}},  // §6.4.6.16; Tessent singleton Source
      {"TCKPort", {}},                    // §6.4.6.17
      {"ToTCKPort", {{"Source", true}}},  // §6.4.6.18; Tessent Source extension
      {"ClockPort", {{"DifferentialInvOf", false}}},  // §6.4.6.19
      {"ToClockPort",
       {{"Source", true},
        {"FreqMultiplier", true},
        {"FreqDivider", true},
        {"DifferentialInvOf", true},
        {"Period", true}}},                // §6.4.6.20
      {"TRSTPort", {}},                    // §6.4.6.21
      {"ToTRSTPort", {{"Source", true}}},  // §6.4.6.22
      {"ToIRSelectPort", {}},              // §6.4.6.23
      {"AddressPort", {}},                 // §6.4.6.24
      {"WriteEnPort", {}},                 // §6.4.6.25
      {"ReadEnPort", {}},                  // §6.4.6.26
      {"Alias",
       {{"RefEnum", true},
        {"AccessTogether", false},
        {"iApplyEndState", false}}},
      {"Instance",
       {{"InputPort", false},
        {"Parameter", false},
        {"AllowBroadcastOnScanInterface", false},
        {"AddressValue", true}}},
      {"ScanInterface", {{"Port", false}, {"DefaultLoadValue", true}}},
      {"Chain", {{"Port", false}, {"DefaultLoadValue", true}}},
      {"ScanRegister",
       {{"ScanInSource", true},
        {"CaptureSource", true},
        {"DefaultLoadValue", true},
        {"ResetValue", true},
        {"RefEnum", true}}},  // §6.4.8
      {"DataRegister",
       {{"WriteDataSource", true},
        {"WriteEnSource", true},
        {"AddressValue", true},
        {"ReadCallBack", true},
        {"ReadDataSource", true},
        {"WriteCallBack", true},
        {"ResetValue", true},
        {"DefaultLoadValue", true},
        {"RefEnum", true}}},  // §6.4.9
  };
  const auto& properties = kRules.at(kind);
  std::unordered_set<std::string> seen;
  bool has_item = false;
  for (const auto* node : body) {
    const auto* statement = dynamic_cast<const AstStatement*>(node);
    if (!statement) continue;
    has_item = true;
    const auto& text = statement->text();
    const std::string keyword = text.substr(0, text.find_first_of(" \t\n;"));
    if (keyword == "Attribute") continue;
    const auto property = properties.find(keyword);
    if (property == properties.end()) {
      SetParseDiagnostic(node->span().first_line,
                         keyword + " is not allowed in " + kind);
    } else if (property->second && !seen.insert(keyword).second) {
      SetParseDiagnostic(node->span().first_line,
                         "duplicate " + keyword + " property");
    }
  }
  if (!has_item && (kind == "ToSelectPort" || kind == "ToResetPort" ||
                    kind == "ToClockPort" || kind == "ToTRSTPort" ||
                    kind == "DataRegister")) {
    SetParseDiagnostic(line, kind + " requires a nonempty body or semicolon");
  }
  if (kind == "ScanInterface") {
    if (!chain_names_.empty() && seen.count("DefaultLoadValue")) {
      SetParseDiagnostic(
          line,
          "DefaultLoadValue belongs inside Chain when chains are declared");
    }
    chain_names_.clear();
  }
  if (kind == "ToClockPort") {
    if ((seen.count("Period") || seen.count("DifferentialInvOf")) &&
        seen.size() > 1) {
      SetParseDiagnostic(
          line, "Period and DifferentialInvOf exclude other clock properties");
    }
    if ((seen.count("FreqMultiplier") || seen.count("FreqDivider")) &&
        !seen.count("Source")) {
      SetParseDiagnostic(line, "clock frequency factors require Source");
    }
  }
  if (kind == "DataRegister") {
    const int types = static_cast<int>(seen.count("WriteDataSource") ||
                                       seen.count("WriteEnSource")) +
                      static_cast<int>(seen.count("AddressValue") != 0) +
                      static_cast<int>(seen.count("ReadCallBack") ||
                                       seen.count("ReadDataSource")) +
                      static_cast<int>(seen.count("WriteCallBack") != 0);
    if (types > 1) {
      SetParseDiagnostic(line,
                         "DataRegister access types are mutually exclusive");
    }
  }
}

void IclDriver::CheckAddressContext(const std::string& header,
                                    const std::vector<AstNode*>& body) {
  const bool one_hot = header.rfind("OneHotDataGroup ", 0) == 0;
  for (const auto* node : body) {
    const auto* reg = dynamic_cast<const AstBlock*>(node);
    if (!reg) continue;
    const bool data_register = reg->header().rfind("DataRegister ", 0) == 0;
    const bool instance = reg->header().rfind("Instance ", 0) == 0;
    if (!data_register && !instance) continue;
    for (const auto* item : reg->body()) {
      const auto* property = dynamic_cast<const AstStatement*>(item);
      if (!property) continue;
      const auto& text = property->text();
      const auto keyword = text.substr(0, text.find_first_of(" \t\n;"));
      const bool addressed = keyword == "AddressValue";
      const bool other_type =
          keyword == "WriteDataSource" || keyword == "WriteEnSource" ||
          keyword == "ReadCallBack" || keyword == "ReadDataSource" ||
          keyword == "WriteCallBack";
      if ((addressed && !one_hot) || (data_register && other_type && one_hot)) {
        SetParseDiagnostic(
            item->span().first_line,
            "addressable DataRegisters/Instances belong only in "
            "OneHotDataGroup; other register access types belong outside it");
      }
    }
  }
}

bool IclDriver::Parse(const std::string& text, std::string* error) {
  // Scan straight from memory via flex's buffer API, so no temporary file is
  // ever created and the input buffer is never written to.
  const auto nul = text.find('\0');
  if (nul != std::string::npos) {
    const int line = 1 + static_cast<int>(std::count(text.begin(),
                                                     text.begin() + nul, '\n'));
    const auto newline = text.rfind('\n', nul);
    const int column =
        static_cast<int>(nul -
                         (newline == std::string::npos ? 0 : newline + 1)) +
        1;
    SetParseDiagnostic(SourceSpan{line, line, column, column + 1},
                       "NUL byte is not valid ICL source");
  } else {
    IclScanBytes(text.data(), text.size());

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
    IclScanEnd();
  }

  if (pending_module_) {
    auto span = pending_module_->span();
    span.last_line =
        1 + static_cast<int>(std::count(text.begin(), text.end(), '\n'));
    const auto last = text.rfind('\n');
    span.last_column =
        static_cast<int>(text.size() -
                         (last == std::string::npos ? 0 : last + 1)) +
        1;
    pending_module_->set_span(span);
  }
  if (has_error_) {
    *error = "line " + std::to_string(error_line()) + ": " + error_message_;
    return false;
  }
  return true;
}

void IclDriver::RecordMatch(const char* text, size_t size, bool icl) {
  const size_t begin = scan_offset_;
  scan_offset_ += size;
  if (!icl || !size || text[0] == ' ' || text[0] == '\t' || text[0] == '\r' ||
      text[0] == '\n' ||
      (size >= 2 && text[0] == '/' && (text[1] == '/' || text[1] == '*')) ||
      (size == 3 && static_cast<unsigned char>(text[0]) == 0xEF)) {
    return;
  }
  unit_.tokens().push_back({std::string(text, size), begin, scan_offset_});
}

std::string IclDriver::Render(int indent_width) const {
  FormatStyle style;
  style.indent_width = indent_width;
  return RenderFormatTree(BuildFormatTree(unit_, style), indent_width);
}

std::string IclDriver::AstToString() const {
  std::ostringstream out;
  AstPrinter printer(&out);
  unit_.Accept(printer);
  return out.str();
}

}  // namespace icl
