// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include "pattern/pattern_expression.h"

#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "icl/icl_semantics.h"

namespace {
int64_t Evaluate(const std::string& expression) {
  const icl::ParsedDocument document(
      "Module M { Parameter value = " + expression + "; }");
  if (!document.ok())
    throw std::runtime_error(document.driver().error_message());
  const auto& tokens = document.driver().unit().tokens();
  size_t begin = 0, end = 0;
  for (size_t i = 0; i < tokens.size(); ++i) {
    if (tokens[i].text == "=") begin = i + 1;
    if (tokens[i].text == ";") end = i;
  }
  return pattern::EvaluateInteger(
      tokens, begin, end, [](const std::string& name) {
        if (name == "width") return int64_t{8};
        throw std::runtime_error("unknown parameter " + name);
      });
}

void Expect(const std::string& expression, int64_t expected) {
  const auto actual = Evaluate(expression);
  if (actual != expected)
    throw std::runtime_error(expression + ": expected " +
                             std::to_string(expected) + ", got " +
                             std::to_string(actual));
}

template <typename Action>
void Rejects(Action action, const std::string& message) {
  try {
    action();
  } catch (const std::runtime_error& error) {
    if (std::string(error.what()).find(message) == std::string::npos) throw;
    return;
  }
  throw std::runtime_error("expected failure containing: " + message);
}

void Fails(const std::string& expression, const std::string& message) {
  Rejects([&] { Evaluate(expression); }, message);
}

pattern::Constant Constant(const std::string& expression, size_t width = 0,
                           size_t max_string_bytes = 1024 * 1024) {
  const std::string header = width
                                 ? "Module M { ScanInPort si; ScanRegister r[" +
                                       std::to_string(width - 1) +
                                       ":0] { ScanInSource si; ResetValue "
                                 : "Module M { Parameter value = ";
  const icl::ParsedDocument document(header + expression +
                                     (width ? "; } }" : "; }"));
  if (!document.ok())
    throw std::runtime_error(document.driver().error_message());
  const auto& tokens = document.driver().unit().tokens();
  size_t begin = 0, end = 0;
  for (size_t i = 0; i < tokens.size(); ++i) {
    if (tokens[i].text == "=" || tokens[i].text == "ResetValue") begin = i + 1;
    if (tokens[i].text == ";") end = i;
  }
  return pattern::EvaluateConstant(
      tokens, begin, end,
      [](const std::string& name) {
        if (name == "width") return pattern::Constant{false, false, "1000"};
        if (name == "value") return pattern::Constant{false, true, "0111"};
        if (name == "text") return pattern::Constant{true, true, "abc"};
        if (name == "unknown") return pattern::Constant{false, true, "x"};
        if (name == "wide")
          return pattern::Constant{false, true, std::string(64, '1')};
        if (name == "zero") return pattern::Constant{false, false, "0"};
        if (name == "too_wide")
          return pattern::Constant{false, false,
                                   "1" + std::string(19, '0') + "1"};
        throw std::runtime_error("unknown parameter " + name);
      },
      width, max_string_bytes);
}

void TestConstantBoundaries() {
  using Value = pattern::Constant;
  const Value maximum{false, true, std::string(63, '1')};
  if (maximum.AsInteger() != std::numeric_limits<int64_t>::max() ||
      Value{false, false, std::string(100, '0') + "101"}.AsInteger() != 5 ||
      Value{false, false, "0"}.Sized(4).data != "0000" ||
      Value{false, false, "x1"}.Sized(4).data != "xxx1" ||
      Value{false, false, "0001"}.Sized(2).data != "01" ||
      Value{false, true, "01"}.Sized(2).data != "01") {
    throw std::runtime_error("constant conversion or sizing boundary changed");
  }
  Rejects([] { Value{true, true, "1"}.AsInteger(); }, "integer parameter");
  Rejects([] { Value{}.AsInteger(); }, "integer parameter");
  Rejects([] { Value{false, true, "x"}.AsInteger(); }, "unknown bit");
  Rejects([] { Value{false, true, "1" + std::string(63, '0')}.AsInteger(); },
          "64-bit limit");
  Rejects([] { Value{true, true, "a"}.Sized(1); }, "numeric value");
  Rejects([] { Value{false, true, "01"}.Sized(3); }, "width does not match");
  Rejects([] { Value{false, false, "1"}.Sized(0); }, "width exceeds");
  Rejects([] { Value{false, false, "1"}.Sized(1048577); }, "width exceeds");
  Rejects([] { Value{false, false, "10"}.Sized(1); }, "significant bits");
  if (Value{false, false, "x"}.Sized(1048576).data !=
      std::string(1048576, 'x')) {
    throw std::runtime_error("maximum supported value width rejected");
  }

  if (Constant("8 /* width */ 'h // digits\n A5").data != "10100101" ||
      Constant("8 // width\n 'H /* digits */ xA").data != "xxxx1010" ||
      Constant("4'd0").data != "0000" ||
      Constant(R"("a\"b\\c")").data != "a\"b\\c" ||
      Constant("\"ab\", \"cd\"", 0, 4).data != "abcd") {
    throw std::runtime_error("numeric comments or string decoding changed");
  }
  Rejects([] { Constant("$text + 1"); }, "integer parameter");
  Rejects([] { Constant("$unknown + 1"); }, "unknown bit");
  Rejects([] { Constant("$wide + 1"); }, "64-bit limit");
  Rejects([] { Constant("$text, 4'b0000"); }, "strings and numbers");
  Rejects([] { Constant("$value", 3); }, "constant width does not match");
  Rejects([] { Constant("$width, $width", 16); }, "one unsized element");
  Rejects([] { Constant("$zero'b0"); }, "width exceeds");
  Rejects([] { Constant("$too_wide'b0"); }, "width exceeds");
  Rejects([] { Constant("~$text"); }, "known numeric width");
  Rejects([] { Constant("\"ab\", \"cd\"", 0, 3); }, "size limit");
  Rejects([] { Constant("524289'b0, 524288'b0"); }, "size limit");
  Rejects([] { Constant("'b" + std::string(1048577, '0')); }, "bit limit");
}

void TestEvaluatorApiBoundaries() {
  // The parser rejects these first; exercise the public token APIs directly.
  const auto integer_parameter = [](const std::string& name) {
    if (name == "width") return int64_t{8};
    throw std::runtime_error("unexpected integer parameter " + name);
  };
  const auto constant_parameter = [](const std::string& name) {
    if (name == "width") return pattern::Constant{false, false, "1000"};
    throw std::runtime_error("unexpected constant parameter " + name);
  };
  const auto integer = [&](const std::vector<icl::SourceToken>& tokens) {
    return pattern::EvaluateInteger(tokens, 0, tokens.size(),
                                    integer_parameter);
  };
  const auto constant = [&](const std::string& text) {
    return pattern::EvaluateConstant({{text, 0, text.size()}}, 0, 1,
                                     constant_parameter);
  };
  const std::vector<icl::SourceToken> addition{
      {"$width", 0, 6}, {"+", 7, 8}, {"2", 9, 10}};
  if (integer(addition) != 10 || constant("$width").AsInteger() != 8 ||
      constant("8'hA5").data != "10100101") {
    throw std::runtime_error("valid public evaluator inputs rejected");
  }
  const std::vector<icl::SourceToken> one{{"1", 0, 1}};
  for (const auto& bounds : {std::pair{size_t{0}, size_t{0}}, {1, 0}, {0, 2}}) {
    Rejects(
        [&] {
          pattern::EvaluateInteger(one, bounds.first, bounds.second,
                                   integer_parameter);
        },
        "empty or invalid ICL integer expression");
    Rejects(
        [&] {
          pattern::EvaluateConstant(one, bounds.first, bounds.second,
                                    constant_parameter);
        },
        "empty ICL constant");
  }
  Rejects([&] { integer({}); }, "empty or invalid ICL integer expression");
  Rejects([&] { constant("8/*"); }, "unterminated numeric comment");
  Rejects([&] { constant("8'b2"); }, "invalid ICL numeric digit");
  Rejects([&] { integer({{"0x1"}}); }, "expected ICL decimal integer");
  Rejects([&] { integer({{"1"}, {"+"}}); }, "missing ICL integer operand");
  Rejects([&] { integer({{"1"}, {")"}}); },
          "unmatched ICL integer parenthesis");
  Rejects([&] { integer({{"1"}, {"??"}}); }, "invalid ICL integer operator");
  Rejects([&] { integer({{"("}, {"1"}}); }, "invalid ICL integer operands");
  Rejects([&] { integer({{"1"}, {"+"}, {"("}, {"2"}}); },
          "invalid ICL integer operator");
}

void TestConstants() {
  bool empty_rejected = false;
  try {
    pattern::Constant{}.Sized(1);
  } catch (const std::runtime_error&) {
    empty_rejected = true;
  }
  if (!empty_rejected) throw std::runtime_error("empty numeric value accepted");
  // IEEE 6.3.8 and 6.3.11 examples, completed in legal module contexts.
  if (Constant("7'hx").data != "xxxxxxx" ||
      Constant("'hx3", 9).data != "xxxxx0011" ||
      Constant("1'b0, 0, 5'h1a", 10).data != "0000011010" ||
      Constant("1'b0, ~4'h7, 5'h1a").data != "0100011010" ||
      Constant("$width'hA5").data != "10100101" ||
      Constant("$value", 4).data != "0111" ||
      Constant("\"ab\", \"cd\"").data != "abcd" ||
      !Constant("\"ab\"").is_string ||
      Constant("128'h1").data != std::string(127, '0') + "1" ||
      Constant("$width * 2").AsInteger() != 16)
    throw std::runtime_error("ICL constant evaluation mismatch");
  for (const auto& expression :
       {"~0", "~'h0", "~'bx", "~$width", "1'b0, 0, 3'b000"}) {
    bool failed = false;
    try {
      Constant(expression, 4);
    } catch (const std::runtime_error&) {
      failed = true;
    }
    if (!failed) {
      throw std::runtime_error("accepted illegal unsized element: " +
                               std::string(expression));
    }
  }
  if (Constant("~$value", 4).data != "1000" ||
      Constant("1'b1, 'bx, 1'b1", 4).data != "1xx1") {
    throw std::runtime_error("sized parameter inversion or X padding changed");
  }
  for (const auto& expression : {"$value, $width", "$width'h1FF", "1 / 0"}) {
    bool failed = false;
    try {
      Constant(expression);
    } catch (const std::runtime_error&) {
      failed = true;
    }
    if (!failed)
      throw std::runtime_error("expected constant error: " +
                               std::string(expression));
  }
}
}  // namespace

int main() {
  try {
    // IEEE 1687-2014 6.3.12: both arithmetic precedence levels are right
    // recursive.
    Expect("20 - 5 - 3", 18);
    Expect("48 / 6 / 2", 16);
    Expect("20 % 7 % 4", 2);
    Expect("2 * 3 + 4", 10);
    Expect("2 + 3 * 4", 14);
    Expect("(20 - 5) - 3", 12);
    Expect("$width - 1", 7);
    Expect("$width /* source comment */ * (2 + 1)", 24);
    Expect("2 + (1 - 2)", 1);
    Expect("1_000 + 002", 1002);
    Fails("1 - 2", "negative");
    Fails("1 / 0", "zero");
    Fails("1 % 0", "zero");
    Fails("$missing", "unknown parameter");
    Fails("9223372036854775807 + 1", "limit");
    Fails("9223372036854775807 * 2", "limit");
    Fails("9223372036854775808", "limit");
    Expect("(0 - 3) * (0 - 2)", 6);
    Expect("(((0 - 9223372036854775807) - 1) + 9223372036854775807) + 1", 0);
    Fails("(0 - 9223372036854775807) - 2", "limit");
    Fails("((0 - 9223372036854775807) - 1) / (0 - 1)", "limit");
    Fails("((0 - 9223372036854775807) - 1) % (0 - 1)", "limit");
    TestConstants();
    TestConstantBoundaries();
    TestEvaluatorApiBoundaries();
    std::cout << "PASS: ICL integer expressions from shared parser tokens\n";
  } catch (const std::exception& error) {
    std::cerr << "FAIL: " << error.what() << '\n';
    return 1;
  }
}
