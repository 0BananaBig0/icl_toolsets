#!/usr/bin/env python3
"""Callback-containing merge calls retain complete serial execution."""
import json
import os
import sys
import time
from pathlib import Path
import subprocess
import unittest

import test_downstream_write_callbacks as downstream


MODEL = downstream.MODEL


class MergedCallbacks(unittest.TestCase):
    setUp = downstream.DownstreamWriteCallbacks.setUp
    run_tool = downstream.DownstreamWriteCallbacks.run_tool
    notes = downstream.DownstreamWriteCallbacks.notes
    generation_replay = downstream.DownstreamWriteCallbacks.generation_replay
    rejected = downstream.DownstreamWriteCallbacks.rejected
    reset_replay = downstream.DownstreamWriteCallbacks.reset_replay

    def merged(self, body):
        return "iProc access {} {" + body + "}; iMerge -begin; iCall access; iMerge -end"

    def frames(self, output):
        frames, pending = [], {}
        for line in output.splitlines():
            words = line.split()
            if words and words[0] == b"iWrite":
                pending[words[1]] = words[2]
            elif words and words[0] == b"iApply":
                frames.append(pending)
                pending = {}
        self.assertFalse(pending)
        return frames

    def test_explicit_writes(self):
        output = self.generation_replay("""
iProc access {data} {iWrite value $data; iApply}
iMerge -begin; iCall access 9; iCall access 6; iMerge -end
if {$::calls != 2} {error callback_count}
""")
        self.assertEqual(self.notes(output), [b"value=0b1001", b"value=0b0110"])
        self.assertEqual([f[b"p"] for f in self.frames(output) if b"p" in f],
                         [b"0b1001", b"0b0110"])

    def test_inferred_writes(self):
        output = self.generation_replay("""
iProc access {data} {iWrite dest $data; iApply}
iMerge -begin; iCall access 9; iCall access 6; iMerge -end
if {$::calls != 2} {error callback_count}
""")
        self.assertEqual(self.notes(output), [b"value=0b1001", b"value=0b0110"])
        frames = [f for f in self.frames(output) if f]
        self.assertEqual(frames, [{b"en": b"0b0", b"p": b"0b1001"},
                                  {b"en": b"0b1"},
                                  {b"en": b"0b0", b"p": b"0b0110"},
                                  {b"en": b"0b1"}])

    def test_read_and_empty_callbacks(self):
        source = MODEL.replace("WriteCallBack Chains store <D> <R>;",
                               "ReadCallBack Chains store <D> <R>;")
        output = self.generation_replay(self.merged("iRead value 9; iApply"), source)
        self.assertEqual(self.notes(output), [b"value=0b1001"])
        output = self.generation_replay(
            "iProc store {data reg} {incr ::calls}; " +
            self.merged("iRead value 9; iApply") +
            "; if {$::calls != 1} {error callback_count}", source)
        self.assertEqual(self.notes(output), [])
        self.assertFalse(any(self.frames(output)))

    def test_mixed_aliases(self):
        source = MODEL.replace(" Alias inverse", " Alias mixed[7:0] = value,spare; Alias inverse")
        output = self.generation_replay(self.merged("iWrite mixed 0x93; iApply"), source)
        self.assertEqual(self.notes(output), [b"value=0b1001"])
        self.assertTrue(any(f.get(b"spare") == b"0b0011" for f in self.frames(output)))

    def test_call_order_and_notes(self):
        output = self.generation_replay("""
iProc pin {target data} {iWrite $target $data; iApply}
iProc access {} {iWrite value 9; iApply; iNote inside}
iMerge -begin
iCall pin spare 1; iCall pin p 2; iCall access
iNote outside
iCall pin spare 4; iCall pin p 5
iMerge -end
""")
        self.assertEqual(self.notes(output), [b"outside", b"value=0b1001", b"inside"])
        frames = self.frames(output)
        for frame, expected in [(frames[0], {b"p": b"0b0010", b"spare": b"0b0001"}),
                                (frames[-1], {b"p": b"0b0101", b"spare": b"0b0100"})]:
            self.assertEqual({name: bits for name, bits in frame.items() if name != b"en"},
                             expected)
            if b"en" in frame:
                self.assertEqual(frame[b"en"], b"0b0")

    def test_nested_calls_and_merges(self):
        output = self.generation_replay("""
iProc endpoint {data} {iWrite en 0; iWrite p $data; iApply}
iProc store {data reg} {
 incr ::calls; iNote "$reg=$data"
 iMerge -begin; iCall endpoint $data; iMerge -end
}
iProc access {} {iWrite dest 9; iApply}
iProc outer {} {iMerge -begin; iCall access; iMerge -end}
iMerge -begin; iCall outer; iMerge -end
if {$::calls != 1} {error callback_count}
""")
        self.assertEqual(self.notes(output), [b"value=0b1001"])

    def test_goals_and_reservations(self):
        output = self.generation_replay(
            self.merged("iWrite spare 3; iWrite value 9; iApply"))
        self.assertTrue(any(f.get(b"spare") == b"0b0011" for f in self.frames(output)))
        output = self.generation_replay(
            "iTake p; iWrite p 9; iApply; " +
            self.merged("iWrite value 9; iApply") + "; iRelease p")
        self.assertEqual(self.notes(output), [b"value=0b1001"])
        self.rejected(self.merged("iWrite p 3; iWrite value 9; iApply"), "caller goal")

    def test_protocol_rejections(self):
        self.rejected("iProc store {data reg} {iReset}; " +
                      self.merged("iWrite value 9; iApply"), "forbidden")
        self.rejected(self.merged("iWrite spare 3"), "unapplied")
        self.rejected("iMerge -begin; iWrite value 9; iMerge -end", "only iCall")
        self.rejected(self.merged("iWrite value 9; iApply -together"), "callback")

    def test_prefix_namespace_split_inputs(self):
        source = """Module Leaf {
 DataInPort data[3:0]; DataInPort enable;
 DataRegister dest[3:0] {WriteDataSource data; WriteEnSource enable;}
}
Module Chains {
 DataInPort en; DataInPort p[3:0];
 DataRegister value[3:0] {WriteCallBack Chains::api store <D> <R>;}
 Instance child Of Leaf {InputPort data=value; InputPort enable=en;}
}"""
        output = self.generation_replay("""
iProcsForModule Chains -iProcNameSpace api
iProc store {data reg} {incr ::calls; iNote "api:$reg=$data"; iWrite en 0; iWrite p $data}
iProc access {data} {iPrefix child; iWrite dest $data; iApply}
set ::args 0
""", source, "iMerge -begin; iCall api::access [expr {[incr ::args]+8}]; "
            "iMerge -end; if {$::calls != 1 || $::args != 1} {error split_count}")
        self.assertEqual(self.notes(output), [b"api:value=0b1001"])

    def test_incomplete_call_cannot_leak_to_sibling(self):
        definitions = """iProc incomplete {} {iWrite value 9}
iProc unrelated {} {iApply}
"""
        self.rejected(definitions + "iMerge -begin; catch {iCall incomplete}; "
                      "iCall unrelated; iMerge -end", "unapplied")
        self.rejected(definitions + """
iMerge -begin
catch {iCall incomplete}
catch {iCall unrelated}
if {$::calls != 0} {error callback_leaked}
error stopped_with_queued_merge
""", "stopped_with_queued_merge")
        source = MODEL.replace("Chains store <D> <R>", "Chains missing <D> <R>")
        output = self.generation_replay("""
iProc request {} {
 iWrite value 9
 if {![catch {iApply}]} {error missing_preflight_error}
 iProc missing {data reg} {incr ::calls; iNote "$reg=$data"; iWrite en 0; iWrite p $data}
 iApply
}
iMerge -begin; iCall request; iMerge -end
if {$::calls != 1} {error retry_count}
""", source)
        self.assertEqual(self.notes(output), [b"value=0b1001"])

    def test_callback_owned_merge_keeps_ordinary_batching(self):
        source = """Module Chains {
 DataInPort p[3:0]; DataInPort spare[3:0];
 DataRegister value[3:0] {WriteCallBack Chains store <D> <R>;}
}"""
        definitions = """iProc pin {name data} {iWrite $name $data; iApply}
iProc store {data reg} {
 iMerge -begin; iCall pin p $data; iCall pin spare 3; iMerge -end
}
"""
        for body in ["iWrite value 9; iApply", self.merged("iWrite value 9; iApply")]:
            with self.subTest(body=body):
                output = self.generation_replay(definitions + body, source)
                self.assertEqual(self.frames(output),
                                 [{b"p": b"0b1001", b"spare": b"0b0011"}, {}])
        source = source.replace("\n}", """
 DataRegister peer[3:0] {WriteCallBack Chains leaf <D> <R>;}
}""")
        nested = """iProc pin {name data} {iWrite $name $data; iApply}
iProc leaf {data reg} {incr ::calls; iWrite p $data}
iProc request {} {iWrite peer 6; iApply}
iProc store {data reg} {
 incr ::calls; iApply
 iMerge -begin; iCall pin p $data; iCall request; iCall pin spare 3; iMerge -end
}
"""
        output = self.generation_replay(nested + "iWrite value 9; iApply; "
                                        "if {$::calls != 2} {error nested_count}", source)
        self.assertEqual([frame for frame in self.frames(output) if frame],
                         [{b"p": b"0b1001"}, {b"p": b"0b0110"}, {b"spare": b"0b0011"}])
        self.rejected(nested + "iWrite p 9; iWrite value 9; iApply", "caller goal", source)

    def test_filtered_prerequisite_keeps_ordinary_batching(self):
        source = """Module Chains {
 DataInPort en; DataInPort p[3:0]; DataInPort spare[3:0];
 DataRegister value[3:0] {WriteCallBack Chains store <D> <R>;}
 DataRegister dest[3:0] {WriteDataSource value; WriteEnSource en;}
}"""
        output = self.generation_replay("""
iProc store {data reg} {error unexpected_callback}
iProc access {} {iWrite dest 9; iApply}
iProc pin {} {iWrite spare 3; iApply}
iState value 9
iMerge -begin; iCall access; iCall pin; iMerge -end
if {$::calls != 0} {error callback_count}
""", source)
        self.assertEqual(self.frames(output), [{b"en": b"0b1", b"spare": b"0b0011"}])

    def test_once_only_arguments(self):
        output = self.generation_replay("""
set ::args 0
iProc access {data} {iWrite dest $data; iApply}
iMerge -begin; iCall access [expr {[incr ::args]+8}]; iMerge -end
if {$::calls != 1 || $::args != 1} {error once_only}
""")
        self.assertEqual(self.notes(output), [b"value=0b1001"])

    def test_reset_rewrite_preserves_procedures(self):
        callbacks = downstream.callbacks
        request = "iProc request {data} {iWrite child.value $data; iApply}\n"
        output, rewritten = self.reset_replay(
            callbacks.RESET_MODEL,
            callbacks.RESET_PROCS + request + "iReset; iMerge -begin; iCall request 9\n",
            (callbacks.STORE_DEF, callbacks.REQUEST_DEF, request), "iMerge -end\n")
        self.assertIn(b"iWrite child.wire 0b1001", output)
        self.assertEqual(rewritten.count(request), 1)

    def test_repeated_and_returned_sites(self):
        request = "iProc request {data} {iWrite dest $data; return [iApply]}\n"
        output, rewritten = self.reset_replay(
            MODEL, downstream.PROCS + request + """
set ::args 0
iMerge -begin
foreach value {1 2 5} {iCall request [expr {$value+[incr ::args]-$::args}]}
iMerge -end
if {$::calls != 3 || $::args != 3} {error repeated_count}
""", (request,))
        self.assertEqual(self.notes(output),
                         [b"value=0b0001", b"value=0b0010", b"value=0b0101"])
        self.assertEqual(rewritten.count(request), 1)

    def test_failures_and_cycles(self):
        for body in ["error failed_body", "iWrite p $data; error failed_body"]:
            self.rejected("iProc store {data reg} {" + body + "}; " +
                          self.merged("iWrite value 9; iApply"), "failed_body")
        self.rejected("iProc store {data reg} {iWrite value $data; iApply}; " +
                      self.merged("iWrite value 9; iApply"), "callback cycle")
        self.rejected("iProc store {data reg} {error failed_body}; " + self.merged(
            "iWrite value 9; catch {iApply}; iApply"), "requires iReset")

    def test_observation_and_physical_guards(self):
        automatic = downstream.PROCS+self.merged("iWrite dest 9; iApply")+'''
; iRead endpoint 9; iRead q 0bx; iApply
iNote [iGetReadData endpoint -dec]; iNote [iGetReadData q -bin]
'''
        automatic_output = downstream.observations.ModeledReadback.replay(self, MODEL, automatic)
        self.assertEqual(self.notes(automatic_output), [b"value=0b1001", b"9", b"0bxxxx"])
        self.rejected(self.merged("iWrite value 9; iApply"), "callbacks",
                      extra=("--physical_pdl",))
        source = MODEL.replace("WriteCallBack Chains store <D> <R>;",
                               "ReadCallBack Chains store <D> <R>;")
        read_body = downstream.PROCS+self.merged('iRead value 9; iApply')+'''
; iRead endpoint 0; iApply; iNote [iGetReadData endpoint -dec]
if {![catch {iGetReadData value}]} {error logical_callback_history}
'''
        read_output = downstream.observations.ModeledReadback.replay(self, source, read_body)
        self.assertEqual(self.notes(read_output), [b'value=0b1001', b'9'])
        body = downstream.PROCS+self.merged("iWrite value 9; iApply") + \
            "; iRead endpoint 0; iApply; iNote [iGetReadData endpoint -bin]"
        output = downstream.observations.ModeledReadback.replay(self, MODEL, body)
        self.assertEqual(self.notes(output), [b"value=0b1001", b"0b1001"])

    @unittest.skipUnless(hasattr(os,"fork"),"Unix process groups are required")
    def test_benchmark_deadline_terminates_measured_children(self):
        started, marker, used = (self.root/name for name in ('started','child-finished','timeout-used'))
        candidate = self.root/'sleeper.py'
        candidate.write_text('#!'+sys.executable+'\nimport os,time\nfrom pathlib import Path\n'
            +'Path('+repr(str(started))+').touch()\n'
            +'if os.fork()==0:\n time.sleep(0.5)\n Path('+repr(str(marker))+').touch()\n os._exit(0)\n'
            +'time.sleep(1)\n')
        candidate.chmod(0o755)
        folder = self.root/'bin'; folder.mkdir()
        shim = folder/'timeout'
        shim.write_text('#!'+sys.executable+'\nimport os,sys\nfrom pathlib import Path\n'
            +'args=sys.argv[1:]\nif len(args)>2 and args[2]=='+repr(str(candidate))+':\n'
            +' assert args[:2]==["--kill-after=2s","27s"]\n'
            +' Path('+repr(str(used))+').write_text("\\n".join(args))\n'
            +' args[:2]=["--kill-after=0.1s","0.2s"]\n'
            +'os.execv("/usr/bin/timeout",["timeout",*args])\n')
        shim.chmod(0o755)
        script = Path(__file__).with_name('benchmark_merged_callbacks.py')
        result = subprocess.run(['python3',str(script),'--binary',str(candidate),
            '--parent',str(downstream.cli.BIN),'--sanitized',str(downstream.cli.BIN),
            '--output',str(self.root/'deadline'),
            '--sizes','1','--wide-widths','--repeats','1'],
            env=dict(os.environ,PATH=str(folder)+os.pathsep+os.environ['PATH']),
            capture_output=True,text=True,timeout=45)
        self.assertNotEqual(result.returncode,0,result.stdout+result.stderr)
        self.assertTrue(started.exists(),'deadline control did not reach measured candidate: '+result.stdout+result.stderr)
        # Wait beyond the child's marker delay to detect a surviving descendant.
        time.sleep(0.6)
        self.assertFalse(marker.exists(),'measured child outlived the deadline')
        self.assertTrue(used.exists(),'candidate did not use the process-group timeout')

    def test_scale_receipts(self):
        script = Path(__file__).with_name("benchmark_merged_callbacks.py")
        output = self.root / "scale"
        result = subprocess.run(
            ["python3", str(script), "--binary", str(downstream.cli.BIN),
             "--parent", str(downstream.cli.BIN), "--sanitized", str(downstream.cli.BIN),
             "--output", str(output), "--sizes", "2", "--wide-widths", "--repeats", "1"],
            capture_output=True, text=True, timeout=90)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        report = json.loads((output / "results.json").read_text())
        self.assertEqual(len(report["runs"]), 10)
        for row in report["runs"]:
            self.assertEqual(row["connected_callback_bits"], 8)
            self.assertEqual(row["callback_invocations"], 0 if row["mode"] == "unused" or
                             row["form"] == "generated" else 8)
            self.assertTrue(row["ordered_postconditions_verified"])

    def test_wide_receipts(self):
        script = Path(__file__).with_name("benchmark_merged_callbacks.py")
        output = self.root / "wide"
        result = subprocess.run(
            ["python3", str(script), "--binary", str(downstream.cli.BIN),
             "--parent", str(downstream.cli.BIN), "--sanitized", str(downstream.cli.BIN),
             "--output", str(output), "--sizes", "--wide-widths", "16", "--repeats", "1"],
            capture_output=True, text=True, timeout=90)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        report = json.loads((output / "results.json").read_text())
        self.assertEqual(report["runs"], [])
        self.assertEqual(len(report["wide_runs"]), 1)
        self.assertEqual(report["wide_runs"][0]["connected_callback_bits"], 16)


if __name__ == "__main__":
    unittest.main()
