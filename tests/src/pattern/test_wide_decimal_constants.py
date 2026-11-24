#!/usr/bin/env python3
"""Wide decimal bit patterns retain actual observations and checked integer limits."""
import os
import unittest

import test_modeled_readback as history
import test_callback_endpoint_readback as endpoints

HEX_CONTROL = os.environ.get('ICL_DECIMAL_CONTROL') == 'hex'


def literal(width, value):
    return (str(width)+"'h"+format(value, 'x') if HEX_CONTROL
            else str(width)+"'d"+str(value))


class WideDecimalConstants(unittest.TestCase):
    setUp = history.ModeledReadback.setUp
    run_tool = history.ModeledReadback.run_tool
    replay = history.ModeledReadback.replay
    notes = endpoints.CallbackEndpointReadback.notes

    def observe(self, model, width, value, target='q'):
        output = self.replay(model, 'iPDLLevel 1; iRead '+target+''' 0; iApply
iNote [iGetReadData '''+target+''' -bin]; iNote [iGetStatus]
''')
        self.assertEqual(self.notes(output), [('0b'+format(value, '0%db'%width)).encode(),
                                               str(value.bit_count()).encode()])
        return output

    def rejected(self, model, diagnostic):
        source, pdl, output = (self.root/n for n in ('source.icl','source.pdl','result.pdl'))
        source.write_text(model); pdl.write_text('iPDLLevel 1; iReset')
        output.write_bytes(b'retained')
        error = self.run_tool(source, pdl, ok=False)
        self.assertIn(diagnostic, error)
        self.assertEqual(output.read_bytes(), b'retained')

    def test_parameters_and_defaults(self):
        for width in (64,65,128,256):
            with self.subTest(width=width):
                value=(1<<width)-1
                model=f'''Module Chains {{
 Parameter V={literal(width,value)};
 DataInPort p[{width-1}:0] {{DefaultLoadValue $V;}}
 DataOutPort q[{width-1}:0] {{Source $V;}}
}}'''
                self.observe(model,width,value)
                self.assertIn("DefaultLoadValue "+str(width)+"'b"+('1'*width),
                              (self.root/'debug/Chains.icl').read_text())

    def test_parameter_width_and_instance_override(self):
        value=(1<<127)+9
        default=literal(128,(1<<128)-1)
        if not HEX_CONTROL: default=default.replace("128'd", "$W'd")
        model=f'''Module Leaf {{
 Parameter W=128; Parameter V={default};
 DataOutPort q[$W-1:0] {{Source $V;}}
}}
Module Chains {{
 Instance child Of Leaf {{Parameter V={literal(128,value)};}}
 DataOutPort q[127:0] {{Source child.q;}}
}}'''
        self.observe(model,128,value)

    def test_wide_enums_reset_and_capture(self):
        high=(1<<96)-1; mixed=(1<<95)+1
        model=f'''Module Chains {{
 ScanInPort si; ScanOutPort so {{Source r[0];}}
 SelectPort sel; CaptureEnPort ce; ShiftEnPort se; UpdateEnPort ue; TCKPort ck;
 Enum Codes {{maximum={literal(96,high)}; captured={literal(96,mixed)};}}
 ScanRegister r[95:0] {{ScanInSource si; RefEnum Codes;
 ResetValue maximum; CaptureSource captured;}}
 DataOutPort q[95:0] {{Source r;}}
 ScanInterface client {{Port si; Port so; Port sel; Port ce; Port se; Port ue; Port ck;}}
}}'''
        output=self.replay(model,'''iPDLLevel 1; iReset
iRead q 0; iApply; iNote [iGetReadData q -bin]
iRead r 0; iApply; iNote [iGetReadData r -bin]
''')
        self.assertEqual(self.notes(output), [('0b'+format(high,'096b')).encode(),
                                               ('0b'+format(mixed,'096b')).encode()])

    def test_attributes_concat_inversion(self):
        value=(1<<96)-1
        model=f'''Module Chains {{
 Attribute wide={literal(128,(1<<128)-1)};
 DataOutPort q[95:0] {{Source ~{literal(32,0)}, {literal(64,(1<<64)-1)};}}
}}'''
        self.observe(model,96,value)
        self.assertIn('wide', (self.root/'debug/Chains.icl').read_text())

    def test_decimal_comments_underscores_unsized(self):
        value=1<<64
        text=literal(128,value)
        if not HEX_CONTROL:
            text="128'D /* chunk boundaries */ 000_18_446_744_073_709_551_616"
        self.observe('Module Chains {DataOutPort q[127:0] {Source '+text+';}}',128,value)
        unsized=("'h"+format(value,'x')) if HEX_CONTROL else str(value)
        self.observe('Module Chains {DataOutPort q[127:0] {Source '+unsized+';}}',128,value)

    def test_sparse_inverted_values(self):
        value=(1<<127)+0x123456789abc
        model=f'''Module Chains {{
 DataOutPort q[127:0] {{Source {literal(128,value)};}}
 Alias inverse[127:0]=~q;
}}'''
        output=self.replay(model,'''iPDLLevel 1; iRead q[127:124] 0; iApply
iNote [iGetReadData q[127:124] -dec]
iNote [iGetReadData inverse[127:124] -dec]; iNote [iGetStatus]
''')
        self.assertEqual(self.notes(output), [b'8',b'7',b'1'])

    def test_integer_and_size_limits(self):
        self.rejected('Module Chains {Parameter V='+literal(128,(1<<128)-1)+
            '; DataInPort p[$V:0];}', '64-bit limit')
        self.rejected('Module Chains {Parameter V=9223372036854775807+1;}','64-bit limit')
        self.rejected("Module Chains {Attribute huge=1048577'd1;}",'width exceeds')

    def test_atomic_rejection_and_existing_unknowns(self):
        self.rejected("Module Chains {DataOutPort q[63:0] {Source 64'd18446744073709551616;}}",
                      'significant')
        for text in ("128'bx", "128'hx"):
            output=self.replay('Module Chains {DataOutPort q[127:0] {Source '+text+';}}',
                '''iPDLLevel 1; iRead q 0; iApply; iNote [iGetReadData q -bin]
if {![catch {iGetStatus}]} {error known_unknown_status}
iNote unknown
''')
            self.assertEqual(self.notes(output), [b'0b'+b'x'*128,b'unknown'])


if __name__ == '__main__':
    unittest.main()
