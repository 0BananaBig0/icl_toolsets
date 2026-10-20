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

    # ---- module ----
    # module_def : 'Module' module_name '{' module_item* '}'
    b.append(('module', [
        'Module m { }',
        'Module m {\n}\n',
        'Module m { Attribute a = 1; }',
        'Module m { ScanInPort si; }',
        'Module m {\n  ScanInPort si;\n  ScanOutPort so;\n}\n',
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
        port_cases.append('Module m { %s p; }' % p)
        port_cases.append('Module m { %s p[3:0]; }' % p)
        port_cases.append('Module m { %s p { } }' % p)
        port_cases.append('Module m { %s p[3:0] { } }' % p)
        port_cases.append('Module m { %s p { Source s; } }' % p)
        port_cases.append('Module m { %s p { Attribute a = 1; } }' % p)
        # a comment in each of three places inside the body
        port_cases.append('Module m { %s p {\n  // before\n  Source s;\n} }' % p)
        port_cases.append('Module m { %s p {\n  Source s; // after\n} }' % p)
        port_cases.append('Module m { %s p {\n  Source s;\n  // trailing\n} }' % p)
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
        '  ScanInSource a,\n'
        '      b;\n'
        '}\n}',
    ]))

    # ---- data register ----
    # dataRegister_type : selectable | addressable | readCallBack | writeCallBack
    b.append(('dataRegister', [
        'Module m { DataRegister dr; }',
        'Module m { DataRegister dr[7:0]; }',
        'Module m { DataRegister dr { Selectable { si; } } }',
        'Module m { DataRegister dr { Addressable { AddressValue 4; } } }',
        'Module m { DataRegister dr { ReadCallBack rd; } }',
        'Module m { DataRegister dr { WriteCallBack wr; } }',
        'Module m { DataRegister dr { WriteDataSource a; WriteEnSource b; } }',
        'Module m { DataRegister dr { ReadDataSource a; } }',
        'Module m { DataRegister dr { ResetValue 0; } }',
        # iProc callbacks with their argument forms
        'Module m { DataRegister dr { WriteCallBack p <D>; } }',
        'Module m { DataRegister dr { WriteCallBack ns::p <R>; } }',
        'Module m { DataRegister dr { WriteCallBack ::p <D>; } }',
        'Module m { DataRegister dr { WriteCallBack p <D> <R> 5 "s" $X; } }',
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
        'Module m { Instance i Of a::b::c; }',
        'Module m { Instance i of M; }',          # Origen spells it lower case
        'Module m { Instance i Of M { Parameter P = 4; } }',
    ]))

    # ---- muxes ----
    # scanMux_def | dataMux_def | clockMux_def
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
        # a comment among the selections
        "Module m { ScanMux mx SelectedBy s {\n"
        "  1'b0 : a;\n"
        "  // which one\n"
        "  1'b1 : b;\n"
        "} }",
    ]))

    # ---- one-hot groups ----
    b.append(('onehot', [
        'Module m { OneHotScanGroup g { a; b; } }',
        'Module m { OneHotScanGroup g[3:0] { a; } }',
        'Module m { OneHotDataGroup g { DataRegister d; } }',
        'Module m { OneHotDataGroup g { Instance i Of M; } }',
        'Module m { OneHotScanInterface g { a; } }',
    ]))

    # ---- access link ----
    # accessLink1149_def : 'AccessLink' name 'Of'
    #                      ('STD_1149_1_2001' | 'STD_1149_1_2013') '{' ... '}'
    b.append(('accessLink', [
        'Module m { AccessLink a Of STD_1149_1_2001 { BSDLEntity b; } }',
        'Module m { AccessLink a Of STD_1149_1_2013 { BSDLEntity b; } }',
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
        'Attribute x = 1; } }',
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
        'Module m { Attribute a = 1, 2; }',
        'Module m { Attribute a = "s", "t"; }',
        'Module m { Parameter p = "s"; }',
    ]))
    # Absent on purpose: `Parameter p = 4;` at top level — parameter_def is a
    # module_item. Pinned in fail/standard/top_level_parameter.icl.

    # ---- connection ----
    b.append(('connection', [
        'Module m { Connection { a; } }',
        'Module m { Connection { a -> b; } }',
        'Module m { Connection {\n  // how the two are joined\n  a -> b;\n} }',
    ]))

    # ---- port group ----
    # PortGroup is not in the published grammar at all; it is Tessent's. The
    # spelling comes from a real file:
    #
    #   PortGroup tap_side {si, so, en}
    #
    # — a comma list with no semicolon. `PortGroup g { a; }` is not it.
    b.append(('portGroup', [
        'Module m { PortGroup g { a } }',
        'Module m { PortGroup g { a, b } }',
        'Module m { PortGroup g {\n  a, // the first\n  b\n} }',
    ]))

    # ---- module-level statements from the standard ----
    b.append(('moduleStatements', [
        'Module m { AllowBroadcastOnScanInterface si; }',
        'Module m { Reg r { Width 4; } }',
        'Module m { Field f { Width 4; } }',
    ]))

    # ---- recursion and nesting ----
    # The recursive rules, checked at depth: nesting costs heap, not C stack.
    b.append(('recursion', [
        'Module m { LogicSignal ls { ' + '(' * 50 + 'a' + ')' * 50 + '; } }',
        'Module m { LogicSignal ls { ' + '(' * 100 + 'a' + ')' * 100 + '; } }',
        'Module m { LogicSignal ls { ' + '!' * 30 + 'a; } }',
        'Module ' + ' '.join(['m {'] * 1) + ' }\n',
        'Module m {\n' + '  Reg r0 {\n' * 40 + '  Width 1;\n'
        + '  }\n' * 40 + '}\n',
        'Module m { ScanRegister sr[a[0]]; }',
        'Module m { ScanRegister sr[a[0][1][2]]; }',
        'Module m { ScanRegister sr[$W - 1:0]; }',
        # `::` chains: accepted where a signal or a module reference is
        # expected. Not in an attribute value — see below.
        'Module m { ScanOutPort so { Source a::b::c::d::e; } }',
        'Module m { Instance i Of a::b::c::d::e; }',
        'Module m { ScanInterface si { Chain c1 { Chain c2 { Port p; } } } }',
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
        "Module m { Parameter p = xx01_x11; }",
        "Module m { Parameter p = 16'shFF; }",
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
    return 0


if __name__ == '__main__':
    sys.exit(main())
