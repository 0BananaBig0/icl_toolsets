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
%token LPAREN "("
%token RPAREN ")"

/* Left associative and all at one level: `a - b - c` must group
   (a - b) - c, and there is no precedence between operators because
   the formatter reproduces them as written rather than evaluating
   them. Without this bison reports 2 shift/reduce conflicts. */
%left OPERATOR

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
%token TODOPORT "ToDoPort"
%token TOSELECTPORT "ToSelectPort"
%token DATAINPORT "DataInPort"
%token DATAOUTPORT "DataOutPort"
%token SCANINTERFACE "ScanInterface"
%token ONEHOTSCANINTERFACE "OneHotScanInterface"
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
%type <AstNode*> module_item instance_item instance_block
%type <AstNode*> port_decl
%type <std::string> port_keyword
%type <std::vector<icl::PortGroupItem>> port_group_items
%type <icl::PortGroupItem> port_group_item
%type <std::string> of_kw opt_range attr_stmt_name value_list module_ref std_tap_kw attr_value value_no_lead_op name_block_kw onehot_block_kw range_block_kw sel_block_kw of_block_kw any_keyword
%type <std::string> value
%type <std::string> postfix
%type <std::string> primary
%type <std::string> attr_name
%type <std::vector<AstNode*>> logic_body
%type <AstNode*> ns_stmt
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
  | PARAMETER IDENT EQUALS value_list SEMICOLON
      { $$ = driver.MakeStatement("Parameter " + $2 + " = " + $4 + ";",
                                 SpanOf(@1.begin.line, @5.end.line)); }
  | LOCALPARAMETER IDENT EQUALS value_list SEMICOLON
      { $$ = driver.MakeStatement("LocalParameter " + $2 + " = " + $4 + ";",
                                 SpanOf(@1.begin.line, @5.end.line)); }
  | ns_stmt { $$ = $1; }
  | ATTRIBUTE IDENT EQUALS value_list SEMICOLON
      { $$ = driver.MakeStatement("Attribute " + $2 + " = " + $4 + ";",
                                 SpanOf(@1.begin.line, @5.end.line)); }
  /* Without a value: the grammar makes `= value` optional —
     attribute_def : 'Attribute' attribute_name ('=' attribute_value)? ';'
     so a bare `Attribute foo;` is legal. */
  | ATTRIBUTE IDENT SEMICOLON
      { $$ = driver.MakeStatement("Attribute " + $2 + ";",
                                  SpanOf(@1.begin.line, @3.end.line)); }
  /* allowBroadcast_def : 'AllowBroadcastOnScanInterface' scanInterface_name ';'
     Marks a scan interface as allowing broadcast writes. It takes a single
     name, not a list — scanInterface_name is one SCALAR_ID — so there is no
     comma form. */
  | ALLOWBROADCAST IDENT SEMICOLON
      { $$ = driver.MakeStatement("AllowBroadcastOnScanInterface " + $2 + ";",
                                  SpanOf(@1.begin.line, @3.end.line)); }
  /* The alias name may carry a bit range: `Alias mode[1:0] = DI[6:5];` —
     both the bare and the ranged form occur in real instrument modules. */
  /* The right side is a *concatenation*, not a single signal:
     concat_hier_data_signal : '~'? hier_data_signal (',' '~'?
     hier_data_signal)* — so `Alias MyName1[2:0] = u1.a[2], u2.u3.b[1:0];`
     and `Alias CNTL[42:0] = ADDR[7:0], DATA[31:0] { AccessTogether; }` are
     both standard forms, and both were rejected at the comma. */
  | ALIAS IDENT opt_range EQUALS value_list SEMICOLON
      { $$ = driver.MakeStatement("Alias " + $2 + $3 + " = " + $5 + ";",
                                 SpanOf(@1.begin.line, @6.end.line)); }
  | ALIAS IDENT opt_range EQUALS value_list LBRACE attr_body RBRACE
      {
        $$ = driver.MakeBlock("Alias " + $2 + $3 + " = " + $5, std::move($7),
                              SpanOf(@1.begin.line, @8.end.line));
      }
  /* Everything below is grouped by *shape*, not by keyword. Five shapes cover
     all 13 ICL description elements, so a new keyword is one line in the right
     group rather than two new productions — and the groups cannot drift apart,
     because there is only one copy of each shape. */
  | name_block_kw IDENT LBRACE attr_body RBRACE
      {
        $$ = driver.MakeBlock($1 + " " + $2, std::move($4),
                              SpanOf(@1.begin.line, @5.end.line));
      }
  /* One-hot groups take a *range* on their name — the published grammar gives
     them `reg_port_signal_id`, the same as a register — and only the block
     form. They are not in `range_block_kw` because that group also permits
     `name;`, which these do not have. */
  | onehot_block_kw IDENT opt_range LBRACE attr_body RBRACE
      {
        $$ = driver.MakeBlock($1 + " " + $2 + $3, std::move($5),
                              SpanOf(@1.begin.line, @6.end.line));
      }
  | range_block_kw IDENT opt_range LBRACE attr_body RBRACE
      {
        $$ = driver.MakeBlock($1 + " " + $2 + $3, std::move($5),
                              SpanOf(@1.begin.line, @6.end.line));
      }
  | range_block_kw IDENT opt_range SEMICOLON
      { $$ = driver.MakeStatement($1 + " " + $2 + $3 + ";",
                                  SpanOf(@1.begin.line, @4.end.line)); }
  | port_decl { $$ = $1; }
  | sel_block_kw IDENT opt_range SELECTEDBY value_list LBRACE mux_body RBRACE
      {
        // `SelectedBy` is part of the standard; the selector may be a
        // hierarchical name such as `MyTap.M3_sel`, hence |value_list| rather
        // than a bare IDENT — and a *list*, because Tessent writes
        // `SelectedBy config_sib_shift_reg, enable_shift_reg` when a mux is
        // selected by more than one signal.
        $$ = driver.MakeBlock($1 + " " + $2 + $3 + " SelectedBy " + $5,
                              std::move($7),
                              SpanOf(@1.begin.line, @8.end.line));
      }
  /* The same thing without `SelectedBy`: `ScanMux mux1 sr { ... }`. Not the
     standard form — IEEE 1687.1 requires the keyword — but it is how the
     pre-standard drafts wrote it, and it survives in published ICL (e.g. the
     SIB module in "Reusing and Retargeting On-Chip Instrument Access
     Procedures in IEEE P1687", 2012). Tolerated for the same reason `Reg` and
     `Field` are: refusing to format a file we can format is worse than
     accepting one that predates the keyword. */
  | sel_block_kw IDENT opt_range value LBRACE mux_body RBRACE
      {
        $$ = driver.MakeBlock($1 + " " + $2 + $3 + " " + $4,
                              std::move($6),
                              SpanOf(@1.begin.line, @7.end.line));
      }
  | instance_item
      { $$ = $1; }
  /* LogicSignal holds a *boolean expression*, not a list of attributes:
     `LogicSignal x { a && b; }`, `LogicSignal y { enable, sib == 2'b11; }`.
     The general attribute body cannot hold either — in `a && b`, LALR(1)
     cannot tell at the operator whether `a` is an attribute name followed by
     a unary value or the start of an expression. Rather than leave that to
     bison's rule ordering, LogicSignal gets a body of its own. */
  | LOGICSIGNAL IDENT opt_range LBRACE logic_body RBRACE
      {
        $$ = driver.MakeBlock("LogicSignal " + $2 + $3, std::move($5),
                              SpanOf(@1.begin.line, @6.end.line));
      }
  | CONNECTION LBRACE conn_body RBRACE
      {
        $$ = driver.MakeBlock("Connection", std::move($3),
                              SpanOf(@1.begin.line, @4.end.line));
      }
  | PORTGROUP IDENT LBRACE port_group_items RBRACE
      { $$ = driver.MakePortGroup($2, std::move($4),
                                  SpanOf(@1.begin.line, @5.end.line)); }
  ;

/* -------------------------------------------------------------------- ports */

port_keyword
  : SCANINPORT       { $$ = "ScanInPort"; }
  /* These four are in the ICL grammar (the `port_def` list of a published
     ANTLR grammar for 1687) though not in any file available here. Added
     together because they are the same kind of thing, and a formatter that
     refuses a legal port declaration is worse than one that accepts an
     unusual one. */
  | TOIRSELECTPORT   { $$ = "ToIRSelectPort"; }
  | ADDRESSPORT      { $$ = "AddressPort"; }
  | WRITEENPORT      { $$ = "WriteEnPort"; }
  | READENPORT       { $$ = "ReadEnPort"; }
  | SCANOUTPORT      { $$ = "ScanOutPort"; }
  | SHIFTENPORT      { $$ = "ShiftEnPort"; }
  | CAPTUREENPORT    { $$ = "CaptureEnPort"; }
  | UPDATEENPORT     { $$ = "UpdateEnPort"; }
  | SELECTPORT       { $$ = "SelectPort"; }
  | RESETPORT        { $$ = "ResetPort"; }
  | TCKPORT          { $$ = "TCKPort"; }
  | CLOCKPORT        { $$ = "ClockPort"; }
  | TOCLOCKPORT      { $$ = "ToClockPort"; }
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


/* A comma-separated run of values that does not start with an operator.
   Used for attribute values, where `name - 1` must not be read as an
   expression. */
attr_value
  : value_no_lead_op                    { $$ = $1; }
  /* A leading `::` at the *start* of the value, as in an iProc namespace:
     `iProc_namespace : (namespace_name? '::')? ref_module_name ...`, where
     the prefix is optional, so `::mod` names a module in the global
     namespace and `WriteCallBack ::mod p <R>;` is legal. Placed here rather
     than in `value_no_lead_op` because there it is genuinely ambiguous: `a
     :: b` would be either one qualified name or two space-separated values,
     and those render differently. At the start of a value there is nothing
     before it to attach to, so there is only one reading — and, unlike the
     change to `value_no_lead_op`, this one adds no conflicts. */
  | DCOLON value_no_lead_op             { $$ = "::" + $2; }
  | attr_value COMMA value_no_lead_op   { $$ = std::move($1) + ", " + $3; }
  /* A later element may carry a unary operator of its own. Every signal type
     in the standard is written `'~'? signal` and the concatenation rules put
     that per element (`concat_data_signal : data_signal (',' data_signal)*`),
     so `Source ~a, b;` and `Source a, ~b;` are both legal and were both
     rejected — the list accepted an operator only in first position. */
  | attr_value COMMA OPERATOR value_no_lead_op
      { $$ = std::move($1) + ", " + $3 + $4; }
  /* Also space-separated: `ReadCallBack m::p <D>;`, where a name, a
     namespace-qualified name and iProc arguments follow the keyword with
     nothing between them. */
  | attr_value value_no_lead_op         { $$ = std::move($1) + " " + $2; }
  /* A binary operator inside an attribute value. `signal : (number |
     reg_port_signal_id | hier_port)` and `number : ... | integer_expr`, so
     `Source 1 + 1;` is reachable in the published grammar. Costs nothing now
     that the left side of an assignment is `postfix`: `value_no_lead_op`
     cannot start with an operator, so `a + b` has one reading only. */
  | attr_value OPERATOR value_no_lead_op
      { $$ = std::move($1) + " " + $2 + " " + $3; }
  ;

value_no_lead_op
  : postfix                   { $$ = $1; }
  /* Left recursive, so a qualified name may have any number of segments:
     `a::b::c`. An iProc namespace is `ns::module::sub` in the grammar, so
     two separators occur in one name, and there is no reason to stop there. */
  | value_no_lead_op DCOLON postfix { $$ = $1 + "::" + $3; }
  ;

/* The body of a LogicSignal: expression statements and the attribute forms
   that cannot be confused with them. Comments too, so a documented
   expression keeps its comment. */
logic_body
  : %empty
      { $$ = std::vector<AstNode*>(); }
  | logic_body COMMENT
      { $$ = std::move($1);
        $$.push_back(driver.AddComment($2,
                                       SpanOf(@2.begin.line, @2.end.line))); }
  | logic_body value_list SEMICOLON
      { $$ = std::move($1);
        $$.push_back(driver.MakeStatement($2 + ";",
                                          SpanOf(@2.begin.line, @3.end.line))); }
  | logic_body attr_name EQUALS value_list SEMICOLON
      { $$ = std::move($1);
        $$.push_back(driver.MakeStatement($2 + " = " + $4 + ";",
                                          SpanOf(@2.begin.line, @5.end.line))); }
  /* A LogicSignal also carries ordinary attributes — `Source TCK;` — and
     nested blocks such as `Select { ... }`. Both are unambiguous against an
     expression: two adjacent names are not a valid expression, and `{`
     cannot follow one. */
  | logic_body attr_name value_no_lead_op SEMICOLON
      { $$ = std::move($1);
        $$.push_back(driver.MakeStatement($2 + " " + $3 + ";",
                                          SpanOf(@2.begin.line, @4.end.line))); }
  | logic_body attr_name LBRACE attr_body RBRACE
      { $$ = std::move($1);
        $$.push_back(driver.MakeBlock($2, std::move($4),
                                      SpanOf(@2.begin.line, @5.end.line))); }
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
  | attr_name OPERATOR value_list SEMICOLON
      { $$ = driver.MakeStatement($1 + " " + $2 + $3 + ";",
                                  SpanOf(@1.begin.line, @4.end.line)); }
  /* Also a list: Tessent writes `Attribute x = "a", "b";` for a pin list
     too long for one line. */
  | attr_name EQUALS value_list SEMICOLON
      { $$ = driver.MakeStatement($1 + " = " + $3 + ";",
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
  /* The block form of an instance, reachable here too: the published grammar
     lists `instance_def` among `oneHotDataGroup_item`.

     Only the block form. The `;` form is not, because `Instance i Of M;` is
     genuinely ambiguous here against `attr_name attr_value ';'` once attribute
     values may be space-separated — a real shift/reduce conflict, not a
     stylistic objection. */
  /* `instance_item`, not `instance_block`: an instance without a body —
     `Instance i Of M;` — is one of the two forms the published grammar gives
     (the other being `{ ... }`), and oneHotDataGroup_item admits an
     instance_def. Referencing only the braced rule meant the semicolon form
     worked at module level and failed inside a block, which is the same
     asymmetry this shared rule exists to prevent. */
  | instance_item
      { $$ = $1; }

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



mux_body
  : %empty
      { $$ = std::vector<AstNode*>(); }
  | mux_body COMMENT
      { $$ = std::move($1); $$.push_back(driver.AddComment($2, SpanOf(@2.begin.line, @2.end.line))); }
  /* Both sides may be comma-separated: Tessent writes
     `1'b1, 3'b00x : STEP_COUNTER_REG[0];` when one selection covers several
     selector codes. */
  | mux_body value_list COLON value_list SEMICOLON
      { $$ = std::move($1);
        $$.push_back(driver.MakeStatement($2 + " : " + $4 + ";",
                                          SpanOf(@2.begin.line, @5.end.line))); }
  /* A bare entry with no selector. ClockMux and DataMux list their sources
     this way, whereas ScanMux always pairs a selector with a source. */
  | mux_body value SEMICOLON
      { $$ = std::move($1);
        $$.push_back(driver.MakeStatement($2 + ";",
                                          SpanOf(@2.begin.line, @3.end.line))); }
  /* A mux body can also carry attributes: Tessent puts
     `Attribute exclude_codes_from_icl_verify = "{2'b01}";` among the
     selection entries. Only the `name = value` and `name ;` forms are
     allowed here — a full attribute body would make `2'b10 : x;` ambiguous,
     since both an attribute value and a selector can start the same way.
     Not `name ;`: that collides with a bare mux entry, which is also a single
     name followed by `;`. */
  | mux_body attr_name EQUALS value_list SEMICOLON
      { $$ = std::move($1);
        $$.push_back(driver.MakeStatement($2 + " = " + $4 + ";",
                                          SpanOf(@2.begin.line, @5.end.line))); }
  /* ...and the `Attribute name = value;` spelling, which is a keyword, then
     the attribute's own name, then `=`. */
  | mux_body attr_name IDENT EQUALS value_list SEMICOLON
      { $$ = std::move($1);
        $$.push_back(driver.MakeStatement($2 + " " + $3 + " = " + $5 + ";",
                                          SpanOf(@2.begin.line, @6.end.line))); }
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
  | attr_body attr_name LBRACE attr_body RBRACE
      {
        AstBlock* nested = driver.MakeBlock($2, std::move($4),
                                           SpanOf(@2.begin.line, @5.end.line));
        $$ = std::move($1);
        $$.push_back(nested);
      }
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
  : IDENT opt_range
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
  : any_keyword { $$ = $1; }
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
attr_name
  : IDENT        { $$ = $1; }
  | any_keyword  { $$ = $1; }
  ;


/* `Instance i Of M { ... }` and `Instance i Of M;`, in one place.

   The published grammar allows an instance inside a one-hot data group
   (`oneHotDataGroup_item : instance_def | dataRegister_def | ...`), so this is
   referenced from the shared attribute list as well as from module_item — one
   definition, no second copy to drift. */
instance_block
  : of_block_kw IDENT of_kw module_ref LBRACE attr_body RBRACE
      {
        $$ = driver.MakeBlock($1 + " " + $2 + " " + $3 + " " + $4, std::move($6),
                              SpanOf(@1.begin.line, @7.end.line));
      }
  ;

instance_item
  : instance_block
      { $$ = $1; }
  | of_block_kw IDENT of_kw module_ref SEMICOLON
      { $$ = driver.MakeStatement($1 + " " + $2 + " " + $3 + " " + $4 + ";",
                                  SpanOf(@1.begin.line, @5.end.line)); }
  ;

/* ------------------------------------------------- keyword groups by shape */

/* Plain identifier only: `enum_name : SCALAR_ID` and
   `scanInterface_name : SCALAR_ID` in the published grammar, no range. */
name_block_kw
  : ENUM                 { $$ = "Enum"; }
  | SCANINTERFACE        { $$ = "ScanInterface"; }
  ;

onehot_block_kw
  : ONEHOTSCANINTERFACE  { $$ = "OneHotScanInterface"; }
  | ONEHOTSCANGROUP      { $$ = "OneHotScanGroup"; }
  | ONEHOTDATAGROUP      { $$ = "OneHotDataGroup"; }
  ;

range_block_kw
  : SCANREGISTER         { $$ = "ScanRegister"; }
  | DATAREGISTER         { $$ = "DataRegister"; }
  | REG                  { $$ = "Reg"; }
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

module_ref
  : IDENT                    { $$ = $1; }
  | std_tap_kw               { $$ = $1; }
  | module_ref DCOLON IDENT  { $$ = $1 + "::" + $3; }
  /* A leading `::` with no namespace in front of it — the global namespace.
     The published grammar allows it *only* here: instance_def writes the
     qualifier as `(namespace_name? '::')?`, namespace optional, while a signal
     reference has no such form. So `Instance i Of ::M;` parses and
     `Source ::M;` does not, and that asymmetry is the standard's, not two
     rules drifting apart.

     Allowing it in `value` too was tried first, for symmetry; it costs three
     shift/reduce conflicts, because `attr_value` also permits space-separated
     values and a bare `::` then has two readings. */
  | DCOLON IDENT             { $$ = "::" + $2; }
  ;


sel_block_kw
  : SCANMUX              { $$ = "ScanMux"; }
  | DATAMUX              { $$ = "DataMux"; }
  | CLOCKMUX             { $$ = "ClockMux"; }
  ;

of_block_kw
  : INSTANCE             { $$ = "Instance"; }
  | ACCESSLINK           { $$ = "AccessLink"; }
  ;

/* Everything above, plus the keywords that have no block form. One place to
   add a keyword; every list that needs "any keyword" refers to this. */
any_keyword
  : name_block_kw   { $$ = $1; }
  | onehot_block_kw { $$ = $1; }
  | range_block_kw  { $$ = $1; }
  | sel_block_kw    { $$ = $1; }
  /* `of_block_kw` is deliberately absent. `Instance` and `AccessLink` are
     always followed by `name Of module`, which `instance_item` already covers;
     letting them also be bare attribute names makes `Instance` ambiguous
     between the two — a real shift/reduce conflict, and the reason an instance
     inside a one-hot data group could not be accepted without this. */
  | port_keyword    { $$ = $1; }
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
  | STD1149_1_2001  { $$ = "STD_1149_1_2001"; }
  | STD1149_1_2013  { $$ = "STD_1149_1_2013"; }
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

value
  : postfix { $$ = $1; }
  /* A connection edge, `a -> b`, is only written inside `Connection`, but
     living here keeps one expression grammar. Split out as `conn_expr` it
     used to cost a second nonterminal and a reduce/reduce conflict. */
  | postfix ARROW postfix { $$ = $1 + " -> " + $3; }
  /* Unary: `ResetValue -1;`, `Source ~a;`. */
  | OPERATOR value { $$ = $1 + $2; }
  /* Binary. No precedence table: the formatter reproduces the operators as
     written, so left-to-right grouping is enough and unambiguous. */
  | value OPERATOR value { $$ = $1 + " " + $2 + " " + $3; }
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
  | STRING   { $$ = $1; }
  | PARAMREF { $$ = $1; }
  /* `ReadCallBack m::p <D>;` — an iProc argument marker. */
  | IPROCARG { $$ = $1; }
  ;

%%
