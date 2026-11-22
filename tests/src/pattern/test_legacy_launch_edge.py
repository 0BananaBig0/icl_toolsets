#!/usr/bin/env python3
"""Preserve accepted legacy launch declarations without adding timing semantics."""
import subprocess
import unittest

import test_simultaneous_scan_chains as cli


class LegacyLaunchEdge(unittest.TestCase):
    setUp = cli.SimultaneousChains.setUp
    run_tool = cli.SimultaneousChains.run_tool
    replay = cli.SimultaneousChains.replay

    def test_vector_inverted_enabled_nonempty_replay(self):
        source = cli.NETWORK.replace("Source a[0], b[0];",
                                     "Source ~a[0], b[0]; Enable 1'b1; "
                                     "LaunchEdge /* legacy */ Falling;")
        result = self.replay(source, cli.PDL)
        self.assertEqual(result.count(b"iApply"), 3)
        flat = (self.root / "debug/Chains.icl").read_text()
        self.assertIn("LaunchEdge Falling;", flat)

    def test_repeated_children_nonempty_replay(self):
        leaf = """Module Leaf {
 ScanInPort si;
 ScanRegister r[3:0] {ScanInSource si; CaptureSource r; ResetValue 4'b0;}
 ScanOutPort so {Source r[0]; LaunchEdge Falling;}
}
"""
        source = leaf + """Module Chains {
 ScanInPort si[1:0];
 Instance left Of Leaf {InputPort si = si[1];}
 Instance right Of Leaf {InputPort si = si[0];}
 ScanOutPort so[1:0] {Source left.so, right.so; LaunchEdge Falling;}
}
"""
        body = """iPDLLevel 1
iReset
iWrite left.r 9; iWrite right.r 6; iApply -together
iRead left.r 9; iRead right.r 6; iApply -together
"""
        result = self.replay(source, body)
        self.assertEqual(result.count(b"iApply"), 2)
        self.assertEqual((self.root / "debug/Chains.icl").read_text()
                         .count("LaunchEdge Falling;"), 3)

    def test_invalid_value_and_placement_preserve_previous_output(self):
        for declaration in (
                "ScanOutPort out {Source si[3]; LaunchEdge Rising;}",
                "ScanOutPort out {Source si[3]; LaunchEdge falling;}",
                "DataOutPort out {Source a; LaunchEdge Falling;}",
                "ScanInPort extra {LaunchEdge Falling;}"):
            with self.subTest(declaration=declaration):
                icl, pdl = self.root / "bad.icl", self.root / "bad.pdl"
                output = self.root / "result.pdl"
                icl.write_text(cli.NETWORK.replace("  ScanInPort",
                               "  " + declaration + "\n  ScanInPort", 1))
                pdl.write_text(cli.PDL)
                output.write_bytes(b"previous result\n")
                process = subprocess.run(
                    [str(cli.BIN), "--icl", str(icl), "--pdl", str(pdl),
                     "--top_module", "Chains", "--output", str(output)],
                    capture_output=True, text=True, timeout=45)
                self.assertNotEqual(process.returncode, 0)
                self.assertIn("LaunchEdge", process.stdout + process.stderr)
                self.assertEqual(output.read_bytes(), b"previous result\n")


if __name__ == "__main__":
    unittest.main()
