#!/usr/bin/env python3
"""Standalone opaque merge calls preserve real frames and explicit history scope."""
import unittest

import test_modeled_child_prefix as helpers
import test_opaque_root_scan as scalar
import test_opaque_parallel_scan as parallel
import test_simultaneous_scan_chains as modeled_parallel

SCALAR = scalar.MODEL.replace('Module Top','Module Chains')
PARALLEL = parallel.MODEL.replace('Module Top','Module Chains')


class OpaqueRootSerialMerges(unittest.TestCase):
    setUp = helpers.ModeledChildPrefix.setUp
    run_tool = helpers.ModeledChildPrefix.run_tool
    notes = helpers.ModeledChildPrefix.notes
    continued = helpers.ModeledChildPrefix.continued

    def test_scalar_calls_match_sequential_unknown_frames(self):
        generated,_ = self.continued(SCALAR,'''iProc first {} {
 iScan client 8 -si 150 -so 0bx -stable; iApply
}
iProc second {} {iScan client 8 -si 105 -so 0bx -stable; iApply}
iReset; iMerge -begin; iCall first; iCall second; iMerge -end
''','''iNote [iGetReadData client -bin]; iNote [iGetMiscompares client -bin]
iNote [iGetStatus -clear]; iNote [iGetStatus]
''',[b'0bxxxxxxxx',b'0b00000000',b'0',b'0'])
        self.assertEqual(generated.count(b'\niApply\n'),2)
        self.assertIn(b'-si 0b10010110',generated)
        self.assertIn(b'-si 0b01101001',generated)

    def test_named_calls_keep_native_completion_and_per_chain_histories(self):
        generated,_ = self.continued(PARALLEL,'''iProc first {} {
 iScan client -chain first 4 -si 9 -so 0bx -stable; iApply
}
iProc second {} {iScan client -chain second 7 -si 65 -so 0bx -stable; iApply}
iReset; iMerge -begin; iCall first; iCall second; iMerge -end
''','''iNote [iGetReadData client -chain first -bin]
iNote [iGetReadData client -chain second -bin]
iNote [iGetMiscompares client -chain first -bin]
iNote [iGetMiscompares client -chain second -bin]
iNote [iGetStatus -clear]; iNote [iGetStatus]
''',[b'0bxxxx',b'0bxxxxxxx',b'0b0000',b'0b0000000',b'0',b'0'])
        self.assertEqual(generated.count(b'\niApply\n'),2)
        self.assertEqual(sum(line.startswith(b'iScan ') for line in generated.splitlines()),4)

    def test_generated_implicit_default_never_invents_unrequested_chain_history(self):
        generated,_ = self.continued(PARALLEL,'''iReset
iScan client -chain first 4 -si 9 -so 0bx -stable; iApply
''','''iNote [iGetReadData client -chain first -bin]
iNote [iGetMiscompares client -chain first -bin]
if {![catch {iGetReadData client -chain second}]} {error {implicit history invented}}
iNote absent; iNote [iGetStatus]
''',[b'0bxxxx',b'0b0000',b'absent',b'0'])
        self.assertEqual(generated.count(b'\niApply\n'),1)

    def test_generated_implicit_stable_fill_preserves_previous_cared_mask(self):
        self.continued(PARALLEL,'''iReset
iScan client -chain first 4 -si 9 -so 0b10xx -stable; iApply
iScan client -chain second 7 -si 65 -so 0bx -stable; iApply
''','''iNote [iGetReadData client -chain first -bin]
iNote [iGetMiscompares client -chain first -bin]
iNote [iGetReadData client -chain second -bin]
if {![catch {iGetStatus -clear}]} {error {unknown status returned}}
if {![catch {iGetStatus}]} {error {unknown status cleared}}
iNote unknown
''',[b'0bxxxx',b'0bxx00',b'0bxxxxxxx',b'unknown'])

    def test_nested_unequal_scalar_calls_keep_latest_variable_width(self):
        generated,_ = self.continued(SCALAR,'''iProc long {} {
 iScan client 13 -si 0b1001011001011 -so 0bx -stable; iApply
 iScan client 8 -si 150 -so 0bx -stable; iApply
}
iProc short {} {iScan client 5 -si 19 -so 0bx -stable; iApply}
iProc nested {} {iMerge -begin; iCall long; iCall short; iMerge -end}
iReset; iMerge -begin; iCall nested; iCall short; iMerge -end
''','''iNote [iGetReadData client -bin]; iNote [iGetMiscompares client -bin]
iNote [iGetStatus -clear]; iNote [iGetStatus]
''',[b'0bxxxxx',b'0b00000',b'0',b'0'])
        scans = [line.split() for line in generated.splitlines() if line.startswith(b'iScan ')]
        self.assertEqual([line[2] for line in scans],[b'13',b'8',b'5',b'5'])
        self.assertEqual(generated.count(b'\niApply\n'),4)

    def test_reset_preserves_old_masks_while_new_named_width_is_explicit(self):
        self.continued(PARALLEL,'''iProc first {} {
 iScan client -chain first 4 -si 9 -so 0b10xx -stable; iApply
}
iProc second {} {iScan client -chain second 9 -si 0b100101101 -so 0bx -stable; iApply}
iReset; iMerge -begin; iCall first; iMerge -end
iReset; iMerge -begin; iCall second; iMerge -end
''','''iNote [iGetReadData client -chain first -bin]
iNote [iGetMiscompares client -chain first -bin]
iNote [iGetReadData client -chain second -bin]
if {![catch {iGetStatus}]} {error {unknown status erased by reset}}
iNote unknown
''',[b'0bxxxx',b'0bxx00',b'0bxxxxxxxxx',b'unknown'])

    def test_reset_cancels_pending_width_without_inventing_a_chain_history(self):
        self.continued(PARALLEL,'''iProc first {} {
 iScan client -chain first 4 -si 9 -so 0bx -stable; iApply
}
iScan client -chain second 12 -si 65
iReset; iMerge -begin; iCall first; iMerge -end
''','''iNote [iGetReadData client -chain first -bin]
if {![catch {iGetReadData client -chain second}]} {error {canceled history invented}}
iNote absent; iNote [iGetStatus]
''',[b'0bxxxx',b'absent',b'0'])

    def test_direct_named_omission_note_cannot_bind_past_an_intervening_call(self):
        for flag in ('-comment','-status'):
            with self.subTest(flag=flag):
                self.continued(PARALLEL,'''iProc second {} {
 iScan client -chain second 7 -si 65 -so 0bx -stable; iApply
}
iProc first {} {iScan client -chain first 4 -si 0b0110 -so 0bxxxx -stable; iApply}
iReset; iScan client -chain first 4 -si 9 -so 0b10xx -stable; iApply
iMerge -begin; iCall second
iNote %s {modeled raw history omitted iScan client -chain first 4 -si 0b0110 -so 0bxxxx -stable}
iCall first; iMerge -end
'''%flag,'''iNote [iGetMiscompares client -chain first -bin]
iNote [iGetReadData client -chain second -bin]
if {![catch {iGetStatus -clear}]} {error {unknown status returned}}
iNote unknown
''',[b'0b0000',b'0bxxxxxxx',b'unknown'])

    def test_invalid_named_omission_notes_preserve_call_produced_arguments(self):
        for text in ('iScan client -chain missing 4 -si 0b1001 -so 0bxxxx -stable',
                     'iScan client -chain first 4 -si 0b1 -so 0bxxxx -stable'):
            with self.subTest(text=text):
                self.continued(PARALLEL,'''iProc seed {} {set ::word 9}
iProc load {data} {iScan client -chain first 4 -si $data -so 0bx -stable; iApply}
iReset; iMerge -begin; iCall seed
iNote -comment {modeled raw history omitted %s}
iCall load $::word; iMerge -end
'''%text,'''iNote [iGetReadData client -chain first -bin]
if {![catch {iGetReadData client -chain second}]} {error {implicit history invented}}
iNote absent; iNote [iGetStatus]
''',[b'0bxxxx',b'absent',b'0'])

    def test_modeled_named_omission_note_preserves_call_produced_arguments(self):
        for flag in ('-comment','-status'):
            with self.subTest(flag=flag):
                self.continued(modeled_parallel.NETWORK,'''iProc seed {} {set ::word 9}
iProc load {data} {iScan client -chain first 4 -si $data -so 0bx -stable; iApply}
iReset; iMerge -begin; iCall seed
iNote %s {modeled raw history omitted iScan client -chain first 5 -si 0b01001 -so 0bxxxxx -stable}
iCall load $::word; iMerge -end
'''%flag,'iNote [iGetReadData client -chain first -bin]\n',[b'0b0000'])

    def test_modeled_named_omission_note_keeps_explicit_scan_history(self):
        for flag in ('-comment','-status'):
            with self.subTest(flag=flag):
                self.continued(modeled_parallel.NETWORK,'''iReset
iNote %s {modeled raw history omitted iScan client -chain first 4 -si 0b1001 -so 0bxxxx -stable}
iScan client -chain first 4 -si 0b1001 -so 0bxxxx -stable; iApply
'''%flag,'iNote [iGetReadData client -chain first -bin]\n',[b'0b0000'])

    def test_call_local_query_sees_only_completed_explicit_chain_requests(self):
        self.continued(PARALLEL,'''iProc first {} {
 iScan client -chain first 4 -si 9 -so 0bx -stable; iApply
 if {![catch {iGetReadData client -chain second}]} {error {early implicit history invented}}
 iNote absent
}
iProc second {} {iScan client -chain second 7 -si 65 -so 0bx -stable; iApply}
iReset; iMerge -begin; iCall first; iCall second; iMerge -end
''','''iNote [iGetReadData client -chain first -bin]
iNote [iGetReadData client -chain second -bin]; iNote [iGetStatus -clear]
iNote [iGetStatus]
''',[b'absent',b'0bxxxx',b'0bxxxxxxx',b'0',b'0'])

    def test_namespaced_named_calls_keep_each_chain_argument(self):
        self.continued(PARALLEL,'''iProcsForModule Chains -iProcNameSpace ops
iProc shift {chain width word} {iScan client -chain $chain $width -si $word -so 0bx -stable; iApply}
iProcsForModule Chains
iReset; iMerge -begin
iCall ops::shift first 4 9; iCall ops::shift second 7 65
iMerge -end
''','''iNote [iGetReadData client -chain first -bin]
iNote [iGetReadData client -chain second -bin]; iNote [iGetStatus]
''',[b'0bxxxx',b'0bxxxxxxx',b'0'])

    def test_original_local_reset_preserves_merge_definitions_and_source_bytes(self):
        icl,pdl = self.root/'source.icl',self.root/'source.pdl'
        icl.write_text(PARALLEL)
        pdl.write_text('''iPDLLevel 1
iProcsForModule Chains
iProc first {} {iScan client -chain first 4 -si 9 -so 0bx -stable; iApply}
iReset; iMerge -begin; iCall first; iMerge -end
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

    def test_default_regeneration_retains_implicit_scope_and_requires_real_provider(self):
        icl,pdl,tail = (self.root/n for n in ('source.icl','source.pdl','tail.pdl'))
        icl.write_text(PARALLEL)
        pdl.write_text('''iPDLLevel 1
iProcsForModule Chains
iProc first {} {iScan client -chain first 4 -si 9 -so 0bx -stable; iApply}
iReset; iMerge -begin; iCall first; iMerge -end
''')
        prefix = self.run_tool(icl,pdl,'prefix.pdl',modeled=False)
        self.assertEqual(self.run_tool(icl,self.root/'prefix.pdl','again.pdl',modeled=False),prefix)
        tail.write_text('iPDLLevel 1\niGetReadData client -chain first\n')
        self.assertIn('requires hardware readback',self.run_tool(icl,self.root/'again.pdl',
                      'missing.pdl',modeled=False,ok=False,extra=('--pdl',str(tail))))
        tail.write_text('''iPDLLevel 1
iNote [iGetReadData client -chain first -bin]
if {![catch {iGetReadData client -chain second}]} {error {implicit history invented}}
iNote [iGetStatus -clear]; iNote [iGetStatus]
''')
        result = self.run_tool(icl,self.root/'again.pdl','observed.pdl',extra=('--pdl',str(tail)))
        self.assertEqual(self.notes(result),[b'0bxxxx',b'0',b'0'])

    def test_empty_merge_is_event_free_with_or_without_prior_scan(self):
        generated,_ = self.continued(SCALAR,'iReset; iMerge -begin; iMerge -end\n',
                                    'iNote [iGetStatus]\n',[b'0'])
        self.assertNotIn(b'iApply',generated)
        generated,_ = self.continued(SCALAR,'''iReset; iScan client 8 -si 150 -so 0bx -stable; iApply
iMerge -begin; iMerge -end
''','iNote [iGetReadData client -bin]; iNote [iGetStatus]\n',[b'0bxxxxxxxx',b'0'])
        self.assertEqual(generated.count(b'\niApply\n'),1)

    def test_later_getter_retains_nested_failing_call_frames(self):
        for merged in (False,True):
            for getter in ('iGetReadData client -chain second -bin',
                           'iGetMiscompares client -chain second -bin',
                           'iGetStatus'):
                with self.subTest(merged=merged,getter=getter):
                    icl,pdl,out = (self.root/n for n in
                                   ('source.icl','source.pdl','protected.pdl'))
                    icl.write_text(PARALLEL)
                    call = ('iMerge -begin; iCall outer; iMerge -end'
                            if merged else 'iCall outer')
                    pdl.write_text('''iPDLLevel 1
iProcsForModule Chains
iProc inner {} {
 iScan client -chain first 4 -si 9 -so 0; iApply
 iScan client -chain second 7 -si 65 -so 0; iApply
}
iProc outer {} {iCall inner}
iReset
'''+call+'\niNote ['+getter+']\n')
                    protected = [out,self.root/'debug/Chains.icl']
                    for path in protected:
                        path.parent.mkdir(parents=True,exist_ok=True)
                        path.write_bytes(b'keep previous artifact\n')
                    error = self.run_tool(icl,pdl,'protected.pdl',ok=False)
                    self.assertIn('source.pdl:10:8:',error)
                    self.assertRegex(error,r'source\.pdl:5:[0-9]+: opaque parallel scan chain length is unknown')
                    frames = ['while iCall outer at ','while iCall inner at ']
                    for frame in frames: self.assertEqual(error.count(frame),1,error)
                    self.assertLess(error.index(frames[0]),error.index(frames[1]))
                    for path in protected:
                        self.assertEqual(path.read_bytes(),b'keep previous artifact\n')

    def test_invalid_calls_modes_and_unknown_lengths_preserve_existing_output(self):
        cases = [
            (SCALAR,'iRunLoop 1; iScan client 8 -si 150; iApply','opaque',()),
            (SCALAR,'iScan client 8 -si 150; iApply','physical',('--physical_pdl',)),
            (PARALLEL,'iScan client -chain missing 4 -si 9; iApply','chain',()),
            (PARALLEL,'iScan client -chain first 4 -si 16; iApply','scan',()),
            (PARALLEL,'iScan client -chain first 1048576; iApply','resource',()),
            (PARALLEL,'iScan client -chain first 4 -si 9; iApply; '
             'iScan client -chain second 7 -si 65; iApply','length',()),
        ]
        for source,body,diagnostic,extra in cases:
            with self.subTest(diagnostic=diagnostic,body=body):
                icl,pdl,out = (self.root/n for n in ('source.icl','source.pdl','protected.pdl'))
                icl.write_text(source)
                pdl.write_text('iPDLLevel 1\niProcsForModule Chains\niProc bad {} {'+body+
                               '}\niReset; iMerge -begin; iCall bad; iMerge -end\n')
                out.write_bytes(b'keep existing output\n')
                self.assertIn(diagnostic,self.run_tool(icl,pdl,'protected.pdl',ok=False,extra=extra))
                self.assertEqual(out.read_bytes(),b'keep existing output\n')


if __name__=='__main__':
    unittest.main(argv=[__file__])
