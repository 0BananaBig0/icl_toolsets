#!/usr/bin/env python3
"""DataRegister outputs participate in the shared reset graph.

Exact arbitrary feedback and unknown-global/history cases live in native
TestUnreleasedDataFeedback/TestDataResetCallbacks, outside ICL source eligibility.
"""
import time
import unittest

import test_simultaneous_scan_chains as helpers

BASE = """Module Child {
 ScanInPort si; ResetPort rst { ActivePolarity 0; }
 ScanRegister value[3:0] { ScanInSource si; CaptureSource value; ResetValue 0; }
 ScanOutPort so { Source value[0]; }
}
Module Chains {
 ScanInPort si; DataInPort data; DataInPort enable; ResetPort global { ActivePolarity 0; }
 DataRegister release { WriteDataSource data; WriteEnSource enable; ResetValue 0; }
 DataMux reset_gate SelectedBy release { 1'b0: 1'b0; 1'b1: global; }
 Instance c Of Child { InputPort si = si; InputPort rst = reset_gate; }
 ScanOutPort so { Source c.so; }
}
"""


class DataResetDependencies(unittest.TestCase):
    setUp = helpers.SimultaneousChains.setUp
    run_tool = helpers.SimultaneousChains.run_tool
    replay = helpers.SimultaneousChains.replay

    def test_preparation_preserves_unread_scan_storage(self):
        source = """Module Sink {
 ScanInPort si; ResetPort rst;
 ScanRegister r { ScanInSource si; CaptureSource r; ResetValue 0; }
 ScanOutPort so { Source r; } DataOutPort q { Source r; }
}
Module Chains {
 ScanInPort si; ResetPort global; DataInPort enable;
 ScanRegister bus { ScanInSource si; CaptureSource bus; ResetValue 0; }
 DataRegister v { WriteDataSource 1'b0; WriteEnSource bus; }
 LogicSignal reset { ~v; }
 DataMux gate SelectedBy reset { 1'b0: global; 1'b1: 1'b1; }
 Instance a Of Sink { InputPort si=bus; InputPort rst=gate; }
 ScanOutPort so { Source a.so; }
 DataOutPort q { Source a.q; Enable bus; }
}
"""
        pio = source.replace('SelectedBy reset', 'SelectedBy enable')
        pio = pio.replace('Source a.q; Enable bus', 'Source a.q; Enable enable')
        data = pio.replace('ScanRegister r { ScanInSource si; CaptureSource r;',
                           "DataRegister r { WriteDataSource 1'b0; WriteEnSource 1'b0;")
        data = data.replace('ScanOutPort so { Source r;',
                            'ScanOutPort so { Source si;')
        icl, pdl = self.root / 'source.icl', self.root / 'source.pdl'
        for label, fixture in (('data', source), ('pio', pio), ('data-pio', data)):
            for comparison in ('1', '0', '0bx'):
                with self.subTest(route=label, comparison=comparison):
                    icl.write_text(fixture)
                    pdl.write_text('iPDLLevel 1; iReset; iState a.r 1; '
                                   'iState enable 0; iState v 1; iState bus 0; '
                                   f'iRead q {comparison}; iApply;\n')
                    (self.root / 'result.pdl').write_bytes(b'prior output\n')
                    error = self.run_tool(icl, pdl, ok=False,
                                          extra=('--model_readback',))
                    self.assertIn('reachability', error)
                    self.assertEqual((self.root / 'result.pdl').read_bytes(),
                                     b'prior output\n')

        safe = source.replace('InputPort rst=gate', 'InputPort rst=global')
        output = self.replay(safe, 'iPDLLevel 1; iReset; iState a.r 1; '
                             'iState enable 0; iState v 1; iState bus 0; '
                             'iRead q 0; iApply;\n')
        self.assertIn(b'iRead q 0b0', output)

    def test_greedy_keeps_read_only_data_state(self):
        source = ("Module Chains { ScanInPort si; DataRegister d { ResetValue 0; } "
                  "ScanRegister gate { ScanInSource si; ResetValue 0; }\n")
        previous = 'gate'
        for i in range(32):
            source += f'ScanRegister r{i} {{ ScanInSource {previous}; ResetValue 0; }}\n'
            previous = f'r{i}'
        source += ("ScanMux mux SelectedBy gate { 1'b0: gate; 1'b1: r31; } "
                   "ScanOutPort so { Source mux; } }\n")
        self.replay(source, 'iReset; iWrite r31 1; iApply;\n')

    def test_assumed_data_control_releases_scan_reset(self):
        self.replay(BASE, 'iReset; iState enable 0; iState release 1; '
                    'iWrite c.value 9; iApply; iRead c.value 9; iApply;\n')

    def test_primary_update_releases_and_asserts_reset(self):
        self.replay(BASE, 'iReset; iWrite data 1; iWrite enable 1; iApply; '
                    'iWrite enable 0; iApply; iWrite c.value 9; iApply; '
                    'iWrite data 0; iWrite enable 1; iApply; '
                    'iRead c.value 0; iApply;\n')

    def test_known_data_reset_control_preserves_snapshot(self):
        self.replay(BASE, 'iReset; iState enable 0; iState release 1; '
                    'iState c.value 9; iRead c.value 9; iApply;\n')

    def test_reverse_declared_mixed_cascade(self):
        source = BASE.replace(' ScanOutPort so { Source value[0]; }',
                              ' ScanOutPort so { Source value[0]; } DataOutPort q[3:0] { Source value; }')
        source = source.replace(' DataRegister release {', 
                                " DataMux last_reset SelectedBy c.q[0] { 1'b0: 1'b0; 1'b1: global; }\n"
                                ' Instance last Of Child { InputPort si = c.so; InputPort rst = last_reset; }\n DataRegister release {')
        source = source.replace('ScanOutPort so { Source c.so; }', 'ScanOutPort so { Source last.so; }')
        self.replay(source, 'iReset; iState enable 0; iState release 1; '
                    'iWrite c.value 1; iApply; iWrite last.value 9; iApply; '
                    'iRead last.value 9; iApply;\n')


    def test_pure_data_snapshot(self):
        source = """Module Cell {
 Parameter W=1; ResetPort rst;
 DataRegister value[$W-1:0] { WriteDataSource 0; WriteEnSource 1'b0; ResetValue 0; }
 DataOutPort q[$W-1:0] { Source value; }
}
Module Chains {
 DataInPort reset; ResetPort global;
 DataMux reset_a SelectedBy reset { 1'b0: global; 1'b1: 1'b1; }
 Instance a Of Cell { InputPort rst=reset_a; }
 LogicSignal reset_b { reset & a.q; }
 DataMux reset_gate_b SelectedBy reset_b { 1'b0: global; 1'b1: 1'b1; }
 Instance b Of Cell { Parameter W=4; InputPort rst=reset_gate_b; }
 DataOutPort q[3:0] { Source b.q; }
}
"""
        self.replay(source, 'iState reset 0; iReset; iState a.value 1; iState b.value 9; iWrite reset 1; iApply;\n')

    def test_transient_scan_fill(self):
        source = """Module D {
 ResetPort rst; DataRegister value { WriteDataSource 1'b0; WriteEnSource 1'b0; ResetValue 0; }
 DataOutPort q { Source value; }
}
Module S {
 ScanInPort si; ResetPort rst;
 ScanRegister value[3:0] { ScanInSource si; CaptureSource value; ResetValue 0; }
 ScanOutPort so { Source value[0]; } DataOutPort q[3:0] { Source value; }
}
Module Chains {
 DataInPort reset; ScanInPort si; ResetPort global;
 DataMux reset_a SelectedBy reset { 1'b0: global; 1'b1: 1'b1; }
 Instance a Of D { InputPort rst=reset_a; }
 LogicSignal reset_b { reset & a.q; }
 DataMux reset_gate_b SelectedBy reset_b { 1'b0: global; 1'b1: 1'b1; }
 Instance b Of S { InputPort si=si; InputPort rst=reset_gate_b; }
 ScanOutPort so { Source b.so; } DataOutPort q[3:0] { Source b.q; }
}
"""
        self.replay(source, 'iState reset 0; iReset; iState a.value 1; iState b.value 9; iWrite reset 1; iApply;\n')


    def test_post_scan_transient_reset(self):
        source = """Module D {
 ResetPort rst;
 DataRegister value { WriteDataSource 1'b0; WriteEnSource 1'b0; ResetValue 0; }
 DataOutPort q { Source value; }
}
Module S {
 ScanInPort si; ResetPort rst;
 ScanRegister value[3:0] { ScanInSource si; CaptureSource value; ResetValue 0; }
 ScanOutPort so { Source value[0]; } DataOutPort q[3:0] { Source value; }
}
Module Chains {
 ScanInPort si; ResetPort global;
 ScanRegister c { ScanInSource si; CaptureSource c; ResetValue 0; }
 DataMux reset_a SelectedBy c { 1'b0: global; 1'b1: 1'b1; }
 Instance a Of D { InputPort rst=reset_a; }
 LogicSignal rb { c & a.q; }
 DataMux reset_b SelectedBy rb { 1'b0: global; 1'b1: 1'b1; }
 Instance b Of S { InputPort si=c; InputPort rst=reset_b; }
 ScanOutPort so { Source b.so; }
}
"""
        self.replay(source, 'iReset; iState a.value 1; iWrite b.value 9; iApply; iWrite c 1; iApply; iRead b.value 0; iApply;\n')

    def test_one_hot_reset_dependency(self):
        source = """Module Cell {
 Parameter W=1; ResetPort rst;
 DataRegister value[$W-1:0] { WriteDataSource 0; WriteEnSource 1'b0; ResetValue 0; }
 DataOutPort q[$W-1:0] { Source value; Enable 1'b1; }
}
Module Chains {
 DataInPort reset; ResetPort global;
 DataMux reset_a SelectedBy reset { 1'b0: global; 1'b1: 1'b1; }
 Instance a Of Cell { InputPort rst=reset_a; }
 OneHotDataGroup group { Port a.q; } LogicSignal reset_b { reset & group; }
 DataMux reset_gate_b SelectedBy reset_b { 1'b0: global; 1'b1: 1'b1; }
 Instance b Of Cell { Parameter W=4; InputPort rst=reset_gate_b; }
 DataOutPort q[3:0] { Source b.q; }
}
"""
        self.replay(source, 'iState reset 0; iReset; iState a.value 1; iState b.value 9; iWrite reset 1; iApply;\n')



    def test_mixed_reset_chain_scale(self):
        for count in (64, 128):
            source = """Module ScanCell {
 ScanInPort si; ResetPort rst;
 ScanRegister value { ScanInSource si; CaptureSource value; ResetValue 0; }
 ScanOutPort so { Source value; } DataOutPort q { Source value; }
}
Module DataCell {
 ResetPort rst;
 DataRegister value { WriteDataSource 1'b0; WriteEnSource 1'b0; ResetValue 0; }
 DataOutPort q { Source value; }
}
Module Chains {
 ScanInPort si; ResetPort global;
"""
            previous_scan = 'si'
            for i in range(count):
                reset = 'global'
                if i:
                    reset = f'reset_{i}'
                    source += (f" DataMux {reset} SelectedBy n{i-1}.q {{ "
                               "1'b0: global; 1'b1: 1'b1; }\n")
                if i % 2:
                    source += f' Instance n{i} Of ScanCell {{ InputPort si = {previous_scan}; InputPort rst = {reset}; }}\n'
                    previous_scan = f'n{i}.so'
                else:
                    source += f' Instance n{i} Of DataCell {{ InputPort rst = {reset}; }}\n'
            source += f' ScanOutPort so {{ Source {previous_scan}; }}\n}}\n'
            body = 'iReset; ' + ''.join(f'iState n{i}.value 0; ' for i in range(count))
            body += f'iWrite n{count-1}.value 1; iApply;\n'
            start = time.monotonic()
            self.replay(source, body)
            print(f'mixed reset chain: {count} registers, {len(source)} ICL bytes, '
                  f'three replays {time.monotonic()-start:.3f}s')



if __name__ == '__main__':
    unittest.main()
