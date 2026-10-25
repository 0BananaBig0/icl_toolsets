// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include "pattern_bsdl.h"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

#include "text_input.h"

namespace {
void Require(bool condition, const std::string& message) {
  if (!condition) throw std::runtime_error(message);
}
template <typename Action>
void Fails(Action action, const std::string& expected) {
  try {
    action();
  } catch (const std::runtime_error& error) {
    Require(std::string(error.what()).find(expected) != std::string::npos,
            "expected " + expected + ", got " + error.what());
    return;
  }
  throw std::runtime_error("expected failure: " + expected);
}
std::string Fixture() {
  std::ifstream input(
      "tests/cases/ours/pattern_generation/access_link/simple.bsdl");
  std::string text;
  Require(input.good() && toolsets::ReadTextStream(input, &text),
          "read BSDL fixture");
  return text;
}
std::string Replace(std::string text, const std::string& from,
                    const std::string& to) {
  const auto pos = text.find(from);
  Require(pos != std::string::npos, "test replacement exists: " + from);
  text.replace(pos, from.size(), to);
  return text;
}
void Reject(const std::string& text, const std::string& expected) {
  Fails([&] { pattern::ReadBsdlTap("input.bsdl", text, "AccessProbe"); },
        expected);
}
void TestConstrainedTapPort() {
  Reject(Replace(Fixture(), "in bit;", "in bit(0 to 1);"), "scalar bit port");
}
void TestRetainedAttributeBudget() {
  std::string targets;
  for (int i = 0; i < 1024; ++i) {
    if (i) targets += ",";
    targets += "p" + std::to_string(i);
  }
  const auto attribute = "attribute EXTRA of " + targets + " : signal is \"" +
                         std::string(8192, 'x') + "\";\n";
  Reject(Replace(Fixture(), "end AccessProbe;", attribute + "end AccessProbe;"),
         "retained-metadata limit exceeded");
}
void TestRegisterFanoutBudget() {
  std::string opcodes, instructions;
  for (int i = 0; i < 1024; ++i) {
    if (i) {
      opcodes += ",";
      instructions += ",";
    }
    const auto instruction = "I" + std::to_string(i);
    opcodes += instruction + "(0000)";
    instructions += instruction;
  }
  auto text = Fixture();
  const auto begin = text.find("\"BYPASS (1111)");
  const auto end = text.find(';', begin);
  text.replace(begin, end - begin, "\"" + opcodes + "\"");
  text = Replace(
      text, "BOUNDARY (EXTEST, SAMPLE), BYPASS (BYPASS), DEVICE_ID (IDCODE)",
      std::string(8192, 'R') + "(" + instructions + ")");
  Reject(text, "retained-metadata limit exceeded");
}
void TestStaticExpressions() {
  auto text = Replace(Fixture(), "is 4;", "is WIDTH;");
  text = Replace(text, "generic (",
                 "constant WIDTH : positive := 4;\n"
                 "constant HI : string := \"00\";\n"
                 "generic (");
  text = Replace(text, "is \"0001\";", "is (HI & \"01\");");
  text = Replace(text, "IJTAG_ON (0011)", "IJTAG_ON (0011, 01XX)");
  text = Replace(text, "BOUNDARY (EXTEST", "BOUNDARY[1] (EXTEST");
  text = Replace(text, "INSTRUCTION_LENGTH of AccessProbe : entity",
                 "instruction_LENGTH OF accessPROBE : ENTITY");
  text = "\xef\xbb\xbf-- entity AccessProbe is a comment\n" + text;
  text += "-- entity AccessProbe is\n";
  const auto tap = pattern::ReadBsdlTap("static.bsdl", text, "ACCESSPROBE");
  Require(tap.capture == "0001" && tap.instruction_width == 4,
          "static constants and string concatenation");
  Require(
      tap.opcodes.at("IJTAG_ON") == std::vector<std::string>({"0011", "01xx"}),
      "alternative and masked opcodes");
  Require(tap.registers.at("EXTEST").width == 1, "explicit register width");
  text.clear();
  Require(tap.ports.at("tap_scan_clock") == "TCK", "metadata owns strings");
}
void TestMalformedMetadata() {
  const auto text = Fixture();
  for (const auto& replacement :
       std::vector<std::pair<std::string, std::string>>{
           {"is 4;", "is UNKNOWN;"},
           {"is 4;", "is 2 + 2;"},
           {"is 4;", "is 1__0;"},
           {"is 4;", "is 9999999999999999999999;"},
           {"is \"0001\";", "is \"001\";"},
           {"is \"0001\";", "is \"000Z\";"},
           {"is \"0001\";", "is UNKNOWN;"},
           {"is \"0001\";", "is \"00\" &;"},
           {"IJTAG_ON (0011)", "IJTAG_ON (001)"},
           {"IJTAG_ON (0011)", "BYPASS (0011)"},
           {"DEVICE_ID (IDCODE)", "DEVICE_ID (UNKNOWN)"},
           {"DEVICE_ID (IDCODE)", "DEVICE_ID[0] (IDCODE)"},
           {"DEVICE_ID (IDCODE)", "DEVICE_ID (EXTEST)"},
           {"TDO : out bit", "TDO : in bit"},
           {"TDO : out bit", "TDO : out bit_vector(1 downto 0)"},
           {"TAP_SCAN_IN of TDI : signal", "TAP_SCAN_IN of NONE : signal"},
           {"TAP_SCAN_IN of TDI : signal", "TAP_SCAN_IN of TDI : entity"},
           {"is true;", "is false;"},
           {"TAP_SCAN_IN", "UNUSED"},
           {"end AccessProbe;", "end Wrong;"}}) {
    Reject(Replace(text, replacement.first, replacement.second), "input.bsdl:");
  }
  Reject(text + text, "duplicate BSDL entity");
  auto cycle =
      Replace(text, "generic (", "constant C : string := C;\ngeneric (");
  Reject(Replace(cycle, "is \"0001\";", "is C;"), "nesting limit");
  cycle = Replace(text, "generic (", "constant N : positive := N;\ngeneric (");
  Reject(Replace(cycle, "is 4;", "is N;"), "nesting limit");
}
void TestTruncationAndMutation() {
  const auto text = Fixture();
  const auto end = text.rfind(';');
  for (size_t length = 0; length <= end; ++length) {
    Reject(text.substr(0, length), "input.bsdl:");
  }
  // Single-byte deletions exercise declaration and embedded-table boundaries.
  for (size_t pos = 0; pos < text.size(); ++pos) {
    auto mutated = text;
    mutated.erase(pos, 1);
    try {
      pattern::ReadBsdlTap("mutation.bsdl", mutated, "AccessProbe");
    } catch (const std::runtime_error& error) {
      Require(std::string(error.what()).find("mutation.bsdl:") == 0,
              "mutations fail with source-located diagnostics");
    }
  }
}
void TestLexicalBoundaries() {
  auto text = Replace(Fixture(), "generic (",
                      "attribute NOTE : string;\n"
                      "attribute NOTE of AccessProbe : entity is "
                      "\"entity AccessProbe is \"\"quoted\"\" \\path\";\n"
                      "constant CH : character := ';';\ngeneric (");
  const auto tap = pattern::ReadBsdlTap("quotes.bsdl", text, "AccessProbe");
  Require(tap.attributes.at({"note", "accessprobe"}).expression ==
              "\"entity AccessProbe is \"\"quoted\"\" \\path\"",
          "quoted fake declarations and character delimiters remain data");
  Reject(Replace(Fixture(), "is \"0001\";", "is \"00\n01\";"), "newline");
  Reject(Replace(Fixture(), "is \"0001\";",
                 "is \"00" + std::string(1, '\0') + "01\";"),
         "NUL");
  Reject(std::string(1, '\0') + Fixture(), "unsupported BSDL character");
  Reject(Replace(Fixture(), "TDO : out bit", "TDO__ : out bit"), "identifier");
  Reject(Replace(Fixture(), "TDO : out bit", "TDO_ : out bit"), "identifier");
}
void TestDuplicateMetadataAndNumericTokens() {
  Reject(Replace(Fixture(), "TDO : out bit", "TDI : out bit"),
         "duplicate BSDL port");
  Reject(Replace(Fixture(), "generic (",
                 "constant C : integer := 1;\n"
                 "constant c : integer := 2;\ngeneric ("),
         "duplicate BSDL constant");
  Reject(
      Replace(
          Fixture(), "end AccessProbe;",
          "attribute TAP_SCAN_IN of TDI : signal is true;\nend AccessProbe;"),
      "duplicate BSDL attribute");
  Reject(
      Replace(
          Fixture(), "end AccessProbe;",
          "attribute TAP_SCAN_IN of PIN : signal is true;\nend AccessProbe;"),
      "duplicate TAP port role");
  auto text = Replace(Fixture(), "is 4;", "is 0_4;");
  text = Replace(text, "10.0e6", "10.0E+6");
  const auto tap = pattern::ReadBsdlTap("numeric.bsdl", text, "AccessProbe");
  Require(tap.instruction_width == 4 &&
              tap.attributes.at({"tap_scan_clock", "tck"}).expression ==
                  "(10.0E+6, BOTH)",
          "decimal separators and opaque clock exponent preserved");
}
void TestResourceLimits() {
  Reject(std::string(16 * 1024 * 1024 + 1, ' '), "source size limit");
  Reject(std::string(1024 * 1024 + 1, '?'), "token limit");
  std::string constants = "constant C0 : string := \"00\";\n";
  for (int i = 1; i <= 22; ++i) {
    const auto previous = "C" + std::to_string(i - 1);
    constants += "constant C" + std::to_string(i) + " : string := " + previous +
                 " & " + previous + ";\n";
  }
  auto text = Replace(Fixture(), "generic (", constants + "generic (");
  Reject(Replace(text, "is \"0001\";", "is C22;"), "decoded-value limit");
  Reject(Replace(Fixture(), "is \"0001\";",
                 "is " + std::string(129, '(') + "\"0001\"" +
                     std::string(129, ')') + ";"),
         "nesting limit");
}
void TestEmbeddedTableTruncation() {
  const std::pair<std::string, std::string> tables[] = {
      {"\"BYPASS (1111), EXTEST (0000), SAMPLE (0001), IDCODE (0010), \" &\n"
       "    \"IJTAG_ON (0011), IJTAG_OFF (0100)\"",
       "IJTAG_ON (0011, 01XX), IJTAG_OFF(0100)"},
      {"\"BOUNDARY (EXTEST, SAMPLE), BYPASS (BYPASS), DEVICE_ID (IDCODE)\"",
       "BOUNDARY[1](EXTEST,SAMPLE), DEVICE_ID[32](IDCODE)"}};
  for (const auto& table : tables) {
    for (size_t length = 0; length < table.second.size(); ++length) {
      const auto text = Replace(Fixture(), table.first,
                                "\"" + table.second.substr(0, length) + "\"");
      try {
        pattern::ReadBsdlTap("table.bsdl", text, "AccessProbe");
      } catch (const std::runtime_error& error) {
        Require(std::string(error.what()).find("table.bsdl:") == 0,
                "truncated tables fail with source-located diagnostics");
      }
    }
  }
}
void TestLargePortList() {
  std::string ports;
  for (size_t i = 0; i < 10000; ++i) {
    ports += "P" + std::to_string(i) + " : in bit; ";
  }
  const auto text = Replace(Fixture(), "port (", "port (" + ports);
  const auto tap = pattern::ReadBsdlTap("large.bsdl", text, "AccessProbe");
  Require(tap.ports.at("tap_scan_in") == "TDI" && tap.capture == "0001",
          "unrelated ports preserve TAP metadata");
}
void TestVendorFixture() {
  const auto tap =
      pattern::ReadBsdlTap("simple.bsdl", Fixture(), "accessprobe");
  Require(tap.entity == "AccessProbe" && tap.instruction_width == 4,
          "entity lookup uses VHDL identifier case rules");
  Require(tap.ports.at("tap_scan_in") == "TDI" &&
              tap.ports.at("tap_scan_out") == "TDO" &&
              tap.ports.at("tap_scan_reset") == "TRST",
          "TAP port roles");
  Require(tap.opcodes.at("IJTAG_ON") == std::vector<std::string>{"0011"} &&
              tap.capture == "0001",
          "vendor-observed instruction/capture values");
  Require(tap.idcode && *tap.idcode == "00010000000000000000000000000011",
          "concatenated IDCODE fields");
  Require(tap.boundary_width == 1 &&
              tap.registers.at("EXTEST").name == "BOUNDARY" &&
              tap.registers.at("IDCODE").name == "DEVICE_ID",
          "register associations");
  Require(tap.attributes.count({"boundary_register", "accessprobe"}),
          "retain unused attributes for subsequent model checks");
}
void TestInvalid() {
  const auto text = Fixture();
  Fails([&] { pattern::ReadBsdlTap("input.bsdl", text, "Missing"); },
        "unknown BSDL entity");
  auto broken = text;
  const auto offset = broken.find("is 4;");
  broken.replace(offset, 5, "is 0;");
  Fails([&] { pattern::ReadBsdlTap("input.bsdl", broken, "AccessProbe"); },
        "instruction width");
}
}  // namespace
int main() {
  try {
    TestVendorFixture();
    TestInvalid();
    TestConstrainedTapPort();
    TestRetainedAttributeBudget();
    TestRegisterFanoutBudget();
    TestStaticExpressions();
    TestMalformedMetadata();
    TestTruncationAndMutation();
    TestLexicalBoundaries();
    TestResourceLimits();
    TestEmbeddedTableTruncation();
    TestLargePortList();
    TestDuplicateMetadataAndNumericTokens();
    std::cout << "pattern BSDL tests passed\n";
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
