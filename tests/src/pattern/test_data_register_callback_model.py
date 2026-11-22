#!/usr/bin/env python3
"""Callback declarations round-trip without inventing procedure execution."""
import unittest

import test_simultaneous_scan_chains as helpers

MODEL = """Module Leaf {
 Parameter owner = "Chains";
 Parameter proc = "save";
 Parameter argument = "a; [b] $c";
 DataRegister value[3:0] {
  WriteCallBack $owner::api $proc <D> <R> "<D>" "<R>" $argument
      4'b10x1 (2 + 3) * 4 7;
  ResetValue 4'b0;
  DefaultLoadValue 4'b0;
 }
 DataRegister observed[3:0] {
  ReadCallBack ::Chains::api load <D> <R>;
  ReadDataSource value;
 }
}
Module Chains {
 Instance first Of Leaf;
 Instance second Of Leaf { Parameter argument = "second"; }
}
"""

class CallbackModel(unittest.TestCase):
    setUp = helpers.SimultaneousChains.setUp
    run_tool = helpers.SimultaneousChains.run_tool
    replay = helpers.SimultaneousChains.replay

    def test_parameterized_nested_callback_model(self):
        output = self.replay(MODEL, "iNote {callback model only}\n")
        self.assertIn(b"callback model only", output)
        flat = (self.root / "debug/Chains.icl").read_text()
        self.assertIn("WriteCallBack Chains::api save <D> <R>", flat)
        self.assertIn("ReadCallBack ::Chains::api load <D> <R>", flat)
        self.assertIn('"<D>" "<R>" "a; [b] $c"', flat)

    def test_namespace_forms_and_no_argument_procedures(self):
        for owner in ("Chains", "::Chains", "Chains::api", "::Chains::api",
                      "vendor::Chains", "vendor::Chains::api"):
            source = "Module Chains {DataRegister r {WriteCallBack " + owner + " save;}}"
            self.replay(source, "iNote {no callback access}\n")

    def test_unknown_numeric_and_string_argument_values(self):
        for value in ("4'bxxxx", "'hxx", "0", "80'h10000000000000000000",
                      '"quotes \\\" and slash \\\\"', "$argument"):
            with self.subTest(value=value):
                # Keep an unsized based literal away from a parameter width.
                source = MODEL.replace("$argument\n      4'b10x1",
                                       "<D> " + value)
                self.replay(source, "iNote {literal callback arguments}\n")

    def test_many_specialized_callbacks(self):
        source = MODEL.split("Module Chains {", 1)[0] + "Module Chains {" + "".join(
            'Instance child' + str(index) + ' Of Leaf {Parameter argument = "'
            + str(index) + '";}' for index in range(64)) + "}"
        self.replay(source, "iNote {many callback declarations}\n")

    def test_invalid_resolved_names_preserve_existing_output(self):
        for value in ('"two names"', '"M::sub"', '"[iReset]"', "4", '""'):
            for variable in ("owner", "proc"):
                source = MODEL.replace('Parameter ' + variable + ' = "' +
                                       ("Chains" if variable == "owner" else "save") + '";',
                                       "Parameter " + variable + " = " + value + ";")
                icl, pdl = self.root / "bad.icl", self.root / "bad.pdl"
                icl.write_text(source)
                pdl.write_text("iNote unused\n")
                output = self.root / "result.pdl"
                output.write_bytes(b"keep existing artifact\n")
                message = self.run_tool(icl, pdl, ok=False)
                self.assertIn("callback identifier", message)
                self.assertEqual(output.read_bytes(), b"keep existing artifact\n")

    def test_applied_callbacks_execute_errors_without_publishing_artifacts(self):
        source = """Module Chains {
 DataRegister value[3:0] {ReadCallBack Chains observe <D>;}
 DataRegister writable[3:0] {WriteCallBack Chains store <D>;}
 Alias observed[3:0] = value;
 Alias written[3:0] = writable;
 DataOutPort q[3:0] {Source value;}
}"""
        for command, target in (("iRead", "value"), ("iRead", "observed[2:1]"),
                                ("iWrite", "writable"), ("iWrite", "written[2:1]")):
            icl, pdl = self.root / "callback.icl", self.root / "callback.pdl"
            icl.write_text(source)
            pdl.write_text("iPDLLevel 1; iProcsForModule Chains; "
                           "iProc observe {args} {error {callback deliberately fails}}; "
                           "iProc store {args} {error {callback deliberately fails}}; "
                           "iState value 0; " + command + " " + target + " 0; iApply;\n")
            output = self.root / "result.pdl"
            output.write_bytes(b"keep callback artifact\n")
            message = self.run_tool(icl, pdl, ok=False)
            self.assertIn("callback deliberately fails", message)
            self.assertEqual(output.read_bytes(), b"keep callback artifact\n")
        self.replay(source, "iWrite writable 3; iRead value 0; iReset; "
                    "iState value 0; iRead q 0; iApply;\n")
        self.replay(source, "iState value 3 -LastReadValue; "
                    "if {[iGetReadData value] ne {0b0011}} {error history}; "
                    "iNote {history is not a procedural access};\n")

    def test_type_mixtures_stay_invalid(self):
        for property_text in ("ReadDataSource 4'b0;", "WriteDataSource 4'b0;",
                              "WriteEnSource 1'b1;", "ReadCallBack Chains load;"):
            source = "Module Chains {DataRegister r[3:0] {WriteCallBack Chains save;" + property_text + "}}"
            icl, pdl = self.root / "bad.icl", self.root / "bad.pdl"
            icl.write_text(source)
            pdl.write_text("iNote unused\n")
            self.assertIn("mutually exclusive", self.run_tool(icl, pdl, ok=False))


if __name__ == "__main__":
    unittest.main()
