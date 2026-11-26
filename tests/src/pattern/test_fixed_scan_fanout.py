#!/usr/bin/env python3
"""Fixed fanout must replay one physical stream and unique register events."""
import json
import time
import unittest

import test_shared_scan_inputs as helpers

SHARED = helpers.NETWORK.replace('ScanInSource si; CaptureSource b;',
                                 'ScanInSource a[0]; CaptureSource b;')
UNEQUAL = helpers.NETWORK.replace('b[3:0]', 'b[1:0]')
FORK = SHARED.replace('b[3:0]', 'b[1:0]').replace(' ScanOutPort oa', '''
 ScanRegister c[3:0] {ScanInSource a[0]; CaptureSource c; ResetValue 0;}
 ScanOutPort oc {Source c[0];}
 ScanOutPort oa''')
RESET_SHARED = '''Module Cell {
 ScanInPort si; ResetPort rst;
 ScanRegister b[3:0] {ScanInSource si; CaptureSource b; ResetValue 0;}
 ScanOutPort so {Source b[0];}
}
Module Chains {
 ScanInPort si; ResetPort global;
 ScanRegister a[3:0] {ScanInSource si; CaptureSource a; ResetValue 0;}
 DataMux reset_gate SelectedBy a[0] {1'b0:global;1'b1:1'b1;}
 Instance cell Of Cell {InputPort si=a[0]; InputPort rst=1'b0;}
 ScanOutPort oa {Source a[0];} ScanOutPort ob {Source cell.so;}
}'''


class FixedScanFanout(unittest.TestCase):
    setUp = helpers.SharedScanInputs.setUp
    run_tool = helpers.SharedScanInputs.run_tool
    replay = helpers.SharedScanInputs.replay
    assert_prefix_notes = helpers.SharedScanInputs.assert_prefix_notes
    notes = staticmethod(helpers.SharedScanInputs.notes)
    reject = helpers.SharedScanInputs.reject

    def test_shared_storage_captures_and_once_only_status(self):
        self.assert_prefix_notes(SHARED,
                                 'iReset; iWrite a 13; iWrite b 5; iApply; '
                                 'iRead a 0; iRead b 0; iApply;\n',
                                 'iNote [iGetReadData a -bin]; iNote [iGetReadData b -bin]; '
                                 'iNote [iGetStatus -clear]; iNote [iGetStatus];\n',
                                 [b'0b1101', b'0b0101', b'5', b'0'])

    def test_unequal_lanes_use_most_significant_prefix(self):
        self.assert_prefix_notes(UNEQUAL,
                                 'iReset; iWrite a 13; iWrite b 3; iApply -together; '
                                 'iRead a 0; iRead b 0; iApply;\n',
                                 'iNote [iGetReadData a -bin]; iNote [iGetReadData b -bin]; '
                                 'iNote [iGetStatus -clear]; iNote [iGetStatus];\n',
                                 [b'0b1101', b'0b11', b'5', b'0'])

    def test_forked_storage_keeps_independent_capture_words(self):
        self.assert_prefix_notes(FORK,
                                 'iReset; iWrite a 13; iWrite b 2; iWrite c 10; iApply; '
                                 'iRead a 0; iRead b 0; iRead c 0; iApply;\n',
                                 'iNote [iGetReadData a -bin]; iNote [iGetReadData b -bin]; '
                                 'iNote [iGetReadData c -bin]; iNote [iGetStatus -clear]; '
                                 'iNote [iGetStatus];\n',
                                 [b'0b1101', b'0b10', b'0b1010', b'6', b'0'])

    def test_wire_observer_does_not_replace_capture_with_incoming_load(self):
        source = helpers.NETWORK.replace('ScanOutPort ob {Source b[0];}',
                                         'ScanOutPort wire {Source si;}')
        source = source.replace('CaptureSource a;', "CaptureSource 4'b0101;")
        self.assert_prefix_notes(source, 'iReset; iWrite a 13; iApply; iRead a 0; iApply;\n',
                                 'iNote [iGetReadData a -bin]; iNote [iGetStatus -clear]; '
                                 'iNote [iGetStatus];\n', [b'0b0101', b'2', b'0'])

    def test_inversions_and_ascending_unequal_lanes(self):
        source = UNEQUAL.replace('b[1:0]', 'b[0:1]').replace('Source b[0];', 'Source ~b[1];')
        source = source.replace('ScanInSource si; CaptureSource b;',
                                'ScanInSource ~si; CaptureSource b;')
        self.assert_prefix_notes(source,
                                 'iReset; iWrite a 13; iWrite b 0; iApply; '
                                 'iRead a 0bx; iRead b 0bx; iApply;\n',
                                 'iNote [iGetReadData a -bin]; iNote [iGetReadData b -bin];\n',
                                 [b'0b1101', b'0b00'])

    def test_hierarchical_fork_and_partial_read_history(self):
        source = FORK.replace('Module Chains', 'Module Cell') + '''
Module Chains { ScanInPort si;
 Instance cell Of Cell {InputPort si=si;}
 ScanOutPort a {Source cell.oa;} ScanOutPort b {Source cell.ob;}
 ScanOutPort c {Source cell.oc;}
}'''
        self.assert_prefix_notes(source,
                                 'iReset; iWrite cell.a 13; iWrite cell.b 2; iWrite cell.c 10; '
                                 'iApply; iRead cell.a[3] 0; iRead cell.c[1] 0; iApply;\n',
                                 'iNote [iGetReadData cell.a -bin]; '
                                 'iNote [iGetReadData cell.b -bin]; '
                                 'iNote [iGetReadData cell.c -bin]; '
                                 'iNote [iGetStatus -clear]; iNote [iGetStatus];\n',
                                 [b'0b1xxx', b'0bxx', b'0bxx1x', b'2', b'0'])

    def test_conflicting_and_omitted_loads_preserve_output(self):
        for source, body in (
                (UNEQUAL, 'iWrite a 13; iWrite b 1; iApply;'),
                (UNEQUAL, 'iWrite a 13; iApply;'),
                (FORK, 'iWrite a 13; iWrite b 1; iWrite c 10; iApply;'),
                (FORK, 'iWrite a 13; iWrite b 2; iWrite c 10; iApply; '
                       'iWrite c 6; iApply;')):
            for modeled in (False, True):
                with self.subTest(body=body, modeled=modeled):
                    self.reject(source, body, 'no schedule within', modeled)
        source = UNEQUAL.replace(' ScanOutPort oa', '''
 DataMux unused SelectedBy b[0] {1'b0:1'b0;1'b1:1'b1;}
 DataOutPort observer {Source unused;}
 ScanOutPort oa''')
        self.reject(source, 'iWrite a 13; iApply;', 'no schedule within')

    def test_inactive_root_reset_polarities(self):
        for active in (0, 1):
            source = SHARED.replace('ScanInPort si;',
                                     f'ScanInPort si; ResetPort rst {{ActivePolarity {active};}}')
            self.assert_prefix_notes(source,
                                     'iReset; iWrite a 13; iWrite b 5; iApply; '
                                     'iRead a 0bx; iRead b 0bx; iApply;\n',
                                     'iNote [iGetReadData a -bin]; iNote [iGetReadData b -bin];\n',
                                     [b'0b1101', b'0b0101'])

    def test_retained_intermediate_and_gated_boundaries(self):
        for source, diagnostic in (
                (SHARED.replace('ScanInSource a[0]', 'ScanInSource a[1]'), 'rightmost bit'),
                (SHARED.replace('Source b[0];', "Source b[0]; Enable 1'b1;"),
                 'overlapping-event')):
            self.reject(source, 'iWrite a 13; iWrite b 5; iApply;', diagnostic)

    def test_primary_only_and_partial_writes(self):
        source = UNEQUAL.replace('ScanInPort si;',
                                 'ScanInPort si; DataInPort p; DataOutPort q {Source p;}')
        source = source.replace('CaptureSource b; ResetValue 0;',
                                'CaptureSource b; ResetValue 3;')
        self.assert_prefix_notes(source, 'iReset; iState p 1; iRead q 1; iApply;\n',
                                 'iNote [iGetReadData q -bin]; iNote [iGetStatus];\n',
                                 [b'0b1', b'0'])
        self.assert_prefix_notes(UNEQUAL,
                                 'iReset; iWrite a 13; iWrite b 3; iApply; '
                                 'iWrite a[0] 0; iApply; iRead a 0bx; iRead b 0bx; iApply;\n',
                                 'iNote [iGetReadData a -bin]; iNote [iGetReadData b -bin];\n',
                                 [b'0b1100', b'0b11'])

    def test_dynamic_reset_requires_separate_scheduling(self):
        self.assert_prefix_notes(RESET_SHARED,
                                 'iReset; iWrite a 13; iWrite cell.b 5; iApply; '
                                 'iRead a 0bx; iRead cell.b 0bx; iApply;\n',
                                 'iNote [iGetReadData a -bin]; iNote [iGetReadData cell.b -bin];\n',
                                 [b'0b1101', b'0b0101'])
        self.reject(RESET_SHARED.replace("InputPort rst=1'b0", 'InputPort rst=reset_gate'),
                    'iWrite a 13; iWrite cell.b 5; iApply;',
                    'invariant inactive resets')

    def test_active_and_unknown_resets_preserve_output(self):
        for level in ("1'b1", "1'bx"):
            with self.subTest(level=level):
                source = RESET_SHARED.replace("InputPort rst=1'b0", 'InputPort rst=' + level)
                self.reject(source, 'iWrite a 13; iWrite cell.b 5; iApply;',
                            'no schedule within')

    def test_fixed_fanout_scale(self):
        for count in (2, 8, 32):
            for width in (4, 64, 256):
                with self.subTest(count=count, width=width):
                    tail = 9 if width == 4 else (9 << (width // 2)) | 9
                    source = ('Module Chains {ScanInPort si; '
                              f'ScanRegister a[{width-1}:0] {{ScanInSource si; '
                              'CaptureSource a; ResetValue 0;} ScanOutPort oa {Source a[0];}')
                    body = 'iProcsForModule Chains\niReset; iWrite a 13; '
                    reads = 'iRead a 0bx; '
                    for i in range(count-1):
                        w = width if i % 2 else width // 2
                        value = tail if w == width else tail >> (width - w)
                        source += (f'ScanRegister r{i}[{w-1}:0] {{ScanInSource a[0]; '
                                   f'CaptureSource r{i}; ResetValue 0;}} '
                                   f'ScanOutPort o{i} {{Source r{i}[0];}}')
                        body += f'iWrite r{i} {value}; '
                        reads += f'iRead r{i} 0bx; '
                    source += '}\n'
                    start = time.monotonic()
                    output = self.replay(source, body + 'iApply; ' + reads + 'iApply;\n')
                    elapsed = time.monotonic() - start
                    self.assertIn(f'iWrite a 0b{13:0{width}b}'.encode(), output)
                    self.assertLess(elapsed, 10)
                    print(json.dumps(dict(fanout=count, width=width, forms=4, seconds=elapsed)))


if __name__ == '__main__':
    unittest.main()
