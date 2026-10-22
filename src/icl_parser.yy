/* -*- mode: c++ -*-
 *
 * Copyright 2024 The ICL Format Authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Grammar for IEEE 1687-2014 (IJTAG) Instrument Connectivity Language.
 *
 * Design rule, stated up front because it explains the shape of everything
 * below: the *structural* level is strict, the *attribute* level is lenient.
 *
 *   Strict  — Module, the 13 port kinds, ScanRegister/Reg/Field, Instance,
 *             ScanMux selection tables, Connection edges, PortGroup comma
 *             lists, AccessLink sub-structures. These are what make an ICL
 *             file a network description, and it is the whole point of the
 *             tool that a non-conforming file is rejected rather than
 *             silently reformatted.
 *
 *   Lenient — inside a block, an attribute is `NAME ...;` where NAME may be
 *             any identifier *or* any ICL keyword. Needed because real ICL
 *             writes `Attribute d = "x";` inside a Field block and
 *             `DataInPort DI = 8'h00;` inside an Instance block: the attribute
 *             name is a keyword there. Encoding every attribute form rigidly
 *             would reject valid files; being lenient here loses nothing,
 *             because attribute spelling does not affect the network topology
 *             or the layout.
 */

%skeleton "lalr1.cc"
/* IELR, not LALR: with LALR the states for an attribute body and for
   LogicSignal's expression body get merged, and a lookahead that is valid in
   one looks valid in the other — producing a reduce/reduce conflict that no
   amount of rearranging removes. IELR splits those states, and the grammar
   is then conflict-free with no duplicated expression rules. */
%define lr.type ielr
%require "3.8"
%language "c++"
%define api.namespace {icl}
%define api.parser.class {IclParser}
%define api.token.constructor
%define api.value.type variant
%define parse.assert
// `detailed` turns "syntax error" into "syntax error, unexpected '}',
// expecting ';' or '{'". A formatter that rejects a file has to say why, and
// the expected-token list is what makes an LSP diagnostic actionable.
%define parse.error detailed
// Lookahead correction: detect the error as soon as the offending token is
// seen instead of after further reductions, so the reported line is the one
// the token is actually on.
%define parse.lac full
%define parse.trace
%defines
%verbose

%code requires {
#include <string>
#include <vector>

#include "icl_ast.h"

namespace icl {

class IclDriver;

}  // namespace icl
}

%param {icl::IclDriver& driver}

%code {
void icl_scan_generic();
// Declared by the generated scanner (see icl_lexer.l). It must be visible at
// the parser's call site; this block lands after the parser class definition,
// so |IclParser::symbol_type| is a complete type here.
namespace icl {
IclParser::symbol_type yylex(IclDriver& driver);
}  // namespace icl
}

%code {
#include "icl_driver.h"

namespace icl {

// Reports the (first) syntax error. The driver owns the diagnostic state (it
// used to be a file-scope global); it keeps only the earliest error because
// later ones are cascades of it.
void IclParser::error(const location& loc, const std::string& message) {
  driver.SetParseDiagnostic(loc.begin.line, message);
}

}  // namespace icl
}

/* Terminals. */
%token <std::string> IDENT "identifier"
%token <std::string> NUMBER "number"
%token <std::string> INTEGER "unsigned decimal integer"
%token <std::string> TIME "time literal"
%token <std::string> STRING "string"
%token <std::string> COMMENT "comment"
%token <std::string> PARAMREF "parameter reference"
%token <std::string> IPROCARG "<D>/<R>"
%token DCOLON "::"
%token TOIRSELECTPORT "ToIRSelectPort"
%token ADDRESSPORT "AddressPort"
%token WRITEENPORT "WriteEnPort"
%token READENPORT "ReadEnPort"
%token NAMESPACE "NameSpace"
%token USENAMESPACE "UseNameSpace"
%token LOCALPARAMETER "LocalParameter"
%token ARROW "->"
%token <std::string> OPERATOR "operator"
%token <std::string> BOOL_OP "Boolean operator"
%token <std::string> BIT_OP "bitwise operator"
%token <std::string> COMPARE_OP "comparison operator"
%token <std::string> NOT_OP "inversion operator"
%token <std::string> MATH_OP "arithmetic operator"
%token LPAREN "("
%token RPAREN ")"

/* Left associative and all at one level: `a - b - c` must group
   (a - b) - c, and there is no precedence between operators because
   the formatter reproduces them as written rather than evaluating
   them. Without this bison reports 2 shift/reduce conflicts. */
%left OPERATOR BOOL_OP COMPARE_OP NOT_OP MATH_OP

%token MODULE "Module"
%token PARAMETER "Parameter"
%token ATTRIBUTE "Attribute"
%token ALLOWBROADCAST "AllowBroadcastOnScanInterface"
%token ACCESSTOGETHER "AccessTogether"
%token ACTIVEPOLARITY "ActivePolarity"
%token ACTIVESIGNALS "ActiveSignals"
%token ADDRESSVALUE "AddressValue"
%token BSDLENTITY "BSDLEntity"
%token CAPTURESOURCE "CaptureSource"
%token CHAIN "Chain"
%token DEFAULTLOADVALUE "DefaultLoadValue"
%token DIFFERENTIALINVOF "DifferentialInvOf"
%token ENABLE "Enable"
%token FREQDIVIDER "FreqDivider"
%token FREQMULTIPLIER "FreqMultiplier"
%token PERIOD "Period"
%token INPUTPORT "InputPort"
%token PORT "Port"
%token READCALLBACK "ReadCallBack"
%token READDATASOURCE "ReadDataSource"
%token REFENUM "RefEnum"
%token RESETVALUE "ResetValue"
%token STD1149_1_2001 "STD_1149_1_2001"
%token STD1149_1_2013 "STD_1149_1_2013"
%token SCANINSOURCE "ScanInSource"
%token SOURCE "Source"
%token WRITECALLBACK "WriteCallBack"
%token WRITEDATASOURCE "WriteDataSource"
%token WRITEENSOURCE "WriteEnSource"
%token IAPPLYENDSTATE "iApplyEndState"
%token ALIAS "Alias"
%token ENUM "Enum"
%token SCANINPORT "ScanInPort"
%token SCANOUTPORT "ScanOutPort"
%token SHIFTENPORT "ShiftEnPort"
%token CAPTUREENPORT "CaptureEnPort"
%token UPDATEENPORT "UpdateEnPort"
%token SELECTPORT "SelectPort"
%token RESETPORT "ResetPort"
%token CLOCKPORT "ClockPort"
%token TOCLOCKPORT "ToClockPort"
%token TCKPORT "TCKPort"
%token TMSPORT "TMSPort"
%token TRSTPORT "TRSTPort"
%token TOTCKPORT "ToTCKPort"
%token TOTMSPORT "ToTMSPort"
%token TOTRSTPORT "ToTRSTPort"
%token TOSHIFTENPORT "ToShiftEnPort"
%token TOCAPTUREENPORT "ToCaptureEnPort"
%token TOUPDATEENPORT "ToUpdateEnPort"
%token TORESETPORT "ToResetPort"
%token TOSELECTPORT "ToSelectPort"
%token DATAINPORT "DataInPort"
%token DATAOUTPORT "DataOutPort"
%token SCANINTERFACE "ScanInterface"
/* IEEE 1687 Table 2 (IJTAG Tutorial, 3rd ed.) lists the ICL description
   elements. The ones that take a register-style body are:
     ScanRegister, DataRegister, DataMux, ClockMux, OneHotScanGroup,
     OneHotDataGroup
   `Reg`, `Field` and `OneHotScanInterface` are not in that list; they
   are accepted anyway because rejecting a file we cannot check is the
   worse failure for a formatter. */
%token SCANREGISTER "ScanRegister"
%token DATAREGISTER "DataRegister"
%token REG "Reg"
%token FIELD "Field"
%token INSTANCE "Instance"
%token <std::string> GENERIC_BODY "generic AccessLink body"
%token SCANMUX "ScanMux"
%token DATAMUX "DataMux"
%token CLOCKMUX "ClockMux"
%token ONEHOTSCANGROUP "OneHotScanGroup"
%token ONEHOTDATAGROUP "OneHotDataGroup"
%token SELECTEDBY "SelectedBy"
%token LOGICSIGNAL "LogicSignal"
%token CONNECTION "Connection"
%token PORTGROUP "PortGroup"
%token ACCESSLINK "AccessLink"
%token OF "Of"
%token OFLOWER "of"
%token INCLUDE "#include"

%token LAUNCHEDGE "LaunchEdge"

%token END 0 "end of file"

/* Punctuation. Named tokens (rather than raw character literals) let the
   scanner reject unknown characters explicitly instead of passing through a
   code the parser has no rule for. */
%token LBRACE "{"
%token RBRACE "}"
%token LBRACKET "["
%token RBRACKET "]"
%token SEMICOLON ";"
%token COLON ":"
%token COMMA ","
%token DOT "."
%token EQUALS "="

/* Non-terminals. */
%type <AstBlock*> module_def
%type <AstNode*> enum_item
%type <std::vector<AstNode*>> enum_body comment_nodes
%type <std::string> scalar_name token_gap
%type <ParsedValue> integer_expr integer_arg attribute_values attribute_atom
%type <std::string> strict_range signal data_signal concat_data_signal any_operator
%type <ParsedReference> signal_path signal_reference
%type <LogicValue> logic_expr logic_bit logic_bit_term logic_compare logic_concat logic_unary logic_integer logic_atom
%type <AstNode*> checked_declaration checked_declaration_item strict_attribute
%type <std::string> checked_declaration_keyword default_value
%type <std::string> frequency_keyword clock_factor clock_period
%type <AstNode*> interface_item interface_property chain
%type <std::vector<AstNode*>> interface_body chain_body
%type <AstNode*> group_port data_register data_register_item
%type <std::vector<AstNode*>> one_hot_scan_body one_hot_data_body
%type <AstNode*> one_hot_data_item instance_declaration instance_property address_value parameter_override
%type <std::vector<AstNode*>> instance_body
%type <std::string> instance_module_ref broadcast_names generic_link_type link_interface_ref
%type <AstNode*> access_link link_instruction link_selected_item link_interface_item link_active_item
%type <std::vector<AstNode*>> link_body link_instructions link_selected_body link_interface_body link_active_body
%type <std::vector<AstNode*>> data_register_body
%type <std::string> write_enable callback_keyword callback_namespace callback_id
%type <std::string> namespace_tail optional_subnamespace callback_args callback_arg ordinary_keyword non_tap_keyword
%type <std::vector<AstNode*>> checked_declaration_body
%type <ParsedValue> concat_number inverted_number number
%type <std::vector<AstNode*>> module_item_list
%type <AstNode*> module_item instance_item
%type <std::vector<icl::PortGroupItem>> port_group_items
%type <icl::PortGroupItem> port_group_item
%type <std::string> decl_name of_kw opt_range attr_stmt_name value_list std_tap_kw attr_value range_block_kw any_keyword
%type <std::string> value
%type <std::string> postfix
%type <std::string> primary
%type <std::string> attr_name
%type <std::vector<AstNode*>> logic_body
%type <AstNode*> ns_stmt
%type <AstNode*> attr_item
%type <std::vector<AstNode*>> attr_body
%type <std::vector<AstNode*>> strict_mux_body mux_prefix
%type <std::string> strict_mux_keyword mux_codes
%type <AstNode*> mux_selection
%type <std::vector<AstNode*>> conn_body

/* No %printer. There used to be one for <std::string>, and it was the only
   line in this file that no test ever reached: it runs only while a trace is
   being printed, and the measured build sets no debug level, so it sat at
   100% minus one forever. The trace that `grammar_coverage.py` reads is
   bison's own "Reducing stack by rule N (line L)", which does not need it —
   dropping it costs only the symbol values in a debug dump nobody reads. */

%start file

%%

/* ---------------------------------------------------------------- top level */

file
  : %empty
  | file module_def
      { driver.unit().top_level().push_back($2); }
  | file ns_stmt
      { driver.unit().top_level().push_back($2); }
  | file COMMENT
      {
        // Unlike the nested bodies below, the top-level rule has no vector to
        // append to, so the comment must be pushed onto the unit explicitly.
        driver.unit().top_level().push_back(
            driver.AddComment($2, SpanOf(@2.begin.line, @2.end.line)));
      }
  /* `#include "file.icl"`, with or without a trailing semicolon.

     Not IEEE 1687: the standard's icl_source has no such item. It is Origen's
     extension — `include_def : "#include" S path:STRING s ";"? s` in its
     treetop grammar — and 16 of its 21 example ICL files use it, so a
     formatter that cannot read the line cannot read the file.

     Kept verbatim and kept at top level, which is where that grammar puts it.
     The path is not opened: this tool formats one document and does not follow
     references. */
  | file INCLUDE STRING
      { driver.unit().top_level().push_back(
            driver.MakeStatement("#include " + $3,
                                 SpanOf(@2.begin.line, @3.end.line))); }
  | file INCLUDE STRING SEMICOLON
      { driver.unit().top_level().push_back(
            driver.MakeStatement("#include " + $3 + ";",
                                 SpanOf(@2.begin.line, @4.end.line))); }
  ;

module_def
  : MODULE decl_name LBRACE module_item_list RBRACE
      {
        $$ = driver.MakeBlock("Module " + $2, std::move($4), SpanOf(@1.begin.line, @5.end.line));
        driver.EndModule();
      }
  ;

module_item_list
  : %empty
      { $$ = std::vector<AstNode*>(); }
  | module_item_list module_item
      { $$ = std::move($1); $$.push_back($2); }
  ;

module_item
  : COMMENT { $$ = driver.AddComment($1, SpanOf(@1.begin.line, @1.end.line)); }
  | PARAMETER decl_name EQUALS value_list SEMICOLON
      { $$ = driver.MakeStatement("Parameter " + $2 + " = " + $4 + ";",
                                 SpanOf(@1.begin.line, @5.end.line)); }
  | LOCALPARAMETER decl_name EQUALS value_list SEMICOLON
      { $$ = driver.MakeStatement("LocalParameter " + $2 + " = " + $4 + ";",
                                 SpanOf(@1.begin.line, @5.end.line)); }
  | ns_stmt { $$ = $1; }
  | ATTRIBUTE decl_name EQUALS value_list SEMICOLON
      { $$ = driver.MakeStatement("Attribute " + $2 + " = " + $4 + ";",
                                 SpanOf(@1.begin.line, @5.end.line)); }
  /* Without a value: the grammar makes `= value` optional —
     attribute_def : 'Attribute' attribute_name ('=' attribute_value)? ';'
     so a bare `Attribute foo;` is legal. */
  | ATTRIBUTE decl_name SEMICOLON
      { $$ = driver.MakeStatement("Attribute " + $2 + ";",
                                  SpanOf(@1.begin.line, @3.end.line)); }
  /* allowBroadcast_def : 'AllowBroadcastOnScanInterface' scanInterface_name ';'
     Marks a scan interface as allowing broadcast writes. It takes a single
     name, not a list — scanInterface_name is one SCALAR_ID — so there is no
     comma form. */
  | ALLOWBROADCAST decl_name SEMICOLON
      { $$ = driver.MakeStatement("AllowBroadcastOnScanInterface " + $2 + ";",
                                  SpanOf(@1.begin.line, @3.end.line)); }
  /* The alias name may carry a bit range: `Alias mode[1:0] = DI[6:5];` —
     both the bare and the ranged form occur in real instrument modules. */
  /* The right side is a *concatenation*, not a single signal:
     concat_hier_data_signal : '~'? hier_data_signal (',' '~'?
     hier_data_signal)* — so `Alias MyName1[2:0] = u1.a[2], u2.u3.b[1:0];`
     and `Alias CNTL[42:0] = ADDR[7:0], DATA[31:0] { AccessTogether; }` are
     both standard forms, and both were rejected at the comma. */
  | ALIAS decl_name opt_range EQUALS value_list SEMICOLON
      { $$ = driver.MakeStatement("Alias " + $2 + $3 + " = " + $5 + ";",
                                 SpanOf(@1.begin.line, @6.end.line)); }
  | ALIAS decl_name opt_range EQUALS value_list LBRACE attr_body RBRACE
      {
        $$ = driver.MakeBlock("Alias " + $2 + $3 + " = " + $5, std::move($7),
                              SpanOf(@1.begin.line, @8.end.line));
      }
  /* Everything below is grouped by *shape*, not by keyword. Five shapes cover
     all 13 ICL description elements, so a new keyword is one line in the right
     group rather than two new productions — and the groups cannot drift apart,
     because there is only one copy of each shape. */
  | ENUM token_gap scalar_name token_gap LBRACE enum_body RBRACE
      { driver.CheckEnum($3, @3.begin.line);
        $$ = driver.MakeBlock("Enum " + $2 + $3 + $4, std::move($6),
                              SpanOf(@1.begin.line, @7.end.line)); }
  | SCANINTERFACE token_gap scalar_name token_gap LBRACE interface_body RBRACE
      { driver.CheckBlockItems("ScanInterface", $6, @1.begin.line);
        $$ = driver.MakeBlock("ScanInterface " + $2 + $3 + $4, std::move($6),
                              SpanOf(@1.begin.line, @7.end.line)); }
  | ONEHOTSCANGROUP token_gap scalar_name token_gap strict_range LBRACE one_hot_scan_body RBRACE
      { $$ = driver.MakeBlock("OneHotScanGroup " + $2 + $3 + $4 + $5, std::move($7),
                              SpanOf(@1.begin.line, @8.end.line)); }
  | ONEHOTDATAGROUP token_gap scalar_name token_gap strict_range LBRACE one_hot_data_body RBRACE
      { $$ = driver.MakeBlock("OneHotDataGroup " + $2 + $3 + $4 + $5, std::move($7),
                              SpanOf(@1.begin.line, @8.end.line)); }
  | range_block_kw decl_name opt_range LBRACE attr_body RBRACE
      {
        $$ = driver.MakeBlock($1 + " " + $2 + $3, std::move($5),
                              SpanOf(@1.begin.line, @6.end.line));
      }
  | checked_declaration { $$ = $1; }
  | data_register { $$ = $1; }
  | strict_mux_keyword token_gap scalar_name token_gap strict_range SELECTEDBY token_gap concat_data_signal LBRACE strict_mux_body RBRACE
      { $$ = driver.MakeBlock($1 + " " + $2 + $3 + $4 + $5 + " SelectedBy " + $7 + $8,
                              std::move($10), SpanOf(@1.begin.line, @11.end.line)); }
  // Published P1687 SIB examples omit SelectedBy (LiU, Listings 3 and 8).
  | strict_mux_keyword token_gap scalar_name token_gap strict_range IDENT token_gap LBRACE strict_mux_body RBRACE
      { if ($1 != "ScanMux") { error(@1, "SelectedBy is required"); YYERROR; }
        $$ = driver.MakeBlock("ScanMux " + $2 + $3 + $4 + $5 + " " + $6 + $7,
                              std::move($9), SpanOf(@1.begin.line, @10.end.line)); }
  | instance_item
      { $$ = $1; }
  /* LogicSignal holds a *boolean expression*, not a list of attributes:
     `LogicSignal x { a && b; }`, `LogicSignal y { enable, sib == 2'b11; }`.
     The general attribute body cannot hold either — in `a && b`, LALR(1)
     cannot tell at the operator whether `a` is an attribute name followed by
     a unary value or the start of an expression. Rather than leave that to
     bison's rule ordering, LogicSignal gets a body of its own. */
  | LOGICSIGNAL token_gap scalar_name token_gap strict_range LBRACE logic_body RBRACE
      { $$ = driver.MakeBlock("LogicSignal " + $2 + $3 + $4 + $5, std::move($7),
                              SpanOf(@1.begin.line, @8.end.line)); }
  | CONNECTION LBRACE conn_body RBRACE
      {
        $$ = driver.MakeBlock("Connection", std::move($3),
                              SpanOf(@1.begin.line, @4.end.line));
      }
  | PORTGROUP decl_name LBRACE port_group_items RBRACE
      { $$ = driver.MakePortGroup($2, std::move($4),
                                  SpanOf(@1.begin.line, @5.end.line)); }
  ;

/* IEEE 1687-2014 §6.5.3: an enumeration has one or more numeric members.
   Comments are retained as AST nodes, but do not count as members. */
scalar_name
  : decl_name { $$ = $1; }
  | INSTANCE { $$ = "Instance"; }
  | ACCESSLINK { $$ = "AccessLink"; }
  ;

token_gap
  : %empty { $$ = ""; }
  | token_gap COMMENT
      { $$ = std::move($1) + " " + $2 + ($2.compare(0, 2, "//") == 0 ? "\n" : " "); }
  ;

comment_nodes
  : %empty { $$ = {}; }
  | comment_nodes COMMENT
      { $$ = std::move($1); $$.push_back(driver.AddComment($2, SpanOf(@2.begin.line, @2.end.line))); }
  ;

enum_body
  : comment_nodes enum_item { $$ = std::move($1); $$.push_back($2); }
  | enum_body enum_item { $$ = std::move($1); $$.push_back($2); }
  | enum_body COMMENT
      { $$ = std::move($1); $$.push_back(driver.AddComment($2, SpanOf(@2.begin.line, @2.end.line))); }
  ;

enum_item
  : scalar_name token_gap EQUALS token_gap concat_number SEMICOLON
      { driver.CheckEnumMember($1, $5, @1.begin.line);
        $$ = driver.MakeStatement($1 + $2 + " = " + $4 + $5.text + ";", SpanOf(@1.begin.line, @6.end.line)); }
  ;

concat_number
  : inverted_number { $$ = $1; }
  | concat_number COMMA token_gap inverted_number { $$ = std::move($1); $$.Append($3, $4); }
  ;

inverted_number
  : number { $$ = $1; }
  | NOT_OP token_gap number
      { if ($1 != "~") { error(@1, "expected numeric inversion '~'"); YYERROR; }
        $$ = std::move($3); $$.text = $1 + $2 + $$.text; }
  ;

number
  : NUMBER token_gap { $$ = driver.NumberLiteral($1); $$.text += $2; }
  | integer_expr { $$ = std::move($1); }
  ;

/* Keep the spelling and parentheses: formatting never evaluates or regroups
   expressions. Only unsigned decimal operands participate in arithmetic. */
integer_expr
  : integer_arg { $$ = $1; }
  | integer_arg MATH_OP token_gap integer_expr
      { $$ = ParsedValue{$1.text + " " + $2 + " " + $3 + $4.text}; }
  ;

integer_arg
  : INTEGER token_gap { $$ = ParsedValue{$1 + $2}; }
  | PARAMREF token_gap { $$ = ParsedValue{$1 + $2, 0, ValueKind::Unknown}; }
  | LPAREN token_gap integer_expr RPAREN token_gap { $$ = ParsedValue{"(" + $2 + $3.text + ")" + $5}; }
  ;

/* Ports and registers share declaration shapes and property productions.
   The driver checks the permitted properties and cardinalities per kind. */
checked_declaration_keyword
  : DATAOUTPORT { $$ = "DataOutPort"; }
  | DATAINPORT { $$ = "DataInPort"; }
  | SCANINPORT { $$ = "ScanInPort"; }
  | SCANOUTPORT { $$ = "ScanOutPort"; }
  | SHIFTENPORT { $$ = "ShiftEnPort"; }
  | CAPTUREENPORT { $$ = "CaptureEnPort"; }
  | UPDATEENPORT { $$ = "UpdateEnPort"; }
  | TOSHIFTENPORT { $$ = "ToShiftEnPort"; }
  | TOCAPTUREENPORT { $$ = "ToCaptureEnPort"; }
  | TOUPDATEENPORT { $$ = "ToUpdateEnPort"; }
  | SELECTPORT { $$ = "SelectPort"; }
  | TOSELECTPORT { $$ = "ToSelectPort"; }
  | RESETPORT { $$ = "ResetPort"; }
  | TORESETPORT { $$ = "ToResetPort"; }
  | READENPORT { $$ = "ReadEnPort"; }
  | WRITEENPORT { $$ = "WriteEnPort"; }
  | ADDRESSPORT { $$ = "AddressPort"; }
  | TOIRSELECTPORT { $$ = "ToIRSelectPort"; }
  | TOTRSTPORT { $$ = "ToTRSTPort"; }
  | TRSTPORT { $$ = "TRSTPort"; }
  | TOCLOCKPORT { $$ = "ToClockPort"; }
  | CLOCKPORT { $$ = "ClockPort"; }
  | TOTCKPORT { $$ = "ToTCKPort"; }
  | TCKPORT { $$ = "TCKPort"; }
  | TOTMSPORT { $$ = "ToTMSPort"; }
  | TMSPORT { $$ = "TMSPort"; }
  | SCANREGISTER { $$ = "ScanRegister"; }
  ;

checked_declaration
  : checked_declaration_keyword token_gap scalar_name token_gap strict_range SEMICOLON
      { $$ = driver.MakeStatement($1 + " " + $2 + $3 + $4 + $5 + ";", SpanOf(@1.begin.line, @6.end.line)); }
  | checked_declaration_keyword token_gap scalar_name token_gap strict_range LBRACE checked_declaration_body RBRACE
      { driver.CheckBlockItems($1, $7, @1.begin.line);
        $$ = driver.MakeBlock($1 + " " + $2 + $3 + $4 + $5, std::move($7), SpanOf(@1.begin.line, @8.end.line)); }
  ;

checked_declaration_body
  : %empty { $$ = {}; }
  | checked_declaration_body checked_declaration_item { $$ = std::move($1); $$.push_back($2); }
  | checked_declaration_body COMMENT
      { $$ = std::move($1); $$.push_back(driver.AddComment($2, SpanOf(@2.begin.line, @2.end.line))); }
  ;

checked_declaration_item
  : SOURCE token_gap concat_data_signal SEMICOLON
      { $$ = driver.MakeStatement("Source " + $2 + $3 + ";", SpanOf(@1.begin.line, @4.end.line)); }
  | SCANINSOURCE token_gap data_signal SEMICOLON
      { $$ = driver.MakeStatement("ScanInSource " + $2 + $3 + ";", SpanOf(@1.begin.line, @4.end.line)); }
  | CAPTURESOURCE token_gap concat_data_signal SEMICOLON
      { $$ = driver.MakeStatement("CaptureSource " + $2 + $3 + ";", SpanOf(@1.begin.line, @4.end.line)); }
  | RESETVALUE token_gap default_value SEMICOLON
      { $$ = driver.MakeStatement("ResetValue " + $2 + $3 + ";", SpanOf(@1.begin.line, @4.end.line)); }
  | DIFFERENTIALINVOF token_gap concat_data_signal SEMICOLON
      { $$ = driver.MakeStatement("DifferentialInvOf " + $2 + $3 + ";", SpanOf(@1.begin.line, @4.end.line)); }
  | ENABLE token_gap data_signal SEMICOLON
      { $$ = driver.MakeStatement("Enable " + $2 + $3 + ";", SpanOf(@1.begin.line, @4.end.line)); }
  | REFENUM token_gap scalar_name token_gap SEMICOLON
      { $$ = driver.MakeStatement("RefEnum " + $2 + $3 + $4 + ";", SpanOf(@1.begin.line, @5.end.line)); }
  | DEFAULTLOADVALUE token_gap default_value SEMICOLON
      { $$ = driver.MakeStatement("DefaultLoadValue " + $2 + $3 + ";", SpanOf(@1.begin.line, @4.end.line)); }
  | ACTIVEPOLARITY token_gap INTEGER token_gap SEMICOLON
      { if ($3 != "0" && $3 != "1") { error(@3, "ActivePolarity requires 0 or 1"); YYERROR; }
        $$ = driver.MakeStatement("ActivePolarity " + $2 + $3 + $4 + ";", SpanOf(@1.begin.line, @5.end.line)); }
  | frequency_keyword token_gap clock_factor SEMICOLON
      { $$ = driver.MakeStatement($1 + " " + $2 + $3 + ";", SpanOf(@1.begin.line, @4.end.line)); }
  | PERIOD token_gap clock_period SEMICOLON
      { $$ = driver.MakeStatement("Period " + $2 + $3 + ";", SpanOf(@1.begin.line, @4.end.line)); }
  | LAUNCHEDGE token_gap IDENT token_gap SEMICOLON
      { if ($3 != "Falling") { error(@3, "unsupported legacy LaunchEdge value"); YYERROR; }
        $$ = driver.MakeStatement("LaunchEdge " + $2 + $3 + $4 + ";", SpanOf(@1.begin.line, @5.end.line)); }
  | strict_attribute { $$ = $1; }
  ;

data_register
  : DATAREGISTER token_gap scalar_name token_gap strict_range SEMICOLON
      { $$ = driver.MakeStatement("DataRegister " + $2 + $3 + $4 + $5 + ";", SpanOf(@1.begin.line, @6.end.line)); }
  | DATAREGISTER token_gap scalar_name token_gap strict_range LBRACE data_register_body RBRACE
      { driver.CheckBlockItems("DataRegister", $7, @1.begin.line);
        $$ = driver.MakeBlock("DataRegister " + $2 + $3 + $4 + $5, std::move($7), SpanOf(@1.begin.line, @8.end.line)); }
  ;

data_register_body
  : %empty { $$ = {}; }
  | data_register_body data_register_item { $$ = std::move($1); $$.push_back($2); }
  | data_register_body COMMENT
      { $$ = std::move($1); $$.push_back(driver.AddComment($2, SpanOf(@2.begin.line, @2.end.line))); }
  ;

data_register_item
  : checked_declaration_item { $$ = $1; }
  | WRITEDATASOURCE token_gap concat_data_signal SEMICOLON
      { $$ = driver.MakeStatement("WriteDataSource " + $2 + $3 + ";", SpanOf(@1.begin.line, @4.end.line)); }
  | WRITEENSOURCE token_gap write_enable SEMICOLON
      { $$ = driver.MakeStatement("WriteEnSource " + $2 + $3 + ";", SpanOf(@1.begin.line, @4.end.line)); }
  | READDATASOURCE token_gap concat_data_signal SEMICOLON
      { $$ = driver.MakeStatement("ReadDataSource " + $2 + $3 + ";", SpanOf(@1.begin.line, @4.end.line)); }
  | address_value { $$ = $1; }
  | callback_keyword token_gap callback_namespace callback_id callback_args SEMICOLON
      { $$ = driver.MakeStatement($1 + " " + $2 + $3 + " " + $4 + $5 + ";", SpanOf(@1.begin.line, @6.end.line)); }
  ;

/* §6.4.9 explicitly permits an extra inversion before data_signal. */
write_enable
  : data_signal { $$ = $1; }
  | NOT_OP token_gap NOT_OP token_gap signal
      { if ($1 != "~" || $3 != "~") { error(@1, "expected signal inversion '~'"); YYERROR; }
        $$ = $1 + $2 + $3 + $4 + $5; }
  ;

callback_keyword
  : READCALLBACK { $$ = "ReadCallBack"; }
  | WRITECALLBACK { $$ = "WriteCallBack"; }
  ;

callback_namespace
  : scalar_name token_gap namespace_tail { $$ = $1 + $2 + $3; }
  | PARAMREF token_gap optional_subnamespace { $$ = $1 + $2 + $3; }
  | DCOLON token_gap callback_id optional_subnamespace { $$ = "::" + $2 + $3 + $4; }
  ;

namespace_tail
  : %empty { $$ = ""; }
  | DCOLON token_gap callback_id optional_subnamespace { $$ = "::" + $2 + $3 + $4; }
  ;

optional_subnamespace
  : %empty { $$ = ""; }
  | DCOLON token_gap callback_id { $$ = "::" + $2 + $3; }
  ;

callback_id
  : scalar_name token_gap { $$ = $1 + $2; }
  | PARAMREF token_gap { $$ = $1 + $2; }
  ;

callback_args
  : %empty { $$ = ""; }
  | callback_args callback_arg { $$ = std::move($1) + " " + $2; }
  ;

callback_arg
  : number { $$ = $1.text; }
  | STRING token_gap { $$ = $1 + $2; }
  | IPROCARG token_gap { $$ = $1 + $2; }
  ;

frequency_keyword
  : FREQMULTIPLIER { $$ = "FreqMultiplier"; }
  | FREQDIVIDER { $$ = "FreqDivider"; }
  ;

clock_factor
  : INTEGER token_gap { $$ = $1 + $2; }
  /* Tessent v2023.4 clockGen example uses parameterized factors. */
  | PARAMREF token_gap { $$ = $1 + $2; }
  ;

clock_period
  : INTEGER token_gap { $$ = $1 + $2; }
  | TIME token_gap { $$ = $1 + $2; }
  | INTEGER token_gap IDENT token_gap
      { if ($3 != "s" && $3 != "ms" && $3 != "us" && $3 != "ns" &&
            $3 != "ps" && $3 != "fs" && $3 != "as") {
          error(@3, "invalid Period time unit"); YYERROR;
        }
        $$ = $1 + " " + $2 + $3 + $4; }
  ;

default_value
  : concat_number { $$ = $1.text; }
  | scalar_name token_gap { $$ = $1 + $2; }
  ;

strict_attribute
  : ATTRIBUTE token_gap scalar_name token_gap SEMICOLON
      { $$ = driver.MakeStatement("Attribute " + $2 + $3 + $4 + ";", SpanOf(@1.begin.line, @5.end.line)); }
  | ATTRIBUTE token_gap scalar_name token_gap EQUALS token_gap attribute_values SEMICOLON
      { $$ = driver.MakeStatement("Attribute " + $2 + $3 + $4 + " = " + $6 + $7.text + ";", SpanOf(@1.begin.line, @8.end.line)); }
  ;

attribute_values
  : attribute_atom { $$ = std::move($1); }
  | attribute_values COMMA token_gap attribute_atom
      { if ($1.kind != ValueKind::Unknown && $4.kind != ValueKind::Unknown && $1.kind != $4.kind) {
          error(@4, "cannot concatenate strings and numbers"); YYERROR;
        }
        $$ = std::move($1); $$.Append($3, $4); }
  ;

attribute_atom
  : inverted_number { $$ = std::move($1); }
  | STRING token_gap { $$ = ParsedValue{$1 + $2, 0, ValueKind::String}; }
  ;

strict_range
  : %empty { $$ = ""; }
  | LBRACKET token_gap integer_expr RBRACKET token_gap { $$ = "[" + $2 + $3.text + "]" + $5; }
  | LBRACKET token_gap integer_expr COLON token_gap integer_expr RBRACKET token_gap
      { $$ = "[" + $2 + $3.text + ":" + $5 + $6.text + "]" + $8; }
  ;

signal_path
  : scalar_name token_gap { $$ = ParsedReference{$1 + $2}; }
  | signal_path DOT token_gap scalar_name token_gap
      { $$ = std::move($1); $$.text += "." + $3 + $4 + $5; $$.is_scalar = false; }
  ;

signal_reference
  : signal_path strict_range
      { $$ = std::move($1); $$.text += $2; $$.is_scalar = $$.is_scalar && $2.empty(); }
  ;

signal
  : number { $$ = $1.text; }
  | signal_reference { $$ = $1.text; }
  ;

data_signal
  : signal { $$ = $1; }
  | NOT_OP token_gap signal
      { if ($1 != "~") { error(@1, "expected signal inversion '~'"); YYERROR; }
        $$ = $1 + $2 + $3; }
  ;

concat_data_signal
  : data_signal { $$ = $1; }
  | concat_data_signal COMMA token_gap data_signal { $$ = std::move($1) + ", " + $3 + $4; }
  ;

/* A comma-separated run of values that does not start with an operator.
   Used for attribute values, where `name - 1` must not be read as an
   expression. */
attr_value
  : postfix                    { $$ = $1; }
  | attr_value COMMA postfix   { $$ = std::move($1) + ", " + $3; }
  /* A later element may carry a unary operator of its own. Every signal type
     in the standard is written `'~'? signal` and the concatenation rules put
     that per element (`concat_data_signal : data_signal (',' data_signal)*`),
     so `Source ~a, b;` and `Source a, ~b;` are both legal and were both
     rejected — the list accepted an operator only in first position. */
  | attr_value COMMA any_operator postfix
      { $$ = std::move($1) + ", " + $3 + $4; }
  /* Legacy space-separated properties in blocks not yet constrained. */
  | attr_value postfix         { $$ = std::move($1) + " " + $2; }
  /* A binary operator inside an attribute value. `signal : (number |
     reg_port_signal_id | hier_port)` and `number : ... | integer_expr`, so
     `Source 1 + 1;` is reachable in the published grammar. Costs nothing now
     that the left side of an assignment is `postfix`: `postfix`
     cannot start with an operator, so `a + b` has one reading only. */
  | attr_value any_operator postfix
      { $$ = std::move($1) + " " + $2 + " " + $3; }
  ;

/* IEEE §6.4.10: one expression, with comments before and after it. */
logic_body
  : comment_nodes logic_expr SEMICOLON comment_nodes
      { if ($2.has_unknown) { error(@2, "unknown digits are allowed only in a comparison's right operand"); YYERROR; }
        $$ = std::move($1);
        $$.push_back(driver.MakeStatement($2.text + ";", SpanOf(@2.begin.line, @3.end.line)));
        $$.insert($$.end(), $4.begin(), $4.end()); }
  ;

logic_expr
  : logic_bit { $$ = std::move($1); }
  | logic_expr BOOL_OP token_gap logic_bit
      { $$.text = std::move($1.text) + " " + $2 + " " + $3 + $4.text;
        $$.has_unknown = $1.has_unknown || $4.has_unknown; }
  ;

logic_bit
  : logic_bit_term { $$ = std::move($1); }
  | logic_bit BIT_OP token_gap logic_bit_term
      { $$.text = std::move($1.text) + " " + $2 + " " + $3 + $4.text;
        $$.has_unknown = $1.has_unknown || $4.has_unknown; }
  ;

logic_bit_term
  : logic_compare { $$ = std::move($1); }
  | BIT_OP token_gap logic_bit_term
      { $$.text = $1 + $2 + $3.text; $$.has_unknown = $3.has_unknown; }
  ;

logic_compare
  : logic_concat { $$ = std::move($1); }
  | logic_concat COMPARE_OP token_gap logic_concat
      { if ($1.has_unknown || !($4.is_numeric_concat || $4.is_enum)) {
          error(@2, "comparison requires numeric or enum right operand and no unknown left operand"); YYERROR;
        }
        $$.text = std::move($1.text) + " " + $2 + " " + $3 + $4.text; }
  ;

logic_concat
  : logic_unary { $$ = std::move($1); }
  | logic_concat COMMA token_gap logic_unary
      { $$.text = std::move($1.text) + ", " + $3 + $4.text;
        $$.is_numeric_concat = $1.is_numeric_concat && $4.is_numeric_concat;
        $$.has_unknown = $1.has_unknown || $4.has_unknown; }
  ;

logic_unary
  : logic_integer { $$ = std::move($1); }
  | NOT_OP token_gap logic_unary
      { $$.text = $1 + $2 + $3.text;
        $$.is_numeric_concat = $1 == "~" && $3.is_number;
        $$.has_unknown = $3.has_unknown; }
  ;

logic_integer
  : logic_atom { $$ = std::move($1); }
  | logic_integer MATH_OP token_gap logic_atom
      { if (!$1.is_integer || !$4.is_integer) { error(@2, "integer arithmetic requires decimal or parameter operands"); YYERROR; }
        $$.text = std::move($1.text) + " " + $2 + " " + $3 + $4.text;
        $$.is_integer = $$.is_number = $$.is_numeric_concat = true; }
  ;

logic_atom
  : INTEGER token_gap
      { $$.text = $1 + $2; $$.is_integer = $$.is_number = $$.is_numeric_concat = true; }
  | PARAMREF token_gap
      { $$.text = $1 + $2; $$.is_integer = $$.is_number = $$.is_numeric_concat = true; }
  | NUMBER token_gap
      { $$.text = $1 + $2; $$.is_number = $$.is_numeric_concat = true;
        $$.has_unknown = $1.find_first_of("xX", $1.find('\'') + 2) != std::string::npos; }
  | signal_reference
      { $$.text = $1.text; $$.is_enum = $1.is_scalar; }
  | LPAREN token_gap logic_expr RPAREN token_gap
      { $$ = std::move($3); $$.text = "(" + $2 + $$.text + ")" + $5;
        $$.is_number = $$.is_integer; }
  ;

/* One or more comma-separated expressions. Built as a string rather than a
   vector because the only consumer is the rendered text.

   Kept separate from the `PortGroup { a, b }` list: that one has no `=` and
   lives in braces, and it also has to accept comments between the names,
   which an expression list has no business doing. Joining the two would
   only make both harder to read. */
value_list
  : value                     { $$ = $1; }
  | value_list COMMA value    { $$ = std::move($1) + ", " + $3; }
  ;

/* A range bound is any expression: a literal, a parameter reference
   (`$MSB`), or a computed bound (`r[-1]`, `top.MSB`). One rule rather
   than a list of literal forms, because any narrower set eventually
   turns out to be too narrow. */
opt_range
  : %empty                    { $$ = std::string(); }
  | LBRACKET value RBRACKET            { $$ = "[" + $2 + "]"; }
  | LBRACKET value COLON value RBRACKET { $$ = "[" + $2 + ":" + $4 + "]"; }
  ;
/* -------------------------------------------------------------------- bodies */

/* Shared attribute forms, used by both ordinary block bodies and AccessLink
   bodies. Factored out because the two bodies accepted the same forms, so
   duplicating them only made the grammar longer and easy to let drift. */
attr_item
  : attr_stmt_name SEMICOLON
      { $$ = driver.MakeStatement($1 + ";",
                                  SpanOf(@1.begin.line, @2.end.line)); }
  /* A value list, not a single value: `CaptureSource Temp, tdr[2:0];`, from
     Tessent's own output. Without this the formatter refused the file at the
     comma.

     `attr_value` rather than `value_list`: a value here may not *start* with
     an operator, so that `name - 1` is unambiguously an attribute followed by
     a value and never the start of an expression. Expression statements live
     in logic_body, which has no such restriction. */
  | attr_name attr_value SEMICOLON
      { $$ = driver.MakeStatement($1 + " " + $2 + ";",
                                  SpanOf(@1.begin.line, @3.end.line)); }
  /* An attribute whose value is a unary expression: `ResetValue -1;`,
     `Source ~a;`. Separate from the case above, and rendered with no space
     after the operator, because `- 1` and `-1` differ to the reader. */
  | attr_name any_operator value_list SEMICOLON
      { $$ = driver.MakeStatement($1 + " " + $2 + $3 + ";",
                                  SpanOf(@1.begin.line, @4.end.line)); }
  /* The left side of an assignment is an expression, not just a name:
     `InputPort datain[5:0] = toplevel_datain[15:10];` is verbatim from the
     Tessent IJTAG manual's parameterised-module example, and the previous
     `attr_name IDENT = value` form stopped dead at the `[`.

     One rule replaces the name-only and name-plus-range forms: `value`
     already covers `datain`, `datain[5:0]` and `a.b.c`. Keeping them as
     separate productions left bison unable to decide, at the `[`, whether it
     was a range on the name or an index in the expression. */
  /* `postfix`, not `value`. As the left side of an assignment only a name, a
     range or a hierarchical name ever appears (`InputPort datain[5:0] = ...`),
     and narrowing it here is what lets an attribute *value* contain a binary
     operator: with `value` on both sides the parser could not decide, at
     `name postfix` with an operator ahead, whether it was reducing the left
     side of an assignment or the first operand of an expression. */
  | attr_name postfix EQUALS value_list SEMICOLON
      { $$ = driver.MakeStatement($1 + " " + $2 + " = " + $4 + ";",
                                  SpanOf(@1.begin.line, @5.end.line)); }
  /* A *named* nested block. IEEE 1687's formal syntax requires it for `Chain`:
     scanInterfaceChain_def : 'Chain' scanInterfaceChain_name '{'
     scanInterfaceChain_item+ '}' — a ScanInterface with several scan chains
     declares one Chain per chain.
     This form was once added on speculation and withdrawn for want of
     evidence; the standard's own BNF is the evidence. */
  | attr_name postfix LBRACE attr_body RBRACE
      {
        // `postfix`, not `IDENT opt_range`: the name may be indexed
        // (`Field f[3:0] { ... }`), and using the same nonterminal as an
        // expression means the parser decides between "block" and
        // "statement" at `{` versus `;` — one token. Spelling it
        // `IDENT opt_range` instead makes it decide at `[`, which is one
        // token too early, and that is a real shift/reduce conflict: both
        // `Source a[3:0];` and `Field f[3:0] { }` start the same way.
        $$ = driver.MakeBlock($1 + " " + $2, std::move($4),
                              SpanOf(@1.begin.line, @5.end.line));
      }
  ;



interface_property
  : strict_attribute { $$ = $1; }
  | PORT token_gap scalar_name token_gap strict_range SEMICOLON
      { $$ = driver.MakeStatement("Port " + $2 + $3 + $4 + $5 + ";", SpanOf(@1.begin.line, @6.end.line)); }
  | DEFAULTLOADVALUE token_gap concat_number SEMICOLON
      { $$ = driver.MakeStatement("DefaultLoadValue " + $2 + $3.text + ";", SpanOf(@1.begin.line, @4.end.line)); }
  ;

chain
  : CHAIN token_gap scalar_name token_gap LBRACE chain_body RBRACE
      { driver.CheckChainName($3, @3.begin.line);
        driver.CheckBlockItems("Chain", $6, @1.begin.line);
        $$ = driver.MakeBlock("Chain " + $2 + $3 + $4, std::move($6), SpanOf(@1.begin.line, @7.end.line)); }
  ;

chain_body
  : comment_nodes interface_property { $$ = std::move($1); $$.push_back($2); }
  | chain_body interface_property { $$ = std::move($1); $$.push_back($2); }
  | chain_body COMMENT
      { $$ = std::move($1); $$.push_back(driver.AddComment($2, SpanOf(@2.begin.line, @2.end.line))); }
  ;

interface_item
  : interface_property { $$ = $1; }
  | chain { $$ = $1; }
  ;

interface_body
  : comment_nodes interface_item { $$ = std::move($1); $$.push_back($2); }
  | interface_body interface_item { $$ = std::move($1); $$.push_back($2); }
  | interface_body COMMENT
      { $$ = std::move($1); $$.push_back(driver.AddComment($2, SpanOf(@2.begin.line, @2.end.line))); }
  ;


group_port
  : PORT token_gap concat_data_signal SEMICOLON
      { $$ = driver.MakeStatement("Port " + $2 + $3 + ";", SpanOf(@1.begin.line, @4.end.line)); }
  ;

one_hot_scan_body
  : comment_nodes group_port { $$ = std::move($1); $$.push_back($2); }
  | one_hot_scan_body group_port { $$ = std::move($1); $$.push_back($2); }
  | one_hot_scan_body COMMENT
      { $$ = std::move($1); $$.push_back(driver.AddComment($2, SpanOf(@2.begin.line, @2.end.line))); }
  ;


one_hot_data_item
  : group_port { $$ = $1; }
  | data_register { $$ = $1; }
  | instance_declaration { $$ = $1; }
  ;

one_hot_data_body
  : comment_nodes one_hot_data_item { $$ = std::move($1); $$.push_back($2); }
  | one_hot_data_body one_hot_data_item { $$ = std::move($1); $$.push_back($2); }
  | one_hot_data_body COMMENT
      { $$ = std::move($1); $$.push_back(driver.AddComment($2, SpanOf(@2.begin.line, @2.end.line))); }
  ;


strict_mux_keyword
  : SCANMUX { $$ = "ScanMux"; }
  | DATAMUX { $$ = "DataMux"; }
  | CLOCKMUX { $$ = "ClockMux"; }
  ;

// At least one selection is required; attributes are a Tessent extension.
mux_prefix
  : %empty { $$ = {}; }
  | mux_prefix COMMENT
      { $$ = std::move($1); $$.push_back(driver.AddComment($2, SpanOf(@2.begin.line, @2.end.line))); }
  | mux_prefix strict_attribute { $$ = std::move($1); $$.push_back($2); }
  ;

strict_mux_body
  : mux_prefix mux_selection { $$ = std::move($1); $$.push_back($2); }
  | strict_mux_body mux_selection { $$ = std::move($1); $$.push_back($2); }
  | strict_mux_body strict_attribute { $$ = std::move($1); $$.push_back($2); }
  | strict_mux_body COMMENT
      { $$ = std::move($1); $$.push_back(driver.AddComment($2, SpanOf(@2.begin.line, @2.end.line))); }
  ;

mux_selection
  : mux_codes COLON token_gap concat_data_signal SEMICOLON
      { $$ = driver.MakeStatement($1 + " : " + $3 + $4 + ";", SpanOf(@1.begin.line, @5.end.line)); }
  ;

mux_codes
  : concat_number { $$ = std::move($1.text); }
  | mux_codes BIT_OP token_gap concat_number
      { if ($2 != "|") { error(@2, "expected '|' between selection codes"); YYERROR; }
        $$ = std::move($1); $$ += " | " + $3 + $4.text; }
  ;


conn_body
  : %empty
      { $$ = std::vector<AstNode*>(); }
  | conn_body COMMENT
      { $$ = std::move($1); $$.push_back(driver.AddComment($2, SpanOf(@2.begin.line, @2.end.line))); }
  | conn_body value_list SEMICOLON
      { $$ = std::move($1);
        $$.push_back(driver.MakeStatement($2 + ";",
                                          SpanOf(@2.begin.line, @3.end.line))); }
  ;

/* 1149.1 sub-structures inside `AccessLink ... { ... }`. */
/* One body rule for every `HEADER { ... }` construct, AccessLink included.
   There used to be a second, block-less variant for ordinary blocks, but
   LogicSignal and friends nest a block inside an attribute position
   (`Select { ... }`), so the nesting form is the general one. */
attr_body
  : %empty
      { $$ = std::vector<AstNode*>(); }
  | attr_body COMMENT
      { $$ = std::move($1);
        $$.push_back(driver.AddComment($2,
                                       SpanOf(@2.begin.line, @2.end.line))); }
  | attr_body attr_item
      { $$ = std::move($1); $$.push_back($2); }

  ;

/* The contents of a `PortGroup { ... }`: port names and comments, in any
   order.

   A name may carry a range, because a port name may:
       port_name : SCALAR_ID | vector_id ;
       vector_id : SCALAR_ID '[' (index | rangex) ']' ;
   so `PortGroup g { si[3:0] }` names the same port that
   `ScanInterface si { Port si[3:0]; }` names — and that second form was
   already accepted. The two disagreed until this was noticed.

   A comment inside the list used to be a syntax error — `PortGroup p { a,
   // c\n b }` was rejected — which is wrong on two counts: a comment can
   appear wherever whitespace can, and a formatter must not lose content.

   Identifier runs stay joined (`a, b`) so the common case still formats on one
   line. A comment forces the group onto several lines, because `//` cannot be
   kept inline without commenting out the closing brace.
   ponytail: comments are emitted before the identifiers rather than in their
   original interleaving — reordering within a port group changes nothing, and
   keeping the order would mean a node type per element. */
port_group_items
  : port_group_item
      { $$ = std::vector<icl::PortGroupItem>(); $$.push_back(std::move($1)); }
  | port_group_items port_group_item
      { $$ = std::move($1); $$.push_back(std::move($2)); }
  ;

port_group_item
  : decl_name opt_range
      { $$ = icl::PortGroupItem::Name($1 + $2,
                                      SpanOf(@1.begin.line, @2.end.line)); }
  | COMMENT
      { $$ = icl::PortGroupItem::Comment(
            driver.AddComment($1, SpanOf(@1.begin.line, @1.end.line)),
            SpanOf(@1.begin.line, @1.end.line)); }
  | COMMA
      { $$ = icl::PortGroupItem::Comma(); }
  ;

/* An attribute name may be an identifier or any ICL keyword: real ICL files
   write `Attribute d = "x";` inside a Field block and
   `DataInPort DI = 8'h00;` inside an Instance block.

   ponytail: accepting any keyword means a *misspelled attribute name* is not
   an error — only structure is checked. Upgrade path: if attribute spellings
   need checking, enumerate the legal ones per enclosing block instead of
   allowing every keyword here. */
/* The name in a bare `name;` statement. Wider than `attr_name`: it must also
   accept a qualified or indexed name, because real ICL writes a bare
   hierarchical name as a statement — `ScanInterface { Top.Gateway; }`, from a
   ScanWorks example. `attr_name` deliberately stays narrow (`IDENT | keyword`)
   so that `attr_name value ;` is unambiguous.

   One nonterminal rather than two competing rules (`attr_name ;` plus
   `value ;`): those reduce the same input two ways, and a bare identifier
   would be a genuine reduce/reduce conflict. Here `IDENT` has exactly one
   path. */
attr_stmt_name
  : ordinary_keyword { $$ = $1; }
  | postfix     { $$ = $1; }
  ;

/* Any keyword may be an attribute name or a nested-block header. Real ICL
   writes `DataInPort DI = 8'h00;` inside an Instance and `Attribute d = "x";`
   inside a Field, so restricting this to identifiers rejects valid files.
   Spelling is therefore not checked — only structure (see the ponytail note
   above).
   Built from the groups above rather than a second hand-written list: the
   first version of this rule listed the keywords itself and silently drifted
   out of sync, so `ClockPort clk = x;` inside an Instance was rejected while
   `ClockPort clk;` at module level was accepted. */
/* A declared name: an identifier, or any ICL keyword.
   The reference grammar says so in a comment at the top of icl.g4:
     "ANTLR4 has all keywords reserved. Since ICL keywords are not
      reserved, an additional rule will be required to properly handle."
   `attr_name` already accepted keywords inside blocks (§5.32); this brings
   the *declaration* positions — port names, module names, register names,
   mux names — to the same rule, so `ScanInPort Source;` is a name like any
   other rather than a syntax error.
*/
decl_name
  : IDENT        { $$ = $1; }
  | any_keyword  { $$ = $1; }
  ;

attr_name
  : IDENT        { $$ = $1; }
  | ordinary_keyword  { $$ = $1; }
  ;


// Instance is reusable inside OneHotDataGroup; AccessLink is not.
instance_declaration
  : INSTANCE token_gap scalar_name token_gap of_kw token_gap instance_module_ref SEMICOLON
      { $$ = driver.MakeStatement("Instance " + $2 + $3 + $4 + " " + $5 + " " + $6 + $7 + ";",
                                  SpanOf(@1.begin.line, @8.end.line)); }
  | INSTANCE token_gap scalar_name token_gap of_kw token_gap instance_module_ref LBRACE instance_body RBRACE
      { driver.CheckBlockItems("Instance", $9, @1.begin.line);
        $$ = driver.MakeBlock("Instance " + $2 + $3 + $4 + " " + $5 + " " + $6 + $7, std::move($9),
                              SpanOf(@1.begin.line, @10.end.line)); }
  ;

instance_module_ref
  : scalar_name token_gap { $$ = $1 + $2; }
  | scalar_name token_gap DCOLON token_gap scalar_name token_gap
      { $$ = $1 + $2 + "::" + $4 + $5 + $6; }
  | DCOLON token_gap scalar_name token_gap { $$ = "::" + $2 + $3 + $4; }
  ;

instance_body
  : %empty { $$ = {}; }
  | instance_body instance_property { $$ = std::move($1); $$.push_back($2); }
  | instance_body COMMENT
      { $$ = std::move($1); $$.push_back(driver.AddComment($2, SpanOf(@2.begin.line, @2.end.line))); }
  ;

instance_property
  : strict_attribute { $$ = $1; }
  | parameter_override { $$ = $1; }
  | address_value { $$ = $1; }
  | INPUTPORT token_gap scalar_name token_gap strict_range EQUALS token_gap concat_data_signal SEMICOLON
      { $$ = driver.MakeStatement("InputPort " + $2 + $3 + $4 + $5 + " = " + $7 + $8 + ";",
                                  SpanOf(@1.begin.line, @9.end.line)); }
  | ALLOWBROADCAST token_gap broadcast_names SEMICOLON
      { $$ = driver.MakeStatement("AllowBroadcastOnScanInterface " + $2 + $3 + ";", SpanOf(@1.begin.line, @4.end.line)); }
  ;

// The normative generic example and Inst4 example both allow comma lists.
broadcast_names
  : scalar_name token_gap { $$ = $1 + $2; }
  | broadcast_names COMMA token_gap scalar_name token_gap { $$ = std::move($1) + ", " + $3 + $4 + $5; }
  ;

parameter_override
  : PARAMETER token_gap scalar_name token_gap EQUALS token_gap attribute_values SEMICOLON
      { $$ = driver.MakeStatement("Parameter " + $2 + $3 + $4 + " = " + $6 + $7.text + ";",
                                  SpanOf(@1.begin.line, @8.end.line)); }
  ;

// Tessent OneHotDataGroup and Instance examples use concatenated addresses.
address_value
  : ADDRESSVALUE token_gap concat_number SEMICOLON
      { $$ = driver.MakeStatement("AddressValue " + $2 + $3.text + ";", SpanOf(@1.begin.line, @4.end.line)); }
  ;

instance_item
  : instance_declaration { $$ = $1; }
  | access_link { $$ = $1; }
  ;

access_link
  : ACCESSLINK token_gap scalar_name token_gap OF token_gap std_tap_kw token_gap LBRACE link_body RBRACE
      { driver.CheckAccessLink($3, @3.begin.line);
        $$ = driver.MakeBlock("AccessLink " + $2 + $3 + $4 + " Of " + $6 + $7 + $8,
                              std::move($10), SpanOf(@1.begin.line, @11.end.line)); }
  | ACCESSLINK token_gap scalar_name token_gap OF token_gap generic_link_type token_gap generic_open GENERIC_BODY
      { driver.CheckAccessLink($3, @3.begin.line);
        // Generic controller contents have implementation-defined semantics.
        $$ = driver.MakeStatement("AccessLink " + $2 + $3 + $4 + " Of " + $6 + $7 + $8 + " {" + $10,
                                  SpanOf(@1.begin.line, @10.end.line)); }
  ;

generic_open
  : LBRACE { ::icl_scan_generic(); }
  ;

generic_link_type
  : IDENT { $$ = $1; }
  | non_tap_keyword { $$ = $1; }
  | DATAREGISTER { $$ = "DataRegister"; }
  | INSTANCE { $$ = "Instance"; }
  | ACCESSLINK { $$ = "AccessLink"; }
  ;

link_body
  : comment_nodes BSDLENTITY token_gap scalar_name token_gap SEMICOLON link_instructions
      { $$ = std::move($1);
        $$.push_back(driver.MakeStatement("BSDLEntity " + $3 + $4 + $5 + ";", SpanOf(@2.begin.line, @6.end.line)));
        $$.insert($$.end(), $7.begin(), $7.end()); }
  ;

link_instructions
  : comment_nodes link_instruction { $$ = std::move($1); $$.push_back($2); }
  | link_instructions link_instruction { $$ = std::move($1); $$.push_back($2); }
  | link_instructions COMMENT
      { $$ = std::move($1); $$.push_back(driver.AddComment($2, SpanOf(@2.begin.line, @2.end.line))); }
  ;

link_instruction
  : scalar_name token_gap LBRACE link_selected_body RBRACE
      { driver.CheckInstructionName($1, @1.begin.line);
        $$ = driver.MakeBlock($1 + $2, std::move($4), SpanOf(@1.begin.line, @5.end.line)); }
  ;

link_selected_body
  : comment_nodes link_selected_item { $$ = std::move($1); $$.push_back($2); }
  | link_selected_body link_selected_item { $$ = std::move($1); $$.push_back($2); }
  | link_selected_body COMMENT
      { $$ = std::move($1); $$.push_back(driver.AddComment($2, SpanOf(@2.begin.line, @2.end.line))); }
  ;

link_selected_item
  : SCANINTERFACE token_gap LBRACE link_interface_body RBRACE
      { $$ = driver.MakeBlock("ScanInterface " + $2, std::move($4), SpanOf(@1.begin.line, @5.end.line)); }
  | ACTIVESIGNALS token_gap LBRACE link_active_body RBRACE
      { $$ = driver.MakeBlock("ActiveSignals " + $2, std::move($4), SpanOf(@1.begin.line, @5.end.line)); }
  ;

link_interface_body
  : comment_nodes link_interface_item { $$ = std::move($1); $$.push_back($2); }
  | link_interface_body link_interface_item { $$ = std::move($1); $$.push_back($2); }
  | link_interface_body COMMENT
      { $$ = std::move($1); $$.push_back(driver.AddComment($2, SpanOf(@2.begin.line, @2.end.line))); }
  ;

link_interface_item
  : link_interface_ref SEMICOLON
      { $$ = driver.MakeStatement($1 + ";", SpanOf(@1.begin.line, @2.end.line)); }
  ;

link_interface_ref
  : scalar_name token_gap { $$ = $1 + $2; }
  | scalar_name token_gap DOT token_gap scalar_name token_gap { $$ = $1 + $2 + "." + $4 + $5 + $6; }
  ;

link_active_body
  : comment_nodes link_active_item { $$ = std::move($1); $$.push_back($2); }
  | link_active_body link_active_item { $$ = std::move($1); $$.push_back($2); }
  | link_active_body COMMENT
      { $$ = std::move($1); $$.push_back(driver.AddComment($2, SpanOf(@2.begin.line, @2.end.line))); }
  ;

link_active_item
  : scalar_name token_gap strict_range SEMICOLON
      { $$ = driver.MakeStatement($1 + $2 + $3 + ";", SpanOf(@1.begin.line, @4.end.line)); }
  ;

/* ------------------------------------------------- keyword groups by shape */


range_block_kw
  : REG                  { $$ = "Reg"; }
  | FIELD                { $$ = "Field"; }
  ;

/* `NameSpace x;` and `UseNameSpace x;` — declared at file scope and, for
   UseNameSpace, inside a module too. The name is optional: IEEE 1687-2014
   Appendix E writes both `NameSpace X;` and a bare `NameSpace;`. */
ns_stmt
  : NAMESPACE SEMICOLON          { $$ = driver.MakeStatement("NameSpace;",
                                     SpanOf(@1.begin.line, @2.end.line)); }
  | NAMESPACE IDENT SEMICOLON    { $$ = driver.MakeStatement("NameSpace " + $2 + ";",
                                     SpanOf(@1.begin.line, @3.end.line)); }
  | USENAMESPACE SEMICOLON       { $$ = driver.MakeStatement("UseNameSpace;",
                                     SpanOf(@1.begin.line, @2.end.line)); }
  | USENAMESPACE IDENT SEMICOLON { $$ = driver.MakeStatement("UseNameSpace " + $2 + ";",
                                     SpanOf(@1.begin.line, @3.end.line)); }
  ;

/* A module reference: `TDR`, or `ns::TDR`. IEEE 1687-2014 Appendix E writes
   `Instance aa Of X::x {`. */
/* The module a construct refers to, optionally namespace-qualified.

   Left recursive, like `value`, so `a::b::c` parses: a namespace may nest and
   there is no reason to stop at two segments. It used to allow exactly one
   `::`, which meant `Instance i Of a::b::c;` was rejected while
   `Source a::b::c;` — going through `value` — was accepted. Same shape, two
   answers; now there is one. */
/* The two standard TAP module names. `AccessLink t Of STD_1149_1_2001 { ... }`
   names a module, and the name is spelled as a reserved word — so once the
   scanner recognised it as a keyword, `module_ref` (which is identifiers only)
   stopped matching and three corpus files broke. They are a group of one
   shape, in one place, for the same reason every other keyword is grouped. */
std_tap_kw
  : STD1149_1_2001 { $$ = "STD_1149_1_2001"; }
  | STD1149_1_2013 { $$ = "STD_1149_1_2013"; }
  ;

/* `Of`, and the lowercase spelling.

   The published grammar writes `'Of'` alone, and so does the standard. But
   Origen's ICL grammar — an independent implementation, and the interface the
   upstream repository lists under "Repositories associated with IJTAG" —
   spells the alternative out:

       instance_def : 'Instance' S name S ('Of' / 'of') S mod ...

   and three of its 21 example files use the lowercase form. Rejecting them
   rejects a real file. The spelling is preserved, not normalised: the
   formatter reproduces what was written.

   A group rather than two inline alternatives, so the two instance rules
   (`;` and `{ }`) each mention it once and cannot fall out of step. */
of_kw
  : OF      { $$ = "Of"; }
  | OFLOWER { $$ = "of"; }
  ;

/* Everything above, plus the keywords that have no block form. One place to
   add a keyword; every list that needs "any keyword" refers to this. */
any_keyword
  : ordinary_keyword { $$ = $1; }
  | DATAREGISTER { $$ = "DataRegister"; }
  ;

ordinary_keyword
  : non_tap_keyword { $$ = $1; }
  | std_tap_kw { $$ = $1; }
  ;

non_tap_keyword
  : SCANINTERFACE { $$ = "ScanInterface"; }
  | ONEHOTSCANGROUP { $$ = "OneHotScanGroup"; }
  | ONEHOTDATAGROUP { $$ = "OneHotDataGroup"; }
  | range_block_kw  { $$ = $1; }
  | strict_mux_keyword { $$ = $1; }
  /* `of_block_kw` is deliberately absent. `Instance` and `AccessLink` are
     always followed by `name Of module`, which `instance_item` already covers;
     letting them also be bare attribute names makes `Instance` ambiguous
     between the two — a real shift/reduce conflict, and the reason an instance
     inside a one-hot data group could not be accepted without this. */
  | ENUM            { $$ = "Enum"; }
  | LAUNCHEDGE      { $$ = "LaunchEdge"; }
  | checked_declaration_keyword { $$ = $1; }
  | MODULE          { $$ = "Module"; }
  | PARAMETER       { $$ = "Parameter"; }
  | ATTRIBUTE       { $$ = "Attribute"; }
  | ALLOWBROADCAST  { $$ = "AllowBroadcastOnScanInterface"; }
  | ACCESSTOGETHER  { $$ = "AccessTogether"; }
  | ACTIVEPOLARITY  { $$ = "ActivePolarity"; }
  | ACTIVESIGNALS   { $$ = "ActiveSignals"; }
  | ADDRESSVALUE    { $$ = "AddressValue"; }
  | BSDLENTITY      { $$ = "BSDLEntity"; }
  | CAPTURESOURCE   { $$ = "CaptureSource"; }
  | CHAIN           { $$ = "Chain"; }
  | DEFAULTLOADVALUE { $$ = "DefaultLoadValue"; }
  | DIFFERENTIALINVOF { $$ = "DifferentialInvOf"; }
  | ENABLE          { $$ = "Enable"; }
  | FREQDIVIDER     { $$ = "FreqDivider"; }
  | FREQMULTIPLIER  { $$ = "FreqMultiplier"; }
  | PERIOD          { $$ = "Period"; }
  | INPUTPORT       { $$ = "InputPort"; }
  | PORT            { $$ = "Port"; }
  | READCALLBACK    { $$ = "ReadCallBack"; }
  | READDATASOURCE  { $$ = "ReadDataSource"; }
  | REFENUM         { $$ = "RefEnum"; }
  | RESETVALUE      { $$ = "ResetValue"; }
  | SCANINSOURCE    { $$ = "ScanInSource"; }
  | SOURCE          { $$ = "Source"; }
  | WRITECALLBACK   { $$ = "WriteCallBack"; }
  | WRITEDATASOURCE { $$ = "WriteDataSource"; }
  | WRITEENSOURCE   { $$ = "WriteEnSource"; }
  | IAPPLYENDSTATE  { $$ = "iApplyEndState"; }
  | ALIAS           { $$ = "Alias"; }
  | SELECTEDBY      { $$ = "SelectedBy"; }
  | LOGICSIGNAL     { $$ = "LogicSignal"; }
  | CONNECTION      { $$ = "Connection"; }
  | PORTGROUP       { $$ = "PortGroup"; }
  | OF              { $$ = "Of"; }
  | OFLOWER        { $$ = "of"; }
  | LOCALPARAMETER  { $$ = "LocalParameter"; }
  | NAMESPACE       { $$ = "NameSpace"; }
  | USENAMESPACE    { $$ = "UseNameSpace"; }
  ;

/* ------------------------------------------------------------------ values */

any_operator
  : OPERATOR { $$ = $1; }
  | BOOL_OP { $$ = $1; }
  | COMPARE_OP { $$ = $1; }
  | NOT_OP { $$ = $1; }
  | MATH_OP { $$ = $1; }
  ;

value
  : postfix { $$ = $1; }
  /* A connection edge, `a -> b`, is only written inside `Connection`, but
     living here keeps one expression grammar. Split out as `conn_expr` it
     used to cost a second nonterminal and a reduce/reduce conflict. */
  | postfix ARROW postfix { $$ = $1 + " -> " + $3; }
  /* Unary: `ResetValue -1;`, `Source ~a;`. */
  | any_operator value %prec OPERATOR { $$ = $1 + $2; }
  /* Binary. No precedence table: the formatter reproduces the operators as
     written, so left-to-right grouping is enough and unambiguous. */
  | value any_operator value %prec OPERATOR { $$ = $1 + " " + $2 + " " + $3; }
  | LPAREN value_list RPAREN { $$ = "(" + $2 + ")"; }
  ;

postfix
  : primary                           { $$ = $1; }
  | postfix DOT IDENT                 { $$ = $1 + "." + $3; }
  /* Index bounds accept a parameter reference: `RegD[ $MSB:0 ]`. */
  | postfix LBRACKET value RBRACKET
      { $$ = $1 + "[" + $3 + "]"; }
  | postfix LBRACKET value COLON value RBRACKET
      { $$ = $1 + "[" + $3 + ":" + $5 + "]"; }
  ;

primary
  : IDENT    { $$ = $1; }
  | NUMBER   { $$ = $1; }
  | INTEGER  { $$ = $1; }
  | STRING   { $$ = $1; }
  | PARAMREF { $$ = $1; }
  ;

%%
