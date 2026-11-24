#!/usr/bin/env python3
"""The maintained matrix must reject missing replay and wrong control output."""
import json
import subprocess
import unittest

import test_simultaneous_scan_chains as cli


class BenchmarkMergedReadback(unittest.TestCase):
    setUp = cli.SimultaneousChains.setUp

    def measure(self, fault='', script_name='benchmark_merged_readback', size='2'):
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
            '--wide-widths', '--repeats', '1', *options], capture_output=True, text=True, timeout=90)
        rows = json.loads((output/'results.json').read_text())['runs']
        return result, rows

    def test_valid_complete_matrix(self):
        result, rows = self.measure()
        self.assertEqual(result.returncode, 0, result.stdout+result.stderr)
        self.assertEqual(len(rows), 24)
        self.assertTrue(all(row['observations_verified'] for row in rows))

    def test_missing_flat_model_fails_and_still_runs_generated_forms(self):
        result, rows = self.measure('missing-flat')
        self.assertNotEqual(result.returncode, 0, result.stdout+result.stderr)
        self.assertEqual(len(rows), 24)
        current = [row for row in rows if row['owner'] == 'current']
        self.assertEqual(sum(row['form'] == 'flat' and not row['observations_verified']
                             for row in current), 6)
        self.assertEqual(sum(row['form'] == 'generated' and row['observations_verified']
                             for row in current), 6)

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
