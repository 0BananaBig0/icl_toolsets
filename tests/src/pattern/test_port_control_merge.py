#!/usr/bin/env python3
"""Original/flat/generated replay for self-constructed Tessent-qualified cases."""
import json
import pathlib
import subprocess
import sys
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[3]
BIN = pathlib.Path(sys.argv.pop(1)).resolve() if len(sys.argv) > 1 else ROOT / 'build/icl-pattern-generation'
FIXTURES = ROOT / 'tests/cases/ours/pattern_generation/port_control_merge'


class PortControlMerge(unittest.TestCase):
    def test_source_flat_and_generated_fixed_points(self):
        for case in json.loads((FIXTURES / 'cases.json').read_text())['cases']:
            with self.subTest(case=case['id']), tempfile.TemporaryDirectory() as directory:
                root = pathlib.Path(directory)

                def run(icl, pdl, name):
                    output = root / name / 'result.pdl'
                    args = [str(BIN), '--icl', str(icl), '--pdl', str(pdl),
                            '--top_module', case['top'], '--output', str(output), '--debug']
                    # Original source needs BSDL; the flattened ICL retains its model.
                    if icl == ROOT / case['icl'][0]:
                        args += ['--bsdl', case['bsdl_entity'], str(ROOT / case['bsdl'])]
                    result = subprocess.run(args, capture_output=True, text=True, timeout=45)
                    self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                    return output

                original = run(ROOT / case['icl'][0], ROOT / case['pdl'][0], 'original')
                flat = original.parent / 'debug' / (case['top'] + '.icl')
                flattened = run(flat, ROOT / case['pdl'][0], 'flattened')
                self.assertEqual(original.read_bytes(), flattened.read_bytes())
                self.assertEqual(flat.read_bytes(),
                                 (flattened.parent / 'debug' / flat.name).read_bytes())
                replayed = run(ROOT / case['icl'][0], original, 'replayed')
                self.assertEqual(original.read_bytes(), replayed.read_bytes())


if __name__ == '__main__':
    unittest.main()
