// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include "pattern/pattern_expression.h"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <limits>
#include <stdexcept>

namespace pattern {
namespace {
constexpr size_t kMaxValueBits = 1024 * 1024;

std::string Binary(int64_t value) {
  std::string result;
  do {
    result += (value & 1) ? '1' : '0';
    value >>= 1;
  } while (value);
  std::reverse(result.begin(), result.end());
  return result;
}

void Resize(std::string* bits, size_t width) {
  if (bits->empty()) throw std::runtime_error("empty ICL number");
  if (!width || width > kMaxValueBits) {
    throw std::runtime_error(
        "ICL value width exceeds supported limit (1..1048576 bits)");
  }
  if (bits->size() > width) {
    const size_t removed = bits->size() - width;
    if (bits->substr(0, removed).find('1') != std::string::npos) {
      throw std::runtime_error("ICL number truncates significant bits");
    }
    bits->erase(0, removed);
  } else {
    bits->insert(0, width - bits->size(), bits->front() == 'x' ? 'x' : '0');
  }
}

std::string CompactNumber(const std::string& token) {
  std::string result;
  for (size_t i = 0; i < token.size();) {
    if (token.compare(i, 2, "/*") == 0) {
      const auto end = token.find("*/", i + 2);
      if (end == std::string::npos) {
        throw std::runtime_error("unterminated numeric comment");
      }
      i = end + 2;
    } else if (token.compare(i, 2, "//") == 0) {
      i = token.find('\n', i + 2);
      if (i == std::string::npos) break;
    } else {
      const auto c = static_cast<unsigned char>(token[i++]);
      if (!std::isspace(c)) result += static_cast<char>(c);
    }
  }
  return result;
}

Constant Literal(const std::string& token,
                 const std::function<Constant(const std::string&)>& parameter) {
  if (token.size() >= 2 && token.front() == '"' && token.back() == '"') {
    std::string decoded;
    for (size_t i = 1; i + 1 < token.size(); ++i) {
      if (token[i] == '\\') ++i;
      decoded += token[i];
    }
    return {true, true, decoded};
  }
  const auto text = CompactNumber(token);
  const auto quote = text.find('\'');
  if (quote == std::string::npos) {
    if (!text.empty() && text[0] == '$') return parameter(text.substr(1));
    return {false, false,
            Binary(EvaluateInteger({{text, 0, text.size()}}, 0, 1,
                                   [&](const std::string& name) {
                                     return parameter(name).AsInteger();
                                   }))};
  }
  size_t width = 0;
  if (quote) {
    const auto size = Literal(text.substr(0, quote), parameter).AsInteger();
    if (size <= 0 || size > static_cast<int64_t>(kMaxValueBits)) {
      throw std::runtime_error(
          "ICL value width exceeds supported limit (1..1048576 bits)");
    }
    width = static_cast<size_t>(size);
  }
  const auto base = static_cast<char>(
      std::tolower(static_cast<unsigned char>(text.at(quote + 1))));
  const auto digits = text.substr(quote + 2);
  std::string bits;
  if (base == 'd') {
    bits = Literal(digits, parameter).data;
  } else {
    for (char c : digits) {
      c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
      if (c == '_') continue;
      if (base == 'b' && (c == '0' || c == '1' || c == 'x')) {
        bits += c;
      } else if (base == 'h' && c == 'x') {
        bits += "xxxx";
      } else if (base == 'h' && std::isxdigit(static_cast<unsigned char>(c))) {
        const int value = c <= '9' ? c - '0' : c - 'a' + 10;
        for (int shift = 3; shift >= 0; --shift) {
          bits += (value >> shift) & 1 ? '1' : '0';
        }
      } else {
        throw std::runtime_error("invalid ICL numeric digit");
      }
      if (bits.size() > kMaxValueBits) {
        throw std::runtime_error("ICL literal exceeds bit limit");
      }
    }
  }
  if (bits.empty()) throw std::runtime_error("empty ICL number");
  if (width) Resize(&bits, width);
  return {false, width != 0, bits};
}

int Precedence(char op) {
  if (op == '+' || op == '-') return 1;
  if (op == '*' || op == '/' || op == '%') return 2;
  return 0;
}

int64_t Calculate(int64_t left, char op, int64_t right) {
  int64_t result = 0;
  bool overflow = false;
  switch (op) {
    case '+':
      overflow = __builtin_add_overflow(left, right, &result);
      break;
    case '-':
      overflow = __builtin_sub_overflow(left, right, &result);
      break;
    case '*':
      overflow = __builtin_mul_overflow(left, right, &result);
      break;
    case '/':
    case '%':
      if (!right) throw std::runtime_error("division or remainder by zero");
      overflow = left == std::numeric_limits<int64_t>::min() && right == -1;
      if (!overflow) result = op == '/' ? left / right : left % right;
      break;
    default:
      throw std::runtime_error("invalid ICL integer operator");
  }
  if (overflow) {
    throw std::runtime_error("ICL integer evaluation exceeds 64-bit limit");
  }
  return result;
}
}  // namespace

int64_t Constant::AsInteger() const {
  if (is_string || data.empty()) {
    throw std::runtime_error("integer parameter required");
  }
  int64_t value = 0;
  for (char c : data) {
    if (c != '0' && c != '1') {
      throw std::runtime_error("unknown bit in integer parameter");
    }
    if (value > (std::numeric_limits<int64_t>::max() - (c - '0')) / 2) {
      throw std::runtime_error("ICL integer parameter exceeds 64-bit limit");
    }
    value = value * 2 + (c - '0');
  }
  return value;
}

Constant Constant::Sized(size_t width) const {
  if (is_string) {
    throw std::runtime_error("numeric value required for bit sizing");
  }
  if (sized && data.size() != width) {
    throw std::runtime_error("sized ICL value width does not match target");
  }
  Constant result = *this;
  Resize(&result.data, width);
  result.sized = true;
  return result;
}

Constant EvaluateConstant(
    const std::vector<icl::SourceToken>& tokens, size_t begin, size_t end,
    const std::function<Constant(const std::string&)>& parameter,
    size_t context_width, size_t max_string_bytes) {
  if (begin >= end || end > tokens.size()) {
    throw std::runtime_error("empty ICL constant");
  }
  std::vector<Constant> parts;
  std::vector<bool> inversions;
  size_t fixed_width = 0, unsized = 0;
  for (size_t first = begin; first < end;) {
    size_t last = first;
    while (last < end && tokens[last].text != ",") ++last;
    const bool invert = tokens[first].text == "~";
    if (invert) ++first;
    if (first == last) throw std::runtime_error("empty ICL constant element");
    Constant part =
        first + 1 == last
            ? Literal(tokens[first].text, parameter)
            : Constant{false, false,
                       Binary(EvaluateInteger(
                           tokens, first, last, [&](const std::string& name) {
                             return parameter(name).AsInteger();
                           }))};
    if (!parts.empty() && part.is_string != parts.front().is_string) {
      throw std::runtime_error("cannot concatenate ICL strings and numbers");
    }
    if (part.sized) {
      fixed_width += part.data.size();
    } else {
      ++unsized;
    }
    if (fixed_width > (part.is_string ? max_string_bytes : kMaxValueBits)) {
      throw std::runtime_error("ICL constant exceeds size limit");
    }
    parts.push_back(std::move(part));
    inversions.push_back(invert);
    first = last + 1;
  }
  if (unsized > 1 || (parts.size() > 1 && unsized && !context_width)) {
    throw std::runtime_error(
        "unsized ICL concatenation needs one unsized element and a known "
        "width");
  }
  if (context_width && (fixed_width > context_width ||
                        (!unsized && fixed_width != context_width))) {
    throw std::runtime_error("ICL constant width does not match target");
  }
  Constant result{parts.front().is_string, unsized == 0 || context_width != 0,
                  ""};
  for (size_t i = 0; i < parts.size(); ++i) {
    auto& part = parts[i];
    if (!part.sized && context_width) {
      part = part.Sized(context_width - fixed_width);
    }
    if (inversions[i]) {
      if (part.is_string || (!part.sized && !context_width)) {
        throw std::runtime_error("ICL inversion needs a known numeric width");
      }
      for (char& c : part.data) c = c == '0' ? '1' : c == '1' ? '0' : 'x';
    }
    result.data += part.data;
  }
  return result;
}

int64_t EvaluateInteger(
    const std::vector<icl::SourceToken>& tokens, size_t begin, size_t end,
    const std::function<int64_t(const std::string&)>& parameter) {
  if (begin >= end || end > tokens.size()) {
    throw std::runtime_error("empty or invalid ICL integer expression");
  }
  std::vector<int64_t> values;
  std::vector<char> operators;
  const auto reduce = [&] {
    if (values.size() < 2 || operators.empty()) {
      throw std::runtime_error("invalid ICL integer operands");
    }
    const auto right = values.back();
    values.pop_back();
    values.back() = Calculate(values.back(), operators.back(), right);
    operators.pop_back();
  };
  bool need_value = true;
  for (size_t i = begin; i < end; ++i) {
    const auto& token = tokens[i].text;
    if (need_value) {
      if (token == "(") {
        operators.push_back('(');
        continue;
      }
      if (!token.empty() && token[0] == '$') {
        values.push_back(parameter(token.substr(1)));
      } else {
        std::string digits;
        for (char c : token) {
          if (c != '_') digits += c;
        }
        int64_t value = 0;
        const auto parsed = std::from_chars(
            digits.data(), digits.data() + digits.size(), value);
        if (parsed.ec == std::errc::result_out_of_range) {
          throw std::runtime_error("ICL integer literal exceeds 64-bit limit");
        }
        if (parsed.ec != std::errc{} ||
            parsed.ptr != digits.data() + digits.size() || value < 0) {
          throw std::runtime_error(
              "expected ICL decimal integer or parameter: " + token);
        }
        values.push_back(value);
      }
      need_value = false;
    } else if (token == ")") {
      while (!operators.empty() && operators.back() != '(') reduce();
      if (operators.empty()) {
        throw std::runtime_error("unmatched ICL integer parenthesis");
      }
      operators.pop_back();
    } else {
      if (token.size() != 1 || !Precedence(token[0])) {
        throw std::runtime_error("invalid ICL integer operator: " + token);
      }
      // Equal-precedence operators remain stacked: IEEE 6.3.12 is right
      // recursive.
      while (!operators.empty() &&
             Precedence(operators.back()) > Precedence(token[0])) {
        reduce();
      }
      operators.push_back(token[0]);
      need_value = true;
    }
  }
  if (need_value) throw std::runtime_error("missing ICL integer operand");
  while (!operators.empty()) reduce();
  if (values.size() != 1) {
    throw std::runtime_error("invalid ICL integer expression");
  }
  if (values[0] < 0) throw std::runtime_error("negative ICL integer result");
  return values[0];
}
}  // namespace pattern
