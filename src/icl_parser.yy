/* -*- mode: c++ -*-
 *
 * Copyright 2024 The ICL Format Authors.
 *
 * SPDX-License-Identifier: MIT
 * See LICENSE in the project root for license information.
 *
 * Grammar for IEEE 1687-2014 (IJTAG) Instrument Connectivity Language.
 *
 * Each IEEE block has its own legal members and multiplicities. Shared
 * productions describe identical lexical/value rules; documented compatibility
 * forms are recorded in docs/icl-conformance-review.md.
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
void IclScanGeneric();
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
  driver.SetParseDiagnostic(SpanOf(loc, loc), message);
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
%token <std::string> BOOL_OP "Boolean operator"
%token <std::string> BIT_OP "bitwise operator"
%token <std::string> COMPARE_OP "comparison operator"
%token <std::string> NOT_OP "inversion operator"
%token <std::string> MATH_OP "arithmetic operator"
%token LPAREN "("
%token RPAREN ")"

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
/* IEEE register, mux and group declarations; undocumented blocks are rejected. */
%token SCANREGISTER "ScanRegister"
%token DATAREGISTER "DataRegister"
%token INSTANCE "Instance"
%token <std::string> GENERIC_BODY "generic AccessLink body"
%token SCANMUX "ScanMux"
%token DATAMUX "DataMux"
%token CLOCKMUX "ClockMux"
%token ONEHOTSCANGROUP "OneHotScanGroup"
%token ONEHOTDATAGROUP "OneHotDataGroup"
%token SELECTEDBY "SelectedBy"
%token LOGICSIGNAL "LogicSignal"
%token ACCESSLINK "AccessLink"
%token OF "Of"
%token OFLOWER "of"
%token PORTGROUP "PortGroup"
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
%type <std::vector<icl::PortGroupItem>> port_group_items
%type <icl::PortGroupItem> port_group_item
%type <std::string> port_group_range
%type <AstBlock*> module_def
%type <AstNode*> enum_item
%type <std::vector<AstNode*>> enum_body comment_nodes
%type <std::string> scalar_name token_gap
%type <ParsedValue> integer_expr integer_arg attribute_values attribute_atom
%type <std::string> strict_range signal data_signal concat_data_signal
%type <ParsedReference> signal_path signal_reference alias_reference
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
%type <AstNode*> alias_item
%type <std::vector<AstNode*>> alias_body
%type <std::string> alias_signal alias_signals
%type <std::string> instance_module_ref broadcast_names generic_link_type link_interface_ref
%type <AstNode*> access_link link_instruction link_selected_item link_interface_item link_active_item
%type <std::vector<AstNode*>> link_body link_instructions link_selected_body link_interface_body link_active_body
%type <std::vector<AstNode*>> data_register_body
%type <std::string> write_enable callback_keyword callback_namespace callback_id
%type <std::string> namespace_tail optional_subnamespace callback_args callback_arg ordinary_keyword non_tap_keyword
%type <std::vector<AstNode*>> checked_declaration_body
%type <ParsedValue> concat_number inverted_number number based_number
%type <std::vector<AstNode*>> module_item_list
%type <AstNode*> module_item instance_item
%type <std::string> decl_name of_kw std_tap_kw any_keyword
%type <std::vector<AstNode*>> logic_body
%type <AstNode*> ns_stmt use_ns_stmt
%type <AstBlock*> module_open
%type <std::vector<AstNode*>> strict_mux_body mux_prefix
%type <std::string> strict_mux_keyword mux_codes
%type <AstNode*> mux_selection

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
  | file ns_stmt
    { driver.unit().top_level().push_back($2); }
  | file use_ns_stmt
    { driver.unit().top_level().push_back($2); }
  | file COMMENT
    {
      // Unlike the nested bodies below, the top-level rule has no vector to
      // append to, so the comment must be pushed onto the unit explicitly.
      driver.unit().top_level().push_back(
          driver.AddComment($2, SpanOf(@2, @2)));
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
    {
      driver.unit().top_level().push_back(
          driver.MakeStatement("#include " + $3, SpanOf(@2, @3)));
    }
  | file INCLUDE STRING SEMICOLON
    {
      driver.unit().top_level().push_back(
          driver.MakeStatement("#include " + $3 + ";", SpanOf(@2, @4)));
    }
  ;

module_open
  : MODULE token_gap scalar_name token_gap LBRACE
    {
      $$ = driver.OpenModule("Module " + $2 + $3 + $4, SpanOf(@1, @5));
    }
  ;

module_def
  : module_open module_item_list RBRACE
    {
      driver.CheckAddressContext($1->header(), $2);
      driver.CheckNamedValues($2, true);
      $1->body() = std::move($2);
      $1->set_span(SpanOf(@1, @3));
      $$ = $1;
      driver.EndModule();
    }
  ;

module_item_list
  : %empty
    { $$ = std::vector<AstNode*>(); }
  | module_item_list module_item
    {
      $$ = std::move($1);
      $$.push_back($2);
      driver.AppendModuleItem($2);
    }
  ;

module_item
  : COMMENT
    { $$ = driver.AddComment($1, SpanOf(@1, @1)); }
  | parameter_override
    { $$ = $1; }
  | LOCALPARAMETER token_gap scalar_name token_gap EQUALS token_gap
    attribute_values SEMICOLON
    {
      const auto text =
          "LocalParameter " + $2 + $3 + $4 + " = " + $6 + $7.text + ";";
      $$ = driver.MakeParameter("LocalParameter", $3, text, std::move($7),
                                SpanOf(@1, @8));
    }
  | use_ns_stmt
    { $$ = $1; }
  | strict_attribute
    { $$ = $1; }
  | ALIAS token_gap scalar_name token_gap strict_range EQUALS token_gap
    alias_signals SEMICOLON
    {
      $$ = driver.MakeStatement(
          "Alias " + $2 + $3 + $4 + $5 + " = " + $7 + $8 + ";", SpanOf(@1, @9));
    }
  | ALIAS token_gap scalar_name token_gap strict_range EQUALS token_gap
    alias_signals LBRACE alias_body RBRACE
    {
      driver.CheckBlockItems("Alias", $10, @1.begin.line);
      $$ = driver.MakeBlock("Alias " + $2 + $3 + $4 + $5 + " = " + $7 + $8,
                            std::move($10), SpanOf(@1, @11));
    }
  | ENUM token_gap scalar_name token_gap LBRACE enum_body RBRACE
    {
      driver.CheckEnum($3, @3.begin.line);
      $$ = driver.MakeBlock("Enum " + $2 + $3 + $4, std::move($6),
                            SpanOf(@1, @7));
    }
  | SCANINTERFACE token_gap scalar_name token_gap LBRACE interface_body RBRACE
    {
      driver.CheckBlockItems("ScanInterface", $6, @1.begin.line);
      $$ = driver.MakeBlock("ScanInterface " + $2 + $3 + $4, std::move($6),
                            SpanOf(@1, @7));
    }
  | ONEHOTSCANGROUP token_gap scalar_name token_gap strict_range LBRACE
    one_hot_scan_body RBRACE
    {
      $$ = driver.MakeBlock("OneHotScanGroup " + $2 + $3 + $4 + $5,
                            std::move($7), SpanOf(@1, @8));
    }
  | ONEHOTDATAGROUP token_gap scalar_name token_gap strict_range LBRACE
    one_hot_data_body RBRACE
    {
      $$ = driver.MakeBlock("OneHotDataGroup " + $2 + $3 + $4 + $5,
                            std::move($7), SpanOf(@1, @8));
    }
  | PORTGROUP token_gap scalar_name token_gap LBRACE port_group_items RBRACE
    {
      $$ = driver.MakePortGroup($2 + $3 + $4, std::move($6), SpanOf(@1, @7));
    }
  | checked_declaration
    { $$ = $1; }
  | data_register
    { $$ = $1; }
  | strict_mux_keyword token_gap scalar_name token_gap strict_range SELECTEDBY
    token_gap concat_data_signal LBRACE strict_mux_body RBRACE
    {
      $$ = driver.MakeBlock(
          $1 + " " + $2 + $3 + $4 + $5 + " SelectedBy " + $7 + $8,
          std::move($10), SpanOf(@1, @11));
    }
  // Published P1687 SIB examples omit SelectedBy (LiU, Listings 3 and 8).
  | strict_mux_keyword token_gap scalar_name token_gap strict_range IDENT
    token_gap LBRACE strict_mux_body RBRACE
    {
      if ($1 != "ScanMux") {
        error(@1, "SelectedBy is required");
        YYERROR;
      }
      $$ = driver.MakeBlock("ScanMux " + $2 + $3 + $4 + $5 + " " + $6 + $7,
                            std::move($9), SpanOf(@1, @10));
    }
  | instance_item
    { $$ = $1; }
  /* LogicSignal holds a *boolean expression*, not a list of attributes:
     `LogicSignal x { a && b; }`, `LogicSignal y { enable, sib == 2'b11; }`.
     The general attribute body cannot hold either — in `a && b`, LALR(1)
     cannot tell at the operator whether `a` is an attribute name followed by
     a unary value or the start of an expression. Rather than leave that to
     bison's rule ordering, LogicSignal gets a body of its own. */
  | LOGICSIGNAL token_gap scalar_name token_gap strict_range LBRACE logic_body
    RBRACE
    {
      $$ = driver.MakeBlock("LogicSignal " + $2 + $3 + $4 + $5, std::move($7),
                            SpanOf(@1, @8));
    }

  ;

/* IEEE 1687-2014 §6.5.3: an enumeration has one or more numeric members.
   Comments are retained as AST nodes, but do not count as members. */
scalar_name
  : decl_name
    { $$ = $1; }
  | INSTANCE
    { $$ = "Instance"; }
  | ACCESSLINK
    { $$ = "AccessLink"; }
  ;

token_gap
  : %empty
    { $$ = ""; }
  | token_gap COMMENT
    {
      $$ =
          std::move($1) + " " + $2 + ($2.compare(0, 2, "//") == 0 ? "\n" : " ");
    }
  ;

comment_nodes
  : %empty
    { $$ = {}; }
  | comment_nodes COMMENT
    {
      $$ = std::move($1);
      $$.push_back(driver.AddComment($2, SpanOf(@2, @2)));
    }
  ;

enum_body
  : comment_nodes enum_item
    {
      $$ = std::move($1);
      $$.push_back($2);
    }
  | enum_body enum_item
    {
      $$ = std::move($1);
      $$.push_back($2);
    }
  | enum_body COMMENT
    {
      $$ = std::move($1);
      $$.push_back(driver.AddComment($2, SpanOf(@2, @2)));
    }
  ;

enum_item
  : scalar_name token_gap EQUALS token_gap concat_number SEMICOLON
    {
      driver.CheckEnumMember($1, $5, @1.begin.line);
      $$ = driver.MakeStatement($1 + $2 + " = " + $4 + $5.text + ";",
                                SpanOf(@1, @6));
    }
  ;

concat_number
  : inverted_number
    { $$ = $1; }
  | concat_number COMMA token_gap inverted_number
    {
      $$ = std::move($1);
      $$.Append($3, $4);
    }
  ;

inverted_number
  : number
    { $$ = $1; }
  | NOT_OP token_gap number
    {
      if ($1 != "~") {
        error(@1, "expected numeric inversion '~'");
        YYERROR;
      }
      $$ = std::move($3);
      $$.text = $1 + $2 + $$.text;
      $$.inverted = true;
      $$.integer.reset();
    }
  ;

number
  : based_number
    { $$ = std::move($1); }
  | integer_expr
    { $$ = std::move($1); }
  ;

based_number
  : NUMBER token_gap
    {
      $$ = driver.NumberLiteral($1, @1.begin.line);
      $$.text += $2;
    }

  ;

/* Keep the spelling and parentheses: formatting never evaluates or regroups
   expressions. Only unsigned decimal operands participate in arithmetic. */
integer_expr
  : integer_arg
    { $$ = std::move($1); }
  | integer_expr MATH_OP token_gap integer_arg
    {
      $$ = ParsedValue{std::move($1.text) + " " + $2 + " " + $3 + $4.text};
      $$.unsized = true;
      $$.dependencies = std::move($1.dependencies);
      $$.dependencies.insert($$.dependencies.end(), $4.dependencies.begin(),
                             $4.dependencies.end());
    }
  ;

integer_arg
  : INTEGER token_gap
    {
      $$ = driver.NumberLiteral($1, @1.begin.line);
      $$.text += $2;
    }
  | PARAMREF token_gap
    {
      $$ = ParsedValue{$1 + $2, 0, ValueKind::kUnknown};
      $$.reference = $1.substr(1);
      $$.dependencies.push_back($$.reference);
    }
  | LPAREN token_gap integer_expr RPAREN token_gap
    {
      $$ = std::move($3);
      $$.text = "(" + $2 + $$.text + ")" + $5;
    }
  ;

/* Ports and registers share declaration shapes and property productions.
   The driver checks the permitted properties and cardinalities per kind. */
checked_declaration_keyword
  : DATAOUTPORT
    { $$ = "DataOutPort"; }
  | DATAINPORT
    { $$ = "DataInPort"; }
  | SCANINPORT
    { $$ = "ScanInPort"; }
  | SCANOUTPORT
    { $$ = "ScanOutPort"; }
  | SHIFTENPORT
    { $$ = "ShiftEnPort"; }
  | CAPTUREENPORT
    { $$ = "CaptureEnPort"; }
  | UPDATEENPORT
    { $$ = "UpdateEnPort"; }
  | TOSHIFTENPORT
    { $$ = "ToShiftEnPort"; }
  | TOCAPTUREENPORT
    { $$ = "ToCaptureEnPort"; }
  | TOUPDATEENPORT
    { $$ = "ToUpdateEnPort"; }
  | SELECTPORT
    { $$ = "SelectPort"; }
  | TOSELECTPORT
    { $$ = "ToSelectPort"; }
  | RESETPORT
    { $$ = "ResetPort"; }
  | TORESETPORT
    { $$ = "ToResetPort"; }
  | READENPORT
    { $$ = "ReadEnPort"; }
  | WRITEENPORT
    { $$ = "WriteEnPort"; }
  | ADDRESSPORT
    { $$ = "AddressPort"; }
  | TOIRSELECTPORT
    { $$ = "ToIRSelectPort"; }
  | TOTRSTPORT
    { $$ = "ToTRSTPort"; }
  | TRSTPORT
    { $$ = "TRSTPort"; }
  | TOCLOCKPORT
    { $$ = "ToClockPort"; }
  | CLOCKPORT
    { $$ = "ClockPort"; }
  | TOTCKPORT
    { $$ = "ToTCKPort"; }
  | TCKPORT
    { $$ = "TCKPort"; }
  | TOTMSPORT
    { $$ = "ToTMSPort"; }
  | TMSPORT
    { $$ = "TMSPort"; }
  | SCANREGISTER
    { $$ = "ScanRegister"; }
  ;

checked_declaration
  : checked_declaration_keyword token_gap scalar_name token_gap strict_range
    SEMICOLON
    {
      $$ = driver.MakeStatement($1 + " " + $2 + $3 + $4 + $5 + ";",
                                SpanOf(@1, @6));
    }
  | checked_declaration_keyword token_gap scalar_name token_gap strict_range
    LBRACE checked_declaration_body RBRACE
    {
      driver.CheckBlockItems($1, $7, @1.begin.line);
      $$ = driver.MakeBlock($1 + " " + $2 + $3 + $4 + $5, std::move($7),
                            SpanOf(@1, @8));
    }
  ;

checked_declaration_body
  : %empty
    { $$ = {}; }
  | checked_declaration_body checked_declaration_item
    {
      $$ = std::move($1);
      $$.push_back($2);
    }
  | checked_declaration_body COMMENT
    {
      $$ = std::move($1);
      $$.push_back(driver.AddComment($2, SpanOf(@2, @2)));
    }
  ;

checked_declaration_item
  : SOURCE token_gap concat_data_signal SEMICOLON
    {
      $$ = driver.MakeStatement("Source " + $2 + $3 + ";", SpanOf(@1, @4));
    }
  | SCANINSOURCE token_gap data_signal SEMICOLON
    {
      $$ =
          driver.MakeStatement("ScanInSource " + $2 + $3 + ";", SpanOf(@1, @4));
    }
  | CAPTURESOURCE token_gap concat_data_signal SEMICOLON
    {
      $$ = driver.MakeStatement("CaptureSource " + $2 + $3 + ";",
                                SpanOf(@1, @4));
    }
  | RESETVALUE token_gap default_value SEMICOLON
    {
      $$ = driver.MakeStatement("ResetValue " + $2 + $3 + ";", SpanOf(@1, @4));
    }
  | DIFFERENTIALINVOF token_gap concat_data_signal SEMICOLON
    {
      $$ = driver.MakeStatement("DifferentialInvOf " + $2 + $3 + ";",
                                SpanOf(@1, @4));
    }
  | ENABLE token_gap data_signal SEMICOLON
    {
      $$ = driver.MakeStatement("Enable " + $2 + $3 + ";", SpanOf(@1, @4));
    }
  | REFENUM token_gap scalar_name token_gap SEMICOLON
    {
      $$ =
          driver.MakeStatement("RefEnum " + $2 + $3 + $4 + ";", SpanOf(@1, @5));
    }
  | DEFAULTLOADVALUE token_gap default_value SEMICOLON
    {
      $$ = driver.MakeStatement("DefaultLoadValue " + $2 + $3 + ";",
                                SpanOf(@1, @4));
    }
  | ACTIVEPOLARITY token_gap INTEGER token_gap SEMICOLON
    {
      if ($3 != "0" && $3 != "1") {
        error(@3, "ActivePolarity requires 0 or 1");
        YYERROR;
      }
      $$ = driver.MakeStatement("ActivePolarity " + $2 + $3 + $4 + ";",
                                SpanOf(@1, @5));
    }
  | frequency_keyword token_gap clock_factor SEMICOLON
    {
      $$ = driver.MakeStatement($1 + " " + $2 + $3 + ";", SpanOf(@1, @4));
    }
  | PERIOD token_gap clock_period SEMICOLON
    {
      $$ = driver.MakeStatement("Period " + $2 + $3 + ";", SpanOf(@1, @4));
    }
  | LAUNCHEDGE token_gap IDENT token_gap SEMICOLON
    {
      if ($3 != "Falling") {
        error(@3, "unsupported legacy LaunchEdge value");
        YYERROR;
      }
      $$ = driver.MakeStatement("LaunchEdge " + $2 + $3 + $4 + ";",
                                SpanOf(@1, @5));
    }
  | strict_attribute
    { $$ = $1; }
  ;

data_register
  : DATAREGISTER token_gap scalar_name token_gap strict_range SEMICOLON
    {
      $$ = driver.MakeStatement("DataRegister " + $2 + $3 + $4 + $5 + ";",
                                SpanOf(@1, @6));
    }
  | DATAREGISTER token_gap scalar_name token_gap strict_range LBRACE
    data_register_body RBRACE
    {
      driver.CheckBlockItems("DataRegister", $7, @1.begin.line);
      $$ = driver.MakeBlock("DataRegister " + $2 + $3 + $4 + $5, std::move($7),
                            SpanOf(@1, @8));
    }
  ;

data_register_body
  : %empty
    { $$ = {}; }
  | data_register_body data_register_item
    {
      $$ = std::move($1);
      $$.push_back($2);
    }
  | data_register_body COMMENT
    {
      $$ = std::move($1);
      $$.push_back(driver.AddComment($2, SpanOf(@2, @2)));
    }
  ;

data_register_item
  : checked_declaration_item
    { $$ = $1; }
  | WRITEDATASOURCE token_gap concat_data_signal SEMICOLON
    {
      $$ = driver.MakeStatement("WriteDataSource " + $2 + $3 + ";",
                                SpanOf(@1, @4));
    }
  | WRITEENSOURCE token_gap write_enable SEMICOLON
    {
      $$ = driver.MakeStatement("WriteEnSource " + $2 + $3 + ";",
                                SpanOf(@1, @4));
    }
  | READDATASOURCE token_gap concat_data_signal SEMICOLON
    {
      $$ = driver.MakeStatement("ReadDataSource " + $2 + $3 + ";",
                                SpanOf(@1, @4));
    }
  | address_value
    { $$ = $1; }
  | callback_keyword token_gap callback_namespace callback_id callback_args
    SEMICOLON
    {
      $$ = driver.MakeStatement($1 + " " + $2 + $3 + " " + $4 + $5 + ";",
                                SpanOf(@1, @6));
    }
  ;

/* §6.4.9 explicitly permits an extra inversion before data_signal. */
write_enable
  : data_signal
    { $$ = $1; }
  | NOT_OP token_gap NOT_OP token_gap signal
    {
      if ($1 != "~" || $3 != "~") {
        error(@1, "expected signal inversion '~'");
        YYERROR;
      }
      $$ = $1 + $2 + $3 + $4 + $5;
    }
  ;

callback_keyword
  : READCALLBACK
    { $$ = "ReadCallBack"; }
  | WRITECALLBACK
    { $$ = "WriteCallBack"; }
  ;

callback_namespace
  : scalar_name token_gap namespace_tail
    { $$ = $1 + $2 + $3; }
  | PARAMREF token_gap optional_subnamespace
    { $$ = $1 + $2 + $3; }
  | DCOLON token_gap callback_id optional_subnamespace
    { $$ = "::" + $2 + $3 + $4; }
  ;

namespace_tail
  : %empty
    { $$ = ""; }
  | DCOLON token_gap callback_id optional_subnamespace
    { $$ = "::" + $2 + $3 + $4; }
  ;

optional_subnamespace
  : %empty
    { $$ = ""; }
  | DCOLON token_gap callback_id
    { $$ = "::" + $2 + $3; }
  ;

callback_id
  : scalar_name token_gap
    { $$ = $1 + $2; }
  | PARAMREF token_gap
    { $$ = $1 + $2; }
  ;

callback_args
  : %empty
    { $$ = ""; }
  | callback_args callback_arg
    { $$ = std::move($1) + " " + $2; }
  ;

callback_arg
  : number
    { $$ = $1.text; }
  | STRING token_gap
    { $$ = $1 + $2; }
  | IPROCARG token_gap
    { $$ = $1 + $2; }
  ;

frequency_keyword
  : FREQMULTIPLIER
    { $$ = "FreqMultiplier"; }
  | FREQDIVIDER
    { $$ = "FreqDivider"; }
  ;

clock_factor
  : INTEGER token_gap
    { $$ = $1 + $2; }
  /* Tessent v2023.4 clockGen example uses parameterized factors. */
  | PARAMREF token_gap
    { $$ = $1 + $2; }
  ;

clock_period
  : INTEGER token_gap
    { $$ = $1 + $2; }
  | TIME token_gap
    { $$ = $1 + $2; }
  | INTEGER token_gap IDENT token_gap
    {
      if ($3 != "s" && $3 != "ms" && $3 != "us" && $3 != "ns" && $3 != "ps" &&
          $3 != "fs" && $3 != "as") {
        error(@3, "invalid Period time unit");
        YYERROR;
      }
      $$ = $1 + " " + $2 + $3 + $4;
    }
  ;

default_value
  : concat_number
    { $$ = $1.text; }
  | scalar_name token_gap
    { $$ = $1 + $2; }
  ;

strict_attribute
  : ATTRIBUTE token_gap scalar_name token_gap SEMICOLON
    {
      $$ = driver.unit().Make<icl::AstStatement>(
          "Attribute " + $2 + $3 + $4 + ";", SpanOf(@1, @5),
          icl::NamedKind::kAttribute, $3);
    }
  | ATTRIBUTE token_gap scalar_name token_gap EQUALS token_gap attribute_values
    SEMICOLON
    {
      const auto text =
          "Attribute " + $2 + $3 + $4 + " = " + $6 + $7.text + ";";
      $$ = driver.unit().Make<icl::AstStatement>(
          text, SpanOf(@1, @8), icl::NamedKind::kAttribute, $3, std::move($7));
    }
  ;

attribute_values
  : attribute_atom
    { $$ = std::move($1); }
  | attribute_values COMMA token_gap attribute_atom
    {
      if ($1.kind != ValueKind::kUnknown && $4.kind != ValueKind::kUnknown &&
          $1.kind != $4.kind) {
        error(@4, "cannot concatenate strings and numbers");
        YYERROR;
      }
      $$ = std::move($1);
      $$.Append($3, $4);
    }
  ;

attribute_atom
  : inverted_number
    { $$ = std::move($1); }
  | STRING token_gap
    { $$ = ParsedValue{$1 + $2, 0, ValueKind::kString}; }
  ;

strict_range
  : %empty
    { $$ = ""; }
  | LBRACKET token_gap integer_expr RBRACKET token_gap
    { $$ = "[" + $2 + $3.text + "]" + $5; }
  | LBRACKET token_gap integer_expr COLON token_gap integer_expr RBRACKET
    token_gap
    { $$ = "[" + $2 + $3.text + ":" + $5 + $6.text + "]" + $8; }
  ;

signal_path
  : scalar_name token_gap
    {
      $$ = ParsedReference{$1 + $2};
      $$.end_line = @1.end.line;
      $$.end_column = @1.end.column;
    }
  | signal_path DOT token_gap scalar_name token_gap
    {
      if ($1.end_line != @2.begin.line || $1.end_column != @2.begin.column ||
          @2.end.line != @4.begin.line || @2.end.column != @4.begin.column) {
        error(@2,
              "hierarchical identifiers cannot contain whitespace or comments");
        YYERROR;
      }
      $$ = std::move($1);
      $$.text += "." + $3 + $4 + $5;
      $$.is_scalar = false;
      ++$$.depth;
      $$.end_line = @4.end.line;
      $$.end_column = @4.end.column;
    }
  ;

signal_reference
  : alias_reference
    {
      if ($1.depth > 1) {
        error(@1, "multi-level hierarchy is allowed only in Alias");
        YYERROR;
      }
      $$ = std::move($1);
    }
  ;

alias_reference
  : signal_path strict_range
    {
      $$ = std::move($1);
      $$.text += $2;
      $$.is_scalar = $$.is_scalar && $2.empty();
    }
  ;

signal
  : number
    { $$ = $1.text; }
  | signal_reference
    { $$ = $1.text; }
  ;

data_signal
  : signal
    { $$ = $1; }
  | NOT_OP token_gap signal
    {
      if ($1 != "~") {
        error(@1, "expected signal inversion '~'");
        YYERROR;
      }
      $$ = $1 + $2 + $3;
    }
  ;

concat_data_signal
  : data_signal
    { $$ = $1; }
  | concat_data_signal COMMA token_gap data_signal
    { $$ = std::move($1) + ", " + $3 + $4; }
  ;

/* IEEE §6.4.10: one expression, with comments before and after it. */
logic_body
  : comment_nodes logic_expr SEMICOLON comment_nodes
    {
      if ($2.has_unknown) {
        error(
            @2,
            "unknown digits are allowed only in a comparison's right operand");
        YYERROR;
      }
      $$ = std::move($1);
      $$.push_back(driver.MakeStatement($2.text + ";", SpanOf(@2, @3)));
      $$.insert($$.end(), $4.begin(), $4.end());
    }
  ;

logic_expr
  : logic_bit
    { $$ = std::move($1); }
  | logic_expr BOOL_OP token_gap logic_bit
    {
      $$.text = std::move($1.text) + " " + $2 + " " + $3 + $4.text;
      $$.has_unknown = $1.has_unknown || $4.has_unknown;
    }
  ;

logic_bit
  : logic_bit_term
    { $$ = std::move($1); }
  | logic_bit BIT_OP token_gap logic_bit_term
    {
      $$.text = std::move($1.text) + " " + $2 + " " + $3 + $4.text;
      $$.has_unknown = $1.has_unknown || $4.has_unknown;
    }
  ;

logic_bit_term
  : logic_compare
    { $$ = std::move($1); }
  | BIT_OP token_gap logic_bit_term
    {
      $$.text = $1 + $2 + $3.text;
      $$.has_unknown = $3.has_unknown;
    }
  ;

logic_compare
  : logic_concat
    { $$ = std::move($1); }
  | logic_concat COMPARE_OP token_gap logic_concat
    {
      if ($1.has_unknown || !($4.is_numeric_concat || $4.is_enum)) {
        error(@2,
              "comparison requires numeric or enum right operand and no "
              "unknown left operand");
        YYERROR;
      }
      if ($4.is_enum) {
        driver.unit().EnumComparisons().push_back(
            {$1.reference.value_or(SourceSpan{}), *$4.reference});
      }
      $$.text = std::move($1.text) + " " + $2 + " " + $3 + $4.text;
    }
  ;

logic_concat
  : logic_unary
    { $$ = std::move($1); }
  | logic_concat COMMA token_gap logic_unary
    {
      $$.text = std::move($1.text) + ", " + $3 + $4.text;
      $$.is_numeric_concat = $1.is_numeric_concat && $4.is_numeric_concat;
      $$.has_unknown = $1.has_unknown || $4.has_unknown;
    }
  ;

logic_unary
  : logic_integer
    { $$ = std::move($1); }
  | NOT_OP token_gap logic_unary
    {
      $$.text = $1 + $2 + $3.text;
      $$.is_numeric_concat = $1 == "~" && $3.is_number;
      $$.has_unknown = $3.has_unknown;
      $$.reference = $3.reference;
    }
  ;

logic_integer
  : logic_atom
    { $$ = std::move($1); }
  | logic_integer MATH_OP token_gap logic_atom
    {
      if (!$1.is_integer || !$4.is_integer) {
        error(@2, "integer arithmetic requires decimal or parameter operands");
        YYERROR;
      }
      $$.text = std::move($1.text) + " " + $2 + " " + $3 + $4.text;
      $$.is_integer = $$.is_number = $$.is_numeric_concat = true;
    }
  ;

logic_atom
  : INTEGER token_gap
    {
      $$.text = $1 + $2;
      $$.is_integer = $$.is_number = $$.is_numeric_concat = true;
    }
  | PARAMREF token_gap
    {
      $$.text = $1 + $2;
      $$.is_integer = $$.is_number = $$.is_numeric_concat = true;
    }
  | based_number
    {
      $$.text = std::move($1.text);
      $$.is_number = $$.is_numeric_concat = true;
      $$.has_unknown = $1.has_unknown;
    }
  | signal_reference
    {
      $$.text = $1.text;
      $$.is_enum = $1.is_scalar;
      $$.reference = SpanOf(@1, @1);
      $$.reference->last_line = $1.end_line;
      $$.reference->last_column = $1.end_column;
    }
  | LPAREN token_gap logic_expr RPAREN token_gap
    {
      $$ = std::move($3);
      $$.text = "(" + $2 + $$.text + ")" + $5;
      $$.is_number = $$.is_integer;
    }
  ;

alias_signal
  : alias_reference
    { $$ = std::move($1.text); }
  | NOT_OP token_gap alias_reference
    {
      if ($1 != "~") {
        error(@1, "expected alias inversion '~'");
        YYERROR;
      }
      $$ = $1 + $2 + $3.text;
    }
  ;

alias_signals
  : alias_signal
    { $$ = std::move($1); }
  | alias_signals COMMA token_gap alias_signal
    { $$ = std::move($1) + ", " + $3 + $4; }
  ;

alias_body
  : %empty
    { $$ = {}; }
  | alias_body alias_item
    {
      $$ = std::move($1);
      $$.push_back($2);
    }
  | alias_body COMMENT
    {
      $$ = std::move($1);
      $$.push_back(driver.AddComment($2, SpanOf(@2, @2)));
    }
  ;

alias_item
  : strict_attribute
    { $$ = $1; }
  | REFENUM token_gap scalar_name token_gap SEMICOLON
    {
      $$ =
          driver.MakeStatement("RefEnum " + $2 + $3 + $4 + ";", SpanOf(@1, @5));
    }
  | ACCESSTOGETHER token_gap SEMICOLON
    {
      $$ = driver.MakeStatement("AccessTogether" + $2 + ";", SpanOf(@1, @3));
    }
  | IAPPLYENDSTATE token_gap concat_number SEMICOLON
    {
      if ($3.has_unknown) {
        error(@3, "iApplyEndState cannot contain unknown digits");
        YYERROR;
      }
      $$ = driver.MakeStatement("iApplyEndState " + $2 + $3.text + ";",
                                SpanOf(@1, @4));
    }
  ;


interface_property
  : strict_attribute
    { $$ = $1; }
  | PORT token_gap scalar_name token_gap strict_range SEMICOLON
    {
      $$ = driver.MakeStatement("Port " + $2 + $3 + $4 + $5 + ";",
                                SpanOf(@1, @6));
    }
  | DEFAULTLOADVALUE token_gap concat_number SEMICOLON
    {
      $$ = driver.MakeStatement("DefaultLoadValue " + $2 + $3.text + ";",
                                SpanOf(@1, @4));
    }
  ;

chain
  : CHAIN token_gap scalar_name token_gap LBRACE chain_body RBRACE
    {
      driver.CheckChainName($3, @3.begin.line);
      driver.CheckBlockItems("Chain", $6, @1.begin.line);
      $$ = driver.MakeBlock("Chain " + $2 + $3 + $4, std::move($6),
                            SpanOf(@1, @7));
    }
  ;

chain_body
  : comment_nodes interface_property
    {
      $$ = std::move($1);
      $$.push_back($2);
    }
  | chain_body interface_property
    {
      $$ = std::move($1);
      $$.push_back($2);
    }
  | chain_body COMMENT
    {
      $$ = std::move($1);
      $$.push_back(driver.AddComment($2, SpanOf(@2, @2)));
    }
  ;

interface_item
  : interface_property
    { $$ = $1; }
  | chain
    { $$ = $1; }
  ;

interface_body
  : comment_nodes interface_item
    {
      $$ = std::move($1);
      $$.push_back($2);
    }
  | interface_body interface_item
    {
      $$ = std::move($1);
      $$.push_back($2);
    }
  | interface_body COMMENT
    {
      $$ = std::move($1);
      $$.push_back(driver.AddComment($2, SpanOf(@2, @2)));
    }
  ;


group_port
  : PORT token_gap concat_data_signal SEMICOLON
    {
      $$ = driver.MakeStatement("Port " + $2 + $3 + ";", SpanOf(@1, @4));
    }
  ;

one_hot_scan_body
  : comment_nodes group_port
    {
      $$ = std::move($1);
      $$.push_back($2);
    }
  | one_hot_scan_body group_port
    {
      $$ = std::move($1);
      $$.push_back($2);
    }
  | one_hot_scan_body COMMENT
    {
      $$ = std::move($1);
      $$.push_back(driver.AddComment($2, SpanOf(@2, @2)));
    }
  ;


one_hot_data_item
  : group_port
    { $$ = $1; }
  | data_register
    { $$ = $1; }
  | instance_declaration
    { $$ = $1; }
  ;

one_hot_data_body
  : comment_nodes one_hot_data_item
    {
      $$ = std::move($1);
      $$.push_back($2);
    }
  | one_hot_data_body one_hot_data_item
    {
      $$ = std::move($1);
      $$.push_back($2);
    }
  | one_hot_data_body COMMENT
    {
      $$ = std::move($1);
      $$.push_back(driver.AddComment($2, SpanOf(@2, @2)));
    }
  ;


strict_mux_keyword
  : SCANMUX
    { $$ = "ScanMux"; }
  | DATAMUX
    { $$ = "DataMux"; }
  | CLOCKMUX
    { $$ = "ClockMux"; }
  ;

// At least one selection is required; attributes are a Tessent extension.
mux_prefix
  : %empty
    { $$ = {}; }
  | mux_prefix COMMENT
    {
      $$ = std::move($1);
      $$.push_back(driver.AddComment($2, SpanOf(@2, @2)));
    }
  | mux_prefix strict_attribute
    {
      $$ = std::move($1);
      $$.push_back($2);
    }
  ;

strict_mux_body
  : mux_prefix mux_selection
    {
      $$ = std::move($1);
      $$.push_back($2);
    }
  | strict_mux_body mux_selection
    {
      $$ = std::move($1);
      $$.push_back($2);
    }
  | strict_mux_body strict_attribute
    {
      $$ = std::move($1);
      $$.push_back($2);
    }
  | strict_mux_body COMMENT
    {
      $$ = std::move($1);
      $$.push_back(driver.AddComment($2, SpanOf(@2, @2)));
    }
  ;

mux_selection
  : mux_codes COLON token_gap concat_data_signal SEMICOLON
    {
      $$ = driver.MakeStatement($1 + " : " + $3 + $4 + ";", SpanOf(@1, @5));
    }
  ;

mux_codes
  : concat_number
    { $$ = std::move($1.text); }
  | mux_codes BIT_OP token_gap concat_number
    {
      if ($2 != "|") {
        error(@2, "expected '|' between selection codes");
        YYERROR;
      }
      $$ = std::move($1);
      $$ += " | " + $3 + $4.text;
    }
  ;



// A scalar name can use any ICL keyword (IEEE6.3.5).
decl_name
  : IDENT
    { $$ = $1; }
  | any_keyword
    { $$ = $1; }
  ;

// Legacy PortGroup lists from LiU P1687 Listing8.
port_group_items
  : port_group_item
    {
      $$ = std::vector<icl::PortGroupItem>();
      $$.push_back(std::move($1));
    }
  | port_group_items port_group_item
    {
      $$ = std::move($1);
      $$.push_back(std::move($2));
    }
  ;

port_group_range
  : %empty
    { $$ = ""; }
  | LBRACKET token_gap integer_expr RBRACKET
    { $$ = "[" + $2 + $3.text + "]"; }
  | LBRACKET token_gap integer_expr COLON token_gap integer_expr RBRACKET
    { $$ = "[" + $2 + $3.text + ":" + $5 + $6.text + "]"; }
  ;

port_group_item
  : scalar_name port_group_range
    { $$ = icl::PortGroupItem::Name($1 + $2, SpanOf(@1, @2)); }
  | COMMENT
    {
      $$ = icl::PortGroupItem::Comment(driver.AddComment($1, SpanOf(@1, @1)),
                                       SpanOf(@1, @1));
    }
  | COMMA
    { $$ = icl::PortGroupItem::Comma(SpanOf(@1, @1)); }
  ;

// Instance is reusable inside OneHotDataGroup; AccessLink is not.
instance_declaration
  : INSTANCE token_gap scalar_name token_gap of_kw token_gap instance_module_ref
    SEMICOLON
    {
      $$ = driver.MakeStatement(
          "Instance " + $2 + $3 + $4 + " " + $5 + " " + $6 + $7 + ";",
          SpanOf(@1, @8));
    }
  | INSTANCE token_gap scalar_name token_gap of_kw token_gap instance_module_ref
    LBRACE instance_body RBRACE
    {
      driver.CheckBlockItems("Instance", $9, @1.begin.line);
      $$ = driver.MakeBlock(
          "Instance " + $2 + $3 + $4 + " " + $5 + " " + $6 + $7, std::move($9),
          SpanOf(@1, @10));
    }
  ;

instance_module_ref
  : scalar_name token_gap
    { $$ = $1 + $2; }
  | scalar_name token_gap DCOLON token_gap scalar_name token_gap
    { $$ = $1 + $2 + "::" + $4 + $5 + $6; }
  | DCOLON token_gap scalar_name token_gap
    { $$ = "::" + $2 + $3 + $4; }
  ;

instance_body
  : %empty
    { $$ = {}; }
  | instance_body instance_property
    {
      $$ = std::move($1);
      $$.push_back($2);
    }
  | instance_body COMMENT
    {
      $$ = std::move($1);
      $$.push_back(driver.AddComment($2, SpanOf(@2, @2)));
    }
  ;

instance_property
  : strict_attribute
    { $$ = $1; }
  | parameter_override
    { $$ = $1; }
  | address_value
    { $$ = $1; }
  | INPUTPORT token_gap scalar_name token_gap strict_range EQUALS token_gap
    concat_data_signal SEMICOLON
    {
      $$ = driver.MakeStatement(
          "InputPort " + $2 + $3 + $4 + $5 + " = " + $7 + $8 + ";",
          SpanOf(@1, @9));
    }
  | ALLOWBROADCAST token_gap broadcast_names SEMICOLON
    {
      $$ = driver.MakeStatement(
          "AllowBroadcastOnScanInterface " + $2 + $3 + ";", SpanOf(@1, @4));
    }
  ;

// The normative generic example and Inst4 example both allow comma lists.
broadcast_names
  : scalar_name token_gap
    { $$ = $1 + $2; }
  | broadcast_names COMMA token_gap scalar_name token_gap
    { $$ = std::move($1) + ", " + $3 + $4 + $5; }
  ;

parameter_override
  : PARAMETER token_gap scalar_name token_gap EQUALS token_gap attribute_values
    SEMICOLON
    {
      const auto text =
          "Parameter " + $2 + $3 + $4 + " = " + $6 + $7.text + ";";
      $$ = driver.MakeParameter("Parameter", $3, text, std::move($7),
                                SpanOf(@1, @8));
    }
  ;

// Tessent OneHotDataGroup and Instance examples use concatenated addresses.
address_value
  : ADDRESSVALUE token_gap concat_number SEMICOLON
    {
      $$ = driver.MakeStatement("AddressValue " + $2 + $3.text + ";",
                                SpanOf(@1, @4));
    }
  ;

instance_item
  : instance_declaration
    { $$ = $1; }
  | access_link
    { $$ = $1; }
  ;

access_link
  : ACCESSLINK token_gap scalar_name token_gap OF token_gap std_tap_kw token_gap
    LBRACE link_body RBRACE
    {
      driver.CheckAccessLink($3, @3.begin.line);
      $$ =
          driver.MakeBlock("AccessLink " + $2 + $3 + $4 + " Of " + $6 + $7 + $8,
                           std::move($10), SpanOf(@1, @11));
    }
  | ACCESSLINK token_gap scalar_name token_gap OF token_gap generic_link_type
    token_gap generic_open GENERIC_BODY
    {
      driver.CheckAccessLink($3, @3.begin.line);
      // Generic controller contents have implementation-defined semantics.
      $$ = driver.MakeStatement(
          "AccessLink " + $2 + $3 + $4 + " Of " + $6 + $7 + $8 + " {" + $10,
          SpanOf(@1, @10));
    }
  ;

generic_open
  : LBRACE
    { ::IclScanGeneric(); }
  ;

generic_link_type
  : IDENT
    { $$ = $1; }
  | non_tap_keyword
    { $$ = $1; }
  | DATAREGISTER
    { $$ = "DataRegister"; }
  | INSTANCE
    { $$ = "Instance"; }
  | ACCESSLINK
    { $$ = "AccessLink"; }
  ;

link_body
  : comment_nodes BSDLENTITY token_gap scalar_name token_gap SEMICOLON
    link_instructions
    {
      $$ = std::move($1);
      $$.push_back(driver.MakeStatement("BSDLEntity " + $3 + $4 + $5 + ";",
                                        SpanOf(@2, @6)));
      $$.insert($$.end(), $7.begin(), $7.end());
    }
  ;

link_instructions
  : comment_nodes link_instruction
    {
      $$ = std::move($1);
      $$.push_back($2);
    }
  | link_instructions link_instruction
    {
      $$ = std::move($1);
      $$.push_back($2);
    }
  | link_instructions COMMENT
    {
      $$ = std::move($1);
      $$.push_back(driver.AddComment($2, SpanOf(@2, @2)));
    }
  ;

link_instruction
  : scalar_name token_gap LBRACE link_selected_body RBRACE
    {
      driver.CheckInstructionName($1, @1.begin.line);
      $$ = driver.MakeBlock($1 + $2, std::move($4), SpanOf(@1, @5));
    }
  ;

link_selected_body
  : comment_nodes link_selected_item
    {
      $$ = std::move($1);
      $$.push_back($2);
    }
  | link_selected_body link_selected_item
    {
      $$ = std::move($1);
      $$.push_back($2);
    }
  | link_selected_body COMMENT
    {
      $$ = std::move($1);
      $$.push_back(driver.AddComment($2, SpanOf(@2, @2)));
    }
  ;

link_selected_item
  : SCANINTERFACE token_gap LBRACE link_interface_body RBRACE
    {
      $$ = driver.MakeBlock("ScanInterface " + $2, std::move($4),
                            SpanOf(@1, @5));
    }
  | ACTIVESIGNALS token_gap LBRACE link_active_body RBRACE
    {
      $$ = driver.MakeBlock("ActiveSignals " + $2, std::move($4),
                            SpanOf(@1, @5));
    }
  ;

link_interface_body
  : comment_nodes link_interface_item
    {
      $$ = std::move($1);
      $$.push_back($2);
    }
  | link_interface_body link_interface_item
    {
      $$ = std::move($1);
      $$.push_back($2);
    }
  | link_interface_body COMMENT
    {
      $$ = std::move($1);
      $$.push_back(driver.AddComment($2, SpanOf(@2, @2)));
    }
  ;

link_interface_item
  : link_interface_ref SEMICOLON
    { $$ = driver.MakeStatement($1 + ";", SpanOf(@1, @2)); }
  ;

link_interface_ref
  : scalar_name token_gap
    { $$ = $1 + $2; }
  | scalar_name token_gap DOT token_gap scalar_name token_gap
    { $$ = $1 + $2 + "." + $4 + $5 + $6; }
  ;

link_active_body
  : comment_nodes link_active_item
    {
      $$ = std::move($1);
      $$.push_back($2);
    }
  | link_active_body link_active_item
    {
      $$ = std::move($1);
      $$.push_back($2);
    }
  | link_active_body COMMENT
    {
      $$ = std::move($1);
      $$.push_back(driver.AddComment($2, SpanOf(@2, @2)));
    }
  ;

link_active_item
  : scalar_name token_gap strict_range SEMICOLON
    {
      $$ = driver.MakeStatement($1 + $2 + $3 + ";", SpanOf(@1, @4));
    }
  ;

/* ------------------------------------------------- keyword groups by shape */



/* `NameSpace x;` and `UseNameSpace x;` — declared at file scope and, for
   UseNameSpace, inside a module too. The name is optional: IEEE 1687-2014
   Appendix E writes both `NameSpace X;` and a bare `NameSpace;`. */
ns_stmt
  : NAMESPACE token_gap SEMICOLON
    {
      $$ = driver.MakeStatement("NameSpace" + $2 + ";", SpanOf(@1, @3));
    }
  | NAMESPACE token_gap scalar_name token_gap SEMICOLON
    {
      $$ = driver.MakeStatement("NameSpace " + $2 + $3 + $4 + ";",
                                SpanOf(@1, @5));
    }
  ;

use_ns_stmt
  : USENAMESPACE token_gap SEMICOLON
    {
      $$ = driver.MakeStatement("UseNameSpace" + $2 + ";", SpanOf(@1, @3));
    }
  | USENAMESPACE token_gap scalar_name token_gap SEMICOLON
    {
      $$ = driver.MakeStatement("UseNameSpace " + $2 + $3 + $4 + ";",
                                SpanOf(@1, @5));
    }
  ;

// The two standard IEEE1149.1 AccessLink types.
std_tap_kw
  : STD1149_1_2001
    { $$ = "STD_1149_1_2001"; }
  | STD1149_1_2013
    { $$ = "STD_1149_1_2013"; }
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
  : OF
    { $$ = "Of"; }
  | OFLOWER
    { $$ = "of"; }
  ;

/* Everything above, plus the keywords that have no block form. One place to
   add a keyword; every list that needs "any keyword" refers to this. */
any_keyword
  : ordinary_keyword
    { $$ = $1; }
  | DATAREGISTER
    { $$ = "DataRegister"; }
  ;

ordinary_keyword
  : non_tap_keyword
    { $$ = $1; }
  | std_tap_kw
    { $$ = $1; }
  ;

non_tap_keyword
  : SCANINTERFACE
    { $$ = "ScanInterface"; }
  | ONEHOTSCANGROUP
    { $$ = "OneHotScanGroup"; }
  | ONEHOTDATAGROUP
    { $$ = "OneHotDataGroup"; }
  | strict_mux_keyword
    { $$ = $1; }
  | ENUM
    { $$ = "Enum"; }
  | LAUNCHEDGE
    { $$ = "LaunchEdge"; }
  | checked_declaration_keyword
    { $$ = $1; }
  | PORTGROUP
    { $$ = "PortGroup"; }
  | MODULE
    { $$ = "Module"; }
  | PARAMETER
    { $$ = "Parameter"; }
  | ATTRIBUTE
    { $$ = "Attribute"; }
  | ALLOWBROADCAST
    { $$ = "AllowBroadcastOnScanInterface"; }
  | ACCESSTOGETHER
    { $$ = "AccessTogether"; }
  | ACTIVEPOLARITY
    { $$ = "ActivePolarity"; }
  | ACTIVESIGNALS
    { $$ = "ActiveSignals"; }
  | ADDRESSVALUE
    { $$ = "AddressValue"; }
  | BSDLENTITY
    { $$ = "BSDLEntity"; }
  | CAPTURESOURCE
    { $$ = "CaptureSource"; }
  | CHAIN
    { $$ = "Chain"; }
  | DEFAULTLOADVALUE
    { $$ = "DefaultLoadValue"; }
  | DIFFERENTIALINVOF
    { $$ = "DifferentialInvOf"; }
  | ENABLE
    { $$ = "Enable"; }
  | FREQDIVIDER
    { $$ = "FreqDivider"; }
  | FREQMULTIPLIER
    { $$ = "FreqMultiplier"; }
  | PERIOD
    { $$ = "Period"; }
  | INPUTPORT
    { $$ = "InputPort"; }
  | PORT
    { $$ = "Port"; }
  | READCALLBACK
    { $$ = "ReadCallBack"; }
  | READDATASOURCE
    { $$ = "ReadDataSource"; }
  | REFENUM
    { $$ = "RefEnum"; }
  | RESETVALUE
    { $$ = "ResetValue"; }
  | SCANINSOURCE
    { $$ = "ScanInSource"; }
  | SOURCE
    { $$ = "Source"; }
  | WRITECALLBACK
    { $$ = "WriteCallBack"; }
  | WRITEDATASOURCE
    { $$ = "WriteDataSource"; }
  | WRITEENSOURCE
    { $$ = "WriteEnSource"; }
  | IAPPLYENDSTATE
    { $$ = "iApplyEndState"; }
  | ALIAS
    { $$ = "Alias"; }
  | SELECTEDBY
    { $$ = "SelectedBy"; }
  | LOGICSIGNAL
    { $$ = "LogicSignal"; }
  | OF
    { $$ = "Of"; }
  | OFLOWER
    { $$ = "of"; }
  | LOCALPARAMETER
    { $$ = "LocalParameter"; }
  | NAMESPACE
    { $$ = "NameSpace"; }
  | USENAMESPACE
    { $$ = "UseNameSpace"; }
  ;

%%
