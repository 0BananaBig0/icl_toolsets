#!/usr/bin/env python3
"""Explicit ordinary read bits retain their logical history after generation."""
import unittest
from pathlib import Path
import hashlib
import json
import time

import test_multiple_client_interfaces as clients
import test_simultaneous_client_interfaces as helpers

SCAN = '''Module Chains {
 ScanInPort si;
 ScanRegister r[3:0] {ScanInSource si; CaptureSource r; ResetValue 0;}
 ScanOutPort so {Source r[0];}
}'''
PRIMARY = 'Module Chains {DataInPort p[3:0]; DataOutPort q[3:0] {Source p;}}'


def cases():
    return [('client', clients.network(1), 'c0.value', 'iWrite c0.value {value}; iApply; '),
            ('scan', SCAN, 'r', 'iWrite r {value}; iApply; '),
            ('primary', PRIMARY, 'q', 'iState p {value}; ')]


class OrdinaryReadScope(unittest.TestCase):
    setUp = helpers.SimultaneousClients.setUp
    assert_prefix_notes = helpers.SimultaneousClients.assert_prefix_notes
    notes = staticmethod(helpers.SimultaneousClients.notes)

    def run_tool(self, icl, pdl, output='result.pdl', ok=True, extra=()):
        if icl == self.root / 'source.icl':
            extra = (*extra, *getattr(self, 'bsdl_models', ()))
        return helpers.SimultaneousClients.run_tool(self, icl, pdl, output, ok, extra)

    def test_single_client_partial_read(self):
        self.assert_prefix_notes(clients.network(1),
                                 'iReset; iWrite c0.value 13; iApply; '
                                 'iRead c0.value[3] 0bx; iApply;\n',
                                 'iNote actual=[iGetReadData c0.value -bin]; '
                                 'iNote status=[iGetStatus -clear]; iNote status=[iGetStatus];\n',
                                 [b'actual=0b1xxx', b'status=0', b'status=0'])

    def test_interface_free_partial_read(self):
        self.assert_prefix_notes(SCAN, 'iReset; iWrite r 13; iApply; '
                                 'iRead r[3] 0bx; iApply;\n',
                                 'iNote actual=[iGetReadData r -bin]; '
                                 'iNote status=[iGetStatus -clear]; iNote status=[iGetStatus];\n',
                                 [b'actual=0b1xxx', b'status=0', b'status=0'])

    def test_primary_output_partial_read(self):
        self.assert_prefix_notes(PRIMARY, 'iState p 13; iRead q[3] 0bx; iApply;\n',
                                 'iNote actual=[iGetReadData q -bin]; '
                                 'iNote status=[iGetStatus -clear]; iNote status=[iGetStatus];\n',
                                 [b'actual=0b1xxx', b'status=0', b'status=0'])

    def test_known_previous_bits_survive_partial_read(self):
        for kind, source, target, write in cases():
            with self.subTest(kind=kind):
                prefix = ('iReset; ' + write.format(value=6) + f'iRead {target} 0bx; iApply; ' +
                          write.format(value=13) + f'iRead {target}[3] 0bx; iApply;\n')
                self.assert_prefix_notes(source, prefix,
                                         f'iNote actual=[iGetReadData {target} -bin]; '
                                         'iNote status=[iGetStatus -clear]; iNote status=[iGetStatus];\n',
                                         [b'actual=0b1110', b'status=0', b'status=0'])

    def test_discontiguous_reads_and_x_override_keep_masks(self):
        for kind, source, target, write in cases():
            for overwrite in (False, True):
                with self.subTest(kind=kind, overwrite=overwrite):
                    prefix = 'iReset; ' + write.format(value=13)
                    prefix += f'iRead {target}[3] 0; iRead {target}[0] 0; '
                    if overwrite:
                        prefix += f'iRead {target}[0] 0bx; '
                    prefix += 'iApply;\n'
                    self.assert_prefix_notes(source, prefix,
                                             f'iNote actual=[iGetReadData {target} -bin]; '
                                             f'iNote mismatch=[iGetMiscompares {target} -bin]; '
                                             'iNote status=[iGetStatus -clear]; iNote status=[iGetStatus];\n',
                                             [b'actual=0b1xx1',
                                              b'mismatch=0b1xx0' if overwrite else b'mismatch=0b1xx1',
                                              b'status=1' if overwrite else b'status=2', b'status=0'])

    def test_separate_applies_preserve_per_bit_history(self):
        for kind, source, target, write in cases():
            with self.subTest(kind=kind):
                prefix = ('iReset; ' + write.format(value=13) +
                          f'iRead {target}[3] 0bx; iApply; ' + write.format(value=6) +
                          f'iRead {target}[0] 0bx; iApply;\n')
                self.assert_prefix_notes(source, prefix,
                                         f'iNote actual=[iGetReadData {target} -bin]; '
                                         'iNote status=[iGetStatus -clear]; iNote status=[iGetStatus];\n',
                                         [b'actual=0b1xx0', b'status=0', b'status=0'])

    def test_inverted_aliases_keep_ascending_and_descending_scope(self):
        for ascending in (False, True):
            for inverted in (False, True):
                with self.subTest(ascending=ascending, inverted=inverted):
                    declared = '0:3' if ascending else '3:0'
                    index = '0' if ascending else '3'
                    source = SCAN.replace('Module Chains {', 'Module Chains { Alias part[' +
                                          declared + '] = ' + ('~' if inverted else '') + 'r;')
                    self.assert_prefix_notes(source, 'iReset; iWrite r 13; iApply; '
                                             f'iRead part[{index}] 0bx; iApply;\n',
                                             'iNote actual=[iGetReadData r -bin]; '
                                             'iNote alias=[iGetReadData part -bin]; '
                                             'iNote status=[iGetStatus -clear]; iNote status=[iGetStatus];\n',
                                             [b'actual=0b1xxx',
                                              b'alias=0b0xxx' if inverted else b'alias=0b1xxx',
                                              b'status=0', b'status=0'])

    def test_ascending_register_scope(self):
        source = SCAN.replace('r[3:0]', 'r[0:3]').replace('Source r[0];', 'Source r[3];')
        self.assert_prefix_notes(source, 'iReset; iWrite r 13; iApply; iRead r[0] 0bx; iApply;\n',
                                 'iNote actual=[iGetReadData r -bin]; '
                                 'iNote status=[iGetStatus -clear]; iNote status=[iGetStatus];\n',
                                 [b'actual=0b1xxx', b'status=0', b'status=0'])

    def test_scan_routed_data_register_keeps_logical_target(self):
        source = clients.network(1).replace(
            'ScanRegister value', "DataRegister obs[$W-1:0] {WriteDataSource 4'b1010; "
            'ResetValue 10;} ScanRegister value').replace('CaptureSource value;',
                                                         'CaptureSource obs;')
        self.assert_prefix_notes(source, 'iReset; iWrite c0.value 13; iApply; '
                                 'iRead c0.obs[3] 0; iApply;\n',
                                 'iNote actual=[iGetReadData c0.obs -bin]; '
                                 'iNote status=[iGetStatus -clear]; iNote status=[iGetStatus];\n',
                                 [b'actual=0b1xxx', b'status=1', b'status=0'])

    def test_primary_data_register_keeps_logical_target(self):
        source = '''Module Chains {
 DataInPort data[3:0]; DataInPort enable;
 DataRegister r[3:0] {WriteDataSource data; WriteEnSource enable; ResetValue 0;}
 DataOutPort q[3:0] {Source r;}
}'''
        self.assert_prefix_notes(source, 'iReset; iState enable 0; iState r 13; '
                                 'iRead r[3] 0; iApply;\n',
                                 'iNote actual=[iGetReadData r -bin]; '
                                 'iNote output=[iGetReadData q -bin]; '
                                 'iNote status=[iGetStatus -clear]; iNote status=[iGetStatus];\n',
                                 [b'actual=0b1xxx', b'output=0bxxxx', b'status=1', b'status=0'])

    def test_primary_bits_keep_separate_selected_frames(self):
        source = '''Module Chains {
 DataInPort sel;
 DataRegister a[1:0] {WriteDataSource 2'b10; ResetValue 2;}
 DataRegister b[1:0] {WriteDataSource 2'b01; ResetValue 1;}
 DataMux selected[1:0] SelectedBy sel {1'b0:a;1'b1:b;}
 DataOutPort q[1:0] {Source selected;}
}'''
        self.assert_prefix_notes(source, 'iReset; iRead a[1] 0; iRead b[0] 0; iApply;\n',
                                 'iNote a=[iGetReadData a -bin]; iNote b=[iGetReadData b -bin]; '
                                 'iNote output=[iGetReadData q -bin]; '
                                 'iNote status=[iGetStatus -clear]; iNote status=[iGetStatus];\n',
                                 [b'a=0b1x', b'b=0bx1', b'output=0bxx', b'status=2', b'status=0'])

    def test_canceled_read_keeps_previous_history(self):
        for kind, source, target, write in cases():
            with self.subTest(kind=kind):
                prefix = ('iReset; ' + write.format(value=6) + f'iRead {target} 0bx; iApply; ' +
                          write.format(value=13) + f'iRead {target}[3] 0; iReset; iApply;\n')
                self.assert_prefix_notes(source, prefix,
                                         f'iNote actual=[iGetReadData {target} -bin]; '
                                         'iNote status=[iGetStatus -clear]; iNote status=[iGetStatus];\n',
                                         [b'actual=0b0110', b'status=0', b'status=0'])

    def tap_source(self):
        fixture = Path(__file__).resolve().parents[3] / 'tests/cases/ours/pattern_generation/access_link'
        model = self.root / 'models.bsdl'
        model.write_text((fixture / 'simple.bsdl').read_text())
        self.bsdl_models = ('--bsdl', 'AccessProbe', str(model))
        return (fixture / 'simple.icl').read_text().replace('Module AccessTop {', 'Module Chains {')

    def test_single_tap_ir_slice_keeps_physical_capture(self):
        self.assert_prefix_notes(self.tap_source(), 'iReset; iWrite tap.INSTRUCTION 3; '
                                 'iApply; iRead tap.INSTRUCTION[3] 0bx; iApply;\n',
                                 'iNote actual=[iGetReadData tap.INSTRUCTION -bin]; '
                                 'iNote status=[iGetStatus -clear]; iNote status=[iGetStatus];\n',
                                 [b'actual=0b0xxx', b'status=0', b'status=0'])

    def test_single_tap_dr_slice_keeps_implicit_ir_check(self):
        source = self.tap_source().replace('ScanInSource si;', 'ScanInSource si; CaptureSource r;')
        self.assert_prefix_notes(source, 'iReset; iWrite core.r 214; iApply; '
                                 'iRead core.r[7] 0bx; iApply;\n',
                                 'iNote actual=[iGetReadData core.r -bin]; '
                                 'iNote status=[iGetStatus -clear]; iNote status=[iGetStatus];\n',
                                 [b'actual=0b1xxxxxxx', b'status=0', b'status=0'])

    def test_invalid_slice_preserves_previous_output(self):
        icl, pdl = self.root / 'source.icl', self.root / 'source.pdl'
        icl.write_text(SCAN)
        pdl.write_text('iReset; iRead r[9] 0; iApply;\n')
        output = self.root / 'result.pdl'
        output.write_bytes(b'previous output\n')
        self.run_tool(icl, pdl, ok=False)
        self.assertEqual(output.read_bytes(), b'previous output\n')

    def test_sparse_scale(self):
        for kind, width in [('primary', 256), ('primary', 512), ('primary', 1024),
                            ('scan', 256), ('client', 256)]:
            with self.subTest(kind=kind, width=width):
                if kind == 'primary':
                    source = PRIMARY.replace('[3:0]', f'[{width-1}:0]')
                    target, write = 'q', 'iState p'
                elif kind == 'scan':
                    source = SCAN.replace('[3:0]', f'[{width-1}:0]')
                    target, write = 'r', 'iWrite r'
                else:
                    source = clients.network(1).replace('Instance c0 Of Cell {',
                                                        f'Instance c0 Of Cell {{ Parameter W = {width};')
                    target, write = 'c0.value', 'iWrite c0.value'
                bits = ('11010110' * ((width + 7) // 8))[:width]
                prefix = (f'iReset; {write} 0b{bits}; iApply; '
                          f'iRead {target}[{width-1}] 0bx; iApply;\n')
                tail = (f'iNote actual=[iGetReadData {target} -bin]; '
                        'iNote status=[iGetStatus -clear]; iNote status=[iGetStatus];\n')
                started = time.monotonic()
                self.assertTrue(self.assert_prefix_notes(
                    source, prefix, tail, [('actual=0b1' + 'x' * (width-1)).encode(),
                                          b'status=0', b'status=0']))
                elapsed = time.monotonic() - started
                self.assertEqual(len(self.prefix_producer_seconds), 2)
                self.assertLess(max(self.prefix_producer_seconds), 10.0)
                print(json.dumps(dict(kind=kind, width=width, cli_processes=10,
                                      paired_producer_seconds=elapsed,
                                      producer_seconds=self.prefix_producer_seconds,
                                      input_bytes=sum((self.root / n).stat().st_size for n in
                                                      ['source.icl', 'prefix.pdl', 'tail.pdl']),
                                      sha256=hashlib.sha256(
                                          (self.root / 'history/result.pdl').read_bytes()).hexdigest())),
                      flush=True)


if __name__ == '__main__':
    unittest.main()
