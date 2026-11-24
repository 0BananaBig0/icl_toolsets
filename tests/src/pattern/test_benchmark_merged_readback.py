#!/usr/bin/env python3
"""The maintained matrix must reject missing replay and wrong control output."""
import json
import subprocess
import unittest

import test_simultaneous_scan_chains as cli


class BenchmarkMergedReadback(unittest.TestCase):
    setUp = cli.SimultaneousChains.setUp

    def measure(self, fault=''):
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
        script = cli.ROOT/'tests/src/pattern/benchmark_merged_readback.py'
        result = subprocess.run(['python3', str(script), '--binary', str(wrapper),
            '--parent', str(cli.BIN), '--output', str(output), '--sizes', '2',
            '--wide-widths', '--repeats', '1'], capture_output=True, text=True, timeout=90)
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


if __name__ == '__main__':
    unittest.main()
