#!/usr/bin/env python3
"""Forced one-frame pin words retain native events while meeting wide budgets."""
import os
import subprocess
import sys
import unittest

import test_simultaneous_scan_chains as cli
import test_modeled_readback as history
import test_merged_readback as merged


def asan_runtime_present():
    env = dict(os.environ)
    options = env.get('ASAN_OPTIONS','')
    env['ASAN_OPTIONS'] = (options+':' if options else '')+'help=1'
    result = subprocess.run([str(cli.BIN),'--version'],env=env,
                            capture_output=True,text=True,timeout=45)
    return 'Available flags for AddressSanitizer:' in result.stderr.splitlines()


ASAN_RUNTIME_PRESENT = asan_runtime_present()


class PrimaryPinLiterals(unittest.TestCase):
    setUp = history.ModeledReadback.setUp
    run_tool = history.ModeledReadback.run_tool
    replay = history.ModeledReadback.replay
    notes = merged.MergedReadback.notes

    def check(self, model, body):
        return self.replay(model, 'iPDLLevel 1\niProcsForModule Chains\n'+body)

    @unittest.skipIf(ASAN_RUNTIME_PRESENT,
                     'normal/static wide budget; smaller sanitizer cases measured separately')
    def test_wide_memory_goal(self):
        width = 1024
        word = '0b'+'10'*(width//2)
        model, pdl = self.root/'source.icl', self.root/'source.pdl'
        model.write_text(history.BASE.replace('[3:0]', f'[{width-1}:0]'))
        pdl.write_text('iPDLLevel 1\niProcsForModule Chains\niState p 0\n'
            'iProc access {word} {iWrite p $word; iRead q 0; iApply; '
            'iNote [iGetReadData q -bin]}\n'
            f'iMerge -begin; iCall access {word}; iMerge -end\n'
            'iNote [iGetStatus]\n')
        metric, output = self.root/'time.txt', self.root/'result.pdl'
        result = subprocess.run(['/usr/bin/time', '-f', '%e %M', '-o', str(metric),
            str(cli.BIN), '--icl', str(model), '--pdl', str(pdl), '--top_module',
            'Chains', '--output', str(output), '--model_readback', '--debug'],
            capture_output=True, text=True, timeout=45)
        self.assertEqual(result.returncode, 0, result.stdout+result.stderr)
        seconds, rss = metric.read_text().split()
        seconds, rss = float(seconds), int(rss)
        print(f'primary pin width={width} wall_seconds={seconds} peak_rss_kib={rss}', flush=True)
        data = output.read_bytes()
        self.assertEqual(self.notes(data), [word.encode(), b'512'])
        self.assertLessEqual(seconds, 10)
        self.assertLessEqual(rss, 262144)
        self.assertEqual(data, self.run_tool(self.root/'debug/Chains.icl', pdl, 'flat/result.pdl'))
        self.assertEqual(data, self.run_tool(model, output, 'generated/result.pdl'))

    def test_normal_budget_runs_with_asan_options(self):
        if getattr(self.test_wide_memory_goal,'__unittest_skip__',False):
            self.skipTest('normal/static wide budget guard; sanitizer wide case remains excluded')
        check = ('import sys,unittest\n'
            + 'sys.path.insert(0,'+repr(os.path.dirname(__file__))+')\n'
            + 'sys.argv=['+repr(__file__)+','+repr(str(cli.BIN))+']\n'
            + 'import test_primary_pin_literals as module\n'
            + 'result=unittest.TextTestRunner().run(unittest.TestSuite(['
            + 'module.PrimaryPinLiterals(\"test_wide_memory_goal\")]))\n'
            + 'raise SystemExit(not result.wasSuccessful())\n')
        result = subprocess.run([sys.executable,'-c',check],
            env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1'),
            capture_output=True,text=True,timeout=45)
        self.assertEqual(result.returncode,0,result.stdout+result.stderr)
        self.assertIn('Ran 1 test',result.stderr)
        self.assertNotIn('skipped=',result.stderr)
        self.assertIn('primary pin width=1024 wall_seconds=',result.stdout)

    def test_zero_and_matching_goals(self):
        output = self.check(history.BASE, '''iState p 0
iWrite p 0; iRead q 15; iApply; iNote [iGetReadData q -dec]
iWrite p 9; iRead q 0; iApply; iNote [iGetReadData q -dec]
iWrite p 9; iRead q 0; iApply; iNote [iGetReadData q -dec]
''')
        self.assertEqual(self.notes(output), [b'0', b'9', b'9'])

    def test_ascending_slices_and_inversion(self):
        output = self.check(history.BASE.replace('[3:0]', '[0:3]'), '''
iState p 0; iWrite p[0:1] 2; iRead q 0; iApply
iNote [iGetReadData q -dec]; iNote [iGetReadData inverted -dec]
''')
        self.assertEqual(self.notes(output), [b'8', b'7'])

    def test_unknown_and_partial_x_history(self):
        output = self.check(history.BASE, '''iRead q 0; iApply
iNote [iGetReadData q -bin]
iState p 9; iWrite p 0bx1x0; iRead q 0; iApply
iNote [iGetReadData q -dec]; iNote [iGetReadData inverted -dec]
''')
        self.assertEqual(self.notes(output), [b'0bxxxx', b'12', b'3'])

    def test_unforced_read_enable(self):
        model = history.BASE.replace(' Alias inverted', '''
 DataInPort enable; DataOutPort gated[3:0] {Source p; Enable enable;}
 Alias inverted''')
        output = self.check(model, '''iWrite p 9; iRead gated 0; iRead q 0; iApply
iNote [iGetReadData gated -dec]; iNote [iGetReadData q -dec]
''')
        self.assertEqual(self.notes(output), [b'9', b'9'])
        self.assertIn(b'iWrite enable 0b1', output)

    def test_equally_preferred_read_enables(self):
        model = '''Module Chains {
 DataInPort p;
 DataInPort e0 {DefaultLoadValue 1'b1;}
 DataOutPort z0 {Source e0;}
 DataInPort e1 {DefaultLoadValue 1'b1;}
 DataOutPort z1 {Source e1;}
 LogicSignal choice {e0 | e1;}
 DataOutPort q {Source p; Enable choice;}
}'''
        output = self.check(model, '''iWrite p 1; iRead q 0; iApply
iRead z0 0; iRead z1 0; iApply
iNote [iGetReadData z0 -bin]; iNote [iGetReadData z1 -bin]
if {[iGetReadData z1 -bin] ne "0b1"} {error "read-enable history changed"}
''')
        self.assertEqual(self.notes(output), [b'0bx', b'0b1'])
        self.assertIn(b'iWrite e1 0b1\n', output)
        self.assertNotIn(b'iWrite e0 ', output)

    def test_equally_preferred_routes_keep_logical_read_identity(self):
        model = '''Module Chains {
 DataInPort p;
 DataRegister value[2:0] {
  WriteDataSource 3'b0; WriteEnSource 1'b0; ResetValue 3'b0;
 }
 DataOutPort q0[2:0] {Source value;}
 DataOutPort q1[2:0] {Source value;}
}'''
        output = self.check(model, '''iReset; iWrite p 1; iRead value 0; iApply
iNote [iGetReadData value -bin]
iNote [iGetReadData q0 -bin]; iNote [iGetReadData q1 -bin]
''')
        self.assertEqual(self.notes(output), [b'0b000', b'0bxxx', b'0bxxx'])
        reads = [line for line in output.splitlines() if line.startswith(b'iRead ')]
        self.assertEqual(reads, [b'iRead value 0b000'])

    def test_reservations_and_end_state(self):
        model = history.BASE.replace(' Alias inverted', '''
 DataInPort other[3:0]; DataOutPort held[3:0] {Source other;}
 Alias idle[3:0] = p {iApplyEndState 4'b0;}
 Alias inverted''')
        output = self.check(model, '''iTake other; iState other 6
iWrite p 9; iRead q 0; iRead held 0; iApply
iNote [iGetReadData q -dec]; iNote [iGetReadData held -dec]; iRelease other
''')
        self.assertEqual(self.notes(output), [b'9', b'6'])
        self.assertNotIn(b'iWrite other ', output)
        self.assertTrue(output.endswith(b'iWrite p 0b0000\niApply\n'), output)

    def test_intermediate_pin_value_in_later_horizon(self):
        model = (cli.ROOT/'tests/cases/ours/pattern_generation/data_registers/addressed.icl').read_text()
        model = model.replace('Module AddressedData', 'Module Chains').replace(
            '  AddressPort address', '''  DataInPort marker;
  DataOutPort bus_value[3:0] {Source data;}
  AddressPort address''')
        output = self.check(model, '''iReset; iState marker 0
iWrite a 9; iWrite b 6; iWrite marker 1; iApply
iRead a 0; iRead b 0; iRead bus_value 0; iApply
iNote [iGetReadData a -dec]; iNote [iGetReadData b -dec]
iNote [iGetReadData bus_value -dec]
''')
        values = [line.split()[2] for line in output.splitlines() if line.startswith(b'iWrite data ')]
        self.assertIn(b'0b1001', values)
        self.assertIn(b'0b0110', values)
        self.assertEqual(self.notes(output)[:2], [b'9', b'6'])
        self.assertEqual(self.notes(output)[2], str(int(values[-1][2:], 2)).encode())

        model = '''Module Chains {
 DataInPort p[3:0]; DataInPort enable;
 DataRegister value {WriteDataSource p[0]; WriteEnSource enable; ResetValue 0;}
 DataOutPort q {Source value;}
 DataOutPort pin_value[3:0] {Source p;}
}'''
        output = self.check(model, '''iReset; iState p 10; iState enable 0; iState value 0
iWrite value 1; iWrite p 8; iApply
iRead q 0; iRead pin_value 0; iApply
iNote [iGetReadData q -dec]; iNote [iGetReadData pin_value -dec]
''')
        self.assertEqual(self.notes(output), [b'1', b'8'])
        values = [line.split()[2] for line in output.splitlines() if line.startswith(b'iWrite p ')]
        self.assertEqual(values, [b'0b1011', b'0b1000'])

    def test_impossible_goals_preserve_artifacts(self):
        model = self.root/'source.icl'
        model.write_text('''Module Chains {
 DataInPort p[3:0];
 DataRegister value[3:0] {WriteDataSource p; WriteEnSource 1'b1; ResetValue 0;}
 DataOutPort q[3:0] {Source value;}
}''')
        pdl = self.root/'source.pdl'
        pdl.write_text('iPDLLevel 1; iState p 0; iState value 0; '
                       'iWrite value 9; iWrite p 6; iApply')
        artifacts = [self.root/'result.pdl', self.root/'debug/Chains.icl']
        for path in artifacts:
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(b'retained\n')
        diagnostic = self.run_tool(model, pdl, ok=False)
        self.assertIn('cannot justify', diagnostic)
        for path in artifacts:
            self.assertEqual(path.read_bytes(), b'retained\n')


if __name__ == '__main__':
    unittest.main()
