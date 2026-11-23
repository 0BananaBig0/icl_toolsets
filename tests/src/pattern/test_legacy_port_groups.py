#!/usr/bin/env python3
"""Legacy named port annotations preserve replay and never become data targets."""
import subprocess
import unittest

import test_simultaneous_scan_chains as cli


def annotation_network(count, segmented=False):
    ports = ("DataInPort bus[63:32]; DataInPort bus[31:0];" if segmented
             else "DataInPort bus[63:0];")
    groups = "".join(f"PortGroup g{i} {{bus[63:0]}}\n" for i in range(count))
    return cli.NETWORK.rsplit("}", 1)[0] + groups + ports + "}\n"


class LegacyPortGroups(unittest.TestCase):
    setUp = cli.SimultaneousChains.setUp
    run_tool = cli.SimultaneousChains.run_tool
    replay = cli.SimultaneousChains.replay

    def reject(self, source, body, diagnostic):
        icl, pdl = self.root / "bad.icl", self.root / "bad.pdl"
        icl.write_text(source)
        pdl.write_text("iPDLLevel 1\n" + body)
        output, flat = self.root / "result.pdl", self.root / "debug/Chains.icl"
        flat.parent.mkdir(parents=True, exist_ok=True)
        output.write_bytes(b"previous output\n")
        flat.write_bytes(b"previous flat\n")
        process = subprocess.run(
            [str(cli.BIN), "--icl", str(icl), "--pdl", str(pdl),
             "--top_module", "Chains", "--output", str(output), "--debug"],
            capture_output=True, text=True, timeout=45)
        message = process.stdout + process.stderr
        self.assertNotEqual(process.returncode, 0, message)
        self.assertIn(diagnostic, message)
        self.assertEqual(output.read_bytes(), b"previous output\n")
        self.assertEqual(flat.read_bytes(), b"previous flat\n")
        return message

    def test_inline_commented_duplicates_and_keywords(self):
        reference = self.replay()
        for members in ("si, so, sel, Source, LaunchEdge, si[2]",
                        "/* lead */ si, so /* after */, // between\n"
                        "sel, Source, LaunchEdge, si[2]"):
            with self.subTest(members=members):
                source = cli.NETWORK.rsplit("}", 1)[0] + (
                    "DataInPort Source; DataInPort LaunchEdge;\n"
                    "PortGroup annotation {" + members + "}\n}\n")
                self.assertEqual(self.replay(source), reference)
                self.assertEqual((self.root / "debug/Chains.icl").read_text()
                                 .count("PortGroup "), 1)

    def test_reversed_and_parameterized_member_ranges(self):
        source = annotation_network(1).replace(
            "DataInPort bus[63:0];",
            "Parameter Width = 64; DataInPort bus[0:$Width-1];")
        source = source.replace("{bus[63:0]}", "{bus[0:$Width-1], si[2:3], so[4:5]}")
        self.assertEqual(self.replay(source).count(b"iApply"), 3)

    def test_nested_repeated_instances_preserve_members(self):
        source = """Module Leaf {
 ScanInPort si;
 ScanRegister r[3:0] {ScanInSource si; CaptureSource r; ResetValue 4'b0;}
 ScanOutPort so {Source r[0]; LaunchEdge Falling;}
 PortGroup ports {si, so}
}
Module Wrapper {
 ScanInPort si; Instance child Of Leaf {InputPort si = si;}
 ScanOutPort so {Source child.so;} PortGroup ports {si, so}
}
Module Chains {
 ScanInPort si[1:0];
 Instance left Of Wrapper {InputPort si = si[1];}
 Instance right Of Wrapper {InputPort si = si[0];}
 ScanOutPort so[1:0] {Source left.so, right.so;}
 PortGroup ports {si, so}
}
"""
        body = """iPDLLevel 1
iReset
iWrite left.child.r 9; iWrite right.child.r 6; iApply -together
iRead left.child.r 9; iRead right.child.r 6; iApply -together
"""
        self.assertEqual(self.replay(source, body).count(b"iApply"), 2)
        self.assertEqual((self.root / "debug/Chains.icl").read_text()
                         .count("PortGroup "), 5)

    def test_segmented_functions_nonempty_replay(self):
        source = annotation_network(2, segmented=True)
        result = self.replay(source)
        self.assertEqual(result, self.replay())
        self.assertEqual(result.count(b"iApply"), 3)
        mixed = cli.NETWORK.rsplit("}", 1)[0] + (
            "DataInPort p[1]; DataOutPort p[0] {Source p[1];} "
            "PortGroup pins {p[0:1]}\n}\n")
        body = cli.PDL + "iWrite p[1] 1; iApply; iRead p[0] 1; iApply\n"
        self.assertEqual(self.replay(mixed, body).count(b"iApply"), 5)

    def test_annotation_does_not_drive_ports_or_change_ownership(self):
        source = cli.NETWORK.rsplit("}", 1)[0] + "PortGroup pins {si, so, sel}\n}\n"
        body = """iPDLLevel 1
iReset
iTake b; iWrite b 2; iApply
iWrite a 9; iApply; iRead b 2; iApply; iRelease b
"""
        self.assertEqual(self.replay(source, body), self.replay(cli.NETWORK, body))

    def test_legacy_sib_with_explicit_client(self):
        path = cli.ROOT / "tests/cases/pass/published/liu_sib_and_chip_icl.icl"
        source = path.read_text().split("Module Chip_ICL", 1)[0]
        source = source.replace("Module SIB {", "Module Chains {", 1)
        source = source.rsplit("}", 1)[0] + (
            "ScanInterface client {Port si; Port so; Port en;}\n}\n")
        body = "iPDLLevel 1\niReset; iWrite sr 1; iApply; iReset; iRead sr 0; iApply\n"
        self.assertEqual(self.replay(source, body).count(b"iApply"), 2)

    def test_invalid_members_preserve_prior_artifacts(self):
        for declaration, diagnostic in (
                ("PortGroup bad {missing}", "PortGroup bad"),
                ("PortGroup bad {a}", "PortGroup bad"),
                ("Alias alias[3:0] = a; PortGroup bad {alias}", "PortGroup bad"),
                ("PortGroup bad {si[99]}", "PortGroup bad"),
                ("PortGroup bad {si[$missing]}", "PortGroup bad")):
            with self.subTest(declaration=declaration):
                source = cli.NETWORK.rsplit("}", 1)[0] + declaration + "}\n"
                self.reject(source, cli.PDL, diagnostic)

        for name, top in (
                ("ours/module/PortGroup_declaration.icl", "M"),
                ("ours/module/portgroup_comment_before_comma.icl", "M"),
                ("ours/module/portgroup_keyword.icl", "M"),
                ("pass/ours/portgroup_comma_list.icl", "m"),
                ("pass/ours/portgroup_with_comments.icl", "m")):
            with self.subTest(corpus=name):
                source = (cli.ROOT / "tests/cases" / name).read_text()
                source = source.replace("Module " + top + " ", "Module Chains ", 1)
                self.reject(source, cli.PDL, "PortGroup")

    def test_group_is_not_a_pdl_access_or_reservation_target(self):
        source = cli.NETWORK.rsplit("}", 1)[0] + "PortGroup pins {si, so, sel}\n}\n"
        for command, diagnostic in (
                ("iWrite pins 0; iApply", "write target requires"),
                ("iRead pins 0; iApply", "reads require"),
                ("iTake pins", "resource")):
            with self.subTest(command=command):
                self.reject(source, command, diagnostic)


if __name__ == "__main__":
    unittest.main()
