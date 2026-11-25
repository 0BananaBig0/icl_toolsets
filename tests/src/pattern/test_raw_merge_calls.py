#!/usr/bin/env python3
"""Supported raw merge calls retain complete native serial execution."""
import subprocess
import unittest

import test_simultaneous_scan_chains as cli
import test_modeled_raw_readback as raw
import test_modeled_root_scan as modeled
from benchmark_opaque_child_raw import model as opaque_child_model

ROOT = raw.ROOT
CHILD = (cli.ROOT/'tests/cases/ours/pattern_generation/static_child_scans/model.icl').read_text().replace(
    'Module Top', 'Module Chains')


class RawMergeCalls(unittest.TestCase):
    setUp = cli.SimultaneousChains.setUp

    def run_tool(self, model, pdl, output='result.pdl', top='Chains', extra=(), inputs=(), ok=True):
        command = [str(cli.BIN), '--icl', str(model), '--pdl', str(pdl),
                   '--top_module', top, '--output', str(self.root/output), '--debug',
                   *inputs, *extra]
        result = subprocess.run(command, capture_output=True, text=True, timeout=45)
        self.assertEqual(result.returncode == 0, ok, result.stdout+result.stderr)
        return (self.root/output).read_bytes() if ok else result.stdout+result.stderr

    def check(self, source, body, top='Chains', inputs=(), compare=True, reset=False, second=None, modeled=False):
        model, pdl = self.root/'source.icl', self.root/'source.pdl'
        model.write_text(source)
        header = 'iPDLLevel 1\niProcsForModule '+top+'\n'
        pdl.write_text(header+body)
        extra = ['--model_readback'] if modeled else []
        if second is not None:
            other = self.root/'second.pdl'
            other.write_text(second)
            extra += ['--pdl', str(other)]
        if reset:
            extra += ['--local_reset', str(self.root/'reset.pdl')]
        output = self.run_tool(model, pdl, top=top, inputs=inputs, extra=extra)
        self.assertEqual(output, self.run_tool(self.root/'debug'/f'{top}.icl', pdl,
                         'flat/result.pdl', top, extra=extra))
        replay_extra = ['--model_readback'] if modeled else []
        self.assertEqual(output, self.run_tool(model, self.root/'result.pdl',
                         'generated/result.pdl', top, inputs=inputs, extra=replay_extra))
        if compare:
            serial = self.root/'serial.pdl'
            serial.write_text((header+body+(second or '')).replace('iMerge -begin', '').replace('iMerge -end', ''))
            self.assertEqual(output, self.run_tool(model, serial, 'serial/result.pdl',
                             top, inputs=inputs, extra=replay_extra))
        if reset:
            rewritten = (self.root/'reset.pdl').read_text()
            self.assertEqual(output, self.run_tool(model, self.root/'reset.pdl',
                             'rewritten/result.pdl', top, inputs=inputs, extra=replay_extra))
            return output, rewritten
        return output

    def rejected(self, source, body, diagnostic, top='Chains', extra=(), inputs=()):
        model, pdl = self.root/'bad.icl', self.root/'bad.pdl'
        model.write_text(source)
        pdl.write_text('iPDLLevel 1\niProcsForModule '+top+'\n'+body)
        target = self.root/'result.pdl'
        target.write_bytes(b'retained artifact\n')
        message = self.run_tool(model, pdl, top=top, extra=extra, inputs=inputs, ok=False)
        self.assertIn(diagnostic, message)
        self.assertEqual(target.read_bytes(), b'retained artifact\n')

    def test_scalar_and_static_child_routes(self):
        for source, target, width in ((ROOT, 'client', 4), (CHILD, 'child.local', 8)):
            with self.subTest(target=target):
                body = ('iProc access {word} {iScan %s %d -si $word; iApply}\n'
                        'iReset; iMerge -begin; iCall access 9; iCall access 6; iMerge -end\n' % (target, width))
                output = self.check(source, body)
                self.assertEqual(sum(line.startswith(b'iScan ') for line in output.splitlines()), 2)
                self.assertEqual(output.count(b'iApply\n'), 2)

    def test_nested_and_repeated_calls(self):
        output = self.check(ROOT, '''iProc leaf {a b} {
iScan client 4 -si $a; iApply; iScan client 4 -si $b; iApply
}
iProc outer {} {iMerge -begin; iCall leaf 9 6; iCall leaf 3 5; iMerge -end}
iReset; iMerge -begin; iCall outer; iCall outer; iMerge -end
''')
        self.assertEqual(sum(line.startswith(b'iScan ') for line in output.splitlines()), 8)
        self.assertEqual(output.count(b'iApply\n'), 8)

    def test_namespaces_prefix_split_and_once_arguments(self):
        definitions = '''iProcsForModule ChildLeaf -iProcNameSpace api
iProc access {word} {iScan local 8 -si $word; iApply}
iProcsForModule Chains
'''
        body = '''set ::args 0
iReset; iPrefix child
iMerge -begin
iCall api::access [expr {[incr ::args]+8}]
iCall api::access [expr {[incr ::args]+4}]
iMerge -end
if {$::args != 2} {error argument_count}
'''
        output = self.check(CHILD, definitions, second=body)
        self.assertIn(b'0b0000000100100000', output)
        self.assertIn(b'0b0000000011000000', output)

    def test_raw_and_ordinary_frame_order(self):
        output = self.check(ROOT, '''iProc access {} {
iWrite r 3; iApply
iScan client 4 -si 9; iApply
iWrite r 6; iApply; iRead r 6; iApply
}
iReset; iMerge -begin; iCall access; iMerge -end
''')
        self.assertLess(output.index(b'iWrite r 0b0011'), output.index(b'iScan '))
        self.assertLess(output.index(b'iScan '), output.index(b'iWrite r 0b0110'))

    def test_parallel_named_chains(self):
        output = self.check(cli.NETWORK, '''iProc access {a b} {
iScan client -chain first 4 -si $a
iScan client -chain second 2 -si $b
iApply -together
}
iReset; iMerge -begin; iCall access 9 2; iCall access 6 1; iMerge -end
''')
        self.assertEqual(sum(line.startswith(b'iScan ') for line in output.splitlines()), 4)
        self.assertEqual(output.count(b'iApply -together'), 2)

    def test_bound_tap_ir_and_dr(self):
        fixture = cli.ROOT/'tests/cases/ours/pattern_generation/raw_tap_dr'
        output = self.check((fixture/'model.icl').read_text(), '''
iProc instruction {} {iScan -ir jtag 4 -si 3; iApply}
iProc data {word} {iScan jtag 8 -si $word; iApply}
iReset; iMerge -begin; iCall instruction; iCall data 0x96; iCall data 0x3c; iMerge -end
''', top='RawDrTop', inputs=('--bsdl', 'RawDrProbe', str(fixture/'model.bsdl')))
        self.assertLess(output.index(b'iScan -ir'), output.index(b'iScan jtag'))
        self.assertIn(b'-si 0b10010110', output)
        self.assertIn(b'-si 0b00111100', output)

    def test_stable_defaults_and_queue_overwrites(self):
        source = ROOT.replace('ScanInterface client {', "ScanInterface client {DefaultLoadValue 4'b0;")
        output = self.check(source, '''iProc access {} {
iScan client 5 -si 31; iScan client 4 -si 6; iApply
iScan client 4 -si 3 -stable; iApply; iScan client 4; iApply
}
iReset; iMerge -begin; iCall access; iMerge -end
''')
        self.assertEqual(sum(line.startswith(b'iScan ') for line in output.splitlines()), 3)
        for value in (b'0110', b'0011', b'0000'):
            self.assertIn(b'-si 0b'+value, output)

    def test_notes_waits_and_ordinary_batching(self):
        output = self.check(cli.NETWORK, '''iProc pin {target word} {iWrite $target $word; iApply}
iProc access {} {
iScan client -chain first 4 -si 9; iScan client -chain second 2 -si 2
iApply; iRunLoop 2
}
iReset; iMerge -begin
iCall pin a 1; iCall pin b 1; iCall access; iCall pin a 0; iCall pin b 0
iNote outside; iMerge -end
''', compare=False)
        self.assertLess(output.index(b'iNote outside'), output.index(b'iWrite a'))
        self.assertLess(output.index(b'iScan '), output.index(b'iRunLoop 2'))
        pending, groups = set(), []
        for line in output.splitlines():
            words = line.split()
            if words and words[0] == b'iWrite':
                pending.add(words[1])
            elif words and words[0] == b'iApply':
                groups.append(pending)
                pending = set()
        self.assertEqual(sum({b'a', b'b'} <= group for group in groups), 2)

    def test_caller_local_reset_preserves_procedure_bytes(self):
        definition = 'iProc access {word} {iScan client 4 -si $word; iApply}\n'
        output, rewritten = self.check(ROOT, definition+
            'iReset; iMerge -begin; iCall access 9; iCall access 6; iMerge -end\n', reset=True)
        self.assertIn(definition, rewritten)
        self.assertEqual(sum(line.startswith(b'iScan ') for line in output.splitlines()), 2)

    def test_reservations_and_failed_output_protection(self):
        source = modeled.SIB.replace('Module Top', 'Module Chains')
        definition = 'iProc access {word} {iScan child.local 8 -si $word; iApply}\n'
        self.check(source, definition+'iReset; iTake open; iState open 1; '
                   'iMerge -begin; iCall access 9; iMerge -end\n')
        self.rejected(source, definition+'iReset; iTake open; iState open 0; '
                      'iMerge -begin; iCall access 9; iMerge -end\n', 'reserved')

    def test_merge_queue_and_reset_protocol_errors(self):
        cases = [
            ('iReset; iMerge -begin; iScan client 4; iMerge -end', 'only iCall'),
            ('iProc access {} {iScan client 4 -si 9}; iReset; iMerge -begin; iCall access; iMerge -end', 'unapplied'),
            ('iProc access {} {iScan client 4 -si 9}; iProc other {} {iApply}; '
             'iReset; iMerge -begin; catch {iCall access}; iCall other; iMerge -end', 'unapplied'),
            ('iProc access {} {iReset}; iReset; iMerge -begin; iCall access; iMerge -end', 'forbidden'),
            ('iProc access {} {iScan client 5 -si 1; iApply}; '
             'iReset; iMerge -begin; iCall access; iMerge -end', 'length'),
        ]
        for body, diagnostic in cases:
            with self.subTest(body=body):
                self.rejected(ROOT, body, diagnostic)

    def test_remaining_history_opaque_and_physical_guards(self):
        body = ('iProc access {} {iScan client 4 -si 9; iApply}\n'
                'iReset; iMerge -begin; iCall access; iMerge -end\n')
        self.check(ROOT, body, modeled=True)
        output = self.check(ROOT, body+'iNote [iGetReadData client]\n', modeled=True)
        self.assertIn(b'iNote 0b0000\n', output)
        self.rejected(ROOT, body, 'physical', extra=('--physical_pdl',))
        opaque_body = ('iProc access {} {iScan client 8 -si 9; iApply}\n'
                       'iReset; iMerge -begin; iCall access; iMerge -end\n')
        self.rejected(opaque_child_model(), opaque_body, 'length', top='Top')


if __name__ == '__main__':
    unittest.main()
