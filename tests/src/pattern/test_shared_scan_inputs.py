#!/usr/bin/env python3
"""One physical serial wire cannot independently load separate scan banks."""
import json
import time
import unittest

import test_simultaneous_client_interfaces as helpers

NETWORK = '''Module Chains {
 ScanInPort si;
 ScanRegister a[3:0] {ScanInSource si; CaptureSource a; ResetValue 0;}
 ScanRegister b[3:0] {ScanInSource si; CaptureSource b; ResetValue 0;}
 ScanOutPort oa {Source a[0];}
 ScanOutPort ob {Source b[0];}
}'''


class SharedScanInputs(unittest.TestCase):
    setUp = helpers.SimultaneousClients.setUp
    run_tool = helpers.SimultaneousClients.run_tool
    replay = helpers.SimultaneousClients.replay
    assert_prefix_notes = helpers.SimultaneousClients.assert_prefix_notes
    notes = staticmethod(helpers.SimultaneousClients.notes)

    def test_actual_captures_stay_separate(self):
        source = NETWORK.replace('CaptureSource a;', "CaptureSource 4'b0101;")
        source = source.replace('CaptureSource b;', "CaptureSource 4'b1010;")
        self.assert_prefix_notes(source, 'iReset; iWrite a 9; iWrite b 9; iApply; '
                                 'iRead a 0; iRead b 0; iApply;\n',
                                 'iNote [iGetReadData a -bin]; iNote [iGetReadData b -bin]; '
                                 'iNote [iGetStatus -clear]; iNote [iGetStatus];\n',
                                 [b'0b0101', b'0b1010', b'4', b'0'])

    def test_hierarchical_shared_input_forwarding(self):
        source = NETWORK.replace('Module Chains', 'Module Cell') + '''
Module Chains { ScanInPort si;
 Instance c Of Cell {InputPort si = si;}
 ScanOutPort oa {Source c.oa;} ScanOutPort ob {Source c.ob;}
}'''
        self.assert_prefix_notes(source, 'iReset; iWrite c.a 9; iWrite c.b 9; iApply; '
                                 'iRead c.a 0bx; iRead c.b 0bx; iApply;\n',
                                 'iNote [iGetReadData c.a -bin]; iNote [iGetReadData c.b -bin];\n',
                                 [b'0b1001', b'0b1001'])

    def test_input_and_output_polarities(self):
        for input_inverted in (False, True):
            for output_inverted in (False, True):
                with self.subTest(input=input_inverted, output=output_inverted):
                    source = NETWORK.replace('ScanInSource si; CaptureSource b;',
                                             'ScanInSource ' + ('~' if input_inverted else '') +
                                             'si; CaptureSource b;')
                    source = source.replace('Source b[0];',
                                            'Source ' + ('~' if output_inverted else '') + 'b[0];')
                    value = 6 if input_inverted else 9
                    output = self.replay(source, 'iPDLLevel 1\niProcsForModule Chains\n'
                                         f'iReset; iWrite a 9; iWrite b {value}; iApply -together; '
                                         f'iRead a 9; iRead b {value}; iApply;\n')
                    self.assertIn(b'iWrite a 0b1001', output)
                    self.assertIn(f'iWrite b 0b{value:04b}'.encode(), output)

    def test_different_register_boundaries_and_ascending_indices(self):
        source = NETWORK.replace('ScanRegister b[3:0] {ScanInSource si; CaptureSource b; ResetValue 0;}',
                                 'ScanRegister hi[0:1] {ScanInSource si; CaptureSource hi; ResetValue 0;} '
                                 'ScanRegister lo[1:0] {ScanInSource hi[1]; CaptureSource lo; ResetValue 0;}')
        source = source.replace('Source b[0];', 'Source lo[0];')
        self.assert_prefix_notes(source, 'iReset; iWrite a 13; iWrite hi 3; iWrite lo 1; '
                                 'iApply -together; iRead a 0bx; iRead hi 0bx; iRead lo 0bx; iApply;\n',
                                 'iNote [iGetReadData a -bin]; iNote [iGetReadData hi -bin]; '
                                 'iNote [iGetReadData lo -bin]; iNote [iGetStatus];\n',
                                 [b'0b1101', b'0b11', b'0b01', b'0'])

    def test_partial_read_scope_and_previous_history(self):
        source = NETWORK.replace('CaptureSource a;', "CaptureSource 4'b0101;")
        source = source.replace('CaptureSource b;', "CaptureSource 4'b1010;")
        self.assert_prefix_notes(source, 'iReset; iWrite a 9; iWrite b 9; iApply; '
                                 'iRead a[3] 0; iRead b[3] 0; iApply;\n',
                                 'iNote [iGetReadData a -bin]; iNote [iGetReadData b -bin]; '
                                 'iNote [iGetStatus -clear]; iNote [iGetStatus];\n',
                                 [b'0b0xxx', b'0b1xxx', b'1', b'0'])

    def test_partial_writes_preserve_other_bits(self):
        self.assert_prefix_notes(NETWORK, 'iReset; iWrite a 9; iWrite b 9; iApply; '
                                 'iWrite a[2:1] 3; iWrite b[2:1] 3; iApply; '
                                 'iRead a 0bx; iRead b 0bx; iApply;\n',
                                 'iNote [iGetReadData a -bin]; iNote [iGetReadData b -bin];\n',
                                 [b'0b1111', b'0b1111'])

    def test_separate_vector_input_groups(self):
        source = NETWORK.replace('ScanInPort si;', 'ScanInPort si[1:0];')
        source = source.replace('ScanInSource si;', 'ScanInSource si[1];')
        source = source.replace('ScanOutPort oa', '''
 ScanRegister c[3:0] {ScanInSource si[0]; CaptureSource c; ResetValue 0;}
 ScanRegister d[3:0] {ScanInSource si[0]; CaptureSource d; ResetValue 0;}
 ScanOutPort oc {Source c[0];} ScanOutPort od {Source d[0];}
 ScanOutPort oa''')
        self.assert_prefix_notes(source, 'iReset; iWrite a 9; iWrite b 9; '
                                 'iWrite c 6; iWrite d 6; iApply -together; '
                                 'iRead a 0bx; iRead b 0bx; iRead c 0bx; iRead d 0bx; iApply;\n',
                                 'iNote [iGetReadData a -bin]; iNote [iGetReadData b -bin]; '
                                 'iNote [iGetReadData c -bin]; iNote [iGetReadData d -bin];\n',
                                 [b'0b1001', b'0b1001', b'0b0110', b'0b0110'])

    def test_primary_only_frame_does_not_require_scan(self):
        source = NETWORK.replace('ScanInPort si;',
                                 'ScanInPort si; DataInPort p; DataOutPort q {Source p;}')
        source = source.replace('CaptureSource b; ResetValue 0;', 'CaptureSource b; ResetValue 15;')
        self.assert_prefix_notes(source, 'iReset; iState p 1; iRead q 1; iApply;\n',
                                 'iNote [iGetReadData q -bin]; iNote [iGetStatus];\n',
                                 [b'0b1', b'0'])

    def test_omitted_selector_bank_keeps_its_load(self):
        source = NETWORK.replace(' ScanOutPort oa', '''
 DataMux unused SelectedBy b[0] {1'b0:1'b0;1'b1:1'b1;}
 DataOutPort observer {Source unused;}
 ScanOutPort oa''')
        for setup in ('', 'iWrite a 8; iWrite b 8; iApply; '):
            for read in ('', 'iRead observer 0bx; '):
                for modeled in (False, True):
                    with self.subTest(setup=setup, read=read, modeled=modeled):
                        self.reject(source, setup + 'iWrite a[0] 1; ' + read + 'iApply;',
                                    'no schedule within', modeled)
        self.assert_prefix_notes(source, 'iReset; iWrite a 9; iWrite b 9; iApply; '
                                 'iRead a 0bx; iRead b 0bx; iApply;\n',
                                 'iNote [iGetReadData a -bin]; iNote [iGetReadData b -bin];\n',
                                 [b'0b1001', b'0b1001'])

    def test_disabled_data_consumer_does_not_authorize_bank_changes(self):
        source = NETWORK.replace(' ScanOutPort oa', '''
 DataRegister unused[3:0] {WriteDataSource b; WriteEnSource 1'b0; ResetValue 0;}
 DataOutPort q {Source 1'b0;}
 ScanOutPort oa''')
        for setup in ('', 'iWrite a 8; iWrite b 8; iApply; '):
            for modeled in (False, True):
                with self.subTest(setup=setup, modeled=modeled):
                    self.reject(source, setup + 'iWrite a 9; iRead q 0bx; iApply;',
                                'no schedule within', modeled)

    def test_requested_data_controller_is_still_programmable(self):
        for width in (4, 256):
            with self.subTest(width=width):
                source = NETWORK.replace(' ScanOutPort oa', '''
 DataRegister sink[3:0] {WriteDataSource b; WriteEnSource 1'b1; ResetValue 0;}
 DataOutPort q[3:0] {Source sink;}
 ScanOutPort oa''').replace('[3:0]', f'[{width-1}:0]')
                value = ('0b' + '0' * (width-4) + '1001').encode()
                self.assertTrue(self.assert_prefix_notes(
                    source, 'iReset; iWrite a 9; iWrite sink 9; iApply; '
                    'iRead a 0bx; iRead b 0bx; iRead q 0bx; iApply;\n',
                    'iNote [iGetReadData a -bin]; iNote [iGetReadData b -bin]; '
                    'iNote [iGetReadData q -bin]; iNote [iGetStatus];\n',
                    [value, value, value, b'0']))
                self.assertLess(max(self.prefix_producer_seconds), 10)
                print(json.dumps(dict(controller_width=width,
                                      producer_seconds=self.prefix_producer_seconds)))

    def test_requested_controller_does_not_free_unrelated_bits(self):
        source = NETWORK.replace(' ScanOutPort oa', '''
 DataRegister sink {WriteDataSource b[0]; WriteEnSource 1'b1; ResetValue 0;}
 DataOutPort q {Source sink;}
 DataMux unused SelectedBy b[1] {1'b0:1'b0;1'b1:1'b1;}
 DataOutPort observer {Source unused;}
 ScanOutPort oa''')
        for modeled in (False, True):
            with self.subTest(modeled=modeled):
                self.reject(source, 'iWrite a 3; iWrite sink 1; iApply;',
                            'no schedule within', modeled)
        self.assert_prefix_notes(source, 'iReset; iWrite a 1; iWrite sink 1; iApply; '
                                 'iRead a 0bx; iRead b 0bx; iRead q 0bx; iApply;\n',
                                 'iNote [iGetReadData a -bin]; iNote [iGetReadData b -bin]; '
                                 'iNote [iGetReadData q -bin]; iNote [iGetStatus];\n',
                                 [b'0b0001', b'0b0001', b'0b1', b'0'])

    def reject(self, source, body, diagnostic=None, modeled=True):
        icl, pdl = self.root / 'source.icl', self.root / 'source.pdl'
        icl.write_text(source)
        pdl.write_text('iPDLLevel 1\niProcsForModule Chains\niReset; ' + body)
        (self.root / 'result.pdl').write_bytes(b'previous output\n')
        error = self.run_tool(icl, pdl, ok=False, extra=('--model_readback',) if modeled else ())
        if diagnostic:
            self.assertIn(diagnostic, error)
        self.assertEqual((self.root / 'result.pdl').read_bytes(), b'previous output\n')

    def test_conflicting_or_omitted_writes_preserve_output(self):
        for body in ('iWrite a 9; iWrite b 6; iApply -together;',
                     'iWrite a 9; iApply;',
                     'iWrite a 9; iWrite b 9; iApply; iWrite a 6; iApply;'):
            with self.subTest(body=body):
                self.reject(NETWORK, body)
        output = self.replay(NETWORK, 'iProcsForModule Chains\niReset; '
                             'iWrite a 0; iApply; iRead b 0; iApply;\n')
        self.assertIn(b'iRead b 0b0000', output)

    def test_unsupported_shared_routes_remain_guarded(self):
        for source in (
                NETWORK.replace('Source b[0];', 'Source b[0]; Enable 1\'b1;'),
                NETWORK.replace('ScanInSource si; CaptureSource b;', 'ScanInSource a[1]; CaptureSource b;')):
            with self.subTest(source=source):
                self.reject(source, 'iWrite a 9; iWrite b 9; iApply;',
                            'rightmost bit' if 'ScanInSource a[1]' in source else 'overlapping-event')

    def test_fixed_lane_scale(self):
        for count in (2, 8, 32):
            for width in (4, 64, 256):
                with self.subTest(count=count, width=width):
                    source = 'Module Chains {ScanInPort si;\n'
                    for i in range(count):
                        source += (f'ScanRegister r{i}[{width-1}:0] {{ScanInSource si; '
                                   f'CaptureSource r{i}; ResetValue 0;}} '
                                   f'ScanOutPort o{i} {{Source r{i}[0];}}\n')
                    source += '}\n'
                    body = ('iProcsForModule Chains\niReset; ' +
                            ''.join(f'iWrite r{i} 9; ' for i in range(count)) + 'iApply -together; ' +
                            ''.join(f'iRead r{i} 9; ' for i in range(count)) + 'iApply;\n')
                    start = time.monotonic()
                    output = self.replay(source, body)
                    elapsed = time.monotonic() - start
                    self.assertEqual(output.count(b'iApply'), 2)
                    for i in range(count):
                        self.assertIn(f'iWrite r{i} 0b{9:0{width}b}'.encode(), output)
                        self.assertIn(f'iRead r{i} 0b{9:0{width}b}'.encode(), output)
                    self.assertLess(elapsed, 10)
                    print(json.dumps(dict(lanes=count, width=width, forms=4, seconds=elapsed)))


if __name__ == '__main__':
    unittest.main()
