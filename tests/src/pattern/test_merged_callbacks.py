#!/usr/bin/env python3
"""Callback-containing merge calls retain complete serial execution."""
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


if __name__ == "__main__":
    unittest.main()
