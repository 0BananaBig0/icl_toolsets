#!/usr/bin/env python3
"""Automatic preparation measurements retain native events and observations."""
from pathlib import Path
import unittest
from unittest.mock import patch

import test_benchmark_dynamic_opaque_routes as shared

SCRIPT = Path(__file__).resolve().parents[4]/'tests/src/pattern/benchmark_opaque_child_preparation.py'


class OpaquePreparationBenchmark(shared.DynamicOpaqueBenchmark):
    def setUp(self):
        selection = patch.object(shared,'SCRIPT',SCRIPT)
        selection.start()
        self.addCleanup(selection.stop)

    def test_parent_is_an_unknown_capture_twin_without_injected_observations(self):
        benchmark = shared.load()
        parent,expected,words,compares = benchmark.commands(2,8,parent=True)
        self.assertEqual((parent,expected,words,compares),benchmark.commands(2,8))
        self.assertNotIn('modeled ',parent)
        self.assertNotIn('-stable',parent)
        self.assertNotIn('iWrite enable',parent)
        self.assertEqual(parent.count('iApply'),9)
        self.assertEqual([len(word)-2 for word in words],[26]*8)
        self.assertEqual(compares,['0b'+'x'*26]*8)
        self.assertEqual(expected[:8],['0bxxxxxxxx','0b00000000']*2+
            ['0b101xxxxxxxx1xxxxxxxx110011','0b'+'0'*26,'0','0'])
        self.assertEqual(words[0],'0b10110100101100111100110011')
        self.assertIn("CaptureSource 8'bx; ResetValue 8'bx;",benchmark.model(2,8,visible=True))
        self.assertNotIn('ScanRegister value',benchmark.model(2,8))


if __name__=='__main__':
    unittest.main()
