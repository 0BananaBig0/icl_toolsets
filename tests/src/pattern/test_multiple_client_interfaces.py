#!/usr/bin/env python3
"""Independently selected client interfaces retain one shared hardware state."""
import time
import unittest

import test_simultaneous_scan_chains as helpers

CELL = """Module Cell {
  Parameter W = 4;
  ScanInPort si; SelectPort sel;
  ScanRegister value[$W-1:0] { ScanInSource si; CaptureSource value; ResetValue 0; }
  ScanOutPort so { Source value[0]; }
  DataOutPort q[$W-1:0] { Source value; }
}
"""


def network(count=2):
    source = CELL + f"""Module Chains {{
  ScanInPort si[{count-1}:0]; SelectPort sel[{count-1}:0];
  CaptureEnPort ce; ShiftEnPort se; UpdateEnPort ue; TCKPort ck;
"""
    for i in range(count):
        source += f"""  Instance c{i} Of Cell {{ InputPort si = si[{i}]; InputPort sel = sel[{i}]; }}
  ScanOutPort so{i} {{ Source c{i}.so; }}
  ScanInterface bus{i} {{ Port si[{i}]; Port so{i}; Port sel[{i}]; Port ce; Port se; Port ue; Port ck; }}
"""
    return source + "}\n"


SHARED = (helpers.ROOT / 'tests/cases/ours/pattern_generation/multiple_clients.icl').read_text()


class MultipleClients(unittest.TestCase):
    setUp = helpers.SimultaneousChains.setUp
    run_tool = helpers.SimultaneousChains.run_tool
    replay = helpers.SimultaneousChains.replay

    def test_separate_clients_ordered_reads_and_writes(self):
        output = self.replay(network(), """iPDLLevel 1
iReset
iWrite c0.value 9; iWrite c1.value 6; iApply
iRead c0.value 9; iRead c1.value 6; iWrite c0.value 3; iWrite c1.value 12; iApply
iRead c0.value 3; iRead c1.value 12; iApply
""")
        self.assertIn(b'iWrite c0.value 0b1001', output)
        self.assertIn(b'iWrite c1.value 0b0110', output)

    def test_shared_storage_across_interface_switch(self):
        self.replay(SHARED, """iPDLLevel 1
iReset
iWrite route 0; iApply
iWrite c.value 9; iApply
iWrite route 1; iApply
iRead c.value 9; iWrite c.value 6; iApply
iWrite route 0; iApply
iRead c.value 6; iApply
""")

    def test_parallel_frame_keeps_selected_pins(self):
        source = SHARED.replace('  LogicSignal selected', """
  DataRegister held { WriteDataSource selected; WriteEnSource route; ResetValue 0; }
  DataOutPort q { Source held; }
  LogicSignal selected""")
        self.replay(source, 'iReset; iWrite route 0; iApply; iWrite c.value 9; iApply; '
                    'iWrite route 1; iApply; iRead q 1; iApply\n')

    def test_inverted_paths(self):
        source = network().replace('InputPort si = si[1]', 'InputPort si = ~si[1]')
        source = source.replace('Source c1.so', 'Source ~c1.so')
        self.replay(source, 'iReset; iWrite c0.value 3; iWrite c1.value 7; iApply\n')

    def test_unequal_widths_and_partial_aliases(self):
        source = network().replace('Instance c1 Of Cell {', 'Instance c1 Of Cell { Parameter W = 2;')
        source = source.replace('  CaptureEnPort', '  Alias low[3:0] = c0.value[1:0], c1.value;\n  CaptureEnPort')
        self.replay(source, 'iReset; iWrite low 7; iApply; iRead c0.value 1; iRead c1.value 3; iApply\n')

    def test_reservation_survives_other_client(self):
        self.replay(network(), 'iReset; iTake c0.value; iWrite c0.value 9; iApply; '
                    'iWrite c1.value 6; iApply; iRead c0.value 9; iApply\n')

    def test_reset_prerequisite_uses_other_interface(self):
        source = network().replace('SelectPort sel;',
                                   'SelectPort sel; ResetPort rst { ActivePolarity 0; }')
        source = source.replace('Module Chains {', "Module Chains { ResetPort global { ActivePolarity 0; } "
                                "DataMux reset_gate SelectedBy c0.q[0] { 1'b0: 1'b0; 1'b1: global; }")
        source = source.replace('Instance c1 Of Cell {',
                                'Instance c1 Of Cell { InputPort rst = reset_gate;')
        output = self.replay(source, 'iReset; iWrite c1.value 9; iApply; iRead c1.value 9; iApply\n')
        self.assertIn(b'iWrite c0.value 0b0001', output)

    def test_interface_switch_applies_selection_driven_reset(self):
        source = network().replace('SelectPort sel;',
                                   'SelectPort sel; ResetPort rst { ActivePolarity 0; }')
        source = source.replace('Module Chains {', 'Module Chains { ResetPort global { ActivePolarity 0; }')
        for i in range(2):
            source = source.replace('Module Chains {',
                                    f"Module Chains {{ DataMux reset_{i} SelectedBy sel[{i}] {{ "
                                    "1'b0: 1'b0; 1'b1: global; }")
            source = source.replace(f'Instance c{i} Of Cell {{',
                                    f'Instance c{i} Of Cell {{ InputPort rst = reset_{i};')
        self.replay(source, 'iReset; iWrite c0.value 9; iApply; iWrite c1.value 6; iApply; '
                    'iRead c0.value 0; iApply; iRead c1.value 0; iApply\n')

    def test_selection_reset_precedes_hidden_capture(self):
        source = (helpers.ROOT / 'tests/cases/ours/pattern_generation/multiple_client_reset_capture.icl').read_text()
        self.replay(source, 'iReset; iWrite c0.value 9; iApply; iWrite c1.value 6; iApply\n')

    def test_transient_selection_reset_precedes_hidden_capture(self):
        source = (helpers.ROOT / 'tests/cases/ours/pattern_generation/multiple_client_reset_capture.icl').read_text()
        source = source.replace('InputPort d = c0.q', 'InputPort d = c1.q')
        source = source.replace('  ScanInPort si[1:0];',
                                "  LogicSignal reset_b { sel[1] & c0.q[0]; }\n"
                                "  DataMux reset_gate_b SelectedBy reset_b { 1'b0: global; 1'b1: 1'b1; }\n"
                                '  ScanInPort si[1:0];')
        source = source.replace("InputPort rst = 1'b0", 'InputPort rst = reset_gate_b')
        self.replay(source, 'iReset; iWrite c1.value 6; iApply; iWrite c0.value 9; iApply; '
                    'iWrite c1.value 3; iApply\n')

    def test_transient_selection_reset_clears_data_controlling_history(self):
        source = (helpers.ROOT / 'tests/cases/ours/pattern_generation/multiple_client_reset_capture.icl').read_text()
        source = """Module DataCell {
 DataInPort d[3:0]; DataInPort en; ResetPort rst;
 DataRegister value[3:0] { WriteDataSource d; WriteEnSource en; ResetValue 0; }
 DataOutPort q[3:0] { Source value; }
}
""" + source.replace('Module Chains {', """Module Chains {
 DataInPort bus[3:0]; DataInPort enable;
 LogicSignal reset_d { sel[1] & c0.q[0]; }
 DataMux reset_gate_d SelectedBy reset_d { 1'b0: global; 1'b1: 1'b1; }
 Instance d Of DataCell { InputPort d = bus; InputPort en = enable; InputPort rst = reset_gate_d; }
 DataOutPort q[3:0] { Source d.q; }
""")
        for initialize in (False, True):
            with self.subTest(initialize=initialize):
                pdl = 'iReset; '
                if initialize:
                    pdl += 'iWrite c0.value 0; iApply; '
                else:
                    pdl += 'iState c0.value 0bxxxx; '
                pdl += ('iWrite bus 9; iWrite enable 1; iApply; iWrite enable 0; iApply; '
                        'iWrite c0.value 9; iApply; iWrite c1.value 3; iApply; iRead q 0; iApply\n')
                self.replay(source, pdl)

    def test_shared_select_with_unique_secondary_controls(self):
        source = network().replace('  CaptureEnPort ce;', '  SelectPort common; CaptureEnPort ce;')
        source = source.replace('Port sel[0];', 'Port sel[0]; Port common;')
        source = source.replace('Port sel[1];', 'Port sel[1]; Port common;')
        self.replay(source, 'iReset; iWrite c0.value 9; iWrite c1.value 6; iApply\n')

    def test_shared_output_selected_by_declared_input(self):
        source = SHARED.replace('Enable ~route;', '').replace(
            '  ScanOutPort so1 { Source c.so; Enable route; }', '')
        source = source.replace('Port so1;', 'Port so0;')
        self.replay(source, 'iReset; iWrite c.value 9; iApply; iRead c.value 9; iApply\n')

    def test_unselectable_or_mismatched_interfaces_preserve_outputs(self):
        sources = (network().replace('Port sel[1]', 'Port sel[0]'),
                   network().replace('InputPort si = si[1]', 'InputPort si = si[0]'))
        for source in sources:
            with self.subTest(source=source):
                icl, pdl = self.root / 'source.icl', self.root / 'source.pdl'
                icl.write_text(source)
                pdl.write_text('iReset; iWrite c1.value 9; iApply\n')
                (self.root / 'result.pdl').write_bytes(b'previous output\n')
                self.run_tool(icl, pdl, ok=False)
                self.assertEqual((self.root / 'result.pdl').read_bytes(), b'previous output\n')

    def test_cross_interface_together_is_rejected(self):
        icl, pdl = self.root / 'source.icl', self.root / 'source.pdl'
        icl.write_text(network())
        pdl.write_text('iReset; iWrite c0.value 9; iWrite c1.value 6; iApply -together\n')
        self.run_tool(icl, pdl, ok=False)

    def test_reset_pass_budget_preserves_artifacts(self):
        source = """Module Bit {
 ScanInPort si; SelectPort sel; ResetPort rst;
 ScanRegister value { ScanInSource si; ResetValue 0; }
 ScanOutPort so { Source value; } DataOutPort q { Source value; }
}
Module Chains {
 ScanInPort si[1:0]; SelectPort sel[1:0]; ResetPort global;
 ScanMux input SelectedBy sel[0] { 1'b0: si[1]; 1'b1: si[0]; }
 LogicSignal selected { sel[0] | sel[1]; }
"""
        for i in range(512):
            scan = f'c{i-1}.so' if i else 'input'
            reset = f'c{i-1}.q' if i else "1'b0"
            if i:
                source += (f"DataMux reset_{i} SelectedBy {reset} {{ "
                           "1'b0: global; 1'b1: 1'b1; }\n")
                reset = f'reset_{i}'
            source += f'Instance c{i} Of Bit {{ InputPort si = {scan}; InputPort rst = {reset}; InputPort sel = selected; }}\n'
        source += """ScanOutPort so { Source c511.so; }
 ScanInterface a { Port si[0]; Port so; Port sel[0]; }
 ScanInterface b { Port si[1]; Port so; Port sel[1]; }
}
"""
        icl, pdl = self.root / 'source.icl', self.root / 'source.pdl'
        icl.write_text(source)
        unknown = ''.join(f'iState c{i}.value 0bx; ' for i in range(512))
        pdl.write_text('iReset; ' + unknown + 'iWrite c511.value 1; iApply\n')
        (self.root / 'result.pdl').write_bytes(b'previous output\n')
        error = self.run_tool(icl, pdl, ok=False)
        self.assertIn('reset resource limit', error)
        self.assertEqual((self.root / 'result.pdl').read_bytes(), b'previous output\n')

    def test_multiple_client_scale(self):
        count = 16
        source = network(count)
        pdl = 'iReset;\n' + ''.join(f'iWrite c{i}.value {i};\n' for i in range(count)) + 'iApply\n'
        start = time.monotonic()
        self.replay(source, pdl)
        print(f'multiple clients: {count} clients, {len(source)} ICL bytes, three replays {time.monotonic()-start:.3f}s')


if __name__ == '__main__':
    unittest.main()
