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
%require "3.8"
%language "c++"
%define api.namespace {icl}
%define api.parser.class {IclParser}
%define api.token.constructor
%define api.value.type variant
%define parse.assert
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
%token <std::string> STRING "string"
%token <std::string> COMMENT "comment"
%token ARROW "->"

%token MODULE "Module"
%token PARAMETER "Parameter"
%token ATTRIBUTE "Attribute"
%token ALIAS "Alias"
%token ENUM "Enum"
%token SCANINPORT "ScanInPort"
%token SCANOUTPORT "ScanOutPort"
%token SHIFTENPORT "ShiftEnPort"
%token CAPTUREENPORT "CaptureEnPort"
%token UPDATEENPORT "UpdateEnPort"
%token SELECTPORT "SelectPort"
%token RESETPORT "ResetPort"
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
%token TODOPORT "ToDoPort"
%token TOSELECTPORT "ToSelectPort"
%token DATAINPORT "DataInPort"
%token DATAOUTPORT "DataOutPort"
%token SCANINTERFACE "ScanInterface"
%token ONEHOTSCANINTERFACE "OneHotScanInterface"
%token SCANREGISTER "ScanRegister"
%token REG "Reg"
%token FIELD "Field"
%token INSTANCE "Instance"
%token SCANMUX "ScanMux"
%token SELECTEDBY "SelectedBy"
%token LOGICSIGNAL "LogicSignal"
%token CONNECTION "Connection"
%token PORTGROUP "PortGroup"
%token ACCESSLINK "AccessLink"
%token IPROCSFORMODULE "iProcsForModule"
%token OF "Of"

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
%type <std::vector<AstNode*>> module_item_list
%type <AstNode*> module_item
%type <AstNode*> port_decl
%type <std::string> port_keyword
%type <std::string> opt_range
%type <std::string> value
%type <std::string> postfix
%type <std::string> primary
%type <std::string> attr_name
%type <std::string> conn_expr
%type <std::string> ident_list
%type <AstNode*> attr_item
%type <std::vector<AstNode*>> attr_body
%type <std::vector<AstNode*>> mux_body
%type <std::vector<AstNode*>> conn_body

%printer { yyoutput << $$; } <std::string>;

%start file

%%

/* ---------------------------------------------------------------- top level */

file
  : %empty
  | file module_def
      { driver.unit().top_level().push_back($2); }
  | file COMMENT
      {
        // Unlike the nested bodies below, the top-level rule has no vector to
        // append to, so the comment must be pushed onto the unit explicitly.
        driver.unit().top_level().push_back(
            driver.AddComment($2, SpanOf(@2.begin.line, @2.end.line)));
      }
  ;

module_def
  : MODULE IDENT LBRACE module_item_list RBRACE
      {
        $$ = driver.MakeBlock("Module " + $2, std::move($4), SpanOf(@1.begin.line, @5.end.line));
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
  | PARAMETER IDENT EQUALS value SEMICOLON
      { $$ = driver.MakeStatement("Parameter " + $2 + " = " + $4 + ";",
                                 SpanOf(@1.begin.line, @5.end.line)); }
  | ATTRIBUTE IDENT EQUALS value SEMICOLON
      { $$ = driver.MakeStatement("Attribute " + $2 + " = " + $4 + ";",
                                 SpanOf(@1.begin.line, @5.end.line)); }
  | ALIAS IDENT EQUALS value SEMICOLON
      { $$ = driver.MakeStatement("Alias " + $2 + " = " + $4 + ";",
                                 SpanOf(@1.begin.line, @5.end.line)); }
  | ALIAS IDENT EQUALS value LBRACE attr_body RBRACE
      {
        $$ = driver.MakeBlock("Alias " + $2 + " = " + $4, std::move($6), SpanOf(@1.begin.line, @7.end.line));
      }
  | ENUM IDENT LBRACE attr_body RBRACE
      {
        $$ = driver.MakeBlock("Enum " + $2, std::move($4), SpanOf(@1.begin.line, @5.end.line));
      }
  | port_decl { $$ = $1; }
  | SCANINTERFACE IDENT LBRACE attr_body RBRACE
      {
        $$ = driver.MakeBlock("ScanInterface " + $2, std::move($4), SpanOf(@1.begin.line, @5.end.line));
      }
  | ONEHOTSCANINTERFACE IDENT LBRACE attr_body RBRACE
      {
        $$ = driver.MakeBlock("OneHotScanInterface " + $2, std::move($4), SpanOf(@1.begin.line, @5.end.line));
      }
  | SCANREGISTER IDENT opt_range LBRACE attr_body RBRACE
      {
        $$ = driver.MakeBlock("ScanRegister " + $2 + $3, std::move($5), SpanOf(@1.begin.line, @6.end.line));
      }
  | SCANREGISTER IDENT opt_range SEMICOLON
      { $$ = driver.MakeStatement("ScanRegister " + $2 + $3 + ";",
                                 SpanOf(@1.begin.line, @4.end.line)); }
  | REG IDENT opt_range LBRACE attr_body RBRACE
      {
        $$ = driver.MakeBlock("Reg " + $2 + $3, std::move($5), SpanOf(@1.begin.line, @6.end.line));
      }
  | REG IDENT opt_range SEMICOLON
      { $$ = driver.MakeStatement( "Reg " + $2 + $3 + ";",
                                 SpanOf(@1.begin.line, @4.end.line)); }
  | FIELD IDENT opt_range OF IDENT LBRACE attr_body RBRACE
      {
        $$ = driver.MakeBlock("Field " + $2 + $3 + " Of " + $5, std::move($7), SpanOf(@1.begin.line, @8.end.line));
      }
  | FIELD IDENT opt_range OF IDENT SEMICOLON
      { $$ = driver.MakeStatement( "Field " + $2 + $3 + " Of " + $5 + ";",
                                 SpanOf(@1.begin.line, @6.end.line)); }
  | INSTANCE IDENT OF IDENT LBRACE attr_body RBRACE
      {
        $$ = driver.MakeBlock("Instance " + $2 + " Of " + $4, std::move($6), SpanOf(@1.begin.line, @7.end.line));
      }
  | INSTANCE IDENT OF IDENT SEMICOLON
      { $$ = driver.MakeStatement("Instance " + $2 + " Of " + $4 + ";",
                                 SpanOf(@1.begin.line, @5.end.line)); }
  | SCANMUX IDENT SELECTEDBY value LBRACE mux_body RBRACE
      {
        // `SelectedBy` is part of the standard (IEEE 1687.1-2014); the
        // selector may be a hierarchical name such as `MyTap.M3_sel`, hence
        // |value| rather than a bare IDENT.
        $$ = driver.MakeBlock("ScanMux " + $2 + " SelectedBy " + $4, std::move($6), SpanOf(@1.begin.line, @7.end.line));
      }
  | LOGICSIGNAL IDENT LBRACE attr_body RBRACE
      {
        $$ = driver.MakeBlock("LogicSignal " + $2, std::move($4), SpanOf(@1.begin.line, @5.end.line));
      }
  | CONNECTION LBRACE conn_body RBRACE
      {
        $$ = driver.MakeBlock("Connection", std::move($3), SpanOf(@1.begin.line, @4.end.line));
      }
  | PORTGROUP IDENT LBRACE ident_list RBRACE
      { $$ = driver.MakeStatement("PortGroup " + $2 + " {" + $4 + "}",
                                  SpanOf(@1.begin.line, @5.end.line)); }
  | ACCESSLINK IDENT OF IDENT LBRACE attr_body RBRACE
      {
        $$ = driver.MakeBlock("AccessLink " + $2 + " Of " + $4, std::move($6), SpanOf(@1.begin.line, @7.end.line));
      }
  ;

/* -------------------------------------------------------------------- ports */

port_keyword
  : SCANINPORT       { $$ = "ScanInPort"; }
  | SCANOUTPORT      { $$ = "ScanOutPort"; }
  | SHIFTENPORT      { $$ = "ShiftEnPort"; }
  | CAPTUREENPORT    { $$ = "CaptureEnPort"; }
  | UPDATEENPORT     { $$ = "UpdateEnPort"; }
  | SELECTPORT       { $$ = "SelectPort"; }
  | RESETPORT        { $$ = "ResetPort"; }
  | TCKPORT          { $$ = "TCKPort"; }
  | TMSPORT          { $$ = "TMSPort"; }
  | TRSTPORT         { $$ = "TRSTPort"; }
  | TOTCKPORT        { $$ = "ToTCKPort"; }
  | TOTMSPORT        { $$ = "ToTMSPort"; }
  | TOTRSTPORT       { $$ = "ToTRSTPort"; }
  | TOSHIFTENPORT    { $$ = "ToShiftEnPort"; }
  | TOCAPTUREENPORT  { $$ = "ToCaptureEnPort"; }
  | TOUPDATEENPORT   { $$ = "ToUpdateEnPort"; }
  | TORESETPORT      { $$ = "ToResetPort"; }
  | TODOPORT         { $$ = "ToDoPort"; }
  | TOSELECTPORT     { $$ = "ToSelectPort"; }
  | DATAINPORT       { $$ = "DataInPort"; }
  | DATAOUTPORT      { $$ = "DataOutPort"; }
  ;

opt_range
  : %empty                    { $$ = std::string(); }
  | LBRACKET NUMBER RBRACKET            { $$ = "[" + $2 + "]"; }
  | LBRACKET NUMBER COLON NUMBER RBRACKET { $$ = "[" + $2 + ":" + $4 + "]"; }
  ;

port_decl
  : port_keyword IDENT opt_range SEMICOLON
      {
        $$ = driver.MakeStatement($1 + " " + $2 + $3 + ";",
                                  SpanOf(@1.begin.line, @4.end.line));
      }
  | port_keyword IDENT opt_range LBRACE attr_body RBRACE
      {
        $$ = driver.MakeBlock($1 + " " + $2 + $3, std::move($5), SpanOf(@1.begin.line, @6.end.line));
      }
  ;

/* -------------------------------------------------------------------- bodies */

/* Shared attribute forms, used by both ordinary block bodies and AccessLink
   bodies. Factored out because the two bodies accepted the same forms, so
   duplicating them only made the grammar longer and easy to let drift. */
attr_item
  : attr_name SEMICOLON
      { $$ = driver.MakeStatement($1 + ";",
                                  SpanOf(@1.begin.line, @2.end.line)); }
  | attr_name value SEMICOLON
      { $$ = driver.MakeStatement($1 + " " + $2 + ";",
                                  SpanOf(@1.begin.line, @3.end.line)); }
  | attr_name EQUALS value SEMICOLON
      { $$ = driver.MakeStatement($1 + " = " + $3 + ";",
                                  SpanOf(@1.begin.line, @4.end.line)); }
  | attr_name IDENT EQUALS value SEMICOLON
      { $$ = driver.MakeStatement($1 + " " + $2 + " = " + $4 + ";",
                                  SpanOf(@1.begin.line, @5.end.line)); }
  ;



mux_body
  : %empty
      { $$ = std::vector<AstNode*>(); }
  | mux_body COMMENT
      { $$ = std::move($1); $$.push_back(driver.AddComment($2, SpanOf(@2.begin.line, @2.end.line))); }
  | mux_body value COLON value SEMICOLON
      { $$ = std::move($1);
        $$.push_back(driver.MakeStatement($2 + " : " + $4 + ";",
                                          SpanOf(@2.begin.line, @5.end.line))); }
  ;

conn_body
  : %empty
      { $$ = std::vector<AstNode*>(); }
  | conn_body COMMENT
      { $$ = std::move($1); $$.push_back(driver.AddComment($2, SpanOf(@2.begin.line, @2.end.line))); }
  | conn_body conn_expr SEMICOLON
      { $$ = std::move($1);
        $$.push_back(driver.MakeStatement( $2 + ";",
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
  | attr_body attr_name LBRACE attr_body RBRACE
      {
        AstBlock* nested = driver.MakeBlock($2, std::move($4),
                                           SpanOf(@2.begin.line, @5.end.line));
        $$ = std::move($1);
        $$.push_back(nested);
      }
  ;

// Built as a string rather than a vector: the only consumer needs the joined
// text, and C++17 has no std::join.
ident_list
  : IDENT                  { $$ = $1; }
  | ident_list COMMA IDENT { $$ = std::move($1) + ", " + $3; }
  ;

/* An attribute name may be an identifier or any ICL keyword: real ICL files
   write `Attribute d = "x";` inside a Field block and
   `DataInPort DI = 8'h00;` inside an Instance block.

   ponytail: accepting any keyword means a *misspelled attribute name* is not
   an error — only structure is checked. Upgrade path: if attribute spellings
   need checking, enumerate the legal ones per enclosing block instead of
   allowing every keyword here. */
attr_name
  : IDENT          { $$ = $1; }
  | MODULE         { $$ = "Module"; }
  | PARAMETER      { $$ = "Parameter"; }
  | ATTRIBUTE      { $$ = "Attribute"; }
  | ALIAS          { $$ = "Alias"; }
  | ENUM           { $$ = "Enum"; }
  | SCANINPORT     { $$ = "ScanInPort"; }
  | SCANOUTPORT    { $$ = "ScanOutPort"; }
  | SHIFTENPORT    { $$ = "ShiftEnPort"; }
  | CAPTUREENPORT  { $$ = "CaptureEnPort"; }
  | UPDATEENPORT   { $$ = "UpdateEnPort"; }
  | SELECTPORT     { $$ = "SelectPort"; }
  | RESETPORT      { $$ = "ResetPort"; }
  | TCKPORT        { $$ = "TCKPort"; }
  | TMSPORT        { $$ = "TMSPort"; }
  | TRSTPORT       { $$ = "TRSTPort"; }
  | TOTCKPORT      { $$ = "ToTCKPort"; }
  | TOTMSPORT      { $$ = "ToTMSPort"; }
  | TOTRSTPORT     { $$ = "ToTRSTPort"; }
  | TOSHIFTENPORT  { $$ = "ToShiftEnPort"; }
  | TOCAPTUREENPORT{ $$ = "ToCaptureEnPort"; }
  | TOUPDATEENPORT { $$ = "ToUpdateEnPort"; }
  | TORESETPORT    { $$ = "ToResetPort"; }
  | TODOPORT       { $$ = "ToDoPort"; }
  | TOSELECTPORT   { $$ = "ToSelectPort"; }
  | DATAINPORT     { $$ = "DataInPort"; }
  | DATAOUTPORT    { $$ = "DataOutPort"; }
  | SCANINTERFACE  { $$ = "ScanInterface"; }
  | ONEHOTSCANINTERFACE { $$ = "OneHotScanInterface"; }
  | SCANREGISTER   { $$ = "ScanRegister"; }
  | REG            { $$ = "Reg"; }
  | FIELD          { $$ = "Field"; }
  | INSTANCE       { $$ = "Instance"; }
  | SCANMUX        { $$ = "ScanMux"; }
  | SELECTEDBY    { $$ = "SelectedBy"; }
  | LOGICSIGNAL    { $$ = "LogicSignal"; }
  | CONNECTION     { $$ = "Connection"; }
  | PORTGROUP      { $$ = "PortGroup"; }
  | ACCESSLINK     { $$ = "AccessLink"; }
  | IPROCSFORMODULE { $$ = "iProcsForModule"; }
  | OF             { $$ = "Of"; }
  ;

/* ------------------------------------------------------------------ values */

conn_expr
  : postfix              { $$ = $1; }
  | postfix ARROW postfix { $$ = $1 + " -> " + $3; }
  ;

value
  : postfix { $$ = $1; }
  ;

postfix
  : primary                           { $$ = $1; }
  | postfix DOT IDENT                 { $$ = $1 + "." + $3; }
  | postfix DOT NUMBER                { $$ = $1 + "." + $3; }
  | postfix LBRACKET NUMBER RBRACKET            { $$ = $1 + "[" + $3 + "]"; }
  | postfix LBRACKET NUMBER COLON NUMBER RBRACKET { $$ = $1 + "[" + $3 + ":" + $5 + "]"; }
  ;

primary
  : IDENT  { $$ = $1; }
  | NUMBER { $$ = $1; }
  | STRING { $$ = $1; }
  ;

%%
