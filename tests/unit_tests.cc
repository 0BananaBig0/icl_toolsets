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
//
// Unit tests for the AST and the layout pass.
//
// These deliberately depend on neither bison nor flex: icl_ast.h and
// format_tree.h take plain ints for source positions rather than Bison's
// location type, precisely so that the tree and the layout can be exercised
// without generating a parser. Run via `make unitcheck`.

#include <iostream>
#include <string>
#include <vector>

#include "format_tree.h"
#include "icl_ast.h"
#include "json.h"

namespace icl {
namespace {

int g_pass = 0;
int g_fail = 0;

void Check(const std::string& name, bool condition,
           const std::string& detail = "") {
  if (condition) {
    ++g_pass;
    std::cout << "  [PASS] " << name << "\n";
  } else {
    ++g_fail;
    std::cout << "  [FAIL] " << name;
    if (!detail.empty()) std::cout << "  -> " << detail;
    std::cout << "\n";
  }
}

void CheckEq(const std::string& name, const std::string& expected,
             const std::string& actual) {
  Check(name, expected == actual, "expected:\n" + expected + "\ngot:\n" + actual);
}

FormatStyle DefaultStyle() { return FormatStyle(); }

// --- AST ownership -----------------------------------------------------------

void TestAstOwnsNodes() {
  std::cout << "AST ownership\n";
  AstUnit unit;
  AstBlock* module = unit.Make<AstBlock>("Module m", SpanOf(1, 3));
  unit.top_level().push_back(module);
  Check("top_level holds the module",
        unit.top_level().size() == 1 && unit.top_level()[0] == module);
  Check("module header and span round-trip",
        module->header() == "Module m" && module->span().first_line == 1 &&
            module->span().last_line == 3);
}

// --- Layout: indentation -----------------------------------------------------

void TestIndentation() {
  std::cout << "Layout: indentation\n";
  AstUnit unit;
  AstBlock* module = unit.Make<AstBlock>("Module m", SpanOf(1, 4));
  module->body().push_back(
      unit.Make<AstStatement>( "Parameter W = 8;",
                              SpanOf(2, 2)));
  AstBlock* block = unit.Make<AstBlock>( "Instance i Of T",
                                        SpanOf(3, 4));
  block->body().push_back(unit.Make<AstStatement>( "InputPort a = b;", SpanOf(4, 4)));
  module->body().push_back(block);
  unit.top_level().push_back(module);

  const std::string out =
      RenderFormatTree(BuildFormatTree(unit, DefaultStyle()), 2);
  const std::string expected =
      "Module m {\n"
      "  Parameter W = 8;\n"
      "  Instance i Of T {\n"
      "    InputPort a = b;\n"
      "  }\n"
      "}\n";
  CheckEq("nested block indents by 2 per level", expected, out);

  const std::string out4 =
      RenderFormatTree(BuildFormatTree(unit, DefaultStyle()), 4);
  Check("indent width is a render-time parameter",
        out4.find("    Parameter W = 8;") != std::string::npos, out4);
}

// --- Layout: comma list vs block ---------------------------------------------
//
// This is the case that motivated keeping layout out of the parser: a comma
// list is semantically a block, but visually must stay on one line.

void TestPortGroupStaysInline() {
  std::cout << "Layout: comma list vs block\n";
  AstUnit unit;
  AstBlock* module = unit.Make<AstBlock>("Module m", SpanOf(1, 3));
  module->body().push_back(
      unit.Make<AstStatement>("PortGroup pg {si, so, en}", SpanOf(2, 2)));
  unit.top_level().push_back(module);

  const std::string out =
      RenderFormatTree(BuildFormatTree(unit, DefaultStyle()), 2);
  Check("PortGroup renders on one line",
        out.find("PortGroup pg {si, so, en}") != std::string::npos, out);
}

// --- Layout: statements that exist as both forms -----------------------------
//
// `Alias a = b;` is an AstStatement, `Alias a = b { ... }` an AstBlock. They
// would have shared one kind tag, which is exactly why dispatch is by class.

void TestDualFormConstructs() {
  std::cout << "Layout: dual-form constructs (Alias)\n";
  AstUnit unit;
  AstBlock* module = unit.Make<AstBlock>("Module m", SpanOf(1, 4));
  module->body().push_back(
      unit.Make<AstStatement>( "Alias a = b;", SpanOf(2, 2)));
  AstBlock* with_block =
      unit.Make<AstBlock>( "Alias a = b", SpanOf(3, 4));
  with_block->body().push_back(
      unit.Make<AstStatement>( "x;", SpanOf(4, 4)));
  module->body().push_back(with_block);
  unit.top_level().push_back(module);

  const std::string out =
      RenderFormatTree(BuildFormatTree(unit, DefaultStyle()), 2);
  const std::string expected =
      "Module m {\n"
      "  Alias a = b;\n"
      "  Alias a = b {\n"
      "    x;\n"
      "  }\n"
      "}\n";
  CheckEq("statement and block forms of one kind both lay out correctly",
          expected, out);
}

// --- Layout: blank lines -----------------------------------------------------

void TestBlankLines() {
  std::cout << "Layout: blank lines\n";
  AstUnit unit;
  // Closing brace on line 6, so that no blank line is implied before it.
  AstBlock* module = unit.Make<AstBlock>("Module m", SpanOf(1, 6));
  module->body().push_back(unit.Make<AstStatement>(
                                                   "a;", SpanOf(2, 2)));
  // Line 5: two blank lines in the source (lines 3 and 4).
  module->body().push_back(unit.Make<AstStatement>(
                                                   "b;", SpanOf(5, 5)));
  unit.top_level().push_back(module);

  const std::string out =
      RenderFormatTree(BuildFormatTree(unit, DefaultStyle()), 2);
  Check("a source blank line is preserved",
        out.find("a;\n\n  b;") != std::string::npos, out);

  FormatStyle capped = DefaultStyle();
  capped.max_blank_lines = 1;
  const std::string capped_out =
      RenderFormatTree(BuildFormatTree(unit, capped), 2);
  Check("blank lines are capped at max_blank_lines",
        capped_out.find("a;\n\n\nb;") == std::string::npos, capped_out);
}

// --- Layout: trailing comments -----------------------------------------------

void TestTrailingComment() {
  std::cout << "Layout: trailing comments\n";
  AstUnit unit;
  AstBlock* module = unit.Make<AstBlock>("Module m", SpanOf(1, 3));
  module->body().push_back(unit.Make<AstStatement>(
                                                   "a;", SpanOf(2, 2)));
  module->body().push_back(
      unit.Make<AstComment>("// note", SpanOf(2, 2)));  // Same line.
  unit.top_level().push_back(module);

  const std::string out =
      RenderFormatTree(BuildFormatTree(unit, DefaultStyle()), 2);
  Check("a comment on the previous source line is joined to it",
        out.find("  a; // note") != std::string::npos, out);
}

// --- Idempotence at the tree level -------------------------------------------

void TestRenderIsStable() {
  std::cout << "Layout: render stability\n";
  AstUnit unit;
  AstBlock* module = unit.Make<AstBlock>("Module m", SpanOf(1, 5));
  module->body().push_back(unit.Make<AstStatement>(
                                                   "a;", SpanOf(2, 2)));
  module->body().push_back(unit.Make<AstComment>("/* x\ny */", SpanOf(3, 4)));
  module->body().push_back(unit.Make<AstStatement>(
                                                   "b;", SpanOf(5, 5)));
  unit.top_level().push_back(module);

  const std::vector<FormatNode> lines = BuildFormatTree(unit, DefaultStyle());
  const std::string first = RenderFormatTree(lines, 2);
  const std::string second = RenderFormatTree(lines, 2);
  Check("rendering the same tree twice is identical", first == second);
}

// The escaping table in SerializeString. Reached through Json::String rather
// than the server, because no LSP response ever carries these characters —
// ICL with a control character in it is rejected by the scanner long before a
// document is formatted. They are still part of JSON, so they are tested here
// instead of left unexercised.
void TestJsonEscapes() {
  Check("serialize escapes a double quote",
        Json::String("a\"b").Serialize() == "\"a\\\"b\"");
  Check("serialize escapes a backslash",
        Json::String("a\\b").Serialize() == "\"a\\\\b\"");
  Check("serialize escapes a backspace",
        Json::String("a\bb").Serialize() == "\"a\\bb\"");
  Check("serialize escapes a form feed",
        Json::String("a\fb").Serialize() == "\"a\\fb\"");
  Check("serialize escapes a carriage return",
        Json::String("a\rb").Serialize() == "\"a\\rb\"");
  Check("serialize escapes a newline",
        Json::String("a\nb").Serialize() == "\"a\\nb\"");
  Check("serialize escapes a tab",
        Json::String("a\tb").Serialize() == "\"a\\tb\"");
  // 0x01 is the branch no real message reaches: a control character with no
  // short escape, so it has to become \u0001.
  Check("serialize escapes any other control character",
        Json::String(std::string("a") + static_cast<char>(0x01) + "b")
            .Serialize() == "\"a\\u0001b\"");
  Check("serialize leaves printable ASCII alone",
        Json::String("plain").Serialize() == "\"plain\"");
  Check("serialize passes non-ASCII through as UTF-8",
        Json::String("\xe4\xb8\xad").Serialize() == "\"\xe4\xb8\xad\"");

  // Numbers and the container shapes, which is everything Serialize can see.
  Check("serialize an integer", Json::Int(42).Serialize() == "42");
  Check("serialize a negative integer", Json::Int(-7).Serialize() == "-7");
  Check("serialize true and false",
        Json::Bool(true).Serialize() == "true" &&
            Json::Bool(false).Serialize() == "false");
  Check("serialize null", Json::Null().Serialize() == "null");
  Check("serialize an empty array", Json::Array({}).Serialize() == "[]");
  Check("serialize an array of one element",
        Json::Array({Json::Int(1)}).Serialize() == "[1]");
  // Two elements and more: the comma between them is the branch no server
  // response reaches. Both arrays the LSP sends — diagnostics and edits —
  // hold at most one entry, because a parse stops at the first error and a
  // document is replaced in a single edit. The separator is still part of
  // Serialize, so it is exercised here rather than left untravelled.
  Check("serialize an array of two elements",
        Json::Array({Json::Int(1), Json::Int(2)}).Serialize() == "[1,2]");
  Check("serialize an array of three elements",
        Json::Array({Json::Int(1), Json::Int(2), Json::Int(3)})
            .Serialize() == "[1,2,3]");
  Check("serialize an array of mixed element types",
        Json::Array({Json::Int(1), Json::String("s"), Json::Bool(true),
                     Json::Null()})
            .Serialize() == "[1,\"s\",true,null]");
  Check("serialize nested arrays",
        Json::Array({Json::Array({Json::Int(1), Json::Int(2)}),
                     Json::Array({Json::Int(3)})})
            .Serialize() == "[[1,2],[3]]");
  Check("serialize an object of two members",
        Json::Object({{"a", Json::Int(1)}, {"b", Json::Int(2)}})
            .Serialize() == "{\"a\":1,\"b\":2}");
  Check("serialize an object",
        Json::Object({{"a", Json::Int(1)}}).Serialize() == "{\"a\":1}");
}

// The decoder's rejection paths, which the LSP tests reach only indirectly:
// a server cannot answer a message it failed to parse, so "still serving the
// next request" is the strongest observable. Here the return value is direct.
void TestJsonRejects() {
  Json out;
  std::string error;
  Check("rejects an empty document", !Json::Parse("", &out, &error));
  // A bare value is valid JSON (RFC 8259 allows any value at the top level),
  // so this is accepted and then falls out as an unknown method.
  Check("accepts a bare value at the top level",
        Json::Parse("1", &out, &error));
  Check("rejects an unclosed object", !Json::Parse("{\"a\":1", &out, &error));
  Check("rejects a member with no value",
        !Json::Parse("{\"a\":", &out, &error));
  Check("rejects a missing colon", !Json::Parse("{\"a\" 1}", &out, &error));
  // Two members with nothing between them: the branch that ends the object
  // loop via `return false` rather than `,`/`}`.
  Check("rejects two members with no comma",
        !Json::Parse("{\"a\":1 \"b\":2}", &out, &error));
  Check("rejects an unclosed array", !Json::Parse("[1", &out, &error));
  Check("rejects an array with no comma",
        !Json::Parse("[1 2]", &out, &error));
  Check("rejects an invalid \\u escape",
        !Json::Parse("{\"a\":\"\\u00ZZ\"}", &out, &error));
  Check("rejects a truncated \\u escape",
        !Json::Parse("{\"a\":\"\\u12\"}", &out, &error));
  Check("rejects an unknown escape",
        !Json::Parse("{\"a\":\"\\q\"}", &out, &error));
  Check("rejects an unterminated string",
        !Json::Parse("{\"a\":\"abc}", &out, &error));
  Check("every rejection reports why", !error.empty(), error);

  // And the shapes it accepts, including ones json.dumps never emits.
  Json parsed;
  Check("accepts \\/ as a solidus",
        Json::Parse("{\"a\":\"\\/\"}", &parsed, &error) &&
            parsed.Find("a")->string_value() == "/");
  Json pair;
  Check("accepts a surrogate pair",
        Json::Parse("{\"a\":\"\\ud83d\\ude00\"}", &pair, &error) &&
            pair.Find("a")->string_value() == "\xf0\x9f\x98\x80");
  Json lone;
  Check("accepts a high surrogate with no partner",
        Json::Parse("{\"a\":\"\\ud83d\\u0041\"}", &lone, &error));
  Json nested;
  Check("accepts an empty array",
        Json::Parse("{\"a\":[]}", &nested, &error) &&
            nested.Find("a")->elements().empty());
}

}  // namespace
}  // namespace icl

int main() {
  using namespace icl;
  std::cout << "========================================================\n";
  std::cout << "icl-format unit tests (AST + layout, no bison/flex)\n";
  std::cout << "========================================================\n";
  TestAstOwnsNodes();
  TestIndentation();
  TestPortGroupStaysInline();
  TestDualFormConstructs();
  TestBlankLines();
  TestTrailingComment();
  TestRenderIsStable();
  TestJsonEscapes();
  TestJsonRejects();
  std::cout << "========================================================\n";
  std::cout << "PASS=" << icl::g_pass << "  FAIL=" << icl::g_fail << "\n";
  std::cout << "========================================================\n";
  return icl::g_fail == 0 ? 0 : 1;
}
