#!/usr/bin/env python3
"""ICL source literals obey sizing rules before contextual expansion."""
import unittest

import test_simultaneous_scan_chains as helpers

BASE = """Module Chains {
 ScanInPort si;
 ScanRegister r[3:0] { ScanInSource si; CaptureSource r; ResetValue 0; }
 ScanOutPort so { Source r[0]; }
}
"""


class SourceLiterals(unittest.TestCase):
    setUp = helpers.SimultaneousChains.setUp
    run_tool = helpers.SimultaneousChains.run_tool
    replay = helpers.SimultaneousChains.replay

    def reject(self, source):
        icl, pdl = self.root / 'source.icl', self.root / 'source.pdl'
        icl.write_text(source)
        pdl.write_text('iReset;\n')
        (self.root / 'result.pdl').write_bytes(b'previous output\n')
        error = self.run_tool(icl, pdl, ok=False)
        self.assertIn('unsized', error.lower())
        self.assertEqual((self.root / 'result.pdl').read_bytes(), b'previous output\n')

    def test_unsized_register_constants(self):
        for property_name in ('ResetValue', 'DefaultLoadValue'):
            for value in ('~0', "~'h0", "~'bx"):
                with self.subTest(property_name=property_name, value=value):
                    self.reject(BASE.replace('ResetValue 0;', f'{property_name} {value};'))

    def test_unsized_signal_sources(self):
        for value in ('~0', "~'h0", "~'bx"):
            for statement in (f'DataOutPort q[3:0] {{ Source {value}; }}',
                              f'DataRegister d[3:0] {{ WriteDataSource {value}; }}'):
                with self.subTest(value=value, statement=statement):
                    self.reject(BASE.replace(' ScanInPort si;', ' '+statement+'\n ScanInPort si;'))
            self.reject(BASE.replace('CaptureSource r;', f'CaptureSource {value};'))

    def test_unsized_mux_branch_and_input_binding(self):
        self.reject(BASE.replace(' ScanInPort si;', """ DataInPort choose;
 DataMux mux[3:0] SelectedBy choose { 1'b0: ~0; 1'b1: 4'hf; }
 ScanInPort si;"""))
        child = 'Module Child { DataInPort d[3:0]; }\n'
        self.reject(child + BASE.replace(' ScanInPort si;', ' Instance c Of Child { InputPort d = ~0; }\n ScanInPort si;'))

    def test_unsized_enum_and_parameter_substitution(self):
        self.reject(BASE.replace(' ScanInPort si;', " Enum E { bad = ~'h0; }\n ScanInPort si;"))
        self.reject(BASE.replace(' ScanInPort si;', ' Parameter P = 0;\n ScanInPort si;')
                    .replace('ResetValue 0;', 'ResetValue ~$P;'))
        self.reject(BASE.replace(' ScanInPort si;', ' Parameter P = 0;\n ScanInPort si;')
                    .replace('CaptureSource r;', 'CaptureSource ~$P;'))

    def test_double_inversion_does_not_hide_unsized_operand(self):
        self.reject(BASE.replace(' ScanInPort si;', " DataRegister d { WriteDataSource 1'b1; WriteEnSource ~~0; }\n ScanInPort si;"))
        for source in ("DataRegister d { WriteDataSource 1'b1; WriteEnSource ~~1'b1; }",
                       "DataInPort enable; DataRegister d { WriteDataSource 1'b1; WriteEnSource ~~enable; }"):
            self.replay(BASE.replace(' ScanInPort si;', source+'\n ScanInPort si;'), 'iReset;\n')

    def test_normative_rule_over_conflicting_example(self):
        source = BASE.replace('r[3:0]', 'r[9:0]').replace('ResetValue 0;', "ResetValue 1'b0, ~0, 5'h1a;")
        self.reject(source)

    def test_sized_inversion_and_legal_padding(self):
        for value, expected in (("~4'h0", 15), ("1'b1, 0, 1'b1", 9), ("~4'hf", 0)):
            with self.subTest(value=value):
                source = BASE.replace('ResetValue 0;', f'ResetValue {value};')
                self.replay(source, f'iReset; iRead r {expected}; iApply;\n')
        source = BASE.replace(' ScanInPort si;', ' Parameter P = 4\'h0;\n ScanInPort si;')
        source = source.replace('ResetValue 0;', 'ResetValue ~$P;')
        self.replay(source, 'iReset; iRead r 15; iApply;\n')

    def test_sized_and_signal_inversion_sources(self):
        source = BASE.replace('CaptureSource r;', "CaptureSource ~4'h0;")
        self.replay(source, 'iReset; iRead r 15; iApply;\n')
        source = BASE.replace('CaptureSource r;', 'CaptureSource ~r;')
        self.replay(source, 'iReset; iRead r 15; iApply;\n')


if __name__ == '__main__':
    unittest.main()
