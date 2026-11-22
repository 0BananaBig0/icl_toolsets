#!/usr/bin/env python3
"""Canonical default control outputs and source-located enum diagnostics."""
import subprocess
import unittest

import test_simultaneous_scan_chains as cli

MODEL = cli.NETWORK


class ModelSourceAudit(unittest.TestCase):
    setUp = cli.SimultaneousChains.setUp
    run_tool = cli.SimultaneousChains.run_tool
    replay = cli.SimultaneousChains.replay

    def reject(self, source, diagnostic, top="Chains"):
        icl, pdl, output = (self.root / name for name in
                            ("invalid.icl", "invalid.pdl", "result.pdl"))
        icl.write_text(source)
        pdl.write_text("iPDLLevel 1; iReset\n")
        output.write_bytes(b"previous artifact\n")
        process = subprocess.run(
            [str(cli.BIN), "--icl", str(icl), "--pdl", str(pdl),
             "--top_module", top, "--output", str(output), "--debug"],
            capture_output=True, text=True, timeout=45)
        self.assertNotEqual(process.returncode, 0)
        self.assertIn(diagnostic, process.stdout + process.stderr)
        self.assertIn(str(icl), process.stdout + process.stderr)
        self.assertEqual(output.read_bytes(), b"previous artifact\n")
        return process.stdout + process.stderr

    def test_root_default_control_outputs_nonempty_replay(self):
        for kind in ("ToSelectPort", "ToCaptureEnPort", "ToShiftEnPort",
                     "ToUpdateEnPort", "ToTCKPort", "ToResetPort",
                     "ToClockPort"):
            with self.subTest(kind=kind):
                clock = "ClockPort clock; " if kind == "ToClockPort" else ""
                source = MODEL.replace(
                    "  ScanInPort", f"  {clock}{kind} unused;\n  ScanInPort", 1)
                output = self.replay(source, cli.PDL)
                self.assertEqual(output.count(b"iApply"), 3)
                self.assertIn(b"iWrite a", output)

    def test_child_default_output_retains_scope(self):
        source = ("Module Defaults {TCKPort tck; "
                  "ToSelectPort out; ToCaptureEnPort capture;}\n" +
                  MODEL.replace("  ScanInPort", "  Instance child Of Defaults "
                                "{InputPort tck = ck;} \n  ScanInPort", 1))
        output = self.replay(source, cli.PDL)
        self.assertEqual(output.count(b"iApply"), 3)

    def test_nonempty_attributes_and_sources_are_retained(self):
        for body in ("Attribute marker;", "Source sel;"):
            source = MODEL.replace("  ScanInPort",
                                   "  ToSelectPort out {" + body +
                                   "}\n  ScanInPort", 1)
            output = self.replay(source, cli.PDL)
            self.assertEqual(output.count(b"iApply"), 3)
            flat = (self.root / "debug/Chains.icl").read_text()
            self.assertIn("marker" if "marker" in body else "Source", flat)

    def test_missing_enum_has_context_for_each_owner(self):
        declarations = (
            "DataInPort bad {RefEnum Missing;}",
            "Alias bad = a {RefEnum Missing;}",
            "ScanRegister bad {ScanInSource si[3]; RefEnum Missing;}",
            "DataRegister bad {RefEnum Missing; ResetValue 0;}",
            "DataInPort bad[3:2] {RefEnum Missing;} DataInPort bad[1:0];",
        )
        for declaration in declarations:
            with self.subTest(declaration=declaration):
                message = self.reject(
                    MODEL.replace("  ScanInPort",
                                  "  " + declaration + "\n  ScanInPort", 1),
                    "Missing")
                self.assertIn("RefEnum", message)
                self.assertIn("bad", message)
                self.assertNotIn("map::at", message)

    def test_scoped_forward_enum_with_parameterized_defaults(self):
        source = """Module Leaf {
 Parameter initial = 4'b0000;
 ScanInPort si;
 ScanRegister r[3:0] {
  ScanInSource si; CaptureSource r; RefEnum E;
  ResetValue closed; DefaultLoadValue closed;
 }
 ScanOutPort so {Source r[0];}
 Enum E {closed = $initial; open = 4'b1111;}
}
Module Chains {
 ScanInPort si[1:0];
 Instance left Of Leaf {InputPort si = si[1];}
 Instance right Of Leaf {InputPort si = si[0]; Parameter initial = 4'b0101;}
 ScanOutPort so[1:0] {Source left.so, right.so;}
}
"""
        output = self.replay(source, """iPDLLevel 1
iReset; iRead left.r 0; iRead right.r 5; iApply -together
iWrite left.r 9; iWrite right.r 6; iApply -together
iRead left.r 9; iRead right.r 6; iApply -together
""")
        self.assertEqual(output.count(b"iApply"), 3)

    def test_existing_wrong_kind_and_width_guards(self):
        for declaration, diagnostic in (
                ("DataInPort E; ScanRegister bad {ScanInSource si[3]; RefEnum E;}",
                 "RefEnum does not name an Enum"),
                ("Enum E {entry = 1'b0;} ScanRegister bad[3:0] "
                 "{ScanInSource si[3]; RefEnum E;}", "enum value width")):
            self.reject(MODEL.replace("  ScanInPort",
                                      "  " + declaration + "\n  ScanInPort", 1),
                        diagnostic)

    def test_corpus_bare_control_roundtrips(self):
        # Native batch audit covers the other eight models beyond ordinary scheduling.
        fixtures = tuple(("ours/to_select_port/" + name + ".icl", "Example")
                         for name in ("bare", "keywords", "range", "range_expression"))
        for name, top in fixtures:
            path = cli.ROOT / "tests/cases" / name
            self.assertTrue(path.is_file(), str(path))
            source = path.read_text().replace("Module " + top, "Module Chains", 1)
            output = self.replay(source, "iPDLLevel 1\niNote {checked corpus default}\n")
            self.assertIn(b"checked corpus default", output)

    def test_corpus_missing_enum_errors_are_named(self):
        for name, top in (
                ("ours/alias/all_items.icl", "M"),
                ("ours/data_in_port/enum.icl", "Example"),
                ("ours/scan_register/ref_enum.icl", "M"),
                ("pass/ours/instrument_with_refenum.icl", "Instrument")):
            source = (cli.ROOT / "tests/cases" / name).read_text()
            message = self.reject(source, "RefEnum", top)
            self.assertNotIn("map::at", message)


if __name__ == "__main__":
    unittest.main()
