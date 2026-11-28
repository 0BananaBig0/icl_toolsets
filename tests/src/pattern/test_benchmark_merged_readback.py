#!/usr/bin/env python3
"""The maintained matrix must reject missing replay and wrong control output."""
import json
from pathlib import Path
import subprocess
import unittest

import test_simultaneous_scan_chains as cli


class BenchmarkMergedReadback(unittest.TestCase):
    setUp = cli.SimultaneousChains.setUp

    def measure(self, fault='', script_name='benchmark_merged_readback', size='2', widths=()):
        wrapper = self.root/'driver.py'
        wrapper.write_text('''#!/usr/bin/python3
import subprocess
import sys
from pathlib import Path
args = sys.argv[1:]
fault = %r
if fault == 'missing-flat':
    args.remove('--debug')
code = subprocess.run([%r, *args]).returncode
if code == 0 and fault == 'san-compare':
    output = Path(args[args.index('--output')+1])
    output.write_text(output.read_text().replace('iRead q 0b0000', 'iRead q 0b1111'))
if code == 0 and fault == 'wrong-unobserved':
    source = Path(args[args.index('--pdl')+1]).read_text()
    if (('iMerge -begin' in source and 'set actual' not in source) or
            ('iPDLLevel 0' in source and sum(line.startswith('iNote ') for line in source.splitlines()) == 1)):
        output = Path(args[args.index('--output')+1])
        output.write_text(output.read_text().replace('0b1010', '0b0001'))
sys.exit(code)
''' % (fault, str(cli.BIN)))
        wrapper.chmod(0o755)
        output = self.root/'measurements'
        script = cli.ROOT/('tests/src/pattern/'+script_name+'.py')
        options = ['--sanitized', str(wrapper)] if fault == 'san-compare' else []
        result = subprocess.run(['python3', str(script), '--binary', str(cli.BIN if options else wrapper),
            '--parent', str(cli.BIN), '--output', str(output), '--sizes', size,
            '--wide-widths', *map(str,widths), '--repeats', '1', *options], capture_output=True, text=True, timeout=90)
        rows = json.loads((output/'results.json').read_text())['runs']
        return result, rows

    def test_wide_raw_frames_pin_upper_bits(self):
        from benchmark_raw_merge_calls import fixture
        for width in (160, 512, 1024):
            with self.subTest(width=width):
                _, commands, frames = fixture(1, 'merged', width)
                zero = '0b'+'0'*width
                first = '0b'+'1'+'0'*(width-2)+'1'
                second = '0b'+'0'*(width//2-1)+'1'+'0'*(width//2-2)+'10'
                self.assertEqual(frames, [(first,zero),(second,first)])
                self.assertIn(f'iCall access {zero} {first} {second}\n',commands)
        self.assertEqual(fixture(2,'merged',4)[2], [
            ('0b0001','0b0000'),('0b0010','0b0001'),
            ('0b0100','0b0010'),('0b0111','0b0100')])

    def test_small_raw_widths_preserve_low_nibbles(self):
        from benchmark_raw_merge_calls import fixture
        for width in range(5,17):
            with self.subTest(width=width):
                _, _, frames = fixture(3,'merged',width)
                self.assertEqual([int(si,2)&15 for si,_ in frames],[1,2,4,7,7,12])
                self.assertEqual([int(so,2)&15 for _,so in frames],[0,1,2,4,7,7])
                self.assertTrue(all(len(si)==len(so)==width+2 for si,so in frames))

    def test_wide_raw_matrix_captures_preceding_upper_bits(self):
        result, rows = self.measure(widths=(160,))
        self.assertEqual(result.returncode,0,result.stdout+result.stderr)
        self.assertEqual(len(rows),48)
        wide = [row for row in rows if row['width']==160]
        self.assertEqual(len(wide),24)
        for row in wide:
            with self.subTest(owner=row['owner'],mode=row['mode'],kind=row['kind'],form=row['form']):
                self.assertTrue(row['observations_verified'],row)
                self.assertEqual(row['calls'],2 if row['kind']=='raw' else 1)
                if row['kind']=='raw' and row['mode']!='unobserved':
                    output = (Path(row['directory'])/'result.pdl').read_text()
                    notes = [line[6:] for line in output.splitlines()
                             if line.startswith('iNote ') and 'modeled ' not in line]
                    self.assertEqual(notes,['0b'+'0'*160,'0b'+'10'*80])

    def assert_receipt_counts(self, rows, missing_flat=False):
        for row in rows:
            absent = missing_flat and row['owner']=='current' and row['form']=='flat'
            expected = 0 if row['mode']=='unobserved' else 2
            self.assertEqual(row['literal_notes'],0 if absent else expected,row)
        for row in rows:
            self.assertNotIn('provider_queries',row)
            self.assertIn('expected_provider_queries',row)
            self.assertIn('expected_literal_notes',row)
            self.assertEqual(row['expected_provider_queries'],
                0 if row['form']=='generated' or row['mode']=='unobserved' else 2)
            self.assertEqual(row['expected_literal_notes'],0 if row['mode']=='unobserved' else 2)

    def test_valid_complete_matrix(self):
        result, rows = self.measure()
        self.assertEqual(result.returncode, 0, result.stdout+result.stderr)
        self.assertEqual(len(rows), 24)
        self.assertTrue(all(row['observations_verified'] for row in rows))
        self.assert_receipt_counts(rows)

    def test_missing_flat_model_fails_and_still_runs_generated_forms(self):
        result, rows = self.measure('missing-flat')
        self.assertNotEqual(result.returncode, 0, result.stdout+result.stderr)
        self.assertEqual(len(rows), 24)
        current = [row for row in rows if row['owner'] == 'current']
        self.assertEqual(sum(row['form'] == 'flat' and not row['observations_verified']
                             for row in current), 6)
        self.assertEqual(sum(row['form'] == 'generated' and row['observations_verified']
                             for row in current), 6)
        self.assert_receipt_counts(rows,missing_flat=True)

    def test_corrupt_unobserved_words_fail_against_serial_control(self):
        result, rows = self.measure('wrong-unobserved')
        self.assertNotEqual(result.returncode, 0, result.stdout+result.stderr)
        self.assertEqual(len(rows), 24)
        controls = [row for row in rows if row['mode'] == 'unobserved']
        self.assertEqual(len(controls), 6)
        self.assertTrue(all(not row['observations_verified'] for row in controls))

    def test_sanitizer_compares_against_parent_with_default_and_custom_sizes(self):
        for script in ('benchmark_merged_readback', 'benchmark_read_callback_histories'):
            for size in ('16', '2'):
                with self.subTest(script=script, size=size):
                    result, rows = self.measure('san-compare', script, size)
                    self.assertNotEqual(result.returncode, 0, result.stdout+result.stderr)
                    wrong = [r for r in rows if r['owner']=='sanitized' and r['kind']=='pio']
                    self.assertEqual(len(wrong), 3)
                    self.assertTrue(all(not r['observations_verified'] for r in wrong))


if __name__ == '__main__':
    unittest.main()
