#!/usr/bin/env python3
"""Opaque merge measurements compare true serial calls without normalization."""
from pathlib import Path
import unittest
from unittest.mock import patch

import test_benchmark_dynamic_opaque_routes as shared

SCRIPT = Path(__file__).resolve().parents[4]/'tests/src/pattern/benchmark_opaque_serial_merges.py'


class OpaqueSerialMergeBenchmark(unittest.TestCase):
    test_generated_rewrite_checks_actual_unknown_bits_and_each_clear = shared.DynamicOpaqueBenchmark.test_generated_rewrite_checks_actual_unknown_bits_and_each_clear
    test_missing_sanitizer_cannot_produce_measurements = shared.DynamicOpaqueBenchmark.test_missing_sanitizer_cannot_produce_measurements
    test_zero_exit_without_products_fails_complete_matrix = shared.DynamicOpaqueBenchmark.test_zero_exit_without_products_fails_complete_matrix

    def setUp(self):
        selection = patch.object(shared,'SCRIPT',SCRIPT)
        selection.start()
        self.addCleanup(selection.stop)

    def test_sequential_parent_and_merge_source_request_identical_native_words(self):
        benchmark = shared.load()
        parent,expected,words,compares = benchmark.commands(2,8,parent=True)
        current,actual,current_words,current_compares = benchmark.commands(2,8)
        self.assertEqual((expected,words,compares),(actual,current_words,current_compares))
        self.assertNotIn('iMerge ',parent)
        self.assertEqual(current.count('iMerge -begin'),4)
        self.assertEqual(current.count('iMerge -end'),4)
        self.assertEqual(parent.count('iCall load '),8)
        self.assertEqual(current.count('iCall load '),8)
        self.assertNotIn('modeled ',parent+current)
        self.assertEqual([len(word)-2 for word in words],[10]+[26]*8)
        self.assertEqual(compares,['0b'+'x'*10]+['0b'+'x'*26]*8)
        self.assertEqual(words[1],'0b10110100101100000000110011')
        self.assertEqual(words[2],'0b10110100101100111100110011')
        self.assertEqual(expected[:8],['0bxxxxxxxx','0b00000000']*2+
                         ['0b0000000000','0b0000000000','0','0'])
        self.assertNotIn('ScanRegister value',benchmark.model(2,8))
        wide = benchmark.commands(32,8)[0]
        calls = [line.split()[2] for line in wide.splitlines() if line.startswith('iCall load ')]
        self.assertEqual(calls,sorted('child%d.local'%i for i in range(32))*4)


if __name__=='__main__':
    unittest.main()
