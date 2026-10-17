#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Test suite for icl-format (IEEE 1687-2014 ICL formatter).

Runs the compiled binary against three families of cases:

  A. Invalid ICL must be *rejected* (exit 2). This is the whole point of using
     a real parser instead of bracket heuristics.
  B. Valid ICL must be *accepted*, covering every construct in the grammar.
  C. Formatting invariants: idempotence, semantic preservation (no character
     is lost or invented), and CLI behaviour.

Usage:  python3 run_tests.py --binary build/icl-format
"""

import argparse
import os
import subprocess
import sys
import tempfile

PASS = []
FAIL = []

# Google style default: 2-space indent.
BASE = """Module TDR {
Parameter Width = 8;
Attribute name = "TDR";
ScanInPort SI; ScanOutPort SO { Source reg; }
ScanRegister reg[7:0] { ScanInSource SI; CaptureSource DataIn[7:0]; }
DataInPort DataIn[7:0]; DataOutPort DataOut[7:0] { Source reg[7:0]; }
}
"""


def check(name, cond, detail=''):
    (PASS if cond else FAIL).append(name)
    tag = '  [PASS] ' if cond else '  [FAIL] '
    print(tag + name + (('  -> ' + detail) if (detail and not cond) else ''))


def ws(text):
    """Removes all whitespace; used for semantic-preservation checks."""
    return ''.join(text.split())


class Runner:
    def __init__(self, binary):
        self.binary = binary

    def run(self, args, stdin_text=None):
        return subprocess.run([self.binary] + args, input=stdin_text,
                              capture_output=True, text=True)

    def format_text(self, text, extra=None):
        return self.run((extra or []) + ['--stdin'], stdin_text=text)

    def format_file(self, path, extra=None):
        return self.run((extra or []) + [path])


def idem(runner, src, tag, extra=None):
    """Format, then assert idempotence and character preservation."""
    first = runner.format_text(src, extra)
    if first.returncode != 0:
        check(tag + ' | accepted', False,
              'rc=%d err=%s' % (first.returncode, first.stderr.strip()[:120]))
        return None
    out = first.stdout
    second = runner.format_text(out, extra)
    if second.returncode != 0:
        check(tag + ' | idempotent (2nd pass)', False,
              'rc=%d err=%s' % (second.returncode, second.stderr.strip()[:120]))
        return out
    check(tag + ' | idempotent f(f(x))==f(x)', out == second.stdout,
          'A=%r\n     B=%r' % (out[:160], second.stdout[:160]))
    check(tag + ' | semantics preserved (no char lost)', ws(src) == ws(out),
          'src=%d chars -> out=%d chars' % (len(ws(src)), len(ws(out))))
    return out


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--binary', required=True)
    args = parser.parse_args()

    binary = args.binary
    if not os.path.exists(binary):
        print('binary not found: %s' % binary)
        return 2
    runner = Runner(binary)

    print('=' * 72)
    print('A. Invalid ICL must be rejected (exit 2)')
    print('=' * 72)
    BAD = {
        'Module without a name':        'Module { ScanInPort SI; }',
        'Misspelled keyword (Modul)':   'Modul m { ScanInPort SI; }',
        'Statement without semicolon':  'Module m { ScanInPort SI }',
        'Instance without Of':          'Module m { Instance a b { } }',
        'Unknown construct FooBar':     'Module m { FooBar x; }',
        'Unclosed brace':               'Module m { ScanInPort SI;',
        'Extra closing brace':          'Module m { ScanInPort SI; } }',
        'ScanMux without select reg':   "Module m { ScanMux mx { 1'b0 : si; } }",
        'ScanMux without SelectedBy':   "Module m { ScanMux mx r { 1'b0 : si; } }",
        'Connection without semicolon': 'Module m { Connection { a -> b } }',
        'Parameter without =':          'Module m { Parameter W 8; }',
        'Port at top level':            'ScanInPort SI;',
        'Illegal character @':          'Module m { @ ScanInPort SI; }',
        'PortGroup without list':       'Module m { PortGroup pg { } }',
        'AccessLink block incomplete':  'Module m { AccessLink a Of T { BSDL_Entity } }',
    }
    for tag, src in BAD.items():
        result = runner.format_text(src)
        check('reject | %s' % tag, result.returncode == 2,
              'accepted, output=%r' % result.stdout[:80])

    print()
    print('=' * 72)
    print('B. Valid ICL must be accepted (grammar coverage)')
    print('=' * 72)
    GOOD = {
        'empty Module':            'Module m { }',
        'single port':             'Module m { ScanInPort SI; }',
        'all 13 port kinds':       'Module m { ScanInPort a; ScanOutPort b; '
                                   'ShiftEnPort c; CaptureEnPort d; UpdateEnPort e; '
                                   'SelectPort f; ResetPort g; TCKPort h; TMSPort i; '
                                   'ToDoPort j; ToSelectPort k; DataInPort l; '
                                   'DataOutPort m2; }',
        # IEEE 1687 splits port functions into client and host sides. The host
        # side (ToTCKPort, ToTRSTPort, ...) was missing entirely.
        'client-TAP ports':        'Module m { TCKPort tck; TMSPort tms; '
                                   'TRSTPort trst; }',
        'host-side ports':         'Module m { ToTCKPort a; ToTMSPort b; '
                                   'ToTRSTPort c; ToShiftEnPort d; '
                                   'ToCaptureEnPort e; ToUpdateEnPort f; '
                                   'ToResetPort g; }',
        'ports with bit ranges':   'Module m { DataInPort din[31:0]; DataOutPort dout[7:0]; }',
        'ScanRegister range':      'Module m { ScanRegister r[7:0] { ScanInSource SI; } }',
        'Reg and Field':           'Module m { Reg r[3:0] { WriteEnSource ue; } '
                                   'Field f[1:0] { Attribute d = "x"; } }',
        'Enum':                    "Module m { Enum e { IDLE = 2'b00; RUN = 2'b01; } }",
        'Alias with iProcs':       'Module m { Alias a = b.c { iProcsForModule Top; } }',
        'Alias without block':     'Module m { Alias a = b.c; }',
        'ScanInterface':           'Module m { ScanInterface si { Port TDI; } }',
        'OneHotScanInterface':     'Module m { OneHotScanInterface o { Port s[3:0]; } }',
        # `SelectedBy` is required by IEEE 1687.1-2014; the selector may be a
        # hierarchical name. Regression: the grammar once accepted a bare
        # second identifier and rejected the standard keyword.
        # IEEE 1687 Table 2 elements that were missing until the grammar was
        # grouped by shape.
        'DataRegister block':     'Module m { DataRegister dr[7:0] { DataInSource a; } }',
        'DataRegister statement': 'Module m { DataRegister dr; }',
        'DataMux':                "Module m { DataMux dm SelectedBy sel { 1'b0 : a; } }",
        'ClockMux':               'Module m { ClockMux cm SelectedBy sel { a; b; } }',
        'OneHotScanGroup':        'Module m { OneHotScanGroup g { Port a.SO; Port b.SO; } }',
        'OneHotDataGroup':        'Module m { OneHotDataGroup g { Port a; } }',
        # Parameterised ICL, verbatim from the Tessent IJTAG User's Manual
        # ("ICL Network Extraction of Parameterized Modules"). Both the `$MSB`
        # reference and the unsized `'b1 literal were rejected before.
        'parameter in range':     'Module bus { DataInPort datain[ $MSB:0 ]; }',
        'parameter in index':     'Module bus { DataOutPort dataout { Source RegD[ $MSB:0 ]; } }',
        'parameter declared':     'Module bus { Parameter MSB = 5; }',
        'parameter overridden':   'Module top { Instance i Of bus { Parameter MSB = 7; } }',
        'unsized binary literal': "Module top { Instance i Of b { InputPort din = 'b1; } }",
        'unsized hex literal':    "Module m { Attribute d = 'hFF; }",
        'separated decimal':      'Module m { Attribute d = 1_000; }',
        'ScanMux table':           "Module m { ScanMux mx SelectedBy r { 1'b0 : si; 1'b1 : fso; } }",
        'ScanMux hierarchical sel': "Module m { ScanMux M3 SelectedBy MyTap.M3_sel { "
                                   "1'b0 : MyTap.tdo; } }",
        # Attribute values are expressions in real ICL (`ResetValue -1;`, LogicSignal terms like `TDR1[61:32] == 32'b0`), so the operators have to
        # survive formatting unchanged.
        'negative literal':       'Module m { Attribute d = -1; }',
        'unary complement':       'Module m { Attribute d = ~a; }',
        'comparison expression':  "Module m { Attribute d = a[61:32] == 32'b0; }",
        'arithmetic expression':  'Module m { Attribute d = a + b * 2; }',
        'parenthesised':          'Module m { Attribute d = (a + b); }',
        'shift and logic':        'Module m { Attribute d = a << 2 || b; }',
        'Attribute numeric value': 'Module m { Attribute d = 8; }',
        'Attribute hierarchical':  'Module m { Attribute d = a.b.c; }',
        'LogicSignal nested':     'Module m { LogicSignal R0_W { Select { a; } } }',
        'LogicSignal':             'Module m { LogicSignal clk { Source TCK; } }',
        'Connection arrows':       'Module m { Connection { a.SO -> b.SI; c -> d; } }',
        'PortGroup comma list':    'Module m { PortGroup pg {si, so, en} }',
        'AccessLink nested':       'Module m { AccessLink t Of STD_1149_1 { '
                                   'BSDL_Entity E; READINSTR { ScanPath { a; b; } '
                                   'ActiveSignals { en; } } } }',
        'Instance port mapping':   'Module m { Instance i Of T { InputPort a = b.c; } }',
        'indexed expressions':     "Module m { ScanRegister r[7:0] { CaptureSource a[3:0]; "
                                   "ScanInSource b[0]; } }",
        'sized literals':          "Module m { ScanRegister r { ResetValue 32'h0000_00FF; } }",
        'several modules':         'Module a { } Module b { }',
        'empty file':              '',
        'only a comment':          '// nothing\n',
    }
    for tag, src in GOOD.items():
        result = runner.format_text(src)
        check('accept | %s' % tag, result.returncode == 0,
              'rc=%d err=%s' % (result.returncode, result.stderr.strip()[:120]))

    print()
    print('=' * 72)
    print('C. Formatting invariants')
    print('=' * 72)
    out = idem(runner, BASE.replace('\n', '\r\n'), 'CRLF')
    if out:
        check('CRLF | output is LF only', '\r' not in out)

    tab = "Module top {\n\tScanInPort SI;\n\tInstance i Of T {\n\t\tInputPort SI = x;\n\t}\n}\n"
    out = idem(runner, tab, 'Tab input')
    if out:
        check('Tab | no tabs in output', '\t' not in out)

    deep = 'Module m0 {\nAccessLink a Of T {\nL1 {\nL2 {\nL3 {\nIdent x;\n}\n}\n}\n}\n}\n'
    out = idem(runner, deep, 'deep nesting')
    if out:
        target = [l for l in out.split('\n') if 'Ident x' in l][0]
        check('nesting | level 5 indents 10 spaces (2 per level)',
              len(target) - len(target.lstrip()) == 10, repr(target))

    # Blank-line preservation.
    blank = 'Module m {\nScanInPort SI;\n\nScanOutPort SO { Source SI; }\n}\n'
    out = idem(runner, blank, 'blank lines')
    if out:
        check('blank lines | one blank line preserved', '\n\n' in out, repr(out))

    print()
    print('=' * 72)
    print('D. Comments and strings in awkward positions')
    print('=' * 72)
    CMT = {
        'standalone line comment':   'Module m {\n// note\nScanInPort SI;\n}',
        'trailing comment':          'Module m {\nScanInPort SI; // note\n}',
        'multi-line block comment':  'Module m {\n/* a\nb */\nScanInPort SI;\n}',
        'block comment after stmt':  'Module m {\nScanInPort SI; /* a */\n}',
        'comment before brace':      'Module m { ScanInPort SI; /* t */ }',
        'consecutive comments':      'Module m {\n// a\n// b\nScanInPort SI;\n}',
        'comment between modules':   'Module a { }\n// between\nModule b { }',
        'CJK comment':               'Module m { // \u4e2d\u6587\u6ce8\u91ca\nScanInPort SI; }',
        'URL inside a string':       'Module m { Attribute u = "http://a/*b*/"; }',
        'escaped quotes in string':  'Module m { Attribute q = "say \\"hi\\""; }',
        'braces inside a comment':   'Module m { // { weird }\nScanInPort SI; }',
    }
    for tag, src in CMT.items():
        out = idem(runner, src, 'comment/' + tag)
        if out and tag == 'URL inside a string':
            check('comment | URL string not truncated', 'http://a/*b*/' in out, out)
        if out and tag == 'trailing comment':
            check('comment | trailing comment joined to line',
                  'ScanInPort SI; // note' in out, out)

    print()
    print('=' * 72)
    print('E. Real ICL sample')
    print('=' * 72)
    samples_dir = os.path.normpath(os.path.join(
        os.path.dirname(os.path.abspath(__file__)), '..', 'samples'))
    if os.path.isdir(samples_dir):
        for name in sorted(os.listdir(samples_dir)):
            if not name.endswith('.icl'):
                continue
            sample = os.path.join(samples_dir, name)
            with open(sample, encoding='utf-8') as handle:
                src = handle.read()
            out = idem(runner, src, 'sample/' + name)
            if out is None:
                continue
            # Keywords that the sample files exercise; a formatter that dropped
            # or mangled any of them would be useless in practice.
            for keyword in ['PortGroup', 'AccessLink', 'ScanPath', 'ScanMux',
                            'ActiveSignals', 'LaunchEdge', 'ScanInPort',
                            'ScanRegister', 'Instance']:
                if keyword in src:
                    check('%s | %s kept' % (name, keyword), keyword in out)
            # Star-aligned block comments must keep their alignment.
            if '/*' in src and '\n *' in src:
                check('%s | star-aligned comment preserved' % name,
                      '\n *' in out, repr(out[:200]))
    else:
        print('  (skipped: %s not found)' % samples_dir)

    print()
    print('=' * 72)
    print('F. CLI behaviour')
    print('=' * 72)
    with tempfile.TemporaryDirectory(prefix='iclfmt_') as tmp:
        paths = []
        for i in range(3):
            path = os.path.join(tmp, 'm%d.icl' % i)
            with open(path, 'w', encoding='utf-8') as handle:
                handle.write(BASE.replace('TDR', 'TDR%d' % i))
            paths.append(path)

        result = runner.run(paths + ['--check'])
        check('CLI | --check reports unformatted (exit 1)', result.returncode == 1,
              'rc=%d' % result.returncode)
        check('CLI | -w rewrites in place (exit 0)',
              runner.run(paths + ['-w']).returncode == 0)
        result = runner.run(paths + ['--check'])
        check('CLI | --check passes after -w (exit 0)', result.returncode == 0,
              'rc=%d out=%s' % (result.returncode, result.stdout))

        out_path = os.path.join(tmp, 'out.icl')
        result = runner.run([paths[0], '-o', out_path])
        check('CLI | -o writes a file',
              result.returncode == 0 and os.path.getsize(out_path) > 0)

        out2 = os.path.join(tmp, 'out2.icl')
        runner.run([paths[0], '-n', '4', '-o', out2])
        lines = [l for l in open(out2).read().split('\n') if 'Parameter' in l]
        check('CLI | -n 4 changes indent width',
              bool(lines) and len(lines[0]) - len(lines[0].lstrip()) == 4,
              repr(lines[0]) if lines else 'no Parameter line')

        check('CLI | -o with multiple inputs rejected',
              runner.run(paths[:2] + ['-o', 'x']).returncode != 0)
        check('CLI | -w with --check rejected',
              runner.run([paths[0], '-w', '--check']).returncode != 0)

        bad = os.path.join(tmp, 'bad.icl')
        with open(bad, 'w', encoding='utf-8') as handle:
            handle.write('Module { }')
        result = runner.run([bad])
        check('CLI | syntax error -> exit 2 with location',
              result.returncode == 2 and '.icl:' in result.stderr,
              'rc=%d err=%r' % (result.returncode, result.stderr[:120]))

        # Regression: the scanner's line counter is file-scope state, so a
        # second file used to inherit the first file's line numbers and report
        # syntax errors at the wrong line.
        good_path = os.path.join(tmp, 'line_ok.icl')
        with open(good_path, 'w', encoding='utf-8') as handle:
            handle.write('Module A {\n  ScanInPort SI;\n}\n')
        bad_path = os.path.join(tmp, 'line_bad.icl')
        with open(bad_path, 'w', encoding='utf-8') as handle:
            handle.write('Module B {\n  @ bad\n}\n')
        result = runner.run([good_path, bad_path])
        check('CLI | error line is per-file, not cumulative',
              result.returncode == 2 and 'line 2' in result.stderr,
              'rc=%d err=%r' % (result.returncode, result.stderr[:120]))

        check('CLI | missing file -> exit 2',
              runner.run([os.path.join(tmp, 'nope.icl')]).returncode == 2)

        binary_path = os.path.join(tmp, 'b.icl')
        with open(binary_path, 'wb') as handle:
            handle.write(b'\x00\xff\xfe Module m { }')
        before = open(binary_path, 'rb').read()
        result = runner.run([binary_path, '-w'])
        check('CLI | binary file refused and left untouched',
              result.returncode == 2 and open(binary_path, 'rb').read() == before)

        check('CLI | --ast works',
              runner.run([paths[0], '--ast']).returncode == 0)
        check('CLI | --version works', runner.run(['--version']).returncode == 0)

    print()
    print('=' * 72)
    print('TOTAL: PASS=%d  FAIL=%d' % (len(PASS), len(FAIL)))
    print('=' * 72)
    if FAIL:
        print('Failures:')
        for name in FAIL:
            print('  - ' + name)
        return 1
    print('All tests passed.')
    return 0


if __name__ == '__main__':
    sys.exit(main())
