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

#include "json.h"

#include <cstdint>
#include <cmath>
#include <limits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

namespace icl {
namespace {

// Appends |code_point| as UTF-8. LSP escapes non-ASCII as \uXXXX, including
// surrogate pairs, so the decoder has to recombine them.
void AppendUtf8(std::string* out, uint32_t code_point) {
  if (code_point < 0x80) {
    out->push_back(static_cast<char>(code_point));
  } else if (code_point < 0x800) {
    out->push_back(static_cast<char>(0xC0 | (code_point >> 6)));
    out->push_back(static_cast<char>(0x80 | (code_point & 0x3F)));
  } else if (code_point < 0x10000) {
    out->push_back(static_cast<char>(0xE0 | (code_point >> 12)));
    out->push_back(static_cast<char>(0x80 | ((code_point >> 6) & 0x3F)));
    out->push_back(static_cast<char>(0x80 | (code_point & 0x3F)));
  } else {
    out->push_back(static_cast<char>(0xF0 | (code_point >> 18)));
    out->push_back(static_cast<char>(0x80 | ((code_point >> 12) & 0x3F)));
    out->push_back(static_cast<char>(0x80 | ((code_point >> 6) & 0x3F)));
    out->push_back(static_cast<char>(0x80 | (code_point & 0x3F)));
  }
}

class Parser {
 public:
  explicit Parser(const std::string& text) : text_(text) {}

  bool ParseDocument(Json* out, std::string* error) {
    SkipWhitespace();
    if (!ParseValue(out)) return Fail("invalid value", error);
    SkipWhitespace();
    if (pos_ != text_.size()) return Fail("trailing content", error);
    return true;
  }

 private:
  bool Fail(const char* what, std::string* error) {
    char buffer[64];
    std::snprintf(buffer, sizeof(buffer), "%s at byte %zu", what, pos_);
    *error = buffer;
    return false;
  }

  void SkipWhitespace() {
    while (pos_ < text_.size()) {
      const char c = text_[pos_];
      if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
        ++pos_;
      } else {
        break;
      }
    }
  }

  bool Peek(char c) const {
    return pos_ < text_.size() && text_[pos_] == c;
  }

  bool Expect(char c) {
    if (!Peek(c)) return false;
    ++pos_;
    return true;
  }

  bool ParseValue(Json* out) {
    if (pos_ >= text_.size() || depth_ >= 256) return false;
    struct DepthGuard { size_t& depth; ~DepthGuard() { --depth; } } guard{depth_};
    ++depth_;
    switch (text_[pos_]) {
      case '{':
        return ParseObject(out);
      case '[':
        return ParseArray(out);
      case '"': {
        std::string value;
        if (!ParseString(&value)) return false;
        *out = Json::String(std::move(value));
        return true;
      }
      case 't':
        return ParseLiteral("true", Json::Bool(true), out);
      case 'f':
        return ParseLiteral("false", Json::Bool(false), out);
      case 'n':
        return ParseLiteral("null", Json::Null(), out);
      default:
        return ParseNumber(out);
    }
  }

  bool ParseLiteral(const char* literal, Json value, Json* out) {
    const size_t length = std::strlen(literal);
    if (text_.compare(pos_, length, literal) != 0) return false;
    pos_ += length;
    *out = std::move(value);
    return true;
  }

  bool ParseObject(Json* out) {
    if (!Expect('{')) return false;
    Json::Members members;
    SkipWhitespace();
    if (Peek('}')) {
      ++pos_;
      *out = Json::Object(std::move(members));
      return true;
    }
    for (;;) {
      SkipWhitespace();
      std::string key;
      if (!ParseString(&key)) return false;
      SkipWhitespace();
      if (!Expect(':')) return false;
      SkipWhitespace();
      Json value;
      if (!ParseValue(&value)) return false;
      members.emplace_back(std::move(key), std::move(value));
      SkipWhitespace();
      if (Expect(',')) continue;
      if (Expect('}')) break;
      return false;
    }
    *out = Json::Object(std::move(members));
    return true;
  }

  bool ParseArray(Json* out) {
    if (!Expect('[')) return false;
    Json::Elements elements;
    SkipWhitespace();
    if (Peek(']')) {
      ++pos_;
      *out = Json::Array(std::move(elements));
      return true;
    }
    for (;;) {
      SkipWhitespace();
      Json value;
      if (!ParseValue(&value)) return false;
      elements.push_back(std::move(value));
      SkipWhitespace();
      if (Expect(',')) continue;
      if (Expect(']')) break;
      return false;
    }
    *out = Json::Array(std::move(elements));
    return true;
  }

  bool ParseHex4(uint32_t* out) {
    if (pos_ + 4 > text_.size()) return false;
    uint32_t value = 0;
    for (int i = 0; i < 4; ++i) {
      const char c = text_[pos_ + i];
      value <<= 4;
      if (c >= '0' && c <= '9') {
        value |= static_cast<uint32_t>(c - '0');
      } else if (c >= 'a' && c <= 'f') {
        value |= static_cast<uint32_t>(c - 'a' + 10);
      } else if (c >= 'A' && c <= 'F') {
        value |= static_cast<uint32_t>(c - 'A' + 10);
      } else {
        return false;
      }
    }
    pos_ += 4;
    *out = value;
    return true;
  }

  bool ParseString(std::string* out) {
    if (!Expect('"')) return false;
    out->clear();
    while (pos_ < text_.size()) {
      const char c = text_[pos_++];
      if (c == '"') return true;
      if (c != '\\') {
        const auto first = static_cast<unsigned char>(c);
        if (first < 0x20) return false;
        if (first >= 0x80) {
          if (first < 0xC2 || first > 0xF4) return false;
          const size_t count = first < 0xE0 ? 2 : first < 0xF0 ? 3 : 4;
          uint32_t codepoint = first & (0x7F >> count);
          const size_t begin = pos_ - 1;
          for (size_t i = 1; i < count; ++i) {
            if (pos_ == text_.size()) return false;
            const auto next = static_cast<unsigned char>(text_[pos_++]);
            if ((next & 0xC0) != 0x80) return false;
            codepoint = (codepoint << 6) | (next & 0x3F);
          }
          if ((count == 3 && codepoint < 0x800) || (count == 4 && codepoint < 0x10000) ||
              codepoint > 0x10FFFF || (codepoint >= 0xD800 && codepoint <= 0xDFFF)) return false;
          out->append(text_, begin, count);
        } else out->push_back(c);
        continue;
      }
      if (pos_ >= text_.size()) return false;
      const char escape = text_[pos_++];
      switch (escape) {
        case '"': out->push_back('"'); break;
        case '\\': out->push_back('\\'); break;
        case '/': out->push_back('/'); break;
        case 'b': out->push_back('\b'); break;
        case 'f': out->push_back('\f'); break;
        case 'n': out->push_back('\n'); break;
        case 'r': out->push_back('\r'); break;
        case 't': out->push_back('\t'); break;
        case 'u': {
          uint32_t unit = 0;
          if (!ParseHex4(&unit)) return false;
          if (unit >= 0xD800 && unit <= 0xDBFF) {
            if (pos_ + 1 >= text_.size() || text_[pos_] != '\\' || text_[pos_ + 1] != 'u') return false;
            pos_ += 2;
            uint32_t low = 0;
            if (!ParseHex4(&low) || low < 0xDC00 || low > 0xDFFF) return false;
            unit = 0x10000 + ((unit - 0xD800) << 10) + (low - 0xDC00);
          } else if (unit >= 0xDC00 && unit <= 0xDFFF) return false;
          AppendUtf8(out, unit);
          break;
        }
        default:
          return false;
      }
    }
    return false;  // Unterminated.
  }

  bool ParseNumber(Json* out) {
    const size_t start = pos_;
    const auto digit = [&] { return pos_ < text_.size() && text_[pos_] >= '0' && text_[pos_] <= '9'; };
    Expect('-');
    if (!digit()) return false;
    if (!Expect('0')) while (digit()) ++pos_;
    if (Expect('.')) {
      if (!digit()) return false;
      while (digit()) ++pos_;
    }
    if (Expect('e') || Expect('E')) {
      if (!Expect('+')) Expect('-');
      if (!digit()) return false;
      while (digit()) ++pos_;
    }
    const double number = std::strtod(text_.substr(start, pos_ - start).c_str(), nullptr);
    if (!std::isfinite(number)) return false;
    *out = Json::Number(number);
    return true;
  }

  const std::string& text_;
  size_t pos_ = 0;
  size_t depth_ = 0;
};

void SerializeString(const std::string& value, std::string* out) {
  out->push_back('"');
  for (unsigned char c : value) {
    switch (c) {
      case '"': *out += "\\\""; break;
      case '\\': *out += "\\\\"; break;
      case '\b': *out += "\\b"; break;
      case '\f': *out += "\\f"; break;
      case '\n': *out += "\\n"; break;
      case '\r': *out += "\\r"; break;
      case '\t': *out += "\\t"; break;
      default:
        if (c < 0x20) {
          char buffer[8];
          std::snprintf(buffer, sizeof(buffer), "\\u%04x", c);
          *out += buffer;
        } else {
          out->push_back(static_cast<char>(c));
        }
    }
  }
  out->push_back('"');
}

}  // namespace

Json Json::Bool(bool value) {
  Json json;
  json.type_ = Type::kBool;
  json.bool_ = value;
  return json;
}

Json Json::Int(long long value) {
  Json json;
  json.type_ = Type::kNumber;
  json.number_ = static_cast<double>(value);
  return json;
}

Json Json::Number(double value) {
  Json json;
  json.type_ = Type::kNumber;
  json.number_ = value;
  return json;
}

Json Json::String(std::string value) {
  Json json;
  json.type_ = Type::kString;
  json.string_ = std::move(value);
  return json;
}

Json Json::Array(Elements elements) {
  Json json;
  json.type_ = Type::kArray;
  json.elements_ = std::move(elements);
  return json;
}

Json Json::Object(Members members) {
  Json json;
  json.type_ = Type::kObject;
  json.members_ = std::move(members);
  return json;
}

const Json* Json::Find(const std::string& key) const {
  if (type_ != Type::kObject) return nullptr;
  for (const Member& member : members_) {
    if (member.first == key) return &member.second;
  }
  return nullptr;
}

long long Json::int_value(long long fallback) const {
  if (type_ != Type::kNumber || !std::isfinite(number_) || std::trunc(number_) != number_ ||
      number_ < static_cast<double>(std::numeric_limits<long long>::min()) ||
      number_ >= -static_cast<double>(std::numeric_limits<long long>::min())) return fallback;
  return static_cast<long long>(number_);
}

std::string Json::string_value(const std::string& fallback) const {
  return type_ == Type::kString ? string_ : fallback;
}

std::string Json::Serialize() const {
  std::string out;
  switch (type_) {
    case Type::kNull:
      out += "null";
      break;
    case Type::kBool:
      out += bool_ ? "true" : "false";
      break;
    case Type::kNumber: {
      char buffer[32];
      std::snprintf(buffer, sizeof(buffer), "%.17g", number_);
      out += buffer;
      break;
    }
    case Type::kString:
      SerializeString(string_, &out);
      break;
    case Type::kArray: {
      out += '[';
      for (size_t i = 0; i < elements_.size(); ++i) {
        if (i != 0) out += ',';
        out += elements_[i].Serialize();
      }
      out += ']';
      break;
    }
    case Type::kObject: {
      out += '{';
      for (size_t i = 0; i < members_.size(); ++i) {
        if (i != 0) out += ',';
        SerializeString(members_[i].first, &out);
        out += ':';
        out += members_[i].second.Serialize();
      }
      out += '}';
      break;
    }
  }
  return out;
}

bool Json::Parse(const std::string& text, Json* out, std::string* error) {
  Parser parser(text);
  return parser.ParseDocument(out, error);
}

}  // namespace icl
