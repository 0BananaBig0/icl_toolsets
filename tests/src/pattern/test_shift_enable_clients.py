#!/usr/bin/env python3
"""ShiftEn-only client selection preserves phase timing and shared hardware."""
import time
import unittest

import test_multiple_client_interfaces as helpers
import test_multiple_client_chain_groups as groups


def network(count=2):
    source = helpers.network(count).replace(' SelectPort sel;', ' ShiftEnPort se;')
    source = source.replace(f'SelectPort sel[{count-1}:0];',
                            f'ShiftEnPort shift[{count-1}:0];')
    source = source.replace('CaptureEnPort ce; ShiftEnPort se;', 'CaptureEnPort ce;')
    for i in range(count):
        source = source.replace(f'InputPort sel = sel[{i}]', f'InputPort se = shift[{i}]')
        source = source.replace(f'Port sel[{i}]; Port ce; Port se;', f'Port shift[{i}]; Port ce;')
    return source


class ShiftEnableClients(unittest.TestCase):
    setUp = helpers.MultipleClients.setUp
    run_tool = helpers.MultipleClients.run_tool
    replay = helpers.MultipleClients.replay

    def test_separate_clients_ordered_reads_and_writes(self):
        self.replay(network(), 'iReset; iWrite c0.value 9; iWrite c1.value 6; iApply; '
                    'iRead c0.value 9; iRead c1.value 6; iApply;\n')

    def test_reservation_keeps_inactive_storage(self):
        self.replay(network(), 'iReset; iTake c0.value; iWrite c0.value 9; iApply; '
                    'iWrite c1.value 6; iApply; iRead c0.value 9; iApply;\n')

    def test_named_groups(self):
        source = groups.network().replace(' SelectPort sel;', ' ShiftEnPort se;')
        source = source.replace('SelectPort sel[1:0];', 'ShiftEnPort shift[1:0];')
        source = source.replace('CaptureEnPort ce; ShiftEnPort se;', 'CaptureEnPort ce;')
        for i in range(2):
            source = source.replace(f'InputPort sel = sel[{i}]', f'InputPort se = shift[{i}]')
            source = source.replace(f'Port sel[{i}]; Port ce; Port se;', f'Port shift[{i}]; Port ce;')
        self.replay(source, 'iReset; iWrite c0_0.value 9; iWrite c0_1.value 2; iApply -together; '
                    'iWrite c1_0.value 6; iWrite c1_1.value 1; iApply -together; '
                    'iRead c0_0.value 9; iRead c0_1.value 2; iApply;\n')

    def test_vector_and_inverted_paths(self):
        source = network().replace('shift[1:0]', 'shift[0:1]')
        source = source.replace('InputPort si = si[1]', 'InputPort si = ~si[1]')
        source = source.replace('Source c1.so', 'Source ~c1.so')
        self.replay(source, 'iReset; iWrite c0.value 3; iWrite c1.value 7; iApply;\n')

    def test_mixed_explicit_select_and_shift_only(self):
        source = network().replace('Module Chains {', 'Module Chains { SelectPort select;')
        source = source.replace('Port shift[0]; Port ce;', 'Port select; Port shift[0]; Port ce;')
        self.replay(source, 'iReset; iWrite c0.value 9; iWrite c1.value 6; iApply; '
                    'iRead c0.value 9; iRead c1.value 6; iApply;\n')

    def test_shared_shift_cannot_select_two_shift_only_clients(self):
        source = network().replace('shift[1]', 'shift[0]')
        self.reject(source, 'iReset; iWrite c0.value 9; iApply;\n')

    def test_shared_shift_with_explicit_client_keeps_all_owners(self):
        source = network().replace('Module Chains {', 'Module Chains { SelectPort select;')
        source = source.replace('Port shift[0]; Port ce;', 'Port select; Port shift[0]; Port ce;')
        source = source.replace('shift[1]', 'shift[0]')
        self.reject(source, 'iReset; iWrite c0.value 9; iApply;\n')
        self.replay(source, 'iReset; iWrite c1.value 6; iApply; iRead c1.value 6; iApply;\n')

    def test_together_cannot_span_clients(self):
        self.reject(network(), 'iReset; iWrite c0.value 9; iWrite c1.value 6; iApply -together;\n')

    def reject(self, source, body):
        icl, pdl = self.root / 'source.icl', self.root / 'source.pdl'
        icl.write_text(source)
        pdl.write_text(body)
        (self.root / 'result.pdl').write_bytes(b'previous output\n')
        self.run_tool(icl, pdl, ok=False)
        self.assertEqual((self.root / 'result.pdl').read_bytes(), b'previous output\n')

    def test_reset_prerequisite_from_other_client(self):
        source = network().replace('ShiftEnPort se;',
                                   'ShiftEnPort se; ResetPort rst { ActivePolarity 0; }')
        source = source.replace('Instance c1 Of Cell {',
                                'Instance c1 Of Cell { InputPort rst = c0.value[0];')
        self.replay(source, 'iReset; iWrite c1.value 9; iApply; iRead c1.value 9; iApply;\n')

    def test_shared_storage_and_shared_output(self):
        source = """Module Chains {
 ScanInPort si[1:0]; ShiftEnPort shift[1:0]; DataInPort route;
 CaptureEnPort ce; UpdateEnPort ue; TCKPort ck;
 ScanMux input SelectedBy route { 1'b0: si[0]; 1'b1: si[1]; }
 ScanRegister value[3:0] { ScanInSource input; CaptureSource value; ResetValue 0; }
 ScanOutPort so { Source value[0]; }
 ScanInterface a { Port si[0]; Port so; Port shift[0]; Port ce; Port ue; Port ck; }
 ScanInterface b { Port si[1]; Port so; Port shift[1]; Port ce; Port ue; Port ck; }
}"""
        self.replay(source, 'iReset; iWrite route 0; iApply; iWrite value 9; iApply; '
                    'iWrite route 1; iApply; iRead value 9; iWrite value 6; iApply; '
                    'iWrite route 0; iApply; iRead value 6; iApply;\n')

    def test_client_scale(self):
        count = 16
        source = network(count)
        body = 'iReset; ' + ''.join(f'iWrite c{i}.value {i}; ' for i in range(count)) + 'iApply;\n'
        start = time.monotonic()
        self.replay(source, body)
        print(f'ShiftEn clients: {count} clients, {len(source)} ICL bytes, three replays {time.monotonic()-start:.3f}s')


if __name__ == '__main__':
    unittest.main()
