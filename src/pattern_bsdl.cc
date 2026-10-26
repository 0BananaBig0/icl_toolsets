// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include "pattern_bsdl.h"

#include <algorithm>
#include <stdexcept>
#include <string_view>
#include <utility>

namespace pattern {
namespace {
constexpr size_t kMaxSourceBytes = 16 * 1024 * 1024;
constexpr size_t kMaxValueBytes = 4 * 1024 * 1024;
constexpr size_t kMaxWidth = 1024 * 1024;
constexpr size_t kMaxTokens = 1024 * 1024;
bool Letter(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }
bool Digit(char c) { return c >= '0' && c <= '9'; }
bool Space(char c) {
  return c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\f' ||
         c == '\v';
}
std::string Lower(std::string_view value) {
  std::string result(value);
  for (char& c : result) {
    if (c >= 'A' && c <= 'Z') c += 'a' - 'A';
  }
  return result;
}
class Context {
 public:
  Context(const std::string& path, const std::string& source) : path_(path) {
    lines_.push_back(0);
    if (source.size() > kMaxSourceBytes) {
      Fail(0, "BSDL source size limit exceeded");
    }
    for (size_t i = 0; i < source.size(); ++i) {
      if (source[i] == '\n') lines_.push_back(i + 1);
    }
  }
  std::string Location(size_t byte) const {
    const auto line = std::upper_bound(lines_.begin(), lines_.end(), byte);
    return path_ + ":" + std::to_string(line - lines_.begin()) + ":" +
           std::to_string(byte - *(line - 1) + 1);
  }
  [[noreturn]] void Fail(size_t byte, const std::string& message) const {
    throw std::runtime_error(Location(byte) + ": " + message);
  }

 private:
  const std::string& path_;
  std::vector<size_t> lines_;
};
enum class Kind { kWord, kNumber, kString, kCharacter, kSymbol, kEnd };
struct Token {
  Kind kind;
  std::string_view text;
  size_t begin;
  bool Is(std::string_view value) const {
    return (kind == Kind::kWord || kind == Kind::kSymbol) &&
           Lower(text) == value;
  }
};
std::vector<Token> Lex(const Context& context, const std::string& text,
                       std::optional<size_t> origin = std::nullopt) {
  std::vector<Token> tokens;
  const auto fail = [&](size_t at, const std::string& message) {
    context.Fail(origin.value_or(at), message);
  };
  size_t pos = text.compare(0, 3, "\xef\xbb\xbf") == 0 ? 3 : 0;
  while (pos < text.size()) {
    if (Space(text[pos])) {
      ++pos;
      continue;
    }
    if (text[pos] == '-' && pos + 1 < text.size() && text[pos + 1] == '-') {
      while (pos < text.size() && text[pos] != '\n') ++pos;
      continue;
    }
    const size_t begin = pos++;
    Kind kind = Kind::kSymbol;
    if (text[begin] == '"') {
      kind = Kind::kString;
      bool closed = false;
      while (pos < text.size()) {
        if (text[pos] == '\n' || text[pos] == '\r' || text[pos] == '\0') {
          fail(begin, "invalid newline or NUL in BSDL string");
        }
        if (text[pos++] != '"') continue;
        if (pos < text.size() && text[pos] == '"') {
          ++pos;
          continue;
        }
        closed = true;
        break;
      }
      if (!closed) fail(begin, "unterminated BSDL string");
    } else if (text[begin] == '\'' && pos + 1 < text.size() &&
               text[pos + 1] == '\'') {
      kind = Kind::kCharacter;
      pos += 2;
    } else if (Letter(text[begin])) {
      kind = Kind::kWord;
      while (pos < text.size() &&
             (Letter(text[pos]) || Digit(text[pos]) || text[pos] == '_')) {
        ++pos;
      }
    } else if (Digit(text[begin])) {
      kind = Kind::kNumber;
      while (pos < text.size() &&
             (Letter(text[pos]) || Digit(text[pos]) || text[pos] == '.' ||
              text[pos] == '_' || text[pos] == '#' ||
              ((text[pos] == '+' || text[pos] == '-') &&
               (text[pos - 1] == 'e' || text[pos - 1] == 'E')))) {
        ++pos;
      }
    } else if (text[begin] == ':' && pos < text.size() && text[pos] == '=') {
      ++pos;
    } else if (static_cast<unsigned char>(text[begin]) < 32 ||
               static_cast<unsigned char>(text[begin]) >= 127) {
      fail(begin, "unsupported BSDL character");
    }
    if (tokens.size() >= kMaxTokens) fail(begin, "BSDL token limit exceeded");
    tokens.push_back(
        {kind, std::string_view(text).substr(begin, pos - begin), begin});
  }
  tokens.push_back({Kind::kEnd, {}, text.size()});
  return tokens;
}
struct Span {
  size_t begin, end;
};
struct Port {
  std::string name, mode;
  bool scalar;
};
struct Attribute {
  std::string name, target, target_class;
  Span value;
  size_t origin;
};
class Reader {
 public:
  Reader(const std::string& path, const std::string& text)
      : context_(path, text), text_(text), tokens_(Lex(context_, text)) {}
  BsdlTap Read(const std::string& entity) {
    size_t start = tokens_.size();
    for (size_t i = 0; i + 2 < tokens_.size(); ++i) {
      if (tokens_[i].Is("entity") && tokens_[i + 1].kind == Kind::kWord &&
          tokens_[i + 2].Is("is") &&
          Lower(tokens_[i + 1].text) == Lower(entity)) {
        if (start != tokens_.size()) Fail(i, "duplicate BSDL entity " + entity);
        start = i;
      }
    }
    if (start == tokens_.size()) Fail(0, "unknown BSDL entity " + entity);
    tap_.entity = Identifier(start + 1);
    tap_.origin = context_.Location(tokens_[start].begin);
    size_t pos = start + 3;
    while (!tokens_[pos].Is("end")) {
      const auto statement = Statement(pos);
      if (tokens_[pos].Is("port")) Ports(statement);
      if (tokens_[pos].Is("constant")) Constant(statement);
      if (tokens_[pos].Is("attribute")) AttributeDeclaration(statement);
      pos = statement.end + 1;
    }
    ++pos;
    if (tokens_[pos].Is("entity")) ++pos;
    if (tokens_[pos].kind == Kind::kWord) {
      if (Lower(tokens_[pos].text) != Lower(tap_.entity)) {
        Fail(pos, "mismatched BSDL end entity");
      }
      ++pos;
    }
    Expect(pos, ";");
    Build();
    return std::move(tap_);
  }

 private:
  [[noreturn]] void Fail(size_t token, const std::string& message) const {
    context_.Fail(tokens_.at(std::min(token, tokens_.size() - 1)).begin,
                  message);
  }
  void Expect(size_t token, const std::string& value) const {
    if (token >= tokens_.size() || !tokens_[token].Is(value)) {
      Fail(token, "expected BSDL '" + value + "'");
    }
  }
  std::string Identifier(size_t token) const {
    const auto& t = tokens_.at(token);
    if (t.kind != Kind::kWord || t.text.back() == '_' ||
        t.text.find("__") != std::string_view::npos) {
      Fail(token, "expected basic BSDL identifier");
    }
    return std::string(t.text);
  }
  Span Statement(size_t begin) const {
    size_t depth = 0;
    for (size_t i = begin; i < tokens_.size(); ++i) {
      if (tokens_[i].kind == Kind::kEnd) {
        Fail(i, "unterminated BSDL declaration/entity");
      }
      if (tokens_[i].Is("(")) {
        if (++depth > 128) Fail(i, "BSDL nesting limit exceeded");
      } else if (tokens_[i].Is(")")) {
        if (!depth) Fail(i, "unmatched BSDL parenthesis");
        --depth;
      } else if (!depth && tokens_[i].Is(";")) {
        return {begin, i};
      }
    }
    Fail(begin, "unterminated BSDL declaration");
  }
  void Ports(Span statement) {
    Expect(statement.begin + 1, "(");
    Expect(statement.end - 1, ")");
    size_t pos = statement.begin + 2;
    while (pos < statement.end - 1) {
      std::vector<std::string> names;
      do {
        names.push_back(Identifier(pos++));
        if (!tokens_[pos].Is(",")) break;
        ++pos;
      } while (pos < statement.end);
      Expect(pos++, ":");
      const auto mode = Lower(Identifier(pos++));
      if (mode != "in" && mode != "out" && mode != "inout" &&
          mode != "buffer" && mode != "linkage") {
        Fail(pos - 1, "unsupported BSDL port mode");
      }
      const bool scalar = tokens_[pos].Is("bit") && (tokens_[pos + 1].Is(";") ||
                                                     tokens_[pos + 1].Is(")") ||
                                                     tokens_[pos + 1].Is(":="));
      Identifier(pos++);
      for (const auto& name : names) {
        if (!ports_.emplace(Lower(name), Port{name, mode, scalar}).second) {
          Fail(pos, "duplicate BSDL port " + name);
        }
      }
      size_t depth = 0;
      while (pos < statement.end - 1 && (!tokens_[pos].Is(";") || depth)) {
        if (tokens_[pos].Is("(")) ++depth;
        if (tokens_[pos].Is(")")) --depth;
        ++pos;
      }
      if (tokens_[pos].Is(";")) ++pos;
    }
  }
  void Constant(Span statement) {
    const auto name = Lower(Identifier(statement.begin + 1));
    Expect(statement.begin + 2, ":");
    size_t pos = statement.begin + 3;
    while (pos < statement.end && !tokens_[pos].Is(":=")) ++pos;
    if (pos == statement.end) {
      Fail(pos, "BSDL constant requires an initializer");
    }
    if (!constants_.emplace(name, Span{pos + 1, statement.end}).second) {
      Fail(pos, "duplicate BSDL constant " + name);
    }
  }
  void AttributeDeclaration(Span statement) {
    const auto name = Lower(Identifier(statement.begin + 1));
    size_t pos = statement.begin + 2;
    if (tokens_[pos].Is(":")) {
      return;  // Attribute type declaration, not a value.
    }
    Expect(pos++, "of");
    std::vector<std::string> targets;
    do {
      targets.push_back(Lower(Identifier(pos++)));
      if (!tokens_[pos].Is(",")) break;
      ++pos;
    } while (pos < statement.end);
    Expect(pos++, ":");
    const auto target_class = Lower(Identifier(pos++));
    Expect(pos++, "is");
    if (pos == statement.end) Fail(pos, "empty BSDL attribute expression");
    const size_t expression_size =
        tokens_[statement.end].begin - tokens_[pos].begin;
    const auto origin = context_.Location(tokens_[statement.begin].begin);
    for (const auto& target : targets) {
      const size_t size = expression_size + name.size() + target.size() +
                          target_class.size() + origin.size();
      Retain(size, statement.begin);
      const auto key = std::make_pair(name, target);
      if (!attributes_
               .emplace(key, Attribute{name,
                                       target,
                                       target_class,
                                       {pos, statement.end},
                                       statement.begin})
               .second) {
        Fail(statement.begin, "duplicate BSDL attribute " + name);
      }
      tap_.attributes.emplace(
          key, BsdlAttribute{target_class,
                             text_.substr(tokens_[pos].begin, expression_size),
                             origin});
    }
  }
  void Retain(size_t bytes, size_t at) {
    if (bytes > kMaxValueBytes - retained_) {
      Fail(at, "BSDL retained-metadata limit exceeded");
    }
    retained_ += bytes;
  }
  const Attribute& EntityAttribute(const std::string& name) const {
    const auto found = attributes_.find({name, Lower(tap_.entity)});
    if (found == attributes_.end()) Fail(0, "missing BSDL attribute " + name);
    if (found->second.target_class != "entity") {
      Fail(found->second.origin,
           "BSDL attribute requires entity target: " + name);
    }
    return found->second;
  }
  bool Has(const std::string& name) const {
    return attributes_.count({name, Lower(tap_.entity)}) != 0;
  }
  size_t Integer(Span span, size_t depth = 0) const {
    if (depth > 128) Fail(span.begin, "BSDL constant nesting limit exceeded");
    if (span.end != span.begin + 1) {
      Fail(span.begin, "unsupported BSDL integer expression");
    }
    const auto& token = tokens_[span.begin];
    if (token.kind == Kind::kWord) {
      const auto found = constants_.find(Lower(token.text));
      if (found == constants_.end()) {
        Fail(span.begin, "unresolved BSDL integer constant");
      }
      return Integer(found->second, depth + 1);
    }
    if (token.kind != Kind::kNumber) Fail(span.begin, "expected BSDL integer");
    size_t result = 0;
    for (size_t i = 0; i < token.text.size(); ++i) {
      const char c = token.text[i];
      if (c == '_' && i && i + 1 < token.text.size() &&
          Digit(token.text[i - 1]) && Digit(token.text[i + 1])) {
        continue;
      }
      if (!Digit(c)) Fail(span.begin, "unsupported BSDL integer literal");
      if (result > (kMaxWidth - (c - '0')) / 10) {
        Fail(span.begin, "BSDL width limit exceeded");
      }
      result = result * 10 + c - '0';
    }
    return result;
  }
  void Append(std::string* result, std::string_view part, size_t at) {
    if (part.size() > kMaxValueBytes - decoded_) {
      Fail(at, "BSDL decoded-value limit exceeded");
    }
    decoded_ += part.size();
    result->append(part);
  }
  std::string String(Span span, size_t depth = 0) {
    if (depth > 128) Fail(span.begin, "BSDL constant nesting limit exceeded");
    std::string result;
    bool atom = true;
    for (size_t i = span.begin; i < span.end; ++i) {
      if (!atom) {
        Expect(i, "&");
        atom = true;
        continue;
      }
      const auto& token = tokens_[i];
      if (token.kind == Kind::kString) {
        // VHDL doubles quotes; a backslash is an ordinary character.
        const auto literal = token.text.substr(1, token.text.size() - 2);
        for (size_t begin = 0; begin < literal.size();) {
          const auto quote = literal.find('"', begin);
          const auto end =
              quote == std::string_view::npos ? literal.size() : quote + 1;
          Append(&result, literal.substr(begin, end - begin), i);
          begin = quote == std::string_view::npos ? end : end + 1;
        }
      } else if (token.kind == Kind::kWord) {
        const auto name = Lower(token.text);
        auto cached = strings_.find(name);
        if (cached == strings_.end()) {
          const auto found = constants_.find(name);
          if (found == constants_.end()) {
            Fail(i, "unresolved BSDL string constant " + name);
          }
          const auto value = String(found->second, depth + 1);
          cached = strings_.emplace(name, value).first;
        }
        Append(&result, cached->second, i);
      } else if (token.Is("(")) {
        size_t end = i + 1, level = 1;
        for (; end < span.end && level; ++end) {
          if (tokens_[end].Is("(")) ++level;
          if (tokens_[end].Is(")")) --level;
        }
        if (level) Fail(i, "unclosed BSDL string expression");
        const auto value = String({i + 1, end - 1}, depth + 1);
        Append(&result, value, i);
        i = end - 1;
      } else {
        Fail(i, "unsupported BSDL string expression");
      }
      atom = false;
    }
    if (atom) Fail(span.begin, "incomplete BSDL string expression");
    return result;
  }
  std::string Bits(const Attribute& attribute, size_t width) {
    auto value = Lower(String(attribute.value));
    if (value.size() != width ||
        value.find_first_not_of("01x") != std::string::npos) {
      Fail(attribute.origin, "BSDL bit pattern width/characters do not match");
    }
    return value;
  }
  void OpcodeTable(const Attribute& attribute) {
    const auto value = String(attribute.value);
    const auto table = Lex(context_, value, tokens_[attribute.origin].begin);
    size_t pos = 0;
    const auto fail = [&] {
      Fail(attribute.origin, "invalid BSDL instruction opcode table");
    };
    while (table[pos].kind != Kind::kEnd) {
      if (table[pos].kind != Kind::kWord) fail();
      const auto name = std::string(table[pos++].text);
      if (!table[pos++].Is("(")) fail();
      std::vector<std::string> codes;
      do {
        if (table[pos].kind != Kind::kWord &&
            table[pos].kind != Kind::kNumber) {
          fail();
        }
        const auto bits = Lower(table[pos++].text);
        if (bits.size() != tap_.instruction_width ||
            bits.find_first_not_of("01x") != std::string::npos) {
          fail();
        }
        codes.push_back(bits);
        if (!table[pos].Is(",")) break;
        ++pos;
      } while (pos < table.size());
      if (!table[pos++].Is(")") ||
          !tap_.opcodes.emplace(name, std::move(codes)).second) {
        fail();
      }
      if (table[pos].kind == Kind::kEnd) break;
      if (!table[pos++].Is(",") || table[pos].kind == Kind::kEnd) fail();
    }
    if (tap_.opcodes.empty()) fail();
  }
  void RegisterTable(const Attribute& attribute) {
    const auto value = String(attribute.value);
    const auto table = Lex(context_, value, tokens_[attribute.origin].begin);
    size_t pos = 0;
    const auto fail = [&] {
      Fail(attribute.origin, "invalid BSDL register access table");
    };
    while (table[pos].kind != Kind::kEnd) {
      if (table[pos].kind != Kind::kWord) fail();
      BsdlRegister reg{std::string(table[pos++].text), std::nullopt};
      if (table[pos].Is("[")) {
        ++pos;
        size_t width = 0;
        if (table[pos].kind != Kind::kNumber) fail();
        for (char c : table[pos++].text) {
          if (!Digit(c) || width > (kMaxWidth - (c - '0')) / 10) fail();
          width = width * 10 + c - '0';
        }
        if (!width || !table[pos++].Is("]")) fail();
        reg.width = width;
      }
      if (!table[pos++].Is("(")) fail();
      do {
        if (table[pos].kind != Kind::kWord) fail();
        const auto instruction = std::string(table[pos++].text);
        Retain(instruction.size() + reg.name.size(), attribute.origin);
        if (!tap_.opcodes.count(instruction) ||
            !tap_.registers.emplace(instruction, reg).second) {
          fail();
        }
        if (!table[pos].Is(",")) break;
        ++pos;
      } while (pos < table.size());
      if (!table[pos++].Is(")")) fail();
      if (table[pos].kind == Kind::kEnd) break;
      if (!table[pos++].Is(",") || table[pos].kind == Kind::kEnd) fail();
    }
    if (tap_.registers.empty()) fail();
  }
  void Build() {
    const auto& width = EntityAttribute("instruction_length");
    tap_.instruction_width = Integer(width.value);
    if (tap_.instruction_width < 2) {
      Fail(width.origin, "BSDL instruction width must be at least two");
    }
    OpcodeTable(EntityAttribute("instruction_opcode"));
    tap_.capture =
        Bits(EntityAttribute("instruction_capture"), tap_.instruction_width);
    if (Has("idcode_register")) {
      tap_.idcode = Bits(EntityAttribute("idcode_register"), 32);
    }
    if (Has("usercode_register")) {
      tap_.usercode = Bits(EntityAttribute("usercode_register"), 32);
    }
    if (Has("boundary_length")) {
      tap_.boundary_width = Integer(EntityAttribute("boundary_length").value);
    }
    if (Has("register_access")) {
      RegisterTable(EntityAttribute("register_access"));
    }
    for (const auto& entry : attributes_) {
      const auto& attr = entry.second;
      if (attr.name != "tap_scan_in" && attr.name != "tap_scan_out" &&
          attr.name != "tap_scan_mode" && attr.name != "tap_scan_clock" &&
          attr.name != "tap_scan_reset") {
        continue;
      }
      const auto port = ports_.find(attr.target);
      if (attr.target_class != "signal" || port == ports_.end() ||
          !port->second.scalar) {
        Fail(attr.origin, "TAP attribute requires a declared scalar bit port");
      }
      const bool output = attr.name == "tap_scan_out";
      if (output ? (port->second.mode != "out" && port->second.mode != "buffer")
                 : port->second.mode != "in") {
        Fail(attr.origin, "TAP port direction mismatch");
      }
      if (attr.name != "tap_scan_clock" &&
          (attr.value.end != attr.value.begin + 1 ||
           !tokens_[attr.value.begin].Is("true"))) {
        Fail(attr.origin, "TAP role must be true");
      }
      if (!tap_.ports.emplace(attr.name, port->second.name).second) {
        Fail(attr.origin, "duplicate TAP port role");
      }
    }
    for (const auto* role :
         {"tap_scan_in", "tap_scan_out", "tap_scan_mode", "tap_scan_clock"}) {
      if (!tap_.ports.count(role)) Fail(0, "missing BSDL " + std::string(role));
    }
  }
  Context context_;
  const std::string& text_;
  std::vector<Token> tokens_;
  BsdlTap tap_;
  std::map<std::string, Port> ports_;
  std::map<std::pair<std::string, std::string>, Attribute> attributes_;
  std::map<std::string, Span> constants_;
  std::map<std::string, std::string> strings_;
  size_t decoded_ = 0;
  size_t retained_ = 0;
};
}  // namespace
BsdlTap ReadBsdlTap(const std::string& path, const std::string& text,
                    const std::string& entity) {
  return Reader(path, text).Read(entity);
}
}  // namespace pattern
