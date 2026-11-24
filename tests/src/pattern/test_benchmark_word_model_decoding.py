#!/usr/bin/env python3
"""A missed timing bound must retain all matched measurements and still fail."""
import contextlib
import io
import json
import sys
import unittest
from unittest.mock import patch

import test_simultaneous_scan_chains as cli
import benchmark_word_model_decoding as benchmark


class BenchmarkWordModelDecoding(unittest.TestCase):
    setUp = cli.SimultaneousChains.setUp

    def test_timing_failures_complete_the_matrix_and_return_failure(self):
        output = self.root/'measurements'
        arguments = ['benchmark', '--binary', str(cli.BIN), '--parent', str(cli.BIN),
                     '--output', str(output), '--sizes', '1', '2', '--repeats', '2',
                     '--wide-widths', '--source-only']
        # Run the real generator; control only the reported elapsed clock.
        with patch.object(sys, 'argv', arguments), \
                patch.object(benchmark.time, 'monotonic', side_effect=[0, 11]*8), \
                contextlib.redirect_stdout(io.StringIO()):
            with self.assertRaises(RuntimeError):
                benchmark.main()
        rows = json.loads((output/'results.json').read_text())['runs']
        self.assertEqual(len(rows), 8)
        self.assertEqual({(x['owner'], x['targets'], x['repeat']) for x in rows},
                         {(owner, size, repeat) for owner in ('parent', 'current')
                          for size in (1, 2) for repeat in range(2)})
        self.assertTrue(all(not x['within_bound'] for x in rows))
        self.assertTrue(all(x['full_parent_output_verified'] for x in rows
                            if x['owner'] == 'current'))


if __name__ == '__main__':
    unittest.main()
