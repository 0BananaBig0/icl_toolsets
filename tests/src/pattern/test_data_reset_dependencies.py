#!/usr/bin/env python3
"""DataRegister outputs participate in the shared reset graph."""
import time
import unittest

import test_simultaneous_scan_chains as helpers

BASE = """Module Child {
 ScanInPort si; ResetPort rst { ActivePolarity 0; }
 ScanRegister value[3:0] { ScanInSource si; CaptureSource value; ResetValue 0; }
 ScanOutPort so { Source value[0]; }
}
Module Chains {
 ScanInPort si; DataInPort data; DataInPort enable;
 DataRegister release { WriteDataSource data; WriteEnSource enable; ResetValue 0; }
 Instance c Of Child { InputPort si = si; InputPort rst = release; }
 ScanOutPort so { Source c.so; }
}
"""


class DataResetDependencies(unittest.TestCase):
    setUp = helpers.SimultaneousChains.setUp
    run_tool = helpers.SimultaneousChains.run_tool
    replay = helpers.SimultaneousChains.replay

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
                                ' Instance last Of Child { InputPort si = c.so; InputPort rst = c.q[0]; }\n DataRegister release {')
        source = source.replace('ScanOutPort so { Source c.so; }', 'ScanOutPort so { Source last.so; }')
        self.replay(source, 'iReset; iState enable 0; iState release 1; '
                    'iWrite c.value 1; iApply; iWrite last.value 9; iApply; '
                    'iRead last.value 9; iApply;\n')

    def test_unreleased_mixed_feedback_write_is_bounded(self):
        source = """Module Scan {
 ScanInPort si; ResetPort rst { ActivePolarity 0; }
 ScanRegister value { ScanInSource si; ResetValue 0; }
 ScanOutPort so { Source value; } DataOutPort q { Source value; }
}
Module Data {
 ResetPort rst { ActivePolarity 0; }
 DataRegister value { WriteDataSource 1'b0; WriteEnSource 1'b0; ResetValue 0; }
 DataOutPort q { Source value; }
}
Module Chains {
 ScanInPort si;
 Instance s Of Scan { InputPort si = si; InputPort rst = d.q; }
 Instance d Of Data { InputPort rst = s.q; }
 ScanOutPort so { Source s.so; }
}
"""
        icl, pdl = self.root / 'source.icl', self.root / 'source.pdl'
        icl.write_text(source)
        pdl.write_text('iReset; iWrite s.value 1; iApply;\n')
        self.assertIn('reachability not proved', self.run_tool(icl, pdl, ok=False))

    def test_pure_data_snapshot(self):
        source = """Module Cell {
 Parameter W=1; ResetPort rst;
 DataRegister value[$W-1:0] { WriteDataSource 0; WriteEnSource 1'b0; ResetValue 0; }
 DataOutPort q[$W-1:0] { Source value; }
}
Module Chains {
 DataInPort reset;
 Instance a Of Cell { InputPort rst=reset; }
 LogicSignal reset_b { reset & a.q; }
 Instance b Of Cell { Parameter W=4; InputPort rst=reset_b; }
 DataOutPort q[3:0] { Source b.q; }
}
"""
        self.replay(source, 'iState reset 0; iState a.value 1; iState b.value 9; iWrite reset 1; iApply;\n')

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
 DataInPort reset; ScanInPort si;
 Instance a Of D { InputPort rst=reset; }
 LogicSignal reset_b { reset & a.q; }
 Instance b Of S { InputPort si=si; InputPort rst=reset_b; }
 ScanOutPort so { Source b.so; } DataOutPort q[3:0] { Source b.q; }
}
"""
        self.replay(source, 'iState reset 0; iState a.value 1; iState b.value 9; iWrite reset 1; iApply;\n')

    def test_reset_after_data_update_preserves_controlling_history(self):
        source = """Module ScanCell {
 Parameter W=1; Parameter R=1; ScanInPort si; ResetPort rst;
 ScanRegister value[$W-1:0] { ScanInSource si; CaptureSource value; ResetValue $R; }
 ScanOutPort so { Source value[0]; } DataOutPort q[$W-1:0] { Source value; }
}
Module DataCell {
 ResetPort rst; DataRegister value { WriteDataSource 1'b1; WriteEnSource 1'b1; ResetValue 0; }
 DataOutPort q { Source value; }
}
Module Chains {
 ScanInPort si;
 Instance a Of ScanCell { InputPort si=si; InputPort rst=z; }
 Instance d Of DataCell { InputPort rst=a.q; }
 DataRegister z { WriteDataSource 1'b1; WriteEnSource 1'b1; ResetValue 0; }
 LogicSignal rb { z & d.q; }
 Instance b Of ScanCell { Parameter W=4; Parameter R=0; InputPort si=a.so; InputPort rst=rb; }
 ScanOutPort so { Source b.so; }
}
"""
        body = 'iState a.value 0; iState b.value 9; iState d.value 0; iState z 0; iRead b.value 9; iApply; iWrite b.value 0bx; iApply;\n'
        self.replay(source, "iReset; " + body)
        self.assertRegex(self.reject(source, body),
                         r"unrepresented scan load|no schedule within .*reachability not proved")

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
 ScanInPort si;
 ScanRegister c { ScanInSource si; CaptureSource c; ResetValue 0; }
 Instance a Of D { InputPort rst=c; }
 LogicSignal rb { c & a.q; }
 Instance b Of S { InputPort si=c; InputPort rst=rb; }
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
 DataInPort reset;
 Instance a Of Cell { InputPort rst=reset; }
 OneHotDataGroup group { Port a.q; } LogicSignal reset_b { reset & group; }
 Instance b Of Cell { Parameter W=4; InputPort rst=reset_b; }
 DataOutPort q[3:0] { Source b.q; }
}
"""
        self.replay(source, 'iState reset 0; iState a.value 1; iState b.value 9; iWrite reset 1; iApply;\n')

    def test_unreleased_self_feedback_write_is_bounded(self):
        source = BASE.replace('InputPort rst = release;', 'InputPort rst = c.q[0];')
        source = source.replace('ScanOutPort so { Source value[0]; }',
                                'ScanOutPort so { Source value[0]; } DataOutPort q[3:0] { Source value; }')
        icl, pdl = self.root / 'source.icl', self.root / 'source.pdl'
        icl.write_text(source)
        pdl.write_text('iReset; iWrite c.value 9; iApply;\n')
        message = self.run_tool(icl, pdl, ok=False)
        self.assertIn('reachability not proved', message)


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
 ScanInPort si;
"""
            previous_scan = 'si'
            for i in range(count):
                reset = f'n{i-1}.q' if i else "1'b0"
                if i % 2:
                    source += f' Instance n{i} Of ScanCell {{ InputPort si = {previous_scan}; InputPort rst = {reset}; }}\n'
                    previous_scan = f'n{i}.so'
                else:
                    source += f' Instance n{i} Of DataCell {{ InputPort rst = {reset}; }}\n'
            source += f' ScanOutPort so {{ Source {previous_scan}; }}\n}}\n'
            body = ''.join(f'iState n{i}.value 0; ' for i in range(count))
            body += f'iWrite n{count-1}.value 1; iApply;\n'
            start = time.monotonic()
            self.replay(source, body)
            print(f'mixed reset chain: {count} registers, {len(source)} ICL bytes, '
                  f'three replays {time.monotonic()-start:.3f}s')

    def reject(self, source, body):
        icl, pdl = self.root / 'source.icl', self.root / 'source.pdl'
        icl.write_text(source)
        pdl.write_text(body)
        output = self.root / 'result.pdl'
        output.write_bytes(b'previous output\n')
        message = self.run_tool(icl, pdl, ok=False)
        self.assertEqual(output.read_bytes(), b'previous output\n')
        return message


if __name__ == '__main__':
    unittest.main()
