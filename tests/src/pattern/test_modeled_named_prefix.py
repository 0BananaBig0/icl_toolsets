#!/usr/bin/env python3
"""Generated implicit named scans preserve actual explicit-only histories."""
import unittest

import test_modeled_child_prefix as helpers
from test_simultaneous_scan_chains import NETWORK
from test_opaque_root_serial_merges import PARALLEL as OPAQUE


class ModeledNamedPrefix(unittest.TestCase):
    setUp = helpers.ModeledChildPrefix.setUp
    run_tool = helpers.ModeledChildPrefix.run_tool
    notes = helpers.ModeledChildPrefix.notes
    continued = helpers.ModeledChildPrefix.continued

    def test_generated_implicit_lane_does_not_invent_history(self):
        generated,_ = self.continued(NETWORK,'''iReset
iScan client -chain first 4 -si 9 -so 0bx -stable; iApply
''','''iNote [iGetReadData client -chain first -bin]
if {![catch {iGetReadData client -chain second}]} {error {implicit named history invented}}
iNote absent; iNote [iGetStatus]
''',[b'0b0000',b'absent',b'0'])
        self.assertEqual(sum(line.startswith(b'iScan ') for line in generated.splitlines()),2)
        self.assertEqual(generated.count(b'\niApply\n'),1)

    def test_generated_implicit_fill_keeps_old_actual_capture_and_cared_mask(self):
        generated,_ = self.continued(NETWORK,'''iReset
iScan client -chain first 4 -si 9 -so 15 -stable; iApply
iScan client -chain second 2 -si 1 -so 0bx -stable; iApply
''','''iNote [iGetReadData client -chain first -bin]
iNote [iGetMiscompares client -chain first -bin]
iNote [iGetReadData client -chain second -bin]
iNote [iGetStatus -clear]; iNote [iGetStatus]
''',[b'0b0000',b'0b1111',b'0b00',b'4',b'0'])
        self.assertEqual(sum(line.startswith(b'iScan ') for line in generated.splitlines()),4)
        self.assertEqual(generated.count(b'\niApply\n'),2)

    def test_direct_canonical_notes_are_hoisted_before_intervening_calls(self):
        for flag in ('-comment','-status'):
            with self.subTest(flag=flag):
                self.continued(NETWORK,'''iProc second {} {
 iScan client -chain second 2 -si 1 -so 0bx -stable; iApply
}
iProc first {} {iScan client -chain first 4 -si 0b0110 -so 0bxxxx -stable; iApply}
iReset; iScan client -chain first 4 -si 9 -so 15 -stable; iApply
iMerge -begin; iCall second
iNote %s {modeled named raw history omitted iScan client -chain first 4 -si 0b0110 -so 0bxxxx -stable}
iCall first; iMerge -end
'''%flag,'''iNote [iGetReadData client -chain first -bin]
iNote [iGetMiscompares client -chain first -bin]
iNote [iGetStatus -clear]; iNote [iGetStatus]
''',[b'0b1001',b'0b0000',b'4',b'0'])

    def test_invalid_and_old_notes_keep_call_produced_arguments(self):
        cases = [
            'modeled raw history omitted iScan client -chain first 4 -si 0b1001 -so 0bxxxx -stable',
            'modeled named raw history omitted iScan client -chain first 5 -si 0b01001 -so 0bxxxxx -stable',
            'modeled named raw history omitted iScan client -chain missing 4 -si 0b1001 -so 0bxxxx -stable',
            'modeled named raw history omitted iScan client -chain first 4 -si 0b1 -so 0bxxxx -stable',
            'modeled named raw history omitted iScan client 4 -si 0b1001 -so 0bxxxx -stable',
        ]
        for text in cases:
            for flag in ('-comment','-status'):
                with self.subTest(text=text,flag=flag):
                    self.continued(NETWORK,'''iProc seed {} {set ::word 9}
iProc load {data} {iScan client -chain first 4 -si $data -so 0bx -stable; iApply}
iReset; iMerge -begin; iCall seed
iNote %s {%s}
iCall load $::word; iMerge -end
'''%(flag,text),'iNote [iGetReadData client -chain first -bin]\n',[b'0b0000'])

    def test_valid_adjacent_note_keeps_physical_comparison_and_prior_history(self):
        for flag in ('-comment','-status'):
            with self.subTest(flag=flag):
                self.continued(NETWORK,'''iReset
iScan client -chain first 4 -si 9 -so 15 -stable; iApply
iNote %s {modeled named raw history omitted iScan client -chain first 4 -si 0b0110 -so 0b1111 -stable}
iScan client -chain first 4 -si 0b0110 -so 0b1111 -stable; iApply
'''%flag,'''iNote [iGetReadData client -chain first -bin]
iNote [iGetMiscompares client -chain first -bin]
iNote [iGetStatus -clear]; iNote [iGetStatus]
''',[b'0b0000',b'0b1111',b'6',b'0'])

    def test_unknown_capture_and_status_survive_implicit_fill_and_reset(self):
        source = NETWORK.replace('CaptureSource a;',"CaptureSource 4'b10xx;")
        self.continued(source,'''iReset
iScan client -chain first 4 -si 9 -so 0 -stable; iApply
iScan client -chain second 2 -si 1 -so 0bx -stable; iApply
iReset
''','''iNote [iGetReadData client -chain first -bin]
iNote [iGetMiscompares client -chain first -bin]
if {![catch {iGetStatus -clear}]} {error {unknown comparison became known}}
if {![catch {iGetStatus}]} {error {unknown comparison cleared}}
iNote unknown
''',[b'0b10xx',b'0b10xx',b'unknown'])

    def test_inverted_capture_is_native_and_implicit_lanes_keep_old_mask(self):
        source = NETWORK.replace('CaptureSource a;',"CaptureSource 4'b0101;").replace('Source a[0]','Source ~a[0]')
        self.continued(source,'''iReset
iScan client -chain first 4 -si 9 -so 0 -stable; iApply
iScan client -chain second 2 -si 1 -so 0bx -stable; iApply
''','''iNote [iGetReadData client -chain first -bin]
iNote [iGetMiscompares client -chain first -bin]
iNote [iGetStatus -clear]; iNote [iGetStatus]
''',[b'0b1010',b'0b1010',b'2',b'0'])

    def test_reset_cancels_pending_lane_without_inventing_history(self):
        self.continued(NETWORK,'''iScan client -chain second 2 -si 2
iReset; iScan client -chain first 4 -si 3
iScan client -chain first 4 -si 9 -so 0bx -stable; iApply
''','''iNote [iGetReadData client -chain first -bin]
if {![catch {iGetReadData client -chain second}]} {error {canceled history invented}}
iNote absent; iNote [iGetStatus]
''',[b'0b0000',b'absent',b'0'])

    def test_new_named_modeled_notes_stay_informational_on_opaque_roots(self):
        self.continued(OPAQUE,'''iProc seed {} {set ::word 9}
iProc load {data} {iScan client -chain first 4 -si $data -so 0bx -stable; iApply}
iReset; iMerge -begin; iCall seed
iNote -comment {modeled named raw history omitted iScan client -chain first 4 -si 0b1001 -so 0bxxxx -stable}
iCall load $::word; iMerge -end
''','iNote [iGetReadData client -chain first -bin]\n',[b'0bxxxx'])

    def test_local_reset_retains_source_bytes_and_history_scope(self):
        icl,pdl = self.root/'source.icl',self.root/'source.pdl'
        icl.write_text(NETWORK)
        pdl.write_text('''iPDLLevel 1
iProcsForModule Chains
iProc first {} {iScan client -chain first 4 -si 9 -so 0bx -stable; iApply}
iReset; iCall first
iNote [iGetReadData client -chain first -bin]
if {![catch {iGetReadData client -chain second}]} {error {implicit history invented}}
iNote [iGetStatus -clear]; iNote [iGetStatus]
''')
        reference = self.run_tool(icl,pdl,'reference.pdl')
        for model in (icl,self.root/'debug/Chains.icl'):
            reset = self.root/'reset.pdl'
            self.assertEqual(self.run_tool(model,pdl,'reset/result.pdl',
                             extra=('--local_reset',str(reset))),reference)
            self.assertEqual(reset.read_bytes(),pdl.read_bytes())
            self.assertEqual(self.run_tool(model,reset,'reset/replayed.pdl'),reference)

    def test_literal_default_chain_keeps_its_explicit_history(self):
        self.continued(NETWORK.replace('Chain first','Chain default'),'''iReset
iScan client -chain default 4 -si 9 -so 15 -stable; iApply
iScan client -chain second 2 -si 1 -so 0bx -stable; iApply
''','''iNote [iGetReadData client -chain default -bin]
iNote [iGetMiscompares client -chain default -bin]
iNote [iGetStatus -clear]; iNote [iGetStatus]
''',[b'0b0000',b'0b1111',b'4',b'0'])

    def test_default_prefix_regeneration_keeps_scope_and_requires_real_provider(self):
        icl,pdl,tail = (self.root/n for n in ('source.icl','source.pdl','tail.pdl'))
        icl.write_text(NETWORK)
        pdl.write_text('''iPDLLevel 1
iProcsForModule Chains
iReset; iScan client -chain first 4 -si 9 -so 0bx -stable; iApply
''')
        prefix = self.run_tool(icl,pdl,'prefix.pdl',modeled=False)
        self.assertEqual(self.run_tool(icl,self.root/'prefix.pdl','again.pdl',modeled=False),prefix)
        tail.write_text('iPDLLevel 1\niGetReadData client -chain first\n')
        self.assertIn('requires hardware readback',self.run_tool(icl,self.root/'again.pdl',
                      'missing.pdl',modeled=False,ok=False,extra=('--pdl',str(tail))))
        tail.write_text('''iPDLLevel 1
iNote [iGetReadData client -chain first -bin]
if {![catch {iGetReadData client -chain second}]} {error {implicit history invented}}
iNote absent; iNote [iGetStatus]
''')
        result = self.run_tool(icl,self.root/'again.pdl','observed.pdl',extra=('--pdl',str(tail)))
        self.assertEqual(self.notes(result),[b'0b0000',b'absent',b'0'])

    def test_namespaced_nested_merge_keeps_only_latest_explicit_history(self):
        self.continued(NETWORK,'''iProcsForModule Chains -iProcNameSpace ops
iProc first {} {iScan client -chain first 4 -si 9 -so 15 -stable; iApply}
iProc second {} {iScan client -chain second 2 -si 1 -so 0bx -stable; iApply}
iProc nested {} {iMerge -begin; iCall ops::second; iCall ops::second; iMerge -end}
iProcsForModule Chains
iReset; iMerge -begin; iCall ops::first; iCall ops::nested; iMerge -end
''','''iNote [iGetReadData client -chain first -bin]
iNote [iGetMiscompares client -chain first -bin]
iNote [iGetReadData client -chain second -bin]
iNote [iGetStatus -clear]; iNote [iGetStatus]
''',[b'0b0000',b'0b1111',b'0b01',b'4',b'0'])

    def test_later_invalid_call_preserves_existing_outputs(self):
        for observe in (False,True):
            for local in (False,True):
                with self.subTest(observe=observe,local=local):
                    icl,pdl,out,reset = (self.root/n for n in ('source.icl','source.pdl','protected.pdl','reset.pdl'))
                    icl.write_text(NETWORK)
                    pdl.write_text('''iPDLLevel 1
iProcsForModule Chains
iProc first {} {iScan client -chain first 4 -si 9 -so 0bx -stable; iApply}
iProc invalid {} {iScan client -chain second 3 -si 1; iApply}
iProc nested {} {iCall invalid}
iReset; iMerge -begin; iCall first; iCall nested; iMerge -end
'''+('iGetReadData client -chain first\n' if observe else ''))
                    out.write_bytes(b'keep output\n');reset.write_bytes(b'keep reset\n')
                    diagnostic = self.run_tool(icl,pdl,'protected.pdl',ok=False,
                                     extra=('--local_reset',str(reset)) if local else ())
                    self.assertIn('length',diagnostic)
                    self.assertEqual(out.read_bytes(),b'keep output\n')
                    self.assertEqual(reset.read_bytes(),b'keep reset\n')


if __name__=='__main__':
    unittest.main(argv=[__file__])
