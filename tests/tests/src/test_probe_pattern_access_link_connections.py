#!/usr/bin/env python3
"""Checks safe, reproducible staging of the manual Tessent connection probes."""
import importlib.util
from pathlib import Path
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[3]
SPEC = importlib.util.spec_from_file_location(
    'connection_probe', ROOT / 'tests/src/probe_pattern_access_link_connections.py')
PROBE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(PROBE)


class ConnectionProbe(unittest.TestCase):
    def test_stages_complete_pairs_without_overwriting(self):
        with tempfile.TemporaryDirectory() as temporary:
            destination = Path(temporary) / 'probe inputs'
            PROBE.stage(destination)
            cases = sorted(destination.iterdir())
            self.assertEqual(len(cases), 8)
            for case in cases:
                self.assertEqual({p.name for p in case.iterdir()},
                                 {'simple.icl', 'simple.bsdl', 'run.tcl'})
                self.assertEqual((case / 'simple.bsdl').read_bytes(),
                                 (PROBE.FIXTURE / 'simple.bsdl').read_bytes())
                self.assertIn('set_current_design AccessTop',
                              (case / 'run.tcl').read_text())
            baseline = destination / 'baseline/simple.icl'
            baseline.write_text('preserve existing evidence')
            with self.assertRaises(FileExistsError):
                PROBE.stage(destination)
            self.assertEqual(baseline.read_text(), 'preserve existing evidence')

    def test_nonunique_fixture_anchor_fails(self):
        for text in ('absent', 'same same'):
            with self.assertRaises(ValueError):
                PROBE.replace(text, 'same', 'replacement')


if __name__ == '__main__':
    unittest.main()
