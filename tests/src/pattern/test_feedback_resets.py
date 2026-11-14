#!/usr/bin/env python3
"""Register-reset feedback preserves sequential snapshot semantics."""
import time
import unittest

import test_simultaneous_scan_chains as helpers
import test_data_reset_releases as releases

LEAF = """Module Cell {
 ScanInPort si; ResetPort rst;
 ScanRegister value[1:0] { ScanInSource si; CaptureSource value; ResetValue 0; }
 ScanOutPort so { Source value[0]; } DataOutPort q[1:0] { Source value; }
}
"""
MUTUAL = LEAF + """Module Chains {
 ScanInPort si; ResetPort global;
 DataMux reset_a SelectedBy b.q[0] { 1'b0: global; 1'b1: 1'b1; }
 DataMux reset_b SelectedBy a.q[0] { 1'b0: global; 1'b1: 1'b1; }
 Instance a Of Cell { InputPort si=si; InputPort rst=reset_a; }
 Instance b Of Cell { InputPort si=a.so; InputPort rst=reset_b; }
 ScanOutPort so { Source b.so; }
}
"""


class FeedbackResets(unittest.TestCase):
    setUp = helpers.SimultaneousChains.setUp
    run_tool = helpers.SimultaneousChains.run_tool
    replay = helpers.SimultaneousChains.replay
    rewrite = releases.DataResetReleases.rewrite

    def test_mutual_scan_reset(self):
        self.replay(MUTUAL, 'iReset; iState a.value 0; iState b.value 0; '
                    'iWrite a.value 2; iWrite b.value 2; iApply; '
                    'iRead a.value 2; iRead b.value 2; iApply;\n')

    def test_capture_reset_then_write(self):
        for together in ('', ' -together'):
            self.replay(MUTUAL, 'iReset; iState a.value 1; iState b.value 1; '
                        'iWrite a.value 2; iWrite b.value 2; iApply' + together + '; '
                        'iRead a.value 2; iRead b.value 2; iApply;\n')

    def test_mutual_reset_rewrite_replays(self):
        reset = self.rewrite(MUTUAL, 'iReset; iState a.value 0; iState b.value 0; '
                             'iWrite a.value 2; iWrite b.value 2; iApply;\n')
        self.assertIn('iWrite a.value 2', reset)

    def test_self_reset(self):
        source = LEAF + """Module Chains {
 ScanInPort si; ResetPort global;
 DataMux reset_a SelectedBy a.q[0] { 1'b0: global; 1'b1: 1'b1; }
 Instance a Of Cell { InputPort si=si; InputPort rst=reset_a; }
 ScanOutPort so { Source a.so; }
}
"""
        self.replay(source, 'iReset; iState a.value 0; iWrite a.value 2; iApply; '
                    'iRead a.value 2; iApply;\n')

    def test_mutual_zero_reset_snapshot(self):
        self.replay(MUTUAL, 'iReset; iState a.value 1; iState b.value 1; '
                    'iRead a.value 0; iRead b.value 0; iApply;\n')

    def test_unknown_feedback_does_not_invent_known_state(self):
        self.replay(MUTUAL, 'iReset; iState a.value 1; iState b.value 0b0x; '
                    'iRead a.value 0b0x; iRead b.value 0; iApply;\n')

    def test_mixed_storage_feedback(self):
        source = LEAF + """Module Data {
 ResetPort rst;
 DataRegister value { WriteDataSource 1'b0; WriteEnSource 1'b0; ResetValue 0; }
 DataOutPort q { Source value; }
}
Module Chains {
 ScanInPort si; ResetPort global;
 DataMux reset_a SelectedBy b.q { 1'b0: global; 1'b1: 1'b1; }
 DataMux reset_b SelectedBy a.q[0] { 1'b0: global; 1'b1: 1'b1; }
 Instance a Of Cell { InputPort si=si; InputPort rst=reset_a; }
 Instance b Of Data { InputPort rst=reset_b; }
 ScanOutPort so { Source a.so; }
}
"""
        self.replay(source, 'iReset; iState a.value 0; iState b.value 0; '
                    'iWrite a.value 2; iApply; iRead a.value 2; iApply;\n')

    def test_feedback_ring_scale(self):
        for count in (8, 32):
            source = LEAF + ('Module GlobalReset { ToResetPort reset; }\n'
                             'Module Chains { ScanInPort si; Instance global Of GlobalReset;\n')
            state = 'iReset; '
            for index in range(count):
                previous = 'si' if index == 0 else f'c{index-1}.so'
                source += (f"DataMux reset_{index} SelectedBy c{(index+1)%count}.q[0] {{ "
                           "1'b0: global.reset; 1'b1: 1'b1; }\n")
                source += (f'Instance c{index} Of Cell {{ InputPort si={previous}; '
                           f'InputPort rst=reset_{index}; }}\n')
                state += f'iState c{index}.value 0; '
            source += f'ScanOutPort so {{ Source c{count-1}.so; }} }}\n'
            start = time.monotonic()
            self.replay(source, state + f'iWrite c{count-1}.value 2; iApply; '
                        f'iRead c{count-1}.value 2; iApply;\n')
            print(f'feedback ring: {count} registers, {len(source)} ICL bytes, '
                  f'three replays {time.monotonic()-start:.3f}s')

    def test_reset_mux_known_under_unknown_selector(self):
        source = (helpers.ROOT / 'tests/cases/ours/pattern_generation/feedback_reset.icl').read_text()
        source = source.replace('Module FeedbackReset {', 'Module Chains {')
        output = self.replay(source, 'iReset; iWrite c.value 2; iRead c.value 0; '
                             'iApply; iRead c.value 2; iApply;\n')
        self.assertEqual(output.count(b'iApply'), 2)

    def test_impossible_mutual_release_is_atomic(self):
        source = MUTUAL.replace('ResetPort rst;', 'ResetPort rst { ActivePolarity 0; }')
        source = source.replace('ResetPort global;', 'ResetPort global { ActivePolarity 0; }')
        source = source.replace("1'b0: global; 1'b1: 1'b1;", "1'b0: 1'b0; 1'b1: global;")
        message = self.rewrite(source, 'iReset; iState a.value 0; iState b.value 0; '
                               'iWrite a.value 1; iWrite b.value 1; iApply;\n', ok=False)
        self.assertNotIn('feedback reset scheduling', message)
        self.assertIn('reset', message)


if __name__ == '__main__':
    unittest.main()
