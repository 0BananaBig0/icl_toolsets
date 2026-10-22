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

#include <map>

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

ParsedValue IclDriver::NumberLiteral(const std::string& text) const {
  ParsedValue value{text};
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
  return value;
}

void IclDriver::CheckBlockItems(const std::string& kind,
                               const std::vector<AstNode*>& body, int line) {
  // true means a singleton. Attribute is repeatable for every kind.
  static const std::map<std::string, std::map<std::string, bool>> rules = {
      {"ScanInPort", {}},  // §6.4.6.1
      {"ScanOutPort", {{"Source", true}, {"Enable", true}, {"LaunchEdge", true}}},  // §6.4.6.2
      {"ShiftEnPort", {}},  // §6.4.6.3
      {"CaptureEnPort", {}},  // §6.4.6.4
      {"UpdateEnPort", {}},  // §6.4.6.5
      {"DataInPort", {{"RefEnum", true}, {"DefaultLoadValue", false}}},  // §6.4.6.6
      {"DataOutPort", {{"Source", true}, {"Enable", true}, {"RefEnum", true}}},  // §6.4.6.7
      {"ToShiftEnPort", {{"Source", true}}},  // §6.4.6.8
      {"ToCaptureEnPort", {{"Source", true}}},  // §6.4.6.9
      {"ToUpdateEnPort", {{"Source", true}}},  // §6.4.6.10
      {"SelectPort", {}},  // §6.4.6.11
      {"ToSelectPort", {{"Source", true}}},  // §6.4.6.12
      {"ResetPort", {{"ActivePolarity", false}}},  // §6.4.6.13
      {"ToResetPort", {{"Source", true}, {"ActivePolarity", true}}},  // §6.4.6.14
      {"TMSPort", {}},  // §6.4.6.15
      {"ToTMSPort", {{"Source", true}}},  // §6.4.6.16; Tessent singleton Source
      {"TCKPort", {}},  // §6.4.6.17
      {"ToTCKPort", {{"Source", true}}},  // §6.4.6.18; Tessent Source extension
      {"ClockPort", {{"DifferentialInvOf", false}}},  // §6.4.6.19
      {"ToClockPort", {{"Source", true}, {"FreqMultiplier", true}, {"FreqDivider", true}, {"DifferentialInvOf", true}, {"Period", true}}},  // §6.4.6.20
      {"TRSTPort", {}},  // §6.4.6.21
      {"ToTRSTPort", {{"Source", true}}},  // §6.4.6.22
      {"ToIRSelectPort", {}},  // §6.4.6.23
      {"AddressPort", {}},  // §6.4.6.24
      {"WriteEnPort", {}},  // §6.4.6.25
      {"ReadEnPort", {}},  // §6.4.6.26
      {"ScanRegister", {{"ScanInSource", true}, {"CaptureSource", true},
                        {"DefaultLoadValue", true}, {"ResetValue", true},
                        {"RefEnum", true}}},  // §6.4.8
      {"DataRegister", {{"WriteDataSource", true}, {"WriteEnSource", true},
                        {"AddressValue", true}, {"ReadCallBack", true},
                        {"ReadDataSource", true}, {"WriteCallBack", true},
                        {"ResetValue", true}, {"DefaultLoadValue", true},
                        {"RefEnum", true}}},  // §6.4.9
  };
  const auto& properties = rules.at(kind);
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
      SetParseDiagnostic(node->span().first_line, keyword + " is not allowed in " + kind);
    } else if (property->second && !seen.insert(keyword).second) {
      SetParseDiagnostic(node->span().first_line, "duplicate " + keyword + " property");
    }
  }
  if (!has_item && (kind == "ToSelectPort" || kind == "ToResetPort" ||
                    kind == "ToClockPort" || kind == "ToTRSTPort" || kind == "DataRegister")) {
    SetParseDiagnostic(line, kind + " requires a nonempty body or semicolon");
  }
  if (kind == "ToClockPort") {
    if ((seen.count("Period") || seen.count("DifferentialInvOf")) && seen.size() > 1) {
      SetParseDiagnostic(line, "Period and DifferentialInvOf exclude other clock properties");
    }
    if ((seen.count("FreqMultiplier") || seen.count("FreqDivider")) && !seen.count("Source")) {
      SetParseDiagnostic(line, "clock frequency factors require Source");
    }
  }
  if (kind == "DataRegister") {
    const int types = static_cast<int>(seen.count("WriteDataSource") || seen.count("WriteEnSource")) +
                      static_cast<int>(seen.count("AddressValue") != 0) +
                      static_cast<int>(seen.count("ReadCallBack") || seen.count("ReadDataSource")) +
                      static_cast<int>(seen.count("WriteCallBack") != 0);
    if (types > 1) SetParseDiagnostic(line, "DataRegister access types are mutually exclusive");
  }
}

void IclDriver::CheckDataRegisterContext(const std::string& header,
                                        const std::vector<AstNode*>& body) {
  const bool one_hot = header.rfind("OneHotDataGroup ", 0) == 0;
  for (const auto* node : body) {
    const auto* reg = dynamic_cast<const AstBlock*>(node);
    if (!reg || reg->header().rfind("DataRegister ", 0) != 0) continue;
    for (const auto* item : reg->body()) {
      const auto* property = dynamic_cast<const AstStatement*>(item);
      if (!property) continue;
      const auto& text = property->text();
      const auto keyword = text.substr(0, text.find_first_of(" \t\n;"));
      const bool addressed = keyword == "AddressValue";
      const bool other_type = keyword == "WriteDataSource" || keyword == "WriteEnSource" ||
                              keyword == "ReadCallBack" || keyword == "ReadDataSource" ||
                              keyword == "WriteCallBack";
      if ((addressed && !one_hot) || (other_type && one_hot)) {
        SetParseDiagnostic(item->span().first_line,
                           "addressable DataRegisters belong only in OneHotDataGroup; other access types belong outside it");
      }
    }
  }
}

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
