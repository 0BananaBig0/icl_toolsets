#!/usr/bin/env python3
"""Walks the grammar one block at a time and checks each one in isolation.

This is the check described as "do one block, confirm it, then the next" —
enum first, then DataOutPort, and so on — written out so it runs the same way
every time instead of being redone by hand each round.

It is separate from the corpus on purpose. The corpus is files somebody once
found or wrote; a failure in it points at a file. These cases are generated
per block from the published grammar, so a failure names the block and the
shape, and the two do not depend on each other: if a corpus file is deleted
this still covers the construct, and if this list is wrong the corpus still
holds.

Every case is checked three ways, because accepting input is not enough:
  1. it parses;
  2. formatting it twice equals formatting it once (idempotent);
  3. it is the same text once whitespace is removed (nothing was dropped).

Usage: block_review.py --binary BIN
"""

import argparse
import os
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))


def run(binary, source):
    """Returns None when fine, else a short reason."""
    result = subprocess.run([binary],
                            input=source.encode(),
                            stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE)
    if result.returncode != 0:
        return 'rejected'
    out = result.stdout.decode('utf-8', 'replace')
    again = subprocess.run([binary], input=out.encode(),
                           stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    if again.returncode != 0:
        return 'accepted once, rejected on the second pass'
    if again.stdout.decode('utf-8', 'replace') != out:
        return 'not idempotent'
    # Comparing without whitespace is what catches a dropped token: a
    # formatter that merely reflows keeps every character.
    if re.sub(r'\s+', '', source) != re.sub(r'\s+', '', out):
        return 'characters lost'
    return None


# ---------------------------------------------------------------------------
# The blocks, in the order the standard presents them. Each entry is
# (name, [source, ...]) and every source is a whole file, not a fragment:
# a formatter sees files, and a fragment that only parses inside some other
# context would be testing a shape that cannot occur.
# ---------------------------------------------------------------------------

def blocks():
    b = []

    # `TOP` separates a top-level statement from the module that follows it.
    # `chr(10)` rather than an inline "\n": an escaped newline written inside
    # a case string is easy to misread as part of the case, and a real
    # newline in the source breaks the literal outright.
    TOP = chr(10)

    # ---- module ----
    # module_def : 'Module' module_name '{' module_item* '}'
    # `module_item` has eighteen alternatives in the published grammar, and
    # module level is the one place most of them can appear. Rather than
    # assume the set was covered by whichever case happened to use each one,
    # all eighteen are listed here — one item, one module, so a failure names
    # the alternative rather than a file that happens to contain it.
    #
    #   module_item : useNameSpace_def | port_def | instance_def
    #               | scanRegister_def | dataRegister_def | logicSignal_def
    #               | scanMux_def | dataMux_def | clockMux_def
    #               | oneHotDataGroup_def | oneHotScanGroup_def
    #               | scanInterface_def | accessLink_def | alias_def
    #               | enum_def | parameter_def | localParameter_def
    #               | attribute_def ;
    #
    b.append(('module', [
        'Module m { }',
        'Module m {\n}\n',
        'Module m { Attribute a = 1; }',
        'Module m { ScanInPort si; }',
        'Module m {\n  ScanInPort si;\n  ScanOutPort so;\n}\n',
        'Module m { UseNameSpace ns; }',
        'Module m { Instance i Of M; }',
        'Module m { ScanRegister sr { ScanInSource si; } }',
        'Module m { DataRegister dr { WriteEnSource we; } }',
        'Module m { LogicSignal ls { a & b; } }',
        "Module m { ScanMux mx SelectedBy s { 1'b0 : a; } }",
        "Module m { DataMux mx SelectedBy s { 1'b0 : a; } }",
        "Module m { ClockMux mx SelectedBy s { 1'b0 : a; } }",
        'Module m { OneHotDataGroup g { Port d; } }',
        'Module m { OneHotScanGroup g { Port a; } }',
        'Module m { ScanInterface si { Port si; } }',
        'Module m { AccessLink al Of STD_1149_1_2001 { BSDLEntity b; I { ScanInterface { u; } } } }',
        'Module m { Alias a = b; }',
        'Module m { Enum E { a = 1; } }',
        'Module m { Parameter p = 4; }',
        'Module m { LocalParameter lp = 4; }',
        # Two whose value or name is optional:
        #   attribute_def : 'Attribute' attribute_name ('=' attribute_value)? ;
        #   useNameSpace_def : 'UseNameSpace' namespace_name? ';' ;
        'Module m { Attribute x; }',
        'Module m { UseNameSpace; }',
        # Tessent's, not the standard's: `->` edges.




    ]))

    # ---- namespace ----
    # namespace_def : 'NameSpace' nameSpace_name? ';'
    b.append(('namespace', [
        'NameSpace;\n',
        'NameSpace ns;\n',
        'NameSpace ns;\nModule m { }\n',
        'UseNameSpace ns;\n',
        'UseNameSpace;\n',
        'UseNameSpace ns;\nModule m { }\n',
    ]))

    # ---- enum ----
    # enum_def : 'Enum' enum_name '{' enum_item+ '}'
    # enum_item : enum_symbol '=' concat_number ';'
    b.append(('enum', [
        'Module m { Enum e { a = 1; } }',
        'Module m { Enum e { a = 1; b = 2; } }',
        "Module m { Enum e { a = 1'b0; } }",
        "Module m { Enum e { a = 3'b101; } }",
        "Module m { Enum e { a = 4'hF; } }",
        "Module m { Enum e { a = 2'd3; } }",
        "Module m { Enum e { a = 3'bx0x; } }",
        "Module m { Enum e { HWDefault = 3'b010; } }",
        # concat_number: an enum value may be a list
        "Module m { Enum e { a = 1'b0, 1'b1; } }",
        # RefEnum pointing at it, which is how an enum is used
        'Module m {\n  Enum e { a = 1; }\n  DataInPort di { RefEnum e; }\n}\n',
        # concat_number is `'~'? number (',' '~'? number)*`, so a value may
        # carry `~` on any element, not only the first.
        'Module m { Enum e { a = ~1; } }',
        'Module m { Enum e { a = 1, ~2; } }',
        'Module m { Enum e {\n  // the values\n  a = 1;\n} }',
    ]))
    # Deliberately absent: `Enum e { a = 1; }` at top level. The published
    # grammar has
    #
    #   iclSource_items : nameSpace_def | useNameSpace_def | module_def
    #
    # and enum_def is a module_item, not an iclSource item — so an Enum
    # outside a module is not ICL. That shape is pinned in
    # fail/standard/top_level_enum.icl rather than here, because here it
    # would read as a case that ought to parse.

    # ---- ports ----
    # Every port block in the standard, each in the four shapes a port can
    # take: plain, with a range, with a body, with both.
    PORTS = [
        'ScanInPort', 'ScanOutPort', 'ShiftEnPort', 'CaptureEnPort',
        'UpdateEnPort', 'SelectPort', 'ResetPort', 'TCKPort', 'TMSPort',
        'TRSTPort', 'ClockPort', 'DataInPort', 'DataOutPort', 'AddressPort',
        'WriteEnPort', 'ReadEnPort', 'ToShiftEnPort', 'ToCaptureEnPort',
        'ToUpdateEnPort', 'ToSelectPort', 'ToResetPort', 'ToTCKPort',
        'ToTMSPort', 'ToTRSTPort', 'ToIRSelectPort', 'ToClockPort',
    ]
    port_cases = []
    for p in PORTS:
        prop = {
            "DataInPort": "RefEnum E;",
            "ScanInPort": "Attribute flag;",
            "ShiftEnPort": "Attribute flag;",
            "CaptureEnPort": "Attribute flag;",
            "UpdateEnPort": "Attribute flag;",
            "SelectPort": "Attribute flag;",
            "ResetPort": "ActivePolarity 0;",
            "TMSPort": "Attribute flag;",
            "TCKPort": "Attribute flag;",
            "ClockPort": "DifferentialInvOf clk;",
            "TRSTPort": "Attribute flag;",
            "ToIRSelectPort": "Attribute flag;",
            "AddressPort": "Attribute flag;",
            "WriteEnPort": "Attribute flag;",
            "ReadEnPort": "Attribute flag;",
        }.get(p, "Source s;")
        port_cases.append('Module m { %s p; }' % p)
        port_cases.append('Module m { %s p[3:0]; }' % p)
        empty_body = "Attribute flag;" if p in ("ToSelectPort", "ToResetPort", "ToClockPort", "ToTRSTPort") else ""
        port_cases.append('Module m { %s p { %s } }' % (p, empty_body))
        port_cases.append('Module m { %s p[3:0] { %s } }' % (p, empty_body))
        port_cases.append('Module m { %s p { %s } }' % (p, prop))
        port_cases.append('Module m { %s p { Attribute a = 1; } }' % p)
        # a comment in each of three places inside the body
        port_cases.append('Module m { %s p {\n  // before\n  %s\n} }' % (p, prop))
        port_cases.append('Module m { %s p {\n  %s // after\n} }' % (p, prop))
        port_cases.append('Module m { %s p {\n  %s\n  // trailing\n} }' % (p, prop))
    b.append(('ports', port_cases))

    # ---- scan register ----
    # scanRegister_item : attribute_def | scanInSource | defaultLoadValue
    #                   | captureSource | resetValue | refEnum
    b.append(('scanRegister', [
        'Module m { ScanRegister sr; }',
        'Module m { ScanRegister sr[7:0]; }',
        'Module m { ScanRegister sr { ScanInSource si; } }',
        'Module m { ScanRegister sr { CaptureSource c; } }',
        "Module m { ScanRegister sr { DefaultLoadValue 1'b0; } }",
        "Module m { ScanRegister sr { ResetValue 1'b0; } }",
        'Module m { ScanRegister sr { RefEnum e; } }',
        'Module m { ScanRegister sr { Attribute a = 1; } }',
        'Module m { ScanRegister sr {\n'
        '  ScanInSource si;\n'
        '  CaptureSource c;\n'
        '  ResetValue 0;\n'
        '}\n}',
        # the three Tessent writes across lines
        'Module m { ScanRegister sr {\n'
        '  CaptureSource a,\n'
        '      b;\n'
        '}\n}',
    ]))

    # ---- data register ----
    # dataRegister_type : selectable | addressable | readCallBack | writeCallBack
    b.append(('dataRegister', [
        'Module m { DataRegister dr; }',
        'Module m { DataRegister dr[7:0]; }',
        'Module m { DataRegister dr { WriteDataSource si; } }',
        'Module m { OneHotDataGroup g { DataRegister dr { AddressValue 4; } } }',
        'Module m { DataRegister dr { ReadCallBack m rd; } }',
        'Module m { DataRegister dr { WriteCallBack m wr; } }',
        'Module m { DataRegister dr { WriteDataSource a; WriteEnSource b; } }',
        'Module m { DataRegister dr { ReadDataSource a; } }',
        'Module m { DataRegister dr { ResetValue 0; } }',
        # iProc callbacks with their argument forms
        'Module m { DataRegister dr { WriteCallBack m p <D>; } }',
        'Module m { DataRegister dr { WriteCallBack ns p <R>; } }',
        'Module m { DataRegister dr { WriteCallBack ::M p <D>; } }',
        'Module m { DataRegister dr { WriteCallBack m p <D> <R> 5 "s" $X; } }',
    ]))

    # ---- scan interface ----
    # scanInterface_item : attribute_def | scanInterfacePort_def
    #                    | defaultLoad_def | scanInterfaceChain_def
    b.append(('scanInterface', [
        'Module m { ScanInterface si { Port p; } }',
        'Module m { ScanInterface si { Port p; Port q; } }',
        "Module m { ScanInterface si { DefaultLoadValue 1'b0; } }",
        'Module m { ScanInterface si { Attribute a = 1; } }',
        'Module m { ScanInterface si { Chain c { Port p; } } }',
        'Module m { ScanInterface si {\n'
        '  Chain c1 { Port SI_0; Port SO_0; }\n'
        '  Chain c2 { Port SI_1; Port SO_1; }\n'
        '}\n}',
        # chain items are the same three, minus Chain
        'Module m { ScanInterface si {\n'
        '  Chain c {\n'
        '    Port p;\n'
        '    DefaultLoadValue 0;\n'
        '    Attribute a = 1;\n'
        '  }\n}\n}',
    ]))

    # ---- alias ----
    # alias_def : 'Alias' alias_name ('[' range ']')? '='
    #             concat_hier_data_signal ';'  |  ... '{' alias_item* '}'
    b.append(('alias', [
        'Module m { Alias a = b; }',
        'Module m { Alias a[3:0] = b; }',
        'Module m { Alias a = b, c; }',
        'Module m { Alias a = ~b; }',
        'Module m { Alias a = b, ~c; }',
        'Module m { Alias a = b.c; }',
        'Module m { Alias a = b.c[1:0]; }',
        # '=' is not optional: alias_def is 'Alias' name '=' signal ...,
        # so `Alias a { ... }` with no value is rejected.
        'Module m { Alias a = b { AccessTogether; } }',
        'Module m { Alias a = b { iApplyEndState 1; } }',
        'Module m { Alias a = b { RefEnum e; } }',
        # the standard's own examples, completed into whole modules
        'Module m {\n'
        '  ScanRegister u1_a { ScanInSource si; }\n'
        '  ScanRegister u2_u3_b { ScanInSource si; }\n'
        '  Alias MyName1[2:0] = u1.a[2], u2.u3.b[1:0];\n'
        '}\n',
    ]))

    # ---- logic signal ----
    # logicSignal_def : 'LogicSignal' logicSignal_name '{' logic_expr+ '}'
    b.append(('logicSignal', [
        'Module m { LogicSignal ls { a; } }',
        'Module m { LogicSignal ls { a && b; } }',
        'Module m { LogicSignal ls { a || b; } }',
        'Module m { LogicSignal ls { !a; } }',
        'Module m { LogicSignal ls { ~a; } }',
        'Module m { LogicSignal ls { (a && b) || (!c && !d); } }',
        'Module m { LogicSignal ls { a & b; } }',
        'Module m { LogicSignal ls { a == 1; } }',
        "Module m { LogicSignal ls { a, b == 2'b11; } }",
        'Module m { LogicSignal ls { a.b && c.d; } }',
        # nested parens, since the expression rule is recursive
        'Module m { LogicSignal ls { ' + '(' * 20 + 'a' + ')' * 20 + '; } }',
    ]))

    # ---- instance ----
    # instance_def : 'Instance' instance_name 'Of' module_ref (';' | '{' ... '}')
    b.append(('instance', [
        'Module m { Instance i Of M; }',
        'Module m { Instance i Of M { } }',
        'Module m { Instance i Of M { InputPort a = b; } }',
        'Module m { Instance i Of M { Attribute a = 1; } }',
        'Module m { Instance i Of ns::M; }',
        'Module m { Instance i Of ::M; }',
        'Module m { Instance i Of a::c; }',
        'Module m { Instance i of M; }',          # Origen spells it lower case
        'Module m { Instance i Of M { Parameter P = 4; } }',
    ]))

    # ---- muxes ----
    # scanMux_def | dataMux_def | clockMux_def
    # The three mux kinds have identical shapes —
    #   scanMux_def : 'ScanMux' scanMux_name 'SelectedBy' scanMux_select
    #                 '{' scanMux_selection+ '}'
    # — and scanMux_name : reg_port_signal_id, so the name and the select may
    # each carry a range. Checked per kind rather than once, because the three
    # used to be three rules and only one of them was ever exercised.
    b.append(('mux', [
        "Module m { ScanMux mx SelectedBy s { 1'b0 : a; } }",
        "Module m { ScanMux mx SelectedBy s { 1'b0 : a; 1'b1 : b; } }",
        "Module m { ScanMux mx SelectedBy s { 1'b0, 1'b1 : a; } }",
        "Module m { ScanMux mx SelectedBy s { 1'b0 | 1'b1 : a; } }",
        "Module m { ScanMux mx SelectedBy s { 1'b0 : a, b; } }",
        "Module m { DataMux mx SelectedBy s { 2'd0 : a; } }",
        "Module m { ClockMux mx SelectedBy s { 1'b0 : a; } }",
        "Module m { ScanMux mx SelectedBy s, t { 1'b0 : a; } }",
        "Module m { ScanMux mx[1:0] SelectedBy s { 1'b0 : a; } }",
        "Module m { ScanMux mx SelectedBy s { 0 : a; } }",
        # Name and select with a range, for each of the three kinds.
        "Module m { DataMux mx[1:0] SelectedBy s { 2'b00 : a; } }",
        "Module m { ClockMux mx[1:0] SelectedBy s { 2'b00 : a; } }",
        "Module m { DataMux mx SelectedBy s[1:0] { 2'b00 : a; } }",
        "Module m { ClockMux mx SelectedBy s[1:0] { 2'b00 : a; } }",
        # concat_number_list : concat_number ('|' concat_number)*
        "Module m { DataMux mx SelectedBy s { 1'b0 | 2'b01 : a; } }",
        "Module m { ClockMux mx SelectedBy s { 1'b0 | 2'b01 : a; } }",
        # `~` on a selection, and an attribute among the selections.
        "Module m { ScanMux mx SelectedBy s { ~1'b0 : a; } }",
        "Module m { ScanMux mx SelectedBy s { Attribute x = 1; 1'b0 : a; } }",
        "Module m { ScanMux mx SelectedBy s {\n  // the table\n  1'b0 : a;\n} }",
        "Module m { ScanMux mx SelectedBy s {\n  1'b0 : a; // after\n} }",
        # a comment among the selections
        "Module m { ScanMux mx SelectedBy s {\n"
        "  1'b0 : a;\n"
        "  // which one\n"
        "  1'b1 : b;\n"
        "} }",
    ]))

    # ---- one-hot groups ----
    # The two one-hot groups look alike and are not. Their names match
    # (`oneHotScanGroup_name : reg_port_signal_id`), but their contents do
    # not, and it is easy to assume they share a body:
    #
    #   oneHotScanGroup_item : 'Port' concat_scan_signal ';' ;
    #   oneHotDataGroup_item : instance_def | dataRegister_def
    #                        | oneHotDataGroup_portSource ;
    #
    # So a scan group holds only `Port` items, while a data group holds
    # instances and data registers as well. Both are `item+` — at least one.
    # Both take a vector name, because reg_port_signal_id is
    # `SCALAR_ID | vector_id`.
    b.append(('onehot', [
        'Module m { OneHotScanGroup g { Port a; Port b; } }',
        'Module m { OneHotScanGroup g[3:0] { Port a; } }',
        'Module m { OneHotDataGroup g { DataRegister d; } }',
        'Module m { OneHotDataGroup g { Instance i Of M; } }',
        'Module m { OneHotScanGroup g { Port a; } }',
        # Both names are reg_port_signal_id, so both take an index as well as
        # a range.
        'Module m { OneHotScanGroup g[1] { Port a; } }',
        'Module m { OneHotDataGroup g[2:0] { Port d; } }',
        # A scan group's only item is Port, and concat_scan_signal means the
        # list takes `~` on any element, not just the first.
        'Module m { ScanInPort a; ScanInPort b; '
        'OneHotScanGroup g { Port a, b; } }',
        'Module m { ScanInPort a; ScanInPort b; '
        'OneHotScanGroup g { Port ~a, b; } }',
        'Module m { ScanInPort a; ScanInPort b; '
        'OneHotScanGroup g { Port ~a, ~b; } }',
        'Module m { ScanInPort a; ScanInPort b; '
        'OneHotScanGroup g { Port a; Port b; } }',
        "Module m { OneHotScanGroup g { Port 1'b0; } }",
        # A data group's Port is concat_data_signal, and it also holds
        # instances and data registers — in both the `;` and `{}` forms of
        # instance (§5.29).
        'Module m { DataInPort d; DataInPort e; '
        'OneHotDataGroup g { Port d, e; } }',
        'Module m { DataInPort d; OneHotDataGroup g { '
        'DataRegister dr { AddressValue 0; } Port d; } }',
        'Module m { DataInPort d; OneHotDataGroup g { '
        'Instance i Of M { InputPort x = d; } Port d; } }',
        # Comments, in the two positions that matter.
        'Module m { DataInPort d; OneHotDataGroup g {\n'
        '  Port d; // the data side\n} }',
        'Module m { DataInPort d; OneHotDataGroup g {\n'
        '  // first\n  Port d;\n} }',
    ]))

    # ---- access link ----
    # accessLink1149_def : 'AccessLink' name 'Of'
    #                      ('STD_1149_1_2001' | 'STD_1149_1_2013') '{' ... '}'
    b.append(('accessLink', [
        'Module m { AccessLink a Of STD_1149_1_2001 { BSDLEntity b; I { ScanInterface { u; } } } }',
        'Module m { AccessLink a Of STD_1149_1_2013 { BSDLEntity b; I { ScanInterface { u; } } } }',
        # A whole access link, in the shape the standard's own example has:
        # entity name, then instruction blocks naming what each selects.
        'Module m {\n'
        '  ScanInterface si { Port p; }\n'
        '  AccessLink a Of STD_1149_1_2001 {\n'
        '    BSDLEntity b;\n'
        '    READINSTR {\n'
        '      ScanInterface { si; }\n'
        '      ActiveSignals { en; }\n'
        '    }\n'
        '  }\n'
        '}\n',
        # The BSDL instruction block, in the spelling two real files use:
        # `BSDL_Entity` in one, `BSDLEntity` in the published grammar. Its
        # name is a plain identifier — `bsdl_instr_name : SCALAR_ID` — so
        # `b.INSTRUCTION.EXTEST` is not accepted, and neither is it ICL.
        'Module m { AccessLink a Of STD_1149_1 { BSDL_Entity b;\n'
        '  READINSTR { ScanPath { s1; } ActiveSignals { e; } }\n} }',
        'Module m { AccessLink a Of STD_1149_1_2001 { BSDLEntity b; '
        'I { ScanInterface { u; } ActiveSignals { x; } } } }',
    ]))
    # Absent on purpose: `AccessLink ...` at top level. It is a module_item
    # in the published grammar, not an iclSource item.

    # ---- parameters and attributes ----
    b.append(('parameter', [
        'Module m { Parameter p = 4; }',
        "Module m { Parameter p = 4'b0011; }",
        'Module m { Parameter p = $q; }',
        'Module m { LocalParameter p = 4; }',
        'Module m { Attribute a = 1; }',
        'Module m { Attribute a = "s"; }',
        'Module m { Attribute a; }',
        "Module m { Attribute a = 2'b01, 2'b10; }",
        'Module m { Attribute a = "s", "t"; }',
        'Module m { Parameter p = "s"; }',
    ]))
    # Absent on purpose: `Parameter p = 4;` at top level — parameter_def is a
    # module_item. Pinned in fail/standard/top_level_parameter.icl.

    # ---- toClockPort items ----
    # The three blocks that nothing else in this file reaches, from
    #
    #   toClockPort_item : attribute_def | toClockPort_source
    #                    | freqMultiplier_def | freqDivider_def
    #                    | differentialInvOf_def | period_def ;
    #   freqMultiplier_def    : 'FreqMultiplier' pos_int ';' ;
    #   freqDivider_def       : 'FreqDivider' pos_int ';' ;
    #   differentialInvOf_def : 'DifferentialInvOf' concat_clock_signal ';' ;
    #
    # They belong to ToClockPort alone — no other port block has a frequency
    # or a differential item — so they sat outside the 26-port sweep above,
    # which covers only what every port shares.
    # The 26-port sweep checked what every port shares — name, range, `;`,
    # `{}`. What it could not see is that the ports do *not* share a body.
    # There are three groups:
    #
    #   1. thirteen whose body is `attribute_def*` only, so in the published
    #      grammar every statement inside must begin with `Attribute`:
    #        ScanInPort ShiftEnPort CaptureEnPort UpdateEnPort SelectPort
    #        TMSPort ToIRSelectPort TCKPort ToTCKPort TRSTPort
    #        AddressPort WriteEnPort ReadEnPort
    #   2. ports with their own items, each `attribute_def | <something>`:
    #        scanOutPort  source, enable
    #        resetPort    polarity        toResetPort  source, polarity
    #        clockPort    diffPort        toClockPort  six items (below)
    #        toShiftEnPort / toCaptureEnPort / toUpdateEnPort / toSelectPort
    #        / toTmsPort / toTrstPort   source
    #   3. bodies that are `item+` rather than `item*`, so empty is illegal:
    #        toSelectPort toResetPort toClockPort toTrstPort
    #
    # Here anything may be an attribute name (§5.32), so group 1 accepting
    # `Source` is deliberate — and the corpus backs it: 292 statements inside
    # those thirteen port bodies, 289 of them do begin with `Attribute`. The
    # three that do not are ours, testing nested blocks.
    b.append(('portItems', [
        # Group 1: in the published grammar these admit Attribute only.
        'Module m { ScanInPort si { Attribute x = 1; } }',
        'Module m { TCKPort tck { Attribute x = 1; } }',
        'Module m { TRSTPort trst { Attribute x = 1; } }',
        'Module m { WriteEnPort we { Attribute x = 1; } }',
        'Module m { ReadEnPort re { Attribute x = 1; } }',
        'Module m { AddressPort ap { Attribute x = 1; } }',
        'Module m { ToIRSelectPort irs { Attribute x = 1; } }',
        # Group 2: their own items.
        'Module m { ScanOutPort so { Source a; } }',
        'Module m { ScanOutPort so { Enable a; } }',
        'Module m { ScanOutPort so { Source a; Enable b; } }',
        # resetPort_polarity : 'ActivePolarity' ('0' | '1') — a bare 0 or 1,
        # not a sized number.
        'Module m { ResetPort rp { ActivePolarity 0; } }',
        'Module m { ResetPort rp { ActivePolarity 1; } }',
        'Module m { ToResetPort trp { ActivePolarity 1; Source s; } }',
        # clockPort_item : attribute_def | clockPort_diffPort, and
        # clockPort_diffPort is 'DifferentialInvOf' concat_clock_signal —
        # the same shape as ToClockPort's, so it takes a list.
        'Module m { ClockPort cp { DifferentialInvOf c; } }',
        'Module m { ClockPort cp { DifferentialInvOf c, d; } }',
        'Module m { ToSelectPort tsp { Source s; } }',
        # The rule names say toTmsPort / toTrstPort, but the keywords are
        # 'ToTMSPort' and 'ToTRSTPort' — the acronym is upper case in the
        # spelling and lower case in the rule name. Writing the rule name
        # instead of the spelling is the easiest mistake to make here, and
        # the two files that failed for exactly that reason are why these
        # are spelled out.
        'Module m { ToTMSPort tms { Source s; } }',
        'Module m { ToTRSTPort trst { Source s; } }',
        'Module m { ToShiftEnPort se { Source s; } }',
        'Module m { ToCaptureEnPort ce { Source s; } }',
        'Module m { ToUpdateEnPort ue { Source s; } }',
        # Group 3: `item+`, so empty is illegal in the published grammar.
        # Empty bodies are covered by the classified rejection fixtures.
        'Module m { ToSelectPort tsp { Attribute flag; } }',
        'Module m { ToResetPort trp { Attribute flag; } }',
        'Module m { ToTRSTPort trst { Attribute flag; } }',
        # scanInterfacePort_def : 'Port' reg_port_signal_id ';' — no body
        # form, and the name may carry a range.
        'Module m { ScanInterface si { Port a; } }',
        'Module m { ScanInterface si { Port a[3:0]; } }',
        # logicSignal_def has only the braced form:
        #   'LogicSignal' logicSignal_name '{' logic_expr ';' '}'
        # so `LogicSignal ls;` is not legal, and is rejected.
        'Module m { LogicSignal ls { a & b; } }',
    ]))

    b.append(('toClockPortItems', [
        'Module m { ToClockPort c { Source clk; FreqMultiplier 4; } }',
        'Module m { ToClockPort c { Source clk; FreqDivider 2; } }',
        'Module m { ToClockPort c { DifferentialInvOf clk; } }',
        # concat_clock_signal, so the differential item takes a list
        'Module m { ToClockPort c { DifferentialInvOf clk, clkb; } }',
        'Module m { ToClockPort c { Period 10ns; } }',
        'Module m { ToClockPort c { Source clk; } }',
        'Module m { ToClockPort c { Attribute a = 1; } }',
        'Module m { ToClockPort c {\n'
        '  Source clk;\n'
        '  FreqMultiplier 2;\n'
        '  FreqDivider 4;\n'
        '  Attribute note = "scaled clock";\n'
        '}\n}',
        'Module m { ToClockPort c[1:0] { Period 10ns; } }',
        'Module m { ToClockPort c; }',
    ]))


    # ---- port group ----
    # PortGroup is a legacy LiU P1687 Listing8 form, not IEEE2014 or Tessent. The
    # spelling comes from a real file:
    #
    #   PortGroup tap_side {si, so, en}
    #
    # — a comma list with no semicolon. `PortGroup g { a; }` is not it.
    b.append(('portGroup', [
        'Module m { PortGroup g { a } }',
        'Module m { PortGroup g { a, b } }',
        'Module m { PortGroup g {\n  a, // the first\n  b\n} }',
        # A name here is a port name, and port_name : SCALAR_ID | vector_id,
        # so it may carry a range — the same port that
        # `ScanInterface { Port si[3:0]; }` names.
        'Module m { PortGroup g { si[3:0] } }',
        'Module m { PortGroup g { si[3:0], so, en[1] } }',
    ]))

    # ---- module-level statements from the standard ----
    # Keywords are not reserved, so a declared name may be one. Every
    # declaration position takes decl_name : IDENT | any_keyword now; the
    # one that does not is an Instance name, where `Instance Of Of M;` would
    # be genuinely ambiguous (see pass/ours/keyword_as_declared_name.icl).
    b.append(('keywordName', [
        'Module Source { }',
        'Module m { ScanInPort Source; }',
        'Module m { ScanInPort Port; }',
        'Module m { ScanOutPort Source { Source si; } }',
        'Module m { DataInPort Enum[3:0]; }',
        'Module m { ScanRegister Source { ScanInSource si; } }',
        'Module m { Enum Source { a = 1; } }',
        'Module m { Alias Port = si; }',
        'Module m { Parameter Source = 1; }',
        'Module m { Attribute Source = 1; }',
        'Module m { Attribute Port; }',
        "Module m { ScanMux Port SelectedBy s { 1'b0 : a; } }",
        'Module m { LogicSignal Port { a & b; } }',
        'Module m { OneHotScanGroup Port { Port si; } }',
        'Module m { PortGroup Port { si } }',
        'Module m { Instance i Of T { AllowBroadcastOnScanInterface Port; } }',
    ]))

    b.append(('moduleStatements', [
        'Module m { Instance i Of T { AllowBroadcastOnScanInterface si; } }',


        # concat_hier_data_signal : '~'? hier_data_signal (',' '~'?
        # hier_data_signal)* — the list takes `~` on any element.
        'Module m { Alias a = ~b; }',
        'Module m { Alias a = b, ~c; }',
        # parameter_value : concat_string | concat_number
        'Module m { Parameter p = "x", "y"; }',
        'Module m { LocalParameter lp = "x"; }',
        # attribute_value likewise, and the value is optional.
        'Module m { Attribute a = "x", "y"; }',
        "Module m { Attribute a = 2'b01, 2'b10; }",
    ]))

    # ---- recursion and nesting ----
    # The recursive rules, checked at depth: nesting costs heap, not C stack.
    # ---- the signal family ----
    # Nine signal rules, and they are not the same rule. Five carry `~`:
    #
    #   data_signal scan_signal clock_signal reset_signal unknown_signal
    #       : '~'? signal ;
    #
    # Six do not — they are a bare `signal`, so `~` is not legal on them:
    #
    #   tck_signal tms_signal trst_signal
    #   shiftEn_signal captureEn_signal updateEn_signal
    #       : signal ;
    #
    # So `ToTMSPort { Source ~s; }` is not in the published grammar. It is
    # accepted here, which is the deliberate-extra-shape rule again; the point
    # of casing it is that the two halves must not be "harmonised" later
    # without noticing that the grammar really does treat them differently.
    #
    # A second asymmetry, this one inside the published grammar itself:
    # some sources are `concat_*` (a comma list) and some are a single
    # signal, with no apparent reason:
    #
    #   scanRegister_scanInSource  : 'ScanInSource' scan_signal ';'      single
    #   scanRegister_captureSource : 'CaptureSource' (concat_data_signal
    #                                | enum_symbol) ';'                 list
    #   toShiftEnPort_source       : 'Source' concat_shiftEn_signal ';'  list
    #   toCaptureEnPort_source     : 'Source' captureEn_signal ';'       single
    #   toUpdateEnPort_source      : 'Source' updateEn_signal ';'        single
    #
    # Both forms are accepted here, and both are cased.
    b.append(('signalFamily', [
        # The five that take `~`.
        'Module m { DataOutPort d { Source ~a; } }',
        'Module m { ScanOutPort so { Source ~a; } }',
        'Module m { ToClockPort c { Source ~a; } }',
        'Module m { ToResetPort r { Source ~a; } }',
        'Module m { Instance i Of M { InputPort p = ~a; } }',
        # The six that do not.
        'Module m { ToTMSPort t { Source ~a; } }',
        'Module m { ToTRSTPort t { Source ~a; } }',
        'Module m { ToShiftEnPort s { Source ~a; } }',
        'Module m { ToCaptureEnPort c { Source ~a; } }',
        'Module m { ToUpdateEnPort u { Source ~a; } }',
        # Single versus list, both ways.
        'Module m { ScanRegister sr { ScanInSource a; } }',
        'Module m { ScanRegister sr { ScanInSource ~a; } }',
        'Module m { ScanRegister sr { CaptureSource a, b; } }',
        'Module m { ToShiftEnPort s { Source a, b; } }',
        'Module m { ToCaptureEnPort c { Source a, b; } }',
        'Module m { ToUpdateEnPort u { Source a, b; } }',
        # hier_data_signal is `(instance_name '.')* reg_port_signal_id` —
        # zero or more, so no dot at all is fine. hier_port is `+`, so it
        # needs at least one; both work because a bare name is a
        # reg_port_signal_id anyway.
        'Module m { Alias a = b; }',
        'Module m { Alias a = i1.i2.b; }',
        'Module m { ScanRegister sr { ScanInSource a; } }',
        'Module m { ScanRegister sr { ScanInSource i1.a; } }',
    ]))

    # ---- the name families ----
    # Which names may carry a subscript is not uniform, and the rules differ
    # by one character:
    #
    #   port_name     : SCALAR_ID | vector_id ;   <- may
    #   register_name : SCALAR_ID | vector_id ;   <- may
    #   reg_port_signal_id : SCALAR_ID | vector_id ;  <- may
    #   instance_name : SCALAR_ID ;               <- may NOT
    #   module_name   : SCALAR_ID ;               <- may NOT
    #
    # So `Instance i[3:0] Of M;` is not legal, even though
    # `ScanRegister sr[3:0]` sitting two lines below it is. The rejection is
    # cased in tests/cases/fail/ours/ as well as here, because widening
    # instance_name to match its neighbours would cost nothing at the grammar
    # level and would be a silent change to what the tool accepts.
    #
    # `hier_port : (instance_name '.')+ port_name` needs at least one dot,
    # while `hier_data_signal : (instance_name '.')*` needs none — which is
    # why `Alias a = b;` is legal with no hierarchy at all.
    b.append(('names', [
        'Module m { Instance i Of M; }',
        'Module m { ScanInPort si[3:0]; }',
        'Module m { ScanRegister sr[3:0] { ScanInSource si; } }',
        'Module m { ScanRegister sr[$W] { ScanInSource si; } }',
        "Module m { ScanRegister sr { ResetValue $W'd5; } }",
        # alias_name and the four shapes of its right-hand side
        'Module m { Alias a = b; }',
        'Module m { Alias a[3:0] = b[3:0]; }',
        'Module m { Alias a = b, c; }',
        'Module m { Alias a = i.b; }',
        'Module m { Alias a = i1.i2.i3.b; }',
        'Module m { Alias a = i.b[3:0]; }',
        # parameter_override : parameter_def — an Instance body may redefine
        # a parameter of the module it instantiates.
        'Module m { Instance i Of M { Parameter W = 8; } }',
        'Module m { Instance i Of M { Parameter W = "s"; } }',
        'Module m { Instance i Of M { InputPort a = b; } }',
        'Module m { OneHotDataGroup g { Instance i Of M { AddressValue 5; } } }',
        'Module m { Instance i Of M { Attribute a = 1; } }',
        'Module m { Instance i Of M { AllowBroadcastOnScanInterface s; } }',
        'Module m { Instance i Of M { } }',
        # defaultLoad_def : 'DefaultLoadValue' concat_number ';' — a list.
        'Module m { ScanRegister sr { DefaultLoadValue 0; } }',
        'Module m { ScanRegister sr { DefaultLoadValue 1, 0; } }',
        "Module m { ScanRegister sr { DefaultLoadValue 4'b1010; } }",
        # nameSpace_def / useNameSpace_def — the name is optional on both,
        # and UseNameSpace is a module item as well as a top-level one.
        'NameSpace ns;' + TOP + 'Module m { ScanInPort si; }',
        'NameSpace;' + TOP + 'Module m { ScanInPort si; }',
        'UseNameSpace ns;' + TOP + 'Module m { ScanInPort si; }',
        'UseNameSpace;' + TOP + 'Module m { ScanInPort si; }',
        'Module m { UseNameSpace ns; }',
        'Module m { UseNameSpace; }',
        # A vector or an index as a signal reference.
        'Module m { ScanOutPort so { Source sr[3:0]; } }',
        'Module m { ScanOutPort so { Source sr[0]; } }',
    ]))

    # ---- where a plain integer is required ----
    # Three items take `pos_int` rather than `number`, so a parameter
    # reference is not legal in them even though it is legal one line away
    # as a size:
    #
    #   period_def         : 'Period' pos_int (unit)? ';' ;
    #   freqMultiplier_def : 'FreqMultiplier' pos_int ';' ;
    #   freqDivider_def    : 'FreqDivider' pos_int ';' ;
    #   size               : pos_int | '$' SCALAR_ID ;
    #
    # Period requires a decimal literal. Tessent documents parameterized
    # frequency factors; both factors require Source (IEEE §6.4.6.20).
    b.append(('posIntOnly', [
        'Module m { ToClockPort cp { Period 10ns; } }',
        'Module m { ToClockPort cp { Period 10; } }',
        'Module m { ToClockPort cp { Period 20; } }',
        'Module m { ToClockPort cp { Source clk; FreqMultiplier 2; } }',
        'Module m { ToClockPort cp { Source clk; FreqMultiplier $W; } }',
        'Module m { ToClockPort cp { Source clk; FreqDivider $W; } }',
        # 'Enable' takes data_signal, not concat_data_signal — a single
        # signal, where its neighbours take lists (§5.62).
        'Module m { ScanOutPort so { Enable e; } }',
        'Module m { ScanOutPort so { Enable ~a; } }',
        'Module m { DataOutPort do { Enable ~a; } }',
        # DefaultLoadValue / ResetValue take (concat_number | enum_symbol),
        # and enum_symbol is one SCALAR_ID — so a *list* of enum symbols is
        # not in the grammar. Accepted here, and cased for the same reason.
        'Module m { ScanRegister sr { DefaultLoadValue A; } }',
        'Module m { ScanRegister sr { DefaultLoadValue A; } }',
        'Module m { ScanRegister sr { ResetValue B; } }',
        'Module m { ScanRegister sr { CaptureSource A; } }',
        'Module m { DataInPort di { DefaultLoadValue A; } }',
        # writeEnSource is '~'? data_signal — single, with an optional not.
        'Module m { DataRegister dr { WriteEnSource ~w; } }',
        'Module m { OneHotDataGroup g { DataRegister dr { AddressValue $A; } } }',
    ]))

    # ---- ScanInterface internals ----
    # scanInterface_item : attribute_def | scanInterfacePort_def
    #                    | defaultLoad_def | scanInterfaceChain_def ;
    # scanInterfaceChain_item : attribute_def | scanInterfacePort_def
    #                         | defaultLoad_def ;
    #
    # The chain item list has no `scanInterfaceChain_def`, so a Chain inside
    # a Chain is not in the grammar. It parses here anyway — see §5.44,
    # where the same single rule lets any keyword open a block anywhere.
    #
    # scanInterface_name is SCALAR_ID, so `ScanInterface si[3:0]` is
    # rejected; that rejection is pinned in
    # fail/ours/scaninterface_name_with_vector.icl.
    b.append(('scanInterfaceItems', [
        'Module m { ScanInterface si { Port a; Port b; } }',
        'Module m { ScanInterface si { Port a; DefaultLoadValue 0; '
        'Attribute x = 1; } }',
        'Module m { ScanInterface si { Chain c1 { Port a; } '
        'Chain c2 { Port b; } } }',
        'Module m { AccessLink al Of STD_1149_1_2013 '
        '{ BSDLEntity e; r { ScanInterface { si; } } } }',
        'Module m { AccessLink al Of STD_1149_1_2001 { BSDLEntity e; '
        'r1 { ScanInterface { si1; } } r2 { ScanInterface { si2; } } } }',
        'Module m { ScanInterface si { Port p; } }',
        'Module m { ScanInterface si { DefaultLoadValue 0; } }',
        'Module m { ScanInterface si { Attribute a = 1; } }',
        'Module m { ScanInterface si { Chain c { Port p; } } }',
        'Module m { ScanInterface si { Chain c { DefaultLoadValue 0; } } }',
        'Module m { ScanInterface si { Chain c { Attribute a = 1; } } }',
        'Module m { ScanInterface si { Chain c { Port p; Port q; } Chain d { Port r; Port s; } } }',
        # AccessLink internals: accessLink1149_ScanInterface_name is
        # instance_name ('.' scanInterface_name)? — the hierarchy optional.
        'Module m { AccessLink al Of STD_1149_1_2001 '
        '{ BSDLEntity e; r { ScanInterface { si; } } } }',
        'Module m { AccessLink al Of STD_1149_1_2001 '
        '{ BSDLEntity e; r { ScanInterface { i.si; } } } }',
        'Module m { AccessLink al Of STD_1149_1_2001 '
        '{ BSDLEntity e; r { ActiveSignals { s; } } } }',
        # alias_item : attribute_def | 'AccessTogether' ';'
        #            | alias_iApplyEndState | alias_refEnum
        # — AccessTogether is a bare keyword statement, not a block.
        'Module m { Alias a = b { AccessTogether; } }',
        'Module m { Alias a = b { iApplyEndState 1; } }',
        'Module m { Alias a = b { RefEnum E; } }',
    ]))

    # ---- the three thin categories, filled out ----
    # connection and portGroup are Tessent's, with no rule in the published
    # grammar, so these cases come from real Tessent output and from the
    # published tutorial: only the unnamed braced form of Connection is
    # attested, and a PortGroup list holds plain port names.

    # portGroup — names only, but those may carry a range (§5.76). A
    # hierarchical name and a `~` are both rejected; see REQUIREMENT §5.81.
    b.append(('portGroup', [
        'Module m { PortGroup pg { si, so, en } }',
        'Module m { PortGroup pg { si[3:0] } }',
        'Module m { PortGroup pg { en[1] } }',
        'Module m { PortGroup pg { a, /* c */ b } }',
        'Module m { PortGroup pg { a } PortGroup pg2 { b, c } }',
    ]))

    # namespace — the name is optional on both statements, which is easy to
    # miss because every other named construct in ICL requires its name.
    b.append(('namespace', [
        'NameSpace;',
        'NameSpace ns;',
        'UseNameSpace;',
        'UseNameSpace ns;',
        'NameSpace a;\nNameSpace b;',
        'NameSpace ns; Module m { UseNameSpace ns; }',
        'Module m { UseNameSpace; }',
    ]))

    # ---- the attribute machinery ----
    # `Attribute` is the most-used block and the least rule-bound one:
    #
    #   attribute_def   : 'Attribute' attribute_name ('=' attribute_value)? ';' ;
    #   attribute_name  : SCALAR_ID ;
    #   attribute_value : (concat_string | concat_number) ;
    #   parameter_value : concat_string | concat_number ;
    #
    # **An attribute value is a string list or a number list — nothing else.**
    # `Attribute foo = abc;` is not in the grammar, because `abc` is neither.
    # It is accepted here. That is not a rounding error: this is the single
    # most consequential piece of permissiveness in the tool, and it is what
    # lets any keyword appear as an attribute name (§5.32) and lets Tessent
    # spellings survive. Casing `Attribute foo = abc;` does not celebrate it —
    # it pins it, so that a later attempt to "tighten" attribute_value has to
    # confront the 292 real statements counted in §5.59.
    #
    # `concat_number` is `'~'? number (',' '~'? number)*` and `number`
    # includes integer_expr, which is why `= 1 + 2` and `= $P` are legal.
    # `concat_string` is `(STRING | parameter_ref)` comma-separated, which is
    # why `= "s", $P` mixes.
    b.append(('attribute', [
        'Module m { Attribute foo; }',
        'Module m { Attribute foo = 1; }',
        "Module m { Attribute foo = 2'b01, 2'b10; }",
        "Module m { Attribute foo = 3'b010; }",
        'Module m { Attribute foo = "s"; }',
        'Module m { Attribute foo = "s", "t"; }',
        'Module m { Attribute foo = $P; }',
        'Module m { Attribute foo = 1 + 2; }',
        'Module m { Attribute foo = ~1; }',
        'Module m { Attribute foo = "s", $P; }',
        # Not in the grammar: a bare name as a value. Accepted, on purpose.
        'Module m { Attribute foo = $abc; }',
        # parameter_value is the same union, on both spellings of parameter.
        'Module m { Parameter p = "s"; }',
        "Module m { Parameter p = 2'b01, 2'b10; }",
        'Module m { LocalParameter p = 1; }',
        'Module m { LocalParameter p = "s"; }',
        # enum_value : concat_number — so an enum symbol takes a list.
        'Module m { Enum E { a = 1, 2; } }',
        # Alias's own items, including the bare-flag shape.
        'Module m { Alias a = b { AccessTogether; } }',
        'Module m { Alias a = b { iApplyEndState 1, 0; } }',
        'Module m { Alias a = b { RefEnum E; } }',
        # period_def : 'Period' pos_int (unit)? ';' — the unit is optional,
        # and there are seven of them, `as` included.
        'Module m { ToClockPort c { Period 10; } }',
        'Module m { ToClockPort c { Period 10s; } }',
        'Module m { ToClockPort c { Period 10ms; } }',
        'Module m { ToClockPort c { Period 10us; } }',
        'Module m { ToClockPort c { Period 10ns; } }',
        'Module m { ToClockPort c { Period 10ps; } }',
        'Module m { ToClockPort c { Period 10fs; } }',
        'Module m { ToClockPort c { Period 10as; } }',
    ]))

    # ---- iProc callbacks ----
    #
    #   dataRegister_writeCallBack : 'WriteCallBack' iProc_namespace
    #                                iProc_name iProc_args* ';' ;
    #   iProc_namespace : (namespace_name? '::')? ref_module_name
    #                     ('::' sub_namespace)? ;
    #   ref_module_name : SCALAR_ID | parameter_ref ;
    #   sub_namespace   : SCALAR_ID | parameter_ref ;
    #   iProc_name      : SCALAR_ID | parameter_ref ;
    #   iProc_args      : '<D>' | '<R>' | number | STRING | parameter_ref ;
    #
    # Two asymmetries worth casing explicitly.
    #
    # **A module reference here may be `$param`; in `Instance ... Of` it may
    # not.** `instance_def` uses `module_name : SCALAR_ID`, so
    # `Instance i Of $MOD;` is rejected — and that rejection is correct, not a
    # gap. Both directions are cased so neither gets "fixed" by accident.
    #
    # **`iProc_args` is `number`**, and number includes integer_expr, so an
    # argument can be arithmetic.
    b.append(('iProc', [
        'Module m { DataRegister d { WriteCallBack m p <D>; } }',
        'Module m { DataRegister d { WriteCallBack $M $P <D>; } }',
        'Module m { DataRegister d { WriteCallBack ::m p <R>; } }',
        'Module m { DataRegister d { WriteCallBack m::sub p <D>; } }',
        'Module m { DataRegister d { WriteCallBack m p <D>; } }',
        'Module m { DataRegister d { WriteCallBack m p <D> <R> 5 "s" $X; } }',
        'Module m { DataRegister d { WriteCallBack m p 1 + 2; } }',
        'Module m { DataRegister d { ReadCallBack m p <R>; } }',
        # The four DataRegister types, each through its own item.
        'Module m { DataRegister d { WriteEnSource ~a; } }',
        'Module m { DataRegister d { WriteDataSource a, b; } }',
        'Module m { OneHotDataGroup g { DataRegister d { AddressValue 5; } } }',
        'Module m { DataRegister d { ReadDataSource a; } }',
        # Enable is a single data_signal, not a list, on both ports that
        # have it.
        'Module m { ScanOutPort so { Enable en; } }',
        'Module m { DataOutPort d { Enable en; } }',
    ]))

    # ---- syntactically legal, semantically invalid ----
    # The reference repo has 13 modules it labels `not_ok_*`, spread over two
    # files: seven in `test_icl_instance.icl` and six in
    # `test_icl_scan_register.icl`. Every one of them is **valid ICL**. They
    # fail upstream for semantic reasons only — a width that does not match,
    # an index out of range, an input assigned twice, an input never
    # assigned.
    #
    # That makes them the most useful cases in the whole repository for a
    # formatter, and the easiest to get wrong in the dangerous direction: a
    # formatter that tried to check semantics would reject them. The contract
    # here is the opposite — accept anything the grammar allows and reproduce
    # it exactly. So the shapes those modules use are lifted out and cased
    # individually, each with a note on why it is not this tool's business.
    b.append(('syntaxNotSemantics', [
        # `ScanRegister ScanReg0[0:7]` — rangex is index ':' index with two
        # independent expressions, so a descending range and an ascending one
        # are both just text. Upstream calls [0:7] an error in one module and
        # accepts it in another (ok_4, ok_5); the difference is semantic.
        'Module m { ScanRegister sr[0:7] { ScanInSource si; } }',
        'Module m { ScanRegister sr[1:0] { ScanInSource si; } }',
        'Module m { ScanRegister sr[0:$MSB] { ScanInSource si; } }',
        'Module m { ScanRegister sr[$MSB:0] { ScanInSource si; } }',
        'Module m { ScanRegister sr[$A:$B] { ScanInSource si; } }',
        # `InputPort SI[1] = ...` — inputPort_name : port_name, and port_name
        # includes vector_id, so a subscripted name is legal. Upstream
        # rejects `SI[2]` on a 2-bit port and `SI[0]` alone (leaving bit 1
        # unassigned); both are width arithmetic, not syntax.
        "Module m { Instance i Of M { InputPort SI[1] = 1'b0; } }",
        "Module m { Instance i Of M { InputPort SI[1:0] = 1'b0, tdi; } }",
        "Module m { Instance i Of M { InputPort SI = 1'b0, tdi; } }",
        # An unsized number as an InputPort source, and a bare 0.
        "Module m { Instance i Of M { InputPort SI = 'b0, tdi; } }",
        'Module m { Instance i Of M { InputPort DI = 0; } }',
        # `Parameter MSB = $Size-1;` — no spaces around the operator. The
        # scanner splits `$Size` from `-1` on the operator, so both spellings
        # work; upstream's own file uses the tight one.
        'Module m { Parameter MSB = $Size-1; }',
        'Module m { Parameter MSB = $Size - 1; }',
        'Module m { Parameter MSB = $Size+1; }',
        # Unknown bits in a reset/load value, from test_icl_syntax_6.icl.
        # The "bad" module there has 'b10 and 'b11 on an 8-bit register —
        # too narrow, which upstream rejects — while the good one has
        # 'b1x00x01x. Both are just numbers here.
        "Module m { ScanRegister sr { ResetValue 'b1x00x01x; } }",
        "Module m { ScanRegister sr { DefaultLoadValue 'bxx0x1010; } }",
        "Module m { ScanRegister sr { ResetValue 'b10; } }",
        # A register capturing itself — legal text, and upstream's ok_* do it.
        'Module m { ScanRegister sr { CaptureSource sr; } }',
        # Ranged and indexed signals as sources.
        'Module m { ScanRegister sr[1:0] { ScanInSource si[1:0]; } }',
        'Module m { ScanOutPort so[1:0] { Source sr[1:0]; } }',
        'Module m { ScanOutPort so[1:0] { Source sr1[0], sr0[0]; } }',
        # Two Chains in one ScanInterface, each with its own pair of Ports —
        # the shape behind "multiple SI -> SO ports" in the README.
        'Module m { ScanInterface s { Port a; '
        'Chain c0 { Port si[0]; Port so[0]; } '
        'Chain c1 { Port si[1]; Port so[1]; } } }',
    ]))

    # ---- LogicSignal expressions ----
    # The other recursive block, and the one with the most surprises. Four
    # levels, and each one has a different shape:
    #
    #   logic_expr_lvl1 : logic_expr_lvl2 (('&&'|'||') logic_expr_lvl1)? ;
    #   logic_expr_lvl2 : logic_expr_lvl3 (('&'|'|'|'^') logic_expr_lvl2)?
    #                   | (('&'|'|'|'^') logic_expr_lvl2) ;   <- leading op!
    #   logic_expr_lvl3 : logic_expr_lvl4 (('=='|'!=') logic_expr_num_arg)? ;
    #   logic_expr_lvl4 : logic_expr_arg (',' logic_expr_lvl4)? ;
    #   logic_expr_arg  : logic_expr_paren | logic_unary_expr
    #                   | concat_data_signal ;
    #   logic_unary_expr: ('~'|'!') logic_expr_arg ;
    #   logic_expr_num_arg : concat_number | enum_name | '(' logic_expr_num_arg ')' ;
    #
    # Four things worth recording.
    #
    # 1. **lvl2 has a second alternative with a leading operator**, so
    #    `LogicSignal ls { & a; }` is legal — an expression that begins with
    #    its own operator. No sample file does this; it is here because the
    #    rule allows it.
    #
    # 2. **`==` and `!=` compare against a number, not a signal.**
    #    `logic_expr_num_arg` is `concat_number | enum_name | (...)` — an
    #    enum_name being a bare SCALAR_ID, so `a == b` parses with `b` as an
    #    enum name rather than a signal. The two readings render identically,
    #    so this matters only as a note on what the grammar says.
    #
    # 3. **`,` is an operator at lvl4**, so `LogicSignal ls { a, b; }` is
    #    legal — and `concat_data_signal` also takes commas, so `a, b` has two
    #    parses. Again, identical output.
    #
    # 4. **Numbers reach an expression through `signal`**, not directly:
    #    `signal : number | reg_port_signal_id | hier_port`. So `1 & a` and
    #    `1'b0 & a` are both expressions whose left operand is a literal.
    b.append(('logicExpr', [
        'Module m { LogicSignal ls { a && b; } }',
        'Module m { LogicSignal ls { a || b; } }',
        'Module m { LogicSignal ls { a && b || c; } }',
        'Module m { LogicSignal ls { a & b; } }',
        'Module m { LogicSignal ls { a | b; } }',
        'Module m { LogicSignal ls { a ^ b; } }',
        'Module m { LogicSignal ls { a & b & c; } }',
        # lvl2's second alternative: the expression opens with an operator.
        'Module m { LogicSignal ls { & a; } }',
        'Module m { LogicSignal ls { | a; } }',
        'Module m { LogicSignal ls { ^ a; } }',
        # `==` / `!=` against logic_expr_num_arg.
        'Module m { LogicSignal ls { a == 1; } }',
        "Module m { LogicSignal ls { a == 3'b010; } }",
        'Module m { LogicSignal ls { a == ENUM; } }',
        'Module m { LogicSignal ls { a != 1; } }',
        'Module m { LogicSignal ls { (a == 1); } }',
        'Module m { LogicSignal ls { a == (1); } }',
        'Module m { LogicSignal ls { a == b; } }',
        # `,` as an operator at lvl4.
        'Module m { LogicSignal ls { a, b; } }',
        'Module m { LogicSignal ls { a & b, c; } }',
        # Unary, and unary applied repeatedly.
        'Module m { LogicSignal ls { ~a; } }',
        'Module m { LogicSignal ls { !a; } }',
        'Module m { LogicSignal ls { ~a & b; } }',
        'Module m { LogicSignal ls { !(a & b); } }',
        'Module m { LogicSignal ls { ~~a; } }',
        'Module m { LogicSignal ls { !!a; } }',
        # Parentheses, and precedence made explicit.
        'Module m { LogicSignal ls { (a & b); } }',
        'Module m { LogicSignal ls { ((a)); } }',
        'Module m { LogicSignal ls { (a & b) | c; } }',
        'Module m { LogicSignal ls { a & b == 1; } }',
        'Module m { LogicSignal ls { a == 1 && b; } }',
        # A literal as an operand: signal : number | ...
        "Module m { LogicSignal ls { 1'b0 & a; } }",
        'Module m { LogicSignal ls { 1 & a; } }',
        # Hierarchical and indexed operands.
        'Module m { LogicSignal ls { i1.a & b; } }',
        'Module m { LogicSignal ls { a[0] & b; } }',
        'Module m { LogicSignal ls { a[3:0] & b; } }',
        # Depth: 100 nested parens, 50 unary operators, 200 terms.
        'Module m { LogicSignal ls { ' + '(' * 100 + 'a' + ')' * 100 + '; } }',
        'Module m { LogicSignal ls { ' + '!' * 50 + 'a; } }',
        'Module m { LogicSignal ls { ' + ' & '.join(['a'] * 200) + '; } }',
    ]))

    # ---- integer expressions ----
    # The expression grammar in the published grammar is two levels, both
    # right-recursive, and the operand set is deliberately narrow:
    #
    #   integer_expr_lvl1 : integer_expr_lvl2 (('+'|'-') integer_expr_lvl1)? ;
    #   integer_expr_lvl2 : integer_expr_arg (('*'|'/'|'%') integer_expr_lvl2)? ;
    #   integer_expr_arg  : integer_expr_paren | pos_int | parameter_ref ;
    #   integer_expr_paren: '(' integer_expr ')' ;
    #   pos_int           : '0' | '1' | POS_INT ;
    #   parameter_ref     : '$' SCALAR_ID ;
    #
    # Two consequences that are easy to miss:
    #
    # - `integer_expr_arg` does **not** include unsized_number or
    #   sized_number, so `1'b0 + 1` is not legal, and there is no unary minus,
    #   so `-1` is not legal either. Negative counterparts are in the classified parameter corpus.
    # - Both levels recurse on themselves, not on each other, so
    #   `1 - 2 - 3` groups as `1 - (2 - 3)`. A formatter reproduces the text
    #   either way, so this matters only as a record of what the grammar does.
    #
    # `index : integer_expr` and `rangex : index ':' index`, so both ends of
    # a range are full expressions — which is what makes `sr[$W - 1:0]` legal.
    b.append(('integerExpr', [
        'Module m { Parameter p = 1 + 2; }',
        'Module m { Parameter p = 5 - 3; }',
        'Module m { Parameter p = 1 + 2 + 3; }',
        'Module m { Parameter p = 9 - 3 - 1; }',
        'Module m { Parameter p = 1 + 2 - 3; }',
        'Module m { Parameter p = 2 * 3; }',
        'Module m { Parameter p = 6 / 2; }',
        'Module m { Parameter p = 7 % 2; }',
        'Module m { Parameter p = 2 * 3 * 4; }',
        # lvl2 binds tighter than lvl1, in both orders.
        'Module m { Parameter p = 1 + 2 * 3; }',
        'Module m { Parameter p = 2 * 3 + 1; }',
        'Module m { Parameter p = (1 + 2) * 3; }',
        'Module m { Parameter p = ((1 + 2)); }',
        'Module m { Parameter p = ((((1)))); }',
        # parameter_ref as an operand, which is the point of the whole rule.
        'Module m { Parameter p = $W + 1; }',
        'Module m { Parameter p = $W * $H; }',
        'Module m { Parameter p = ($W - 1); }',
        'Module m { Parameter p = 0 + 1; }',
        'Module m { Parameter p = 1_000 + 2; }',
        # index and rangex are full expressions.
        'Module m { ScanRegister sr[$W - 1:0] { ScanInSource si; } }',
        'Module m { ScanRegister sr[(1 + 2) * 2:0] { ScanInSource si; } }',
        'Module m { ScanRegister sr[(3)] { ScanInSource si; } }',
        # DefaultLoadValue and ResetValue take concat_number, hence an
        # expression; inside ScanRegister they also take a bare enum symbol,
        # which the separate defaultLoad_def (used by ScanInterface) does not:
        #   scanRegister_defaultLoadValue : 'DefaultLoadValue'
        #                                   (concat_number | enum_symbol) ';' ;
        #   defaultLoad_def               : 'DefaultLoadValue' concat_number ';' ;
        'Module m { ScanRegister sr { DefaultLoadValue 1 + 2; } }',
        'Module m { ScanRegister sr { DefaultLoadValue SYM; } }',
        'Module m { ScanRegister sr { ResetValue 1 + 2; } }',
        'Module m { ScanRegister sr { ResetValue SYM; } }',
        'Module m { ScanInterface si { Port a; DefaultLoadValue 1 + 2; } }',
        'Module m { ScanRegister sr { ResetValue 4 + 1; } }',
        # Recursion, at depth: 300 terms and 200 nested parens.
        'Module m { Parameter p = ' + ' + '.join(['1'] * 300) + '; }',
        'Module m { Parameter p = ' + '(' * 200 + '1' + ')' * 200 + '; }',
        # Numeric expressions use decimal operands; invalid original forms
        # are preserved in the classified parameter rejection corpus.
        'Module m { Parameter p = 0 - 1; }',
        'Module m { Parameter p = 0 + 1; }',
    ]))

    # A vector name has exactly one subscript in the published grammar —
    # `vector_id : SCALAR_ID '[' (index | rangex) ']'` — so `sr[0][1]` is not
    # legal, and is rejected here.
    #
    # What *is* accepted is a subscript *inside* an index, `sr[a[0][1][2]]`,
    # because `index : integer_expr` and this implementation's integer_expr
    # takes a name with postfixes. That asymmetry is worth stating plainly:
    # an earlier round reported "index chains to 50 levels" as passing, which
    # was true only of the inner form and gave the wrong impression about
    # `sr[0][1]`. Both are now cased.
    b.append(('recursion', [
        'Module m { LogicSignal ls { ' + '(' * 50 + 'a' + ')' * 50 + '; } }',
        'Module m { LogicSignal ls { ' + '(' * 100 + 'a' + ')' * 100 + '; } }',
        'Module m { LogicSignal ls { ' + '!' * 30 + 'a; } }',
        'Module ' + ' '.join(['m {'] * 1) + ' }\n',

        'Module m { ScanRegister sr[($A + 1)]; }',
        # One subscript on a name; two is not legal (vector_id has one).
        'Module m { ScanRegister sr[0]; }',
        # But a subscript inside an index is accepted — see integerExpr above.
        'Module m { ScanRegister sr[(($A + 1) * (2 + 3))]; }',
        'Module m { ScanRegister sr[$W - 1:0]; }',
        # `::` chains: accepted where a signal or a module reference is
        # expected. Not in an attribute value — see below.
        'Module m { Alias x = a.b.c.d.e; }',
        'Module m { Instance i Of ::e; }',
        'Module m { ScanInterface si { Chain c1 { Port p; } Chain c2 { Port q; } } }',
        'Module m { Alias a = ~b, ~c, ~d; }',
    ]))

    # ---- scanner-level: literals and the pieces of text ----
    b.append(('literals', [
        "Module m { Parameter p = 1'b0; }",
        "Module m { Parameter p = 1'B0; }",
        "Module m { Parameter p = 'b1010; }",
        "Module m { Parameter p = 'h ef931832; }",   # 1687 Appendix E
        "Module m { Parameter p = 'd 134; }",
        "Module m { Parameter p = 'b11_00; }",
        "Module m { Parameter p = 7'bxx01_x11; }",
        # IEEE §6.3.8 has no Verilog signed-base marker; negative in enum corpus.
        "Module m { Parameter p = 16'hFF; }",
        "Module m { Parameter p = $W'd5; }",
        "Module m { Parameter p = 0235; }",
        "Module m { Parameter p = 1_2_3; }",
        'Module m { ToClockPort c { Period 10ns; } }',
        'Module m { ToClockPort c { Period 10s; } }',
        'Module m { ToClockPort c { Period 10; } }',
        'Module m { Attribute a = "x\\"y"; }',
        'Module m { Attribute a = "x\\\\y"; }',
        'Module m { Attribute a = ""; }',
        'Module m { /* c */ Attribute a = 1; }',
        'Module m { Attribute a = 1; // c\n}',
        'Module m {\n  /* a\n     b */\n  Attribute a = 1;\n}\n',
    ]))

    return b


# Every block keyword in the reference grammar — Honza255/icl_parser,
# commit 333f8bc, `src/icl_parser/icl.g4` — taken mechanically, not typed:
# every rule named `*_def`, its first quoted literal. 53 keywords from 54
# rules (AccessLink appears in two).
#
# PortGroup is retained only as a documented LiU P1687 legacy form.
# Reg, Field and Connection have no supported declaration grammar; their
# original positive cases now belong to the classified rejection corpus.
EXPECTED_BLOCK_KEYWORDS = [
    'AccessLink', 'AddressPort', 'Alias', 'AllowBroadcastOnScanInterface',
    'Attribute', 'CaptureEnPort', 'Chain', 'ClockMux', 'ClockPort',
    'DataInPort', 'DataMux', 'DataOutPort', 'DataRegister',
    'DefaultLoadValue', 'DifferentialInvOf', 'Enum', 'FreqDivider',
    'FreqMultiplier', 'Instance', 'LocalParameter', 'LogicSignal', 'Module',
    'NameSpace', 'OneHotDataGroup', 'OneHotScanGroup', 'Parameter', 'Period',
    'Port', 'ReadEnPort', 'ResetPort', 'ScanInPort', 'ScanInterface',
    'ScanMux', 'ScanOutPort', 'ScanRegister', 'SelectPort', 'ShiftEnPort',
    'TCKPort', 'TMSPort', 'TRSTPort', 'ToCaptureEnPort', 'ToClockPort',
    'ToIRSelectPort', 'ToResetPort', 'ToSelectPort', 'ToShiftEnPort',
    'ToTCKPort', 'ToTMSPort', 'ToTRSTPort', 'ToUpdateEnPort',
    'UpdateEnPort', 'UseNameSpace', 'WriteEnPort',
]


def check_block_coverage():
    """Fails when a grammar block has no cases in `blocks()`."""
    covered = set()
    for _, cases in blocks():
        for source in cases:
            covered.update(re.findall(r'[A-Za-z][A-Za-z0-9_]*', source))
    return [k for k in EXPECTED_BLOCK_KEYWORDS if k not in covered]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--binary', required=True)
    args = parser.parse_args()
    if not os.path.exists(args.binary):
        print('binary not found: %s' % args.binary, file=sys.stderr)
        return 2

    total = failed = 0
    for name, cases in blocks():
        bad = []
        for source in cases:
            total += 1
            reason = run(args.binary, source)
            if reason:
                bad.append((source, reason))
        failed += len(bad)
        mark = 'ok  ' if not bad else 'FAIL'
        print('  %s %-16s %d cases' % (mark, name, len(cases)))
        for source, reason in bad:
            shown = ' '.join(source.split())
            print('       [%s] %s' % (reason, shown[:90]))

    print()
    print('block review: %d cases, %d failed' % (total, failed))
    if failed:
        return 1
    print('Every block parses, is idempotent, and keeps every character.')

    # A block present in the grammar but with no cases here is a gap in this
    # file, not a pass. Checking it is what stops the list from quietly
    # drifting as blocks are added.
    absent = check_block_coverage()
    if absent:
        print()
        print('BLOCKS WITH NO CASES: %s' % ', '.join(absent))
        return 1
    print('Every block in the reference grammar has cases.')
    return 0


if __name__ == '__main__':
    sys.exit(main())
