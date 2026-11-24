#!/usr/bin/env python3
"""Observed merge histories preserve the generated native event trajectory."""
import subprocess
import unittest

import test_simultaneous_scan_chains as cli
import test_modeled_readback as history
import test_modeled_raw_readback as raw
import test_modeled_tap_readback as tap
import test_downstream_write_callbacks as callbacks

PIO = history.BASE
EVENTS = """Module Chains {
 DataInPort p[3:0]; DataInPort q;
 DataRegister d[3:0] {WriteDataSource p; WriteEnSource 1'b1;}
 DataOutPort observed[3:0] {Source d;}
}"""


def merged(body, suffix=''):
    return ('iProc access {} {' + body + '}\n'
            'iMerge -begin; iCall access; iMerge -end\n' + suffix)


class MergedReadback(unittest.TestCase):
    setUp = cli.SimultaneousChains.setUp

    def run_tool(self, model, pdl, output='result.pdl', top='Chains', inputs=(), extra=(), ok=True):
        result = subprocess.run([str(cli.BIN), '--icl', str(model), '--pdl', str(pdl),
            '--top_module', top, '--output', str(self.root/output), '--debug',
            '--model_readback', *inputs, *extra], capture_output=True, text=True, timeout=45)
        self.assertEqual(result.returncode == 0, ok, result.stdout+result.stderr)
        return (self.root/output).read_bytes() if ok else result.stdout+result.stderr

    def check(self, model, body, top='Chains', inputs=(), reset=False, second=''):
        source, pdl = self.root/'source.icl', self.root/'source.pdl'
        source.write_text(model)
        pdl.write_text('iPDLLevel 1\niProcsForModule '+top+'\n'+body)
        split = ()
        if second:
            other = self.root/'second.pdl'
            other.write_text(second)
            split = ('--pdl', str(other))
        original_reset, flat_reset = self.root/'reset-source.pdl', self.root/'reset-flat.pdl'
        output = self.run_tool(source, pdl, top=top, inputs=inputs,
            extra=(*split, *(('--local_reset', str(original_reset)) if reset else ())))
        self.assertEqual(output, self.run_tool(self.root/'debug'/f'{top}.icl', pdl,
            'flat/result.pdl', top, extra=(*split, *(('--local_reset', str(flat_reset)) if reset else ()))))
        self.assertEqual(output, self.run_tool(source, self.root/'result.pdl',
            'generated/result.pdl', top, inputs))
        if reset:
            rewritten = original_reset.read_text()
            self.assertEqual(rewritten, flat_reset.read_text())
            self.assertEqual(output, self.run_tool(source, original_reset,
                'reset-replay/result.pdl', top, inputs))
            return output, rewritten
        return output

    def notes(self, output):
        return [line[6:] for line in output.splitlines()
                if line.startswith(b'iNote ') and b'modeled ' not in line]

    def rejected(self, model, body, diagnostic, extra=()):
        source, pdl = self.root/'bad.icl', self.root/'bad.pdl'
        source.write_text(model)
        pdl.write_text('iPDLLevel 1\niProcsForModule Chains\n'+body)
        artifacts = [self.root/'result.pdl', self.root/'debug/Chains.icl', self.root/'reset.pdl']
        for path in artifacts:
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(b'retained\n')
        reset = () if '--physical_pdl' in extra else ('--local_reset', str(artifacts[-1]))
        message = self.run_tool(source, pdl, ok=False, extra=(*reset, *extra))
        self.assertIn(diagnostic, message)
        for path in artifacts:
            self.assertEqual(path.read_bytes(), b'retained\n')

    def event_program(self, query=''):
        return '''iState p 0; iState q 0; iState d 0
iProc first {} {iWrite q 1; iApply; iRead d 0; iApply; %s}
iProc second {} {iWrite p 9; iApply}
iMerge -begin; iCall first; iCall second; iMerge -end
''' % query

    def capture_probe(self, output):
        probe = self.root/'probe.pdl'
        probe.write_bytes(output+b'\niNote [iGetReadData observed -dec]\n')
        return self.notes(self.run_tool(self.root/'source.icl', probe, 'probe/result.pdl'))[-1]

    def test_pio_inside_and_after_calls(self):
        output = self.check(PIO, merged('iWrite p 9; iRead q 0; iApply; iNote [iGetReadData q -dec]',
            'iNote [iGetReadData q -dec]'))
        self.assertEqual(self.notes(output), [b'9', b'9'])

    def test_observed_event_order_matches_generated_capture(self):
        output = self.check(EVENTS, self.event_program('set ::seen [iGetReadData d -dec]')+
            'iNote $::seen\n')
        self.assertEqual(self.notes(output), [b'0'])
        self.assertEqual(self.capture_probe(output), b'0')
        self.assertEqual(output.count(b'iApply\n'), 3)

    def test_no_provider_request_keeps_batching(self):
        output = self.check(EVENTS, self.event_program())
        self.assertEqual(output.count(b'iApply\n'), 2)
        self.assertEqual(self.capture_probe(output), b'9')

    def test_cached_manual_and_zero_status_keep_batching(self):
        for query, expected in [('iState d 5 -LastReadValue; set ::seen [iGetReadData d -dec]', b'5'),
                ('iState d 5 -LastReadValue; iGetReadData d; set ::seen [iGetReadData d -dec]', b'5'),
                ('set ::seen [iGetReadData observed -dec]', b'X')]:
            with self.subTest(query=query):
                suffix = ('iNote $::seen\n'
                    'iProc pinA {} {iWrite p 9; iApply}\n'
                    'iProc pinB {} {iWrite q 1; iApply}\n'
                    'iMerge -begin; iCall pinA; iCall pinB; iMerge -end\n')
                output = self.check(EVENTS, self.event_program(query)+suffix)
                self.assertEqual(self.notes(output), [expected])
                self.assertEqual(output.count(b'iApply\n'), 4 if 'iState' in query else 3)
        output = self.check(EVENTS, self.event_program().replace(
            'iWrite q 1;', 'set ::seen [iGetStatus]; iWrite q 1;')+'iNote $::seen\n')
        self.assertEqual(self.notes(output), [b'0'])
        self.assertEqual(output.count(b'iApply\n'), 2)

    def test_pre_merge_observation_keeps_batching(self):
        output = self.check(EVENTS, 'iState p 0; iState d 0; iRead d 0; iApply; '
            'iNote [iGetReadData d -dec];\n'+self.event_program())
        self.assertEqual(output.count(b'iApply\n'), 3)
        self.assertEqual(self.capture_probe(output), b'9')

    def test_unknown_sparse_and_inverted(self):
        output = self.check(PIO, merged('iRead q[1] 0; iApply; '
            'iNote [iGetReadData q -bin]; iNote [iGetReadData inverted -bin]'))
        self.assertEqual(self.notes(output), [b'0bxxxx', b'0bxxxx'])
        output = self.check(PIO, merged('iState p 9; iRead q[1:0] 0; iApply; '
            'iNote [iGetReadData q -bin]; iNote [iGetReadData inverted -bin]'))
        self.assertEqual(self.notes(output), [b'0bxx01', b'0bxx10'])

    def test_status_and_clear(self):
        output = self.check(PIO, merged('iState p 15; iRead q 0; iApply; '
            'iNote [iGetStatus -clear]; iNote [iGetStatus]',
            'iRead q 0; iApply; iNote [iGetStatus]'))
        self.assertEqual(self.notes(output), [b'4', b'0', b'4'])

    def test_read_before_write_and_tcl_branch(self):
        output = self.check(PIO, 'iState p 3; '+merged('iRead q 0; iApply; '
            'if {[iGetReadData q -dec] != 3} {error capture}; iWrite p 6; iApply',
            'iRead q 0; iApply; iNote [iGetReadData q -dec]'))
        self.assertEqual(self.notes(output), [b'6'])

    def test_serial_register_history(self):
        output = self.check(raw.ROOT, 'iReset; '+merged('iWrite r 9; iApply; '
            'iRead r 0; iApply; iNote [iGetReadData r -dec]'))
        self.assertEqual(self.notes(output), [b'9'])

    def test_bound_tap_history(self):
        source = tap.ModeledTapReadback.source(self)
        output = self.check(source, 'iReset; '+merged('iWrite core.r 0x96; iApply; '
            'iRead core.r 0; iApply; iNote [iGetReadData core.r -hex]'), top='AccessTop',
            inputs=('--bsdl', 'AccessProbe', str(tap.FIXTURE/'simple.bsdl')))
        self.assertEqual(self.notes(output), [b'0x96'])

    def test_raw_and_register_planes(self):
        output = self.check(raw.ROOT, 'iReset; '+merged('iWrite r 9; iApply; '
            'iRead r 0; iApply; iScan client 4 -si 6 -so 0; iApply; '
            'iNote [iGetReadData r -dec]; iNote [iGetReadData client -dec]',
            'iRead r 0; iApply; iNote [iGetReadData r -dec]; '
            'iNote [iGetReadData client -dec]'))
        self.assertEqual(self.notes(output), [b'9', b'9', b'6', b'9'])

    def test_named_parallel_raw(self):
        output = self.check(cli.NETWORK, 'iReset; '+merged('iWrite a 9; iWrite b 2; iApply; '
            'iScan client -chain first 4 -si 6; iScan client -chain second 2 -si 1; '
            'iApply -together; iNote [iGetReadData client -chain first -dec]; '
            'iNote [iGetReadData client -chain second -dec]'))
        self.assertEqual(self.notes(output), [b'9', b'2'])

    def test_nested_repeated_and_returned_calls(self):
        output = self.check(PIO, '''iProc leaf {word} {
iWrite p $word; iRead q 0; iApply; iNote [iGetReadData q -dec]; return done
}
iProc outer {} {iMerge -begin; iCall leaf 9; iCall leaf 6; iMerge -end}
iMerge -begin; iCall outer; iCall outer; iMerge -end
''')
        self.assertEqual(self.notes(output), [b'9', b'6', b'9', b'6'])

    def test_namespaces_prefix_split_once_arguments(self):
        model = 'Module Leaf {DataInPort p[3:0]; DataOutPort q[3:0] {Source p;}}\n'+'''Module Chains {
DataInPort p[3:0]; Instance leaf Of Leaf {InputPort p = p;}
DataOutPort q[3:0] {Source leaf.q;}
}'''
        output = self.check(model, '''iProcsForModule Leaf -iProcNameSpace api
iProc access {} {iRead q 0; iApply; iNote [iGetReadData q -dec]}
iProcsForModule Chains
set ::args 0; iState p [expr {[incr ::args]+8}]; iPrefix leaf
''', second='iMerge -begin; iCall api::access; iMerge -end; '
            'if {$::args != 1} {error argument_count}')
        self.assertEqual(self.notes(output), [b'9'])

    def test_reset_replay_preserves_procedure_bytes(self):
        definition = 'iProc access {} {iWrite p 9; iRead q 0; iApply; iNote [iGetReadData q -dec]}\n'
        output, rewritten = self.check(PIO, definition+
            'iMerge -begin; iCall access; iMerge -end\n', reset=True)
        self.assertIn(definition, rewritten)
        self.assertEqual(self.notes(output), [b'9'])

    def test_scoped_reservations(self):
        output = self.check(PIO, merged('iTake p; iState p 9; iRead q 0; iApply; '
            'iNote [iGetReadData q -dec]', 'iWrite p 6; iRead q 0; iApply; '
            'iNote [iGetReadData q -dec]'))
        self.assertEqual(self.notes(output), [b'9', b'6'])

    def test_explicit_write_callback_endpoint(self):
        output, rewritten = self.check(callbacks.MODEL, callbacks.PROCS+merged(
            'iWrite value 9; iApply; iRead endpoint 0; iApply; '
            'iNote [iGetReadData endpoint -dec]'), reset=True)
        self.assertEqual(self.notes(output), [b'value=0b1001', b'9'])
        self.assertIn('iProc store {data reg}', rewritten)

    def test_callback_guards(self):
        self.rejected(callbacks.MODEL, callbacks.PROCS+merged(
            'iWrite dest 9; iApply'), 'callback histories')
        source = callbacks.MODEL.replace('WriteCallBack Chains', 'ReadCallBack Chains')
        self.rejected(source, callbacks.PROCS+merged('iRead value 9; iApply'), 'callback histories')

    def test_timed_opaque_and_physical_guards(self):
        self.rejected(PIO, merged('iState p 9; iRead q 0; iApply; iRunLoop 1; '
            'iGetReadData q'), 'iRunLoop histories')
        self.rejected(raw.OPAQUE, merged('iScan client -chain first 4 -si 9; iApply; '
            'iGetReadData client -chain first'), 'iMerge histories')
        self.rejected(PIO, merged('iWrite p 9; iRead q 0; iApply; iGetReadData q'),
            'iMerge histories', ('--physical_pdl',))

    def test_queue_failures_preserve_all_artifacts(self):
        for body, diagnostic in [('iWrite p 9', 'unapplied'),
                ('iReset', 'forbidden'), ('iWrite p 9; iMerge -begin', 'unapplied')]:
            with self.subTest(body=body):
                self.rejected(PIO, merged(body), diagnostic)
        output = self.check(PIO, 'iState p 0; iProc access {word} {'
            'iWrite p $word; iRead q 0; iApply; iNote [iGetReadData q -dec]}\n'
            'iMerge -begin; foreach word {1 2 3 4 5 6 7 8} {iCall access $word}; iMerge -end')
        self.assertEqual(self.notes(output), [str(i).encode() for i in range(1, 9)])


if __name__ == '__main__':
    unittest.main()
