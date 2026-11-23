#!/usr/bin/env python3
"""Goal scheduling retains callback obligations; abstract observations stay unknown."""
import shutil
import subprocess
import unittest

import test_simultaneous_scan_chains as cli

DECL = "DataRegister value[3:0] {WriteCallBack Chains store <D> <R>;}"
BASE = "Module Chains {DataInPort p[3:0];" + DECL + "}"
PROCS = "iProc store {data reg} {iWrite p $data}\n"


class PrimaryCallbackState(unittest.TestCase):
    setUp = cli.SimultaneousChains.setUp

    def tool(self, model, commands, output, modeled=False, ok=True):
        args = [str(cli.BIN), "--icl", str(model), "--pdl", str(commands),
                "--top_module", "Chains", "--output", str(self.root / output), "--debug"]
        if modeled:
            args.append("--model_readback")
        result = subprocess.run(args, capture_output=True, text=True, timeout=45)
        self.assertEqual(result.returncode == 0, ok, result.stdout + result.stderr)
        return (self.root / output).read_bytes() if ok else result.stdout + result.stderr

    def files(self, source, body):
        # Keep eligible unused storage alongside each retained-state case.
        padding = "".join(
            "DataRegister spare%d {WriteCallBack Chains store <D> <R>;}" % i
            for i in range(32))
        source = source.replace("Module Chains {", "Module Chains {" + padding, 1)
        model, commands = self.root / "source.icl", self.root / "source.pdl"
        model.write_text(source)
        commands.write_text("iPDLLevel 1\niProcsForModule Chains\n" + PROCS + body)
        return model, commands

    def check(self, source, body, modeled=False):
        model, commands = self.files(source, body)
        result = self.tool(model, commands, "result.pdl", modeled)
        self.assertEqual(result, self.tool(
            self.root / "debug/Chains.icl", commands, "flat/result.pdl", modeled))
        self.assertEqual(result, self.tool(
            model, self.root / "result.pdl", "generated/result.pdl", modeled))
        return result

    def test_fresh_callback_postconditions_between_applies(self):
        output = self.check(BASE, """iState value 0
iWrite value 9; iApply; iWrite p 6; iApply
iWrite value 2; iApply; iWrite p 5; iApply
""")
        self.assertIn(b"iState value 0b1001", output)
        self.assertIn(b"iState value 0b0010", output)
        self.assertIn(b"iWrite p 0b0101", output)

    def test_alias_reference_preserves_goals_with_unknown_observations(self):
        for alias_source in ("value", "nested"):
            aliases = "Alias nested[3:0]=value;" if alias_source == "nested" else ""
            source = BASE[:-1] + aliases + """
Alias view[3:0] = """ + alias_source + """;
DataOutPort q[3:0] {Source view;}
}"""
            output = self.check(source, """iState value 9
iWrite p 3; iApply; iRead q 6; iApply
iNote "[iGetReadData q] [iGetMiscompares q]"
if {![catch {iGetStatus}]} {error {abstract alias produced known status}}
iState value 0b1xx0; iWrite p 4; iApply; iRead q; iApply
iNote [iGetReadData q]
    """, modeled=True)
            self.assertIn(b"0bxxxx 0bxxxx", output)
            self.assertIn(b"iRead q 0b0110", output)
            self.assertIn(b"iNote 0bxxxx", output)

    def test_mux_and_logic_references_retain_callback_values(self):
        source = BASE[:-1] + """
DataInPort pick;
DataMux selected[3:0] SelectedBy pick {1'b0: value; 1'b1: ~value;}
LogicSignal parity {^value;}
DataOutPort q[3:0] {Source selected;}
DataOutPort bit {Source parity;}
}"""
        output = self.check(source, """iState value 9; iState pick 1
iWrite p 3; iApply; iRead q 6; iRead bit 0; iApply
iNote "[iGetReadData q -dec] [iGetReadData bit -dec]"
iState value 7; iRead bit 1; iApply; iNote [iGetReadData bit -dec]
""", modeled=True)
        self.assertIn(b"iNote {X X}", output)
        self.assertIn(b"iRead q 0b0110", output)
        self.assertIn(b"iNote X", output)

    def test_one_hot_reference_retains_callback_values(self):
        source = """Module Leaf {
DataInPort d[3:0]; DataInPort en;
DataOutPort q[3:0] {Source d; Enable en;}
}
""" + BASE[:-1] + """
DataInPort pick;
Instance first Of Leaf {InputPort d = value; InputPort en = pick;}
Instance second Of Leaf {InputPort d = ~value; InputPort en = ~pick;}
OneHotDataGroup selected[3:0] {Port first.q; Port second.q;}
DataOutPort q[3:0] {Source selected;}
}"""
        output = self.check(source, """iState value 9; iState pick 0
iWrite p 3; iApply; iRead q 6; iApply; iNote [iGetReadData q -dec]
iState pick 1; iRead q 9; iApply; iNote [iGetReadData q -dec]
""", modeled=True)
        self.assertEqual(output.count(b"iNote X\n"), 2)
        self.assertIn(b"iRead q 0b0110", output)
        self.assertIn(b"iRead q 0b1001", output)

    def test_resettable_callback_storage_retains_reset_value(self):
        source = BASE.replace("WriteCallBack", "ResetValue 0; WriteCallBack")
        source = source[:-1] + "DataOutPort q[3:0] {Source value;}}"
        output = self.check(source, """iState value 9; iReset
iWrite p 3; iApply; iRead q 0; iApply; iNote [iGetReadData q -dec]
""", modeled=True)
        self.assertIn(b"iNote X", output)
        self.assertIn(b"iRead q 0b0000", output)

    def test_callback_state_feeding_child_reset_is_retained(self):
        source = """Module Leaf {
DataInPort d[3:0]; ResetPort reset;
DataRegister storage[3:0] {WriteDataSource d; ResetValue 0;}
DataOutPort q[3:0] {Source storage;}
}
Module Chains {
DataInPort p[3:0]; ResetPort global;
DataRegister gate {WriteCallBack Chains store <D> <R>;}
DataMux reset_gate SelectedBy gate {1'b0: global; 1'b1: 1'b1;}
Instance child Of Leaf {InputPort d = p; InputPort reset = reset_gate;}
DataOutPort q[3:0] {Source child.q;}
}"""
        source = source.replace("DataMux reset_gate SelectedBy gate",
                                "Alias gate_view=gate; Alias gate_nested=gate_view; "
                                "DataMux reset_gate SelectedBy gate_nested")
        output = self.check(source, """iReset; iState gate 0
iWrite p 9; iApply; iRead q 9; iApply; iNote [iGetReadData q -dec]
iState gate 1; iWrite p 6; iApply; iRead q 0; iApply
iNote [iGetReadData q -dec]
""", modeled=True)
        self.assertEqual(output.count(b"iNote X\n"), 2)
        self.assertIn(b"iState gate 0b0", output)
        self.assertIn(b"iState gate 0b1", output)
        self.assertIn(b"iRead q 0b1001", output)
        self.assertIn(b"iRead q 0b0000", output)

    def test_ordinary_storage_ownership_still_blocks_automatic_change(self):
        source = BASE[:-1] + """
DataRegister storage[3:0] {WriteDataSource p;}
DataOutPort q[3:0] {Source storage;}
}"""
        model, commands = self.files(source, "iWrite p 3; iApply; iTake storage; iWrite storage 3; iApply; iWrite p 6; iApply")
        output = self.root / "result.pdl"
        output.write_bytes(b"retained artifact")
        self.tool(model, commands, "result.pdl", ok=False)
        self.assertEqual(output.read_bytes(), b"retained artifact")

    def test_end_state_callback_source_remains_constrained(self):
        source = BASE.replace("WriteCallBack", "ResetValue 9; WriteCallBack")
        source = source[:-1] + """
DataOutPort q[3:0] {Source value; Attribute iApplyEndState = 9;}
}"""
        output = self.check(source, """iReset; iWrite p 3; iApply
iRead q 9; iApply; iNote [iGetReadData q -dec]
""", modeled=True)
        self.assertIn(b"iNote X", output)
        self.assertIn(b"iRead q 0b1001", output)

    def test_nested_callback_storage_flat_identity_is_retained(self):
        source = """Module Leaf {
DataRegister held[3:0] {WriteCallBack Leaf store <D> <R>;}
DataOutPort q[3:0] {Source held;}
}
Module Chains {
DataInPort p[3:0]; Instance child Of Leaf;
DataOutPort q[3:0] {Source child.q;}
}"""
        output = self.check(source, """iState child.held 9
iWrite p 3; iApply; iRead q 9; iApply; iNote [iGetReadData q -dec]
iState child.held 2; iWrite p 4; iApply
iRead q 2; iApply; iNote [iGetReadData q -dec]
""", modeled=True)
        self.assertEqual(output.count(b"iNote X\n"), 2)
        self.assertIn(b"iState child.held 0b1001", output)
        self.assertIn(b"iState child.held 0b0010", output)
        self.assertIn(b"iRead q 0b1001", output)
        self.assertIn(b"iRead q 0b0010", output)


    def test_benchmark_rejects_stale_output_from_noop_candidate(self):
        candidate = shutil.which("true")
        self.assertIsNotNone(candidate, "benchmark regression requires true")
        driver = cli.ROOT / "tests/src/pattern/benchmark_primary_callback_state.py"
        output = self.root / "benchmark"
        args = ["python3", str(driver), "--parent", str(cli.BIN),
                "--output", str(output), "--sizes", "1", "--repeats", "1"]
        control = subprocess.run(args + ["--binary", str(cli.BIN)],
                                 capture_output=True, text=True, timeout=45)
        self.assertEqual(control.returncode, 0, control.stdout + control.stderr)
        result = subprocess.run(args + ["--binary", candidate],
                                capture_output=True, text=True, timeout=45)
        self.assertNotEqual(result.returncode, 0,
                            "no-output candidate reused old benchmark artifacts")
        self.assertIn("fresh pattern", result.stdout + result.stderr)


if __name__ == "__main__":
    unittest.main()
