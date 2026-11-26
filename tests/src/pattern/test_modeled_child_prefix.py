#!/usr/bin/env python3
"""Static modeled child histories survive root-lowered generated prefixes."""
import unittest

import test_modeled_readback as helpers
import test_modeled_raw_readback as raw
import test_merged_readback as merged
from benchmark_modeled_child_prefix import model




class ModeledChildPrefix(unittest.TestCase):
    setUp = helpers.ModeledReadback.setUp
    run_tool = helpers.ModeledReadback.run_tool
    notes = raw.ModeledRawReadback.notes

    def continued(self, source, prefix, suffix, expected):
        icl,first,full,tail = (self.root/n for n in ('source.icl','first.pdl','full.pdl','tail.pdl'))
        icl.write_text(source)
        header = 'iPDLLevel 1\niProcsForModule Chains\n'
        first.write_text(header+prefix);full.write_text(header+prefix+suffix)
        tail.write_text('iPDLLevel 1\n'+suffix)
        generated = self.run_tool(icl,first,'prefix.pdl')
        reference = self.run_tool(icl,full,'reference.pdl')
        self.assertEqual(self.notes(reference),expected)
        for source_icl in (icl,self.root/'debug/Chains.icl'):
            output = 'replayed.pdl' if source_icl==icl else 'flat/replayed.pdl'
            replay = self.run_tool(source_icl,self.root/'prefix.pdl',output,
                                   extra=('--pdl',str(tail)))
            self.assertEqual(replay,reference)
        return generated,reference

    def test_single_child_actual_return_mask_and_once_only_count(self):
        generated,_ = self.continued(model(),'''iReset; iWrite head 5; iWrite tail 19;
iWrite child0.value 165; iApply
iScan child0.local 8 -si 60 -so 0 -stable; iApply
''','''iNote [iGetReadData child0.local -dec]
iNote [iGetMiscompares child0.local -dec]; iNote [iGetStatus]
if {![catch {iGetReadData client}]} {error {root history invented}}
''',[b'165',b'165',b'4'])
        self.assertEqual(sum(line.startswith(b'iScan client ') for line in generated.splitlines()),1)
        self.assertIn(b'modeled child raw history client child0.local 0b00000000',generated)

    def test_same_type_disjoint_children_use_each_local_output_parity(self):
        for inverted in (False,True):
            with self.subTest(inverted=inverted):
                values = [6,10] if inverted else [9,5]
                self.continued(model(2,4,inverted),'''iReset; iWrite head 5; iWrite tail 19;
iWrite child0.value 9; iWrite child1.value 5; iApply
iScan child0.local 4 -si 6 -so 0; iScan child1.local 4 -si 3 -so 0; iApply
''','''iNote [iGetReadData child0.local -dec]; iNote [iGetReadData child1.local -dec]
iNote [iGetMiscompares child0.local -dec]; iNote [iGetMiscompares child1.local -dec]
iNote [iGetStatus]
''',[str(x).encode() for x in values+values+[4]])

    def test_child_prefix_preserves_earlier_root_history(self):
        self.continued(model(),'''iReset; iWrite head 5; iWrite tail 19;
iWrite child0.value 165; iApply
iScan client 16 -si 0 -so 0 -stable; iApply
iScan child0.local 8 -si 60 -so 0 -stable; iApply
''','''iNote [iGetReadData child0.local -dec]; iNote [iGetMiscompares child0.local -dec]
iNote [iGetReadData client -bin]; iNote [iGetMiscompares client -bin]
iNote [iGetStatus -clear]; iNote [iGetStatus]
''',[b'0',b'0',b'0b1011010010110011',b'0b1011010010110011',b'9',b'0'])

    def test_winning_partial_mask_and_canceled_requests_preserve_old_history(self):
        self.continued(model(),'''iReset; iWrite head 5; iWrite tail 19;
iWrite child0.value 165; iApply
iScan child0.local 8 -si 0 -so 255
iScan child0.local 8 -si 60 -so 0b0xxx1xx1; iApply
''','''iScan child0.local 8 -si 0 -so 0; iReset
iNote [iGetReadData child0.local -dec]
iNote [iGetMiscompares child0.local -dec]; iNote [iGetStatus -clear]; iNote [iGetStatus]
''',[b'165',b'136',b'2',b'0'])

    def test_unknown_actual_child_bits_keep_unknown_status(self):
        source = model().replace('CaptureSource value; ResetValue 0;',
                                 "CaptureSource value; ResetValue 8'bxxxxxxxx;")
        self.continued(source,'''iReset; iScan child0.local 8 -si 60 -so 0b0xxxxxxx; iApply
''','''iNote [iGetReadData child0.local -bin]
iNote [iGetMiscompares child0.local -bin]
if {![catch {iGetStatus -clear}]} {error {unknown status returned}}
if {![catch {iGetStatus}]} {error {unknown status cleared}}
''',[b'0bxxxxxxxx',b'0bx0000000'])

    def test_rejected_notes_preserve_earlier_child_history(self):
        for text in ('client child0.local 0b11111111',
                     'client child0.local 0b0000',
                     'client missing.local 0b00000000',
                     'client child0.local 0b00000000 extra'):
            with self.subTest(text=text):
                self.continued(model(),'''iReset; iWrite head 5; iWrite tail 19;
iWrite child0.value 165; iApply
iScan child0.local 8 -si 60 -so 0 -stable; iApply
iNote [iGetReadData child0.local -dec]
iScan client 16 -si 0 -so 0 -stable; iApply
iNote -comment {modeled child raw history %s}
''' % text,'''iNote [iGetReadData child0.local -dec]
iNote [iGetMiscompares child0.local -dec]; iNote [iGetStatus]
''',[b'165',b'165',b'165',b'13'])

    def test_typed_duplicates_do_not_recount_or_change_read_planes(self):
        prefix,_ = self.continued(model(),'''iReset; iWrite head 5; iWrite tail 19;
iWrite child0.value 165; iApply
iScan child0.local 8 -si 60 -so 0 -stable; iApply
''','''iNote [iGetReadData child0.local -dec]
iNote [iGetMiscompares child0.local -dec]; iNote [iGetStatus]
''',[b'165',b'165',b'4'])
        extra = b'iNote -status {modeled child raw history client child0.local 0b00000000}\n'
        (self.root/'duplicate.pdl').write_bytes(prefix+extra+extra)
        output = self.run_tool(self.root/'source.icl',self.root/'duplicate.pdl','duplicate-result.pdl',
                               extra=('--pdl',str(self.root/'tail.pdl')))
        self.assertEqual(self.notes(output),[b'165',b'165',b'4'])
        (self.root/'again.pdl').write_bytes(output)
        self.assertEqual(self.run_tool(self.root/'source.icl',self.root/'again.pdl','again-result.pdl'),output)

    def test_no_provider_cannot_invent_child_return_data(self):
        icl,first,tail = (self.root/n for n in ('source.icl','first.pdl','tail.pdl'))
        icl.write_text(model())
        first.write_text('iPDLLevel 1\niReset; iScan child0.local 8 -si 60; iApply\n')
        prefix = self.run_tool(icl,first,'prefix.pdl')
        self.assertIn(b'modeled child raw history ',prefix)
        tail.write_text('iPDLLevel 1\niGetReadData child0.local\n')
        failure = self.run_tool(icl,self.root/'prefix.pdl','unavailable.pdl',ok=False,modeled=False,
                                extra=('--pdl',str(tail)))
        self.assertIn('preceding applied iScan',failure)

    def test_default_generation_preserves_intent_without_observing_data(self):
        icl,script = self.root/'source.icl',self.root/'source.pdl'
        icl.write_text(model())
        script.write_text('iPDLLevel 1\niReset; iWrite child0.value 165; iApply; '
                          'iScan child0.local 8 -si 60 -so 0; iApply\n')
        first = self.run_tool(icl,script,'prefix.pdl',modeled=False)
        self.assertIn(b'modeled child raw history ',first)
        self.assertNotIn(b'informational modeled child raw history ',first)
        again = self.run_tool(icl,self.root/'prefix.pdl','again.pdl',modeled=False)
        self.assertEqual(again,first)
        tail = self.root/'tail.pdl'
        tail.write_text('iPDLLevel 1\niGetReadData child0.local\n')
        self.assertIn('preceding applied iScan',self.run_tool(icl,self.root/'prefix.pdl',
            'unavailable.pdl',ok=False,modeled=False,extra=('--pdl',str(tail))))
        tail.write_text('''iPDLLevel 1
iNote [iGetReadData child0.local -dec]
iNote [iGetMiscompares child0.local -dec]; iNote [iGetStatus]
''')
        observed = self.run_tool(icl,self.root/'again.pdl','observed.pdl',
                                 extra=('--pdl',str(tail)))
        self.assertEqual(self.notes(observed),[b'165',b'165',b'4'])

    def test_noncanonical_root_cannot_activate_an_unconsumed_note(self):
        icl,script = self.root/'source.icl',self.root/'source.pdl'
        icl.write_text(model())
        for modeled in (False,True):
            with self.subTest(modeled=modeled):
                script.write_text('''iPDLLevel 1
iReset; iScan client 16 -si 0 -so 0; iApply
iNote -comment {modeled child raw history client child0.local 0b00000000}
''')
                first = self.run_tool(icl,script,'prefix.pdl',modeled=modeled)
                self.assertIn(b'informational modeled child raw history ',first)
                again = self.run_tool(icl,self.root/'prefix.pdl','again.pdl',modeled=modeled)
                self.assertEqual(again,first)

    def test_called_merge_keeps_child_capture_before_later_writes(self):
        self.continued(model(),'''iReset; iWrite head 5; iWrite tail 19;
iWrite child0.value 165; iApply
iProc scan {} {iScan child0.local 8 -si 60 -so 0 -stable; iApply}
iProc write {} {iWrite child0.value 170; iApply}
iMerge -begin; iCall scan; iCall write; iMerge -end
''','''iNote [iGetReadData child0.local -dec]
iNote [iGetMiscompares child0.local -dec]; iNote [iGetStatus]
''',[b'165',b'165',b'4'])

    def test_namespaced_local_reset_rewrite_preserves_procedure_source(self):
        definition = 'iProc scan {data} {iScan child0.local 8 -si $data -so 0 -stable; iApply}'
        _,reference = self.continued(model(),
            'iProcsForModule Chains -iProcNameSpace ops\n'+definition+'''
iReset; iWrite head 5; iWrite tail 19; iWrite child0.value 165; iApply
iCall ops::scan 60
''','''iNote [iGetReadData child0.local -dec]
iNote [iGetMiscompares child0.local -dec]; iNote [iGetStatus]
''',[b'165',b'165',b'4'])
        reset = self.root/'reset.pdl'
        rewritten = self.run_tool(self.root/'source.icl',self.root/'full.pdl','reset-output.pdl',
                                  extra=('--local_reset',str(reset)))
        self.assertEqual(rewritten,reference)
        self.assertIn(definition,reset.read_text())
        self.assertEqual(self.run_tool(self.root/'source.icl',reset,'reset/replay.pdl'),reference)

    def test_rejected_parsed_note_does_not_mark_merge_observed(self):
        body = merged.EVENTS.split('Module Chains {',1)[1].rsplit('}',1)[0]
        source = model().replace('Module Chains {','Module Chains {'+body)
        icl,script,tail = (self.root/n for n in ('source.icl','source.pdl','tail.pdl'))
        icl.write_text(source)
        program = merged.MergedReadback.event_program(self)
        tail.write_text('iPDLLevel 1\niNote [iGetReadData d -dec]; '
                        'iNote [iGetReadData observed -dec]; iNote [iGetStatus]\n')
        results = []
        for annotated in (False,True):
            text = program+'iApply\n'+(
                'iNote -comment {modeled child raw history client child0.local 0b00000000}'
                if annotated else 'iNote ordinary')+'\n'
            script.write_text('iPDLLevel 1\niProcsForModule Chains\n'+text)
            generated = self.run_tool(icl,script,'merge-prefix.pdl')
            replay = self.run_tool(icl,self.root/'merge-prefix.pdl','merge/replay.pdl',
                                   extra=('--pdl',str(tail)))
            results.append([line for line in generated.splitlines() if not line.startswith(b'iNote ')])
            self.assertEqual(self.notes(replay)[-3:],[b'9',b'X',b'2'])
        self.assertEqual(results[0],results[1])

    def test_generated_prefix_local_reset_verifier_restores_child_history(self):
        icl,script,tail = (self.root/n for n in ('source.icl','source.pdl','tail.pdl'))
        icl.write_text(model())
        script.write_text('''iPDLLevel 1
iReset; iWrite child0.value 165; iApply
iScan child0.local 8 -si 60 -so 0 -stable; iApply
''')
        self.run_tool(icl,script,'prefix.pdl')
        definition = 'iProc result {} {iNote [iGetReadData child0.local -dec]; iNote [iGetMiscompares child0.local -dec]; iNote [iGetStatus -clear]}'
        tail.write_text('iPDLLevel 1\niProcsForModule Chains -iProcNameSpace ops\n'+
                        definition+'\niCall ops::result; iNote [iGetStatus]\n')
        reference = self.run_tool(icl,self.root/'prefix.pdl','reference.pdl',
                                  extra=('--pdl',str(tail)))
        self.assertEqual(self.notes(reference),[b'165',b'165',b'4',b'0'])
        reset = self.root/'rewritten.pdl'
        for source_icl in (icl,self.root/'debug/Chains.icl'):
            rewritten = self.run_tool(source_icl,self.root/'prefix.pdl','reset/result.pdl',
                extra=('--pdl',str(tail),'--local_reset',str(reset)))
            self.assertEqual(rewritten,reference)
            self.assertIn(definition,reset.read_text())
            self.assertEqual(self.run_tool(source_icl,reset,'reset/replayed.pdl'),reference)

    def test_default_chain_spelling_cannot_activate_child_metadata(self):
        prefix = '''iReset; iWrite child0.value 165; iApply
iScan child0.local 8 -si 60 -so 0; iApply
iScan client -chain default 16 -si 0b0000000000000000 -so 0b0000000000000000; iApply
iNote -comment {modeled child raw history client child0.local 0b00000000}
'''
        self.continued(model(),prefix,'''iNote [iGetReadData child0.local -dec]
iNote [iGetMiscompares child0.local -dec]; iNote [iGetStatus]
''',[b'165',b'165',b'8'])
        first = self.run_tool(self.root/'source.icl',self.root/'first.pdl',
                               'default-prefix.pdl',modeled=False)
        self.assertIn(b'informational modeled child raw history ',first)
        self.assertEqual(self.run_tool(self.root/'source.icl',self.root/'default-prefix.pdl',
            'default-again.pdl',modeled=False),first)

    def test_off_route_child_note_stays_informational(self):
        source = model().replace(' ScanInterface local {',''' ScanInPort alt;
 ScanInterface other {Port alt; Port so; Port sel; Port ce; Port se; Port ue; Port tck; Port reset;}
 ScanInterface local {''')
        source = source.replace('InputPort si = head[0];','InputPort si = head[0]; InputPort alt = head[0];')
        prefix = '''iReset
iScan client 16 -si 0b0000000000000000 -so 0b0000000000000000; iApply
iNote -comment {modeled child raw history client child0.other 0b00000000}
'''
        generated,_ = self.continued(source,prefix,'''iNote [iGetReadData client -dec]; iNote [iGetStatus]
if {![catch {iGetReadData child0.other}]} {error {off-route history invented}}
''',[b'0',b'0'])
        self.assertIn(b'informational modeled child raw history ',generated)
        first = self.run_tool(self.root/'source.icl',self.root/'first.pdl',
                               'default-prefix.pdl',modeled=False)
        self.assertIn(b'informational modeled child raw history ',first)
        self.assertEqual(self.run_tool(self.root/'source.icl',self.root/'default-prefix.pdl',
            'default-again.pdl',modeled=False),first)


if __name__ == '__main__':
    unittest.main(argv=[__file__])
