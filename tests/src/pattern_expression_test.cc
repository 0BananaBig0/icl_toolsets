// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include "pattern/pattern_expression.h"

#include <iostream>
#include <stdexcept>
#include <string>

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

void Fails(const std::string& expression, const std::string& message) {
  try {
    Evaluate(expression);
  } catch (const std::runtime_error& error) {
    if (std::string(error.what()).find(message) == std::string::npos) throw;
    return;
  }
  throw std::runtime_error("expected failure: " + expression);
}

pattern::Constant Constant(const std::string& expression, size_t width = 0) {
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
        throw std::runtime_error("unknown parameter " + name);
      },
      width);
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
    TestConstants();
    std::cout << "PASS: ICL integer expressions from shared parser tokens\n";
  } catch (const std::exception& error) {
    std::cerr << "FAIL: " << error.what() << '\n';
    return 1;
  }
}
