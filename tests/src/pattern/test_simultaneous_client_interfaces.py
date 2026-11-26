#!/usr/bin/env python3
"""Disjoint standalone clients share one requested update event."""
import unittest
import time
import json
import hashlib

import test_multiple_client_interfaces as helpers


class SimultaneousClients(unittest.TestCase):
    setUp = helpers.MultipleClients.setUp
    run_tool = helpers.MultipleClients.run_tool

    def replay(self, source, body, extra=()):
        icl, pdl = self.root / 'source.icl', self.root / 'source.pdl'
        icl.write_text(source)
        pdl.write_text(body)
        output = self.run_tool(icl, pdl, extra=extra)
        for model, commands, result in [(self.root / 'debug/Chains.icl', pdl, 'flat/result.pdl'),
                                        (icl, self.root / 'result.pdl', 'replay/result.pdl'),
                                        (self.root / 'debug/Chains.icl', self.root / 'result.pdl',
                                         'flat-replay/result.pdl')]:
            self.assertEqual(output, self.run_tool(model, commands, result, extra=extra))
        return output

    def test_two_clients_update_together(self):
        output = self.replay(helpers.network(), '''iPDLLevel 1
iProcsForModule Chains
iReset
iWrite c0.value 13; iWrite c1.value 6; iApply -together
iRead c0.value 13; iRead c1.value 6; iApply
''')
        first = output.split(b'iApply', 1)[0]
        self.assertIn(b'iWrite c0.value 0b1101', first)
        self.assertIn(b'iWrite c1.value 0b0110', first)
        self.assertIn(b'iApply -together', output)

    def test_actual_alias_capture_and_once_only_status(self):
        source = helpers.network().replace('Module Chains {',
                                           'Module Chains { Alias pair[7:0] = '
                                           'c0.value,c1.value {AccessTogether;}')
        output = self.replay(source, '''iPDLLevel 1
iProcsForModule Chains
iReset; iWrite pair 214; iApply
iRead pair 0; iApply
iNote [iGetReadData pair -bin]; iNote [iGetMiscompares pair -bin]
iNote [iGetStatus -clear]; iNote [iGetStatus]
''', extra=('--model_readback',))
        self.assertEqual(self.notes(output), [b'0b11010110', b'0b11010110', b'5', b'0'])

    @staticmethod
    def notes(output):
        return [line[6:] for line in output.splitlines()
                if line.startswith(b'iNote ') and b'modeled ' not in line]

    def test_unequal_widths_and_inverted_paths(self):
        source = helpers.network().replace('Instance c1 Of Cell {',
                                           'Instance c1 Of Cell { Parameter W = 7;')
        source = source.replace('InputPort si = si[1]', 'InputPort si = ~si[1]')
        source = source.replace('Source c1.so', 'Source ~c1.so')
        self.replay(source, 'iProcsForModule Chains\niReset; iWrite c0.value 13; '
                    'iWrite c1.value 83; iApply -together; '
                    'iRead c0.value 13; iRead c1.value 83; iApply\n')

    def test_held_unrequested_lane_is_preserved(self):
        self.replay(helpers.network(3), 'iProcsForModule Chains\niReset; '
                    'iTake c2.value; iWrite c2.value 11; iApply; '
                    'iWrite c0.value 13; iWrite c1.value 6; iApply -together; '
                    'iRead c2.value 11; iApply\n')

    def test_partial_access_together_alias_preserves_other_bits(self):
        source = helpers.network().replace('Module Chains {',
                                           'Module Chains { Alias low[3:0] = '
                                           'c0.value[2:1],c1.value[1:0] {AccessTogether;}')
        output = self.replay(source, '''iPDLLevel 1
iProcsForModule Chains
iReset; iWrite c0.value 13; iWrite c1.value 6; iApply -together
iWrite low 7; iApply; iRead low 0; iApply
iNote [iGetReadData low -bin]; iNote [iGetStatus -clear]; iNote [iGetStatus]
''', extra=('--model_readback',))
        self.assertIn(b'iWrite c0.value 0b1011', output)
        self.assertIn(b'iWrite c1.value 0b0111', output)
        self.assertEqual(self.notes(output), [b'0b0111', b'3', b'0'])

    def test_generated_partial_alias_prefix_preserves_read_history_scope(self):
        for ascending in (False, True):
            for inverted in (False, True):
                for known in (False, True):
                    for modeled in (False, True):
                        with self.subTest(ascending=ascending, inverted=inverted,
                                          known=known, modeled=modeled):
                            alias_range = '0:7' if ascending else '7:0'
                            source = helpers.network().replace(
                                'Module Chains {', 'Module Chains { Alias pair[' + alias_range +
                                '] = ' + ('~' if inverted else '') +
                                'c0.value,c1.value {AccessTogether;}')
                            icl, pdl, tail = (self.root / n for n in
                                              ('source.icl', 'prefix.pdl', 'tail.pdl'))
                            icl.write_text(source)
                            setup = ('iWrite c1.value 6; iApply; iRead c1.value 0bx; iApply; '
                                     if known else '')
                            read_range = '0:1' if ascending else '7:6'
                            pdl.write_text('iPDLLevel 1\niProcsForModule Chains\niReset; ' + setup +
                                           'iWrite c0.value 13; iWrite c1.value 9; iApply -together; '
                                           'iRead pair[' + read_range + '] 0bx; '
                                           'iRead c0.value[0] 0bx; iApply;\n')
                            tail.write_text('iPDLLevel 1\niNote c0=[iGetReadData c0.value -bin]; '
                                            'iNote c1=[iGetReadData c1.value -bin]; '
                                            'iNote status=[iGetStatus -clear]; iNote status=[iGetStatus];\n')
                            self.run_tool(icl, pdl, 'prefix-generated.pdl',
                                          extra=('--model_readback',) if modeled else ())
                            expected = [b'c0=0b11x1', b'c1=0b0110' if known else b'c1=0bxxxx',
                                        b'status=0', b'status=0']
                            for model in (icl, self.root / 'debug/Chains.icl'):
                                for prefix in (pdl, self.root / 'prefix-generated.pdl'):
                                    output = self.run_tool(model, prefix, 'history/result.pdl',
                                                           extra=('--model_readback', '--pdl', str(tail)))
                                    self.assertEqual(self.notes(output), expected)

    def assert_prefix_notes(self, source, prefix, tail, expected):
        icl, pdl, continuation = (self.root / n for n in
                                  ('source.icl', 'prefix.pdl', 'tail.pdl'))
        icl.write_text(source)
        pdl.write_text('iPDLLevel 1\niProcsForModule Chains\n' + prefix)
        continuation.write_text('iPDLLevel 1\n' + tail)
        for modeled in (False, True):
            with self.subTest(modeled=modeled):
                self.run_tool(icl, pdl, 'prefix-generated.pdl',
                              extra=('--model_readback',) if modeled else ())
                for model in (icl, self.root / 'debug/Chains.icl'):
                    for commands in (pdl, self.root / 'prefix-generated.pdl'):
                        output = self.run_tool(model, commands, 'history/result.pdl',
                                               extra=('--model_readback', '--pdl',
                                                      str(continuation)))
                        self.assertEqual(self.notes(output), expected)

    def test_routed_comparison_preserves_logical_history_and_failure(self):
        source = helpers.network().replace(
            'ScanRegister value', "DataRegister obs[$W-1:0] {WriteDataSource 4'b1010; "
            'ResetValue 10;} ScanRegister value').replace('CaptureSource value;',
                                                         'CaptureSource obs;')
        source = source.replace('Module Chains {', 'Module Chains { Alias pair[7:0] = '
                                'c0.value,c1.value {AccessTogether;}')
        self.assert_prefix_notes(source, 'iReset; iWrite c0.value 13; iWrite c1.value 6; '
                                 'iApply; iRead pair[0] 0bx; iRead c0.obs[3] 0; iApply;\n',
                                 'iNote actual=[iGetReadData c0.obs -bin]; '
                                 'iNote status=[iGetStatus -clear]; iNote status=[iGetStatus];\n',
                                 [b'actual=0b1xxx', b'status=1', b'status=0'])

    def test_other_client_sparse_read_preserves_its_own_frame(self):
        source = helpers.network(3).replace('Module Chains {', 'Module Chains { '
                                           'Alias pair[7:0] = c0.value,c1.value {AccessTogether;}')
        self.assert_prefix_notes(source, 'iReset; iWrite c0.value 13; iWrite c1.value 6; '
                                 'iWrite c2.value 11; iApply; iRead pair[7:6] 0bx; '
                                 'iRead c2.value[3] 0bx; iApply;\n',
                                 'iNote actual=[iGetReadData c2.value -bin]; '
                                 'iNote status=[iGetStatus -clear]; iNote status=[iGetStatus];\n',
                                 [b'actual=0b1xxx', b'status=0', b'status=0'])

    def test_grouped_primary_slice_preserves_history_and_failure(self):
        source = helpers.network().replace('Module Chains {', 'Module Chains { '
                                           'DataOutPort obs[3:0] {Source c0.q;} '
                                           'Alias pair[7:0] = c0.value,c1.value {AccessTogether;}')
        self.assert_prefix_notes(source, 'iReset; iWrite c0.value 13; iWrite c1.value 6; '
                                 'iApply; iRead pair[0] 0bx; iRead obs[3] 0; iApply;\n',
                                 'iNote actual=[iGetReadData obs -bin]; '
                                 'iNote status=[iGetStatus -clear]; iNote status=[iGetStatus];\n',
                                 [b'actual=0b1xxx', b'status=1', b'status=0'])

    def test_write_group_with_direct_sparse_read_preserves_history(self):
        self.assert_prefix_notes(helpers.network(),
                                 'iReset; iWrite c0.value 13; iWrite c1.value 6; iApply; '
                                 'iWrite c0.value 3; iWrite c1.value 4; '
                                 'iRead c0.value[3] 0; iApply -together;\n',
                                 'iNote actual=[iGetReadData c0.value -bin]; '
                                 'iNote status=[iGetStatus -clear]; iNote status=[iGetStatus];\n',
                                 [b'actual=0b1xxx', b'status=1', b'status=0'])

    def test_parallel_preparation_preserves_hidden_toggle(self):
        source = helpers.network().replace('ScanInSource si; CaptureSource value;',
                                           'ScanInSource gate; CaptureSource value;')
        source = source.replace('ScanRegister value', '''
 ScanRegister gate {ScanInSource si; CaptureSource gate; ResetValue 0;}
 ScanMux ret SelectedBy gate {1'b0:gate;1'b1:value[0];}
 ScanRegister value''').replace('Source value[0];', 'Source ret;')
        source = '''Module Hidden {
 ScanInPort si; ScanOutPort so {Source value;}
 SelectPort selected; CaptureEnPort capture; ShiftEnPort shift;
 UpdateEnPort update; TCKPort clock;
 ScanRegister value {ScanInSource si; CaptureSource ~value; ResetValue 0;}
 DataOutPort q {Source value;}
 ScanInterface client {Port si;Port so;Port selected;Port capture;Port shift;Port update;Port clock;}
}
''' + source.replace('Module Chains {', '''Module Chains {
 LogicSignal both {sel[0] & sel[1];}
 Instance hidden Of Hidden {
  InputPort si = 1'b0; InputPort selected = both;
  InputPort capture = ce; InputPort shift = 1'b0;
  InputPort update = ue; InputPort clock = ck;
 }
 DataOutPort observed {Source hidden.q;}
''')
        self.assert_prefix_notes(source, 'iReset; iWrite c0.value 13; iWrite c1.value 6; '
                                 'iApply -together;\n',
                                 'iRead observed 0; iApply; iNote actual=[iGetReadData observed -bin]; '
                                 'iNote status=[iGetStatus -clear]; iNote status=[iGetStatus];\n',
                                 [b'actual=0b0', b'status=0', b'status=0'])
        plain = helpers.CELL.replace('Module Cell', 'Module Plain').replace(
            'SelectPort sel;', 'SelectPort sel; ResetPort rst {ActivePolarity 0;}')
        reset_source = plain + source.replace('Instance c1 Of Cell {',
                                               'Instance c1 Of Plain {InputPort rst = rst_gate;')
        reset_source = reset_source.replace('Module Chains {', '''Module Chains {
 ResetPort global {ActivePolarity 0;}
 DataMux rst_gate SelectedBy sel[1] {1'b0:1'b0;1'b1:global;}
 Alias pair[7:0] = c0.value,c1.value {AccessTogether;}
''')
        self.assert_prefix_notes(reset_source, 'iReset; iRead pair 0; iApply;\n',
                                 'iRead observed 0; iApply; iNote actual=[iGetReadData observed -bin]; '
                                 'iNote status=[iGetStatus -clear]; iNote status=[iGetStatus];\n',
                                 [b'actual=0b1', b'status=1', b'status=0'])

    def test_named_chain_group_and_second_client_update_together(self):
        source = helpers.network(3).replace('InputPort sel = sel[2]', 'InputPort sel = sel[0]')
        source = source.replace('Port si[0]; Port so0;',
                                'Chain first {Port si[0]; Port so0;} '
                                'Chain second {Port si[2]; Port so2;}')
        source = source.replace('  ScanInterface bus2 { Port si[2]; Port so2; '
                                'Port sel[2]; Port ce; Port se; Port ue; Port ck; }', '')
        self.replay(source, 'iProcsForModule Chains\niReset; iWrite c0.value 13; '
                    'iWrite c1.value 6; iWrite c2.value 11; iApply -together; '
                    'iRead c0.value 13; iRead c1.value 6; iRead c2.value 11; iApply\n')

    def test_overlapping_read_alias_groups_preserve_sparse_scope(self):
        source = helpers.network().replace('Module Chains {', '''Module Chains {
 Alias left[7:0] = c0.value,c1.value {AccessTogether;}
 Alias right[7:0] = c1.value,c0.value {AccessTogether;}
''')
        output = self.replay(source, '''iPDLLevel 1
iProcsForModule Chains
iReset; iWrite c0.value 13; iWrite c1.value 9; iApply -together
iRead left[7:6] 0bx; iRead right[7:6] 0bx; iApply
iNote [iGetReadData c0.value -bin]; iNote [iGetReadData c1.value -bin]
iNote [iGetStatus -clear]; iNote [iGetStatus]
''', extra=('--model_readback',))
        self.assertEqual(self.notes(output), [b'0b11xx', b'0b10xx', b'0', b'0'])

    def test_shared_select_and_unique_secondary_selects(self):
        source = helpers.network().replace('CaptureEnPort ce;',
                                           'SelectPort common; CaptureEnPort ce;')
        source = source.replace('Port sel[0];', 'Port sel[0]; Port common;')
        source = source.replace('Port sel[1];', 'Port sel[1]; Port common;')
        self.replay(source, 'iProcsForModule Chains\niReset; iWrite c0.value 13; '
                    'iWrite c1.value 6; iApply -together\n')

    def test_coupled_selects_without_unique_selection_are_rejected(self):
        source = helpers.network().replace('InputPort sel = sel[1]', 'InputPort sel = sel[0]')
        source = source.replace('Port sel[1];', 'Port sel[0];')
        icl, pdl, output = (self.root / n for n in ('source.icl', 'source.pdl', 'result.pdl'))
        icl.write_text(source)
        pdl.write_text('iReset; iWrite c0.value 13; iWrite c1.value 6; iApply -together\n')
        output.write_bytes(b'keep output\n')
        self.run_tool(icl, pdl, ok=False)
        self.assertEqual(output.read_bytes(), b'keep output\n')

    def test_other_client_prepares_reset_before_common_update(self):
        source = helpers.network().replace('SelectPort sel;',
                                           'SelectPort sel; ResetPort rst {ActivePolarity 0;}')
        source = source.replace('Module Chains {', 'Module Chains { '
                                'ResetPort global {ActivePolarity 0;} '
                                "DataMux reset_gate SelectedBy c0.q[0] {1'b0:1'b0; 1'b1:global;}")
        source = source.replace('Instance c1 Of Cell {',
                                'Instance c1 Of Cell { InputPort rst = reset_gate;')
        self.replay(source, 'iProcsForModule Chains\niReset; iWrite c0.value 13; '
                    'iWrite c1.value 6; iApply -together; '
                    'iRead c0.value 13; iRead c1.value 6; iApply\n')

    def test_reset_held_alias_read_retains_capture_group(self):
        source = helpers.network().replace('SelectPort sel;',
                                           'SelectPort sel; ResetPort rst {ActivePolarity 0;}')
        source = source.replace('Instance c1 Of Cell {',
                                "Instance c1 Of Cell { InputPort rst = 1'b0;")
        source = source.replace('Module Chains {',
                                'Module Chains { Alias pair[7:0] = '
                                'c0.value,c1.value {AccessTogether;}')
        self.replay(source, 'iProcsForModule Chains\niReset; iRead pair 0; iApply\n')

    def test_hidden_capture_enabled_only_by_both_client_selects(self):
        hidden = '''Module Hidden {
 ScanInPort si; ScanOutPort so {Source value[0];}
 SelectPort selected; CaptureEnPort capture; ShiftEnPort shift;
 UpdateEnPort update; TCKPort clock; DataInPort data[3:0];
 ScanRegister value[3:0] {ScanInSource si; CaptureSource data; ResetValue 0;}
 DataOutPort q[3:0] {Source value;}
 ScanInterface client {Port si;Port so;Port selected;Port capture;Port shift;Port update;Port clock;}
}
'''
        source = hidden + helpers.network().replace('Module Chains {', '''Module Chains {
 LogicSignal simultaneous_selection {sel[0] & sel[1];}
 Instance hidden Of Hidden {
  InputPort si = 1'b0; InputPort selected = simultaneous_selection;
  InputPort capture = ce; InputPort shift = 1'b0;
  InputPort update = ue; InputPort clock = ck; InputPort data = c0.q;
 }
 DataOutPort observed[3:0] {Source hidden.q;}
''')
        output = self.replay(source, '''iPDLLevel 1
iProcsForModule Chains
iReset; iWrite c0.value 11; iApply
iWrite c0.value 13; iWrite c1.value 6; iApply -together
iRead observed 0; iApply
iNote [iGetReadData observed -bin]; iNote [iGetStatus -clear]; iNote [iGetStatus]
''', extra=('--model_readback',))
        self.assertEqual(self.notes(output), [b'0b1011', b'3', b'0'])

    def test_local_reset_rewrite_preserves_group(self):
        source = helpers.network()
        icl, pdl = self.root / 'source.icl', self.root / 'source.pdl'
        icl.write_text(source)
        pdl.write_text('iProcsForModule Chains\niReset; iWrite c0.value 13; '
                       'iWrite c1.value 6; iApply -together\n')
        output = self.run_tool(icl, pdl, extra=('--local_reset', str(self.root / 'reset.pdl')))
        self.assertEqual(output, self.run_tool(icl, self.root / 'reset.pdl', 'reset-replay.pdl'))

    def test_shared_serial_resources_reject_without_publication(self):
        sources = [helpers.network().replace('InputPort si = si[1]', 'InputPort si = si[0]'),
                   helpers.network().replace('Source c1.so', 'Source c0.so')]
        for source in sources:
            with self.subTest(source=source):
                icl, pdl, output = (self.root / n for n in ('source.icl', 'source.pdl', 'result.pdl'))
                icl.write_text(source)
                pdl.write_text('iReset; iWrite c0.value 13; iWrite c1.value 6; iApply -together\n')
                output.write_bytes(b'keep output\n')
                self.run_tool(icl, pdl, ok=False)
                self.assertEqual(output.read_bytes(), b'keep output\n')

    def test_shared_storage_switching_retains_ordinary_behavior(self):
        self.replay(helpers.SHARED, 'iReset; iWrite route 0; iApply; iWrite c.value 13; '
                    'iApply; iWrite route 1; iApply; iRead c.value 13; iApply\n')

    def test_valid_overlapping_clients_remain_serial(self):
        source = '''Module Chains {
 ScanInPort si[1:0]; SelectPort sel[1:0];
 CaptureEnPort ce; ShiftEnPort se; UpdateEnPort ue; TCKPort ck;
 ScanMux input SelectedBy sel[0] {1'b0:si[1];1'b1:si[0];}
 ScanRegister common[3:0] {ScanInSource input;CaptureSource common;ResetValue 0;}
 ScanRegister a[3:0] {ScanInSource common[0];CaptureSource a;ResetValue 0;}
 ScanRegister b[3:0] {ScanInSource common[0];CaptureSource b;ResetValue 0;}
 ScanOutPort so0 {Source a[0];} ScanOutPort so1 {Source b[0];}
 ScanInterface bus0 {Port si[0];Port so0;Port sel[0];Port ce;Port se;Port ue;Port ck;}
 ScanInterface bus1 {Port si[1];Port so1;Port sel[1];Port ce;Port se;Port ue;Port ck;}
}
'''
        self.replay(source, 'iReset; iWrite a 13; iApply; iWrite b 6; iApply; '
                    'iRead a 13; iRead b 6; iApply\n')
        icl, pdl, output = (self.root / n for n in ('source.icl', 'source.pdl', 'result.pdl'))
        icl.write_text(source)
        pdl.write_text('iReset; iWrite a 13; iWrite b 6; iApply -together\n')
        output.write_bytes(b'keep output\n')
        self.run_tool(icl, pdl, ok=False)
        self.assertEqual(output.read_bytes(), b'keep output\n')

    def test_incompatible_controls_or_clocks_do_not_qualify(self):
        for replacement in ('Port other;', 'Port alternate;'):
            with self.subTest(replacement=replacement):
                source = helpers.network().replace(
                    'CaptureEnPort ce;',
                    ('CaptureEnPort other; CaptureEnPort ce;'
                     if replacement == 'Port other;' else
                     'TCKPort alternate; CaptureEnPort ce;'))
                edited = source.replace('Port ce; Port se; Port ue; Port ck; }',
                                        ('Port other; Port se; Port ue; Port ck; }'
                                         if replacement == 'Port other;' else
                                         'Port ce; Port se; Port ue; Port alternate; }'), 1)
                self.replay(edited, 'iProcsForModule Chains\niReset; '
                            'iWrite c0.value 13; iApply; iWrite c1.value 6; iApply\n')
                icl, pdl, output = (self.root / n for n in ('source.icl', 'source.pdl', 'result.pdl'))
                icl.write_text(edited)
                pdl.write_text('iReset; iWrite c0.value 13; iWrite c1.value 6; iApply -together\n')
                output.write_bytes(b'keep output\n')
                self.run_tool(icl, pdl, ok=False)
                self.assertEqual(output.read_bytes(), b'keep output\n')

    def scale(self, simultaneous):
        for count, width in [(2, 8), (2, 32), (2, 64), (2, 256), (4, 8), (8, 8), (16, 8)]:
            with self.subTest(count=count, width=width):
                source = helpers.network(count)
                for i in range(count):
                    source = source.replace(f'Instance c{i} Of Cell {{',
                                            f'Instance c{i} Of Cell {{ Parameter W = {width};')
                values = [format((0xa3 ^ (i * 0x13)) & 0xff, '08b') * (width // 8)
                          for i in range(count)]
                body = 'iPDLLevel 1\niProcsForModule Chains\niReset; '
                body += ''.join(f'iWrite c{i}.value 0b{bits}; ' for i, bits in enumerate(values))
                body += 'iApply -together; ' if simultaneous else 'iApply; '
                body += ''.join(f'iRead c{i}.value 0; ' for i in range(count)) + 'iApply; '
                body += ''.join(f'iNote c{i}=[iGetReadData c{i}.value -bin]; '
                                for i in range(count)) + 'iNote status=[iGetStatus];\n'
                started = time.monotonic()
                output = self.replay(source, body, extra=('--model_readback',))
                elapsed = time.monotonic() - started
                if simultaneous and width == 256:
                    self.assertLess(elapsed, 10.0)
                self.assertEqual(self.notes(output),
                                 [f'c{i}=0b{bits}'.encode() for i, bits in enumerate(values)] +
                                 [f'status={sum(bits.count("1") for bits in values)}'.encode()])
                print(json.dumps(dict(simultaneous=simultaneous, clients=count, width=width,
                                      input_bytes=len(source) + len(body),
                                      four_replays_seconds=elapsed,
                                      sha256=hashlib.sha256(output).hexdigest())), flush=True)

    def test_simultaneous_scale(self):
        self.scale(True)

    def test_ordinary_control_scale(self):
        self.scale(False)


if __name__ == '__main__':
    unittest.main()
