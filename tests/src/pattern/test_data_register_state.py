#!/usr/bin/env python3
"""DataRegister iState updates stored knowledge without a hardware event."""
import time
import unittest

import test_simultaneous_scan_chains as helpers

BASE = """Module Chains {
 DataInPort data[3:0]; DataInPort enable;
 DataRegister value[3:0] { WriteDataSource data; WriteEnSource enable; ResetValue 0; }
 DataOutPort q[3:0] { Source value; }
}
"""


class DataState(unittest.TestCase):
    setUp = helpers.SimultaneousChains.setUp
    run_tool = helpers.SimultaneousChains.run_tool
    replay = helpers.SimultaneousChains.replay

    def test_assumption_without_reset(self):
        self.replay(BASE, 'iState enable 0; iState value 9; iRead q 9; iApply;\n')

    def test_explicit_last_written(self):
        self.replay(BASE, 'iState enable 0; iState value 6 -LastWrittenValue; iRead q 6; iApply;\n')

    def test_partial_alias_and_ascending_ranges(self):
        for ascending in (False, True):
            source = BASE.replace(' DataOutPort q', ' Alias inverted[3:0] = ~value;\n DataOutPort q')
            if ascending:
                source = source.replace('value[3:0]', 'value[0:3]')
            self.replay(source, 'iState enable 0; iState value 9; iState inverted 5; '
                        'iRead q 10; iApply;\n')
        self.replay(BASE, 'iState enable 0; iState value 9; iState value[1:0] 2; iRead q 10; iApply;\n')

    def test_partial_unknown_preserves_explicit_comparisons(self):
        self.replay(BASE, 'iState enable 0; iState value 0b1x01; iRead q 0b1x01; iApply;\n')
        output = self.replay(BASE, 'iState enable 0; iState value 0b1x01; iRead q 9; iApply;\n')
        self.assertNotIn(b'iWrite', output)
        self.assertIn(b'iRead q', output)

    def test_taken_assumption_protects_against_automatic_update(self):
        prefix = 'iState enable 0; iTake value; iState value 9; '
        self.reject(BASE, prefix + 'iWrite data 6; iWrite enable 1; iApply;\n')
        self.replay(BASE, prefix + 'iRelease value; iWrite data 6; iWrite enable 1; iApply; iRead q 6; iApply;\n')

    def test_hierarchical_state(self):
        source = BASE.replace('Module Chains', 'Module Child') + """Module Chains {
 DataInPort data[3:0]; DataInPort enable;
 Instance c Of Child { InputPort data = data; InputPort enable = enable; }
 DataOutPort q[3:0] { Source c.q; }
}
"""
        self.replay(source, 'iState enable 0; iState c.value 9; iRead q 9; iApply;\n')

    def test_no_event_with_held_reset_and_unused_scan_clients(self):
        child = BASE.replace('Module Chains', 'Module Child').replace(
            'DataInPort enable;', 'DataInPort enable; ResetPort rst;')
        for clients in (0, 1, 2):
            scan = ''
            for i in range(clients):
                scan += f"""
 ScanInPort si{i};
 ScanRegister dummy{i} {{ ScanInSource si{i}; ResetValue 0; }}
 ScanOutPort so{i} {{ Source dummy{i}; }}
"""
                if clients == 2:
                    scan += f"""
 SelectPort sel{i}; ShiftEnPort se{i}; CaptureEnPort ce{i}; UpdateEnPort ue{i}; TCKPort ck{i};
 ScanInterface client{i} {{ Port si{i}; Port so{i}; Port sel{i}; Port se{i}; Port ce{i}; Port ue{i}; Port ck{i}; }}
"""
            source = child + """Module Chains {
 DataInPort data[3:0]; DataInPort enable; DataInPort reset;
 Instance c Of Child { InputPort data = data; InputPort enable = enable; InputPort rst = reset; }
 DataOutPort q[3:0] { Source c.q; }
""" + scan + '}'
            output = self.replay(source, 'iState enable 0; iState reset 1; '
                                 'iState c.value 9; iRead q 9; iApply; iRead q 9; iApply;\n')
            self.assertNotIn(b'iWrite', output)

    def test_no_event_preserves_hidden_client_state(self):
        source = """Module D {
 DataInPort data[3:0]; DataInPort en; ResetPort rst;
 DataRegister value[3:0] { WriteDataSource data; WriteEnSource en; ResetValue 0; }
 DataOutPort q[3:0] { Source value; }
}
Module S {
 ScanInPort si; SelectPort sel; CaptureEnPort ce; ShiftEnPort se; ResetPort rst;
 ScanRegister value[3:0] { ScanInSource si; CaptureSource value; ResetValue 0; }
 ScanOutPort so { Source value[0]; }
}
Module Chains {
 DataInPort data[3:0]; DataInPort enable; DataInPort reset;
 Instance d Of D { InputPort data=data; InputPort en=enable; InputPort rst=reset; }
 DataOutPort q[3:0] { Source d.q; }
 ScanInPort si; SelectPort sel; ShiftEnPort se;
 Instance s Of S { InputPort si=si; InputPort sel=sel; InputPort ce=1'b0; InputPort se=se; InputPort rst=reset; }
 ScanOutPort so { Source s.so; }
}
"""
        output = self.replay(source, 'iState enable 0; iState reset 1; iState s.value 9; iState d.value 9; iRead q 9; iApply;\n')
        self.assertNotIn(b"iWrite", output)

    def test_data_state_releases_serial_reset(self):
        source = """Module Child {
 ScanInPort si; ResetPort rst { ActivePolarity 0; }
 ScanRegister value[3:0] { ScanInSource si; CaptureSource value; ResetValue 0; }
 ScanOutPort so { Source value[0]; }
}
Module Chains {
 ScanInPort si;
 DataRegister release { WriteDataSource 1'b0; WriteEnSource 1'b0; ResetValue 0; }
 Instance c Of Child { InputPort si = si; InputPort rst = release; }
 ScanOutPort so { Source c.so; }
}
"""
        self.replay(source, 'iReset; iState release 1; iWrite c.value 9; iApply; iRead c.value 9; iApply;\n')

    def test_output_state_remains_rejected(self):
        for suffix in ('', ' -LastReadValue', ' -LastMiscompareValue'):
            self.reject(BASE, f'iState q 9{suffix};\n')

    def test_wide_state_scale(self):
        width = 16384
        source = BASE.replace('[3:0]', f'[{width-1}:0]')
        body = 'iState value 0b' + '1'*width + ';\n'
        start = time.monotonic()
        self.replay(source, body)
        print(f'DataRegister state: {width} bits, {len(source)} ICL bytes, three replays {time.monotonic()-start:.3f}s')

    def reject(self, source, body):
        icl, pdl = self.root / 'source.icl', self.root / 'source.pdl'
        icl.write_text(source)
        pdl.write_text(body)
        (self.root / 'result.pdl').write_bytes(b'previous output\n')
        message = self.run_tool(icl, pdl, ok=False)
        self.assertEqual((self.root / 'result.pdl').read_bytes(), b'previous output\n')
        return message


if __name__ == '__main__':
    unittest.main()
