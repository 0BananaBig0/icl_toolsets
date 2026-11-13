#!/usr/bin/env python3
"""Named chain groups share phases within each independently selected client."""
import time
import unittest

import test_multiple_client_interfaces as helpers


def network(groups=(2, 2)):
    lanes = sum(groups)
    source = helpers.CELL + f"""Module Chains {{
 ScanInPort si[{lanes-1}:0]; SelectPort sel[{len(groups)-1}:0];
 CaptureEnPort ce; ShiftEnPort se; UpdateEnPort ue; TCKPort ck;
"""
    lane = 0
    for client, count in enumerate(groups):
        declarations = ''
        for chain in range(count):
            width = 4 if chain % 2 == 0 else 2
            source += f""" Instance c{client}_{chain} Of Cell {{ Parameter W = {width}; InputPort si = si[{lane}]; InputPort sel = sel[{client}]; }}
 ScanOutPort so{lane} {{ Source c{client}_{chain}.so; }}
"""
            declarations += f' Chain chain{chain} {{ Port si[{lane}]; Port so{lane}; }}'
            lane += 1
        source += f' ScanInterface bus{client} {{ Port sel[{client}]; Port ce; Port se; Port ue; Port ck;{declarations} }}\n'
    return source + '}\n'


class MultipleClientChains(unittest.TestCase):
    setUp = helpers.MultipleClients.setUp
    run_tool = helpers.MultipleClients.run_tool
    replay = helpers.MultipleClients.replay

    def test_each_clients_unequal_chains_shift_together(self):
        output = self.replay(network(), 'iReset; iWrite c0_0.value 9; iWrite c0_1.value 2; '
                             'iWrite c1_0.value 6; iWrite c1_1.value 1; iApply;\n')
        self.assertEqual(output.count(b'iApply'), 2)
        self.replay(network(), 'iReset; iWrite c0_0.value 9; iWrite c0_1.value 2; iApply -together; '
                    'iWrite c1_0.value 6; iWrite c1_1.value 1; iApply -together; '
                    'iRead c0_0.value 9; iRead c0_1.value 2; iRead c1_0.value 6; iRead c1_1.value 1; iApply;\n')

    def test_different_group_sizes(self):
        source = network((2, 3))
        pdl = 'iReset; ' + ''.join(f'iWrite c{client}_{chain}.value {chain+1}; '
                                  for client, count in enumerate((2, 3))
                                  for chain in range(count)) + 'iApply;\n'
        self.assertEqual(self.replay(source, pdl).count(b'iApply'), 2)

    def test_named_group_alongside_single_pair_client(self):
        source = network((2, 1)).replace('Chain chain0 { Port si[2]; Port so2; }',
                                          'Port si[2]; Port so2;')
        self.replay(source, 'iReset; iWrite c0_0.value 9; iWrite c0_1.value 2; '
                    'iWrite c1_0.value 6; iApply;\n')

    def test_inversion_and_ascending_indices(self):
        source = network().replace('si[3:0]', 'si[0:3]')
        source = source.replace('InputPort si = si[2]', 'InputPort si = ~si[2]')
        source = source.replace('Source c1_0.so', 'Source ~c1_0.so')
        self.replay(source, 'iReset; iWrite c1_0.value 9; iWrite c1_1.value 2; iApply -together;\n')

    def test_alias_and_reservation_within_group(self):
        source = network().replace(' CaptureEnPort ce;',
                                    ' Alias low[3:0] = c0_0.value[1:0], c0_1.value;\n CaptureEnPort ce;')
        self.replay(source, 'iReset; iTake c0_0.value; iWrite c0_0.value 9; iApply; '
                    'iWrite c1_0.value 6; iApply; iRead c0_0.value 9; iApply; '
                    'iRelease c0_0.value; iWrite low 7; iApply -together;\n')

    def test_shared_storage_across_selected_groups(self):
        source = network()
        source = '\n'.join(line for line in source.splitlines()
                           if not line.startswith((' Instance c0_0 ', ' Instance c1_0 '))) + '\n'
        source = source.replace('c0_0.so', 'shared.so').replace('c1_0.so', 'shared.so')
        source = source.replace(' CaptureEnPort ce;', """
 ScanMux input SelectedBy sel[0] { 1'b1: si[0]; 1'b0: si[2]; }
 LogicSignal selected { sel[0] | sel[1]; }
 Instance shared Of Cell { InputPort si = input; InputPort sel = selected; }
 CaptureEnPort ce;""")
        body = ('iReset; iWrite shared.value 9; iWrite c0_1.value 2; iApply -together; '
                'iRead shared.value 9; iWrite shared.value 6; iWrite c1_1.value 1; iApply -together; '
                'iRead shared.value 6; iRead c0_1.value 2; iApply;\n')
        self.replay(source, body)
        shared_output = source.replace(' ScanOutPort so2 { Source shared.so; }\n', '')
        shared_output = shared_output.replace('Port so2;', 'Port so0;')
        self.replay(shared_output, body)

    def test_other_group_releases_reset(self):
        source = network().replace('SelectPort sel;', 'SelectPort sel; ResetPort rst { ActivePolarity 0; }')
        source = source.replace('Instance c1_0 Of Cell {', 'Instance c1_0 Of Cell { InputPort rst = c0_0.q[0];')
        self.replay(source, 'iReset; iWrite c1_0.value 9; iApply; iRead c1_0.value 9; iApply;\n')

    def test_wrong_chain_input_and_disabled_output_keep_artifacts(self):
        swapped = network().replace('InputPort si = si[0]', 'InputPort si = si[1]')
        exchanged = network().replace('InputPort si = si[0]', 'InputPort si = temporary')
        exchanged = exchanged.replace('InputPort si = si[1]', 'InputPort si = si[0]')
        exchanged = exchanged.replace('InputPort si = temporary', 'InputPort si = si[1]')
        disabled = network().replace('Source c0_1.so;', "Source c0_1.so; Enable 1'b0;")
        for source in (swapped, exchanged, disabled):
            with self.subTest(source=source):
                icl, pdl = self.root / 'source.icl', self.root / 'source.pdl'
                icl.write_text(source)
                pdl.write_text('iReset; iWrite c0_0.value 9; iApply;\n')
                (self.root / 'result.pdl').write_bytes(b'previous output\n')
                self.run_tool(icl, pdl, ok=False)
                self.assertEqual((self.root / 'result.pdl').read_bytes(), b'previous output\n')

    def test_duplicate_chain_port_is_still_invalid(self):
        source = network().replace('Port si[1]; Port so1;', 'Port si[0]; Port so1;')
        icl, pdl = self.root / 'source.icl', self.root / 'source.pdl'
        icl.write_text(source)
        pdl.write_text('iReset;\n')
        self.assertIn('repeated ScanInterface port', self.run_tool(icl, pdl, ok=False))

    def test_together_cannot_span_different_clients(self):
        icl, pdl = self.root / 'source.icl', self.root / 'source.pdl'
        icl.write_text(network())
        pdl.write_text('iReset; iWrite c0_0.value 9; iWrite c1_0.value 6; iApply -together;\n')
        self.run_tool(icl, pdl, ok=False)

    def test_group_scale(self):
        groups = (2,) * 8
        source = network(groups)
        pdl = 'iReset; ' + ''.join(f'iWrite c{i}_0.value {i}; iWrite c{i}_1.value {i%4}; '
                                  for i in range(len(groups))) + 'iApply;\n'
        start = time.monotonic()
        self.replay(source, pdl)
        print(f'client groups: {len(groups)} clients, {sum(groups)} chains, {len(source)} ICL bytes, three replays {time.monotonic()-start:.3f}s')


if __name__ == '__main__':
    unittest.main()
