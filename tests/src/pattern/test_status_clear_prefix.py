#!/usr/bin/env python3
"""Successful clears remain ordered events in generated level-zero prefixes."""
import unittest

import test_modeled_readback as helpers
import test_modeled_raw_readback as raw
import test_modeled_tap_readback as tap
import test_merged_readback as merged
from benchmark_multiple_opaque_children import model


class StatusClearPrefix(unittest.TestCase):
    setUp = helpers.ModeledReadback.setUp
    run_tool = helpers.ModeledReadback.run_tool
    notes = raw.ModeledRawReadback.notes

    def merge_continuation(self, body, label):
        icl,script,tail = (self.root/n for n in ('merge.icl',label+'.pdl',label+'-tail.pdl'))
        icl.write_text(merged.EVENTS)
        script.write_text('iPDLLevel 1\niProcsForModule Chains\n'+body)
        generated = self.run_tool(icl,script,label+'-prefix.pdl')
        tail.write_text('''iPDLLevel 1
iNote [iGetReadData observed -dec]; iNote [iGetStatus]
''')
        replay = self.run_tool(icl,self.root/(label+'-prefix.pdl'),label+'-replay.pdl',
                               extra=('--pdl',str(tail)))
        hardware = [line for line in generated.splitlines() if not line.startswith(b'iNote ')]
        return generated,hardware,self.notes(replay)[-2:]

    def test_rejected_status_note_keeps_unobserved_merge_trajectory(self):
        program = merged.MergedReadback.event_program(self)
        ordinary,hardware,observed = self.merge_continuation(program+'iNote ordinary\n','ordinary')
        annotated,changed,actual = self.merge_continuation(program+
            'iNote -comment {modeled readback status clear 8}\n','rejected')
        self.assertEqual(actual,observed)
        self.assertEqual(actual,[b'9',b'2'])
        self.assertEqual(changed,hardware)
        self.assertEqual(ordinary.count(b'iApply\n'),2)
        self.assertIn(b'informational modeled readback status clear 8',annotated)

    def test_edge_zero_clears_keep_same_side_merge_batching(self):
        program = merged.MergedReadback.event_program(self)
        for edge in ('leading','trailing'):
            for extra_clear in (False,True):
                with self.subTest(edge=edge,extra_clear=extra_clear):
                    body = program if edge=='leading' else program.replace('iRead d 0;','iRead d 0bxxxx;')
                    baseline,hardware,observed = self.merge_continuation(body,edge+'-plain')
                    if edge=='leading':
                        changed = body.replace('iProc first {} {','iProc first {} {iGetStatus -clear; ')
                    else:
                        changed = body.replace('iWrite p 9; iApply}', 'iWrite p 9; iApply; iGetStatus -clear}')
                    if extra_clear:
                        changed = 'iProc clearonly {} {iGetStatus -clear}\n'+changed
                        old = 'iMerge -begin;' if edge=='leading' else 'iMerge -end'
                        new = 'iMerge -begin; iCall clearonly;' if edge=='leading' else 'iCall clearonly; iMerge -end'
                        changed = changed.replace(old,new)
                    generated,actual_hardware,actual = self.merge_continuation(changed,edge+'-clear')
                    self.assertEqual(actual,observed)
                    self.assertEqual(actual,[b'9',b'2' if edge=='leading' else b'0'])
                    self.assertEqual(actual_hardware,hardware)
                    self.assertEqual(generated.count(b'iApply\n'),2)
                    self.assertEqual(generated.count(b'modeled readback status clear 0'),1+extra_clear)

    def continued(self, source, prefix, suffix, expected):
        icl, first, full, tail = (self.root/n for n in
                                 ('source.icl','first.pdl','full.pdl','tail.pdl'))
        icl.write_text(source)
        header = 'iPDLLevel 1\niProcsForModule Chains\n'
        first.write_text(header+prefix);full.write_text(header+prefix+suffix)
        tail.write_text('iPDLLevel 1\n'+suffix)
        generated = self.run_tool(icl,first,'prefix.pdl')
        reference = self.run_tool(icl,full,'reference.pdl')
        for source_icl in (icl,self.root/'debug/Chains.icl'):
            output = 'replayed.pdl' if source_icl == icl else 'flat/replayed.pdl'
            replay = self.run_tool(source_icl,self.root/'prefix.pdl',output,
                                   extra=('--pdl',str(tail)))
            self.assertEqual(reference,replay)
            self.assertEqual(self.notes(replay),expected)
        self.assertTrue(generated.startswith(b'iPDLLevel 0\n'))
        return generated,reference

    def test_pio_clear_keeps_old_histories_and_later_failures(self):
        self.continued(helpers.BASE,'''iReset; iWrite p 15; iApply
iRead q 0; iApply; iNote [iGetStatus -clear]
''','''iNote [iGetReadData q -bin]; iNote [iGetMiscompares q -bin]
iNote [iGetStatus]; iRead q 14; iApply; iNote [iGetStatus]
''',[b'4',b'0b1111',b'0b1111',b'0',b'1'])

    def test_raw_clear_keeps_actual_mask_and_later_count(self):
        self.continued(raw.ROOT,'''iReset; iWrite r 9; iApply
iScan client 4 -si 6 -so 0; iApply; iNote [iGetStatus -clear]
''','''iNote [iGetReadData client -dec]; iNote [iGetMiscompares client -bin]
iNote [iGetStatus]; iScan client 4 -si 0 -so 0; iApply; iNote [iGetStatus]
''',[b'2',b'9',b'0b1001',b'0',b'2'])

    def test_opaque_clear_survives_child_scan_without_recounting(self):
        source = model().replace('Module Top','Module Chains')
        self.continued(source,'''iReset; iWrite head 5; iWrite tail 19; iApply
iScan client 24 -si 0b101000000000000000010011 -so 0b000xxxxxxxxxxxxxxxx00000 -stable; iApply
iNote [iGetStatus -clear]; iScan child0.local 8 -si 0 -stable; iApply
''','''iNote [iGetMiscompares child0.local -bin]
iNote [iGetMiscompares client -bin]; iNote [iGetStatus]
iRead head 0; iRead tail 0; iApply; iNote [iGetStatus]
''',[b'5',b'0b00000000',b'0b101' + b'0'*16 + b'10011',b'0',b'5'])

    def test_zero_repeated_and_unknown_clears_keep_provider_boundaries(self):
        generated,_ = self.continued(helpers.BASE,'''iNote [iGetStatus -clear]
iNote [iGetStatus -clear]
''','iNote [iGetStatus]\n',[b'0',b'0',b'0'])
        self.assertEqual(generated.count(b'modeled readback status clear 0'),2)
        icl,first = self.root/'source.icl',self.root/'first.pdl'
        unobserved = self.run_tool(icl,first,'unobserved.pdl',modeled=False)
        self.assertEqual(unobserved,generated.replace(
            b'iNote {modeled register/PIO readback; no live device}\n',b''))
        first.write_text('''iPDLLevel 1
iWrite p 15; iApply; iRead q 0; iApply
iNote -comment {modeled readback status clear 4}
if {![catch {iGetStatus} message] || [string first hardware $message] < 0} {
 error {unobserved count cleared}
}
iNote unavailable
''')
        result = self.run_tool(icl,first,'unavailable.pdl',modeled=False)
        self.assertEqual(self.notes(result),[b'unavailable'])
        self.assertIn(b'informational modeled readback status clear 4',result)
        self.continued(helpers.BASE,'''iReset; iRead q 0; iApply
iNote -comment {modeled readback status clear 0}
if {![catch {iGetStatus -clear}]} {error {unknown count cleared}}
''','''if {![catch {iGetStatus}]} {error {unknown count invented}}
iNote unknown
''',[b'unknown'])

    def test_typed_duplicate_clear_notes_do_not_recount(self):
        for tag in ('-comment','-status'):
            with self.subTest(tag=tag):
                marker = 'iNote '+tag+' {modeled readback status clear 4}\n'
                generated,_ = self.continued(helpers.BASE,
                    'iWrite p 15; iApply; iRead q 0; iApply\n'+marker+marker,
                    'iNote [iGetStatus]; iRead q 14; iApply; iNote [iGetStatus]\n',
                    [b'0',b'1'])
                self.assertEqual(generated.count(b'iNote -comment "modeled readback status clear 4"'),1)
                self.assertIn(b'informational modeled readback status clear 4',generated)

    def test_invalid_notes_cannot_clear_a_known_count(self):
        for count in ('','-1','04','5','18446744073709551616','4 extra','4'+'0'*100):
            with self.subTest(count=count):
                self.continued(helpers.BASE,
                    'iWrite p 15; iApply; iRead q 0; iApply\n'+
                    'iNote -comment {modeled readback status clear '+count+'}\n',
                    'iNote [iGetStatus]; iNote [iGetStatus -clear]; iNote [iGetStatus]\n',
                    [b'4',b'4',b'0'])
        for marker in ('iNote -bad {modeled readback status clear 4}',
                       'iNote -comment {modeled readback status clear 4} extra'):
            with self.subTest(marker=marker):
                self.continued(helpers.BASE,
                    'iWrite p 15; iApply; iRead q 0; iApply\n'+marker+'\n',
                    'iNote [iGetStatus]\n',[b'4'])

    def test_clear_does_not_apply_a_pending_read_or_reset_old_histories(self):
        self.continued(helpers.BASE,'''iWrite p 15; iApply; iRead q 0; iApply
iRead q 14; iNote [iGetStatus -clear]; iReset
''','''iNote [iGetReadData q -bin]; iNote [iGetMiscompares q -bin]
iNote [iGetStatus]
''',[b'4',b'0b1111',b'0b1111',b'0'])

    def test_called_merge_clears_stay_between_comparison_groups(self):
        self.continued(helpers.BASE,'''iProc compare_clear {value} {
 iWrite p $value; iApply; iRead q 0; iApply; iNote [iGetStatus -clear]
}
iMerge -begin; iCall compare_clear 15; iCall compare_clear 3; iMerge -end
''','''iNote [iGetStatus]; iNote [iGetReadData q -dec]
iRead q 0; iApply; iNote [iGetStatus]
''',[b'4',b'2',b'0',b'3',b'2'])
        self.continued(helpers.BASE,'''iProc compare {} {iWrite p 15; iApply; iRead q 0; iApply}
iMerge -begin
iNote -comment {modeled readback status clear 4}
iCall compare; iMerge -end
''','iNote [iGetStatus]\n',[b'4'])

    def test_read_callback_clear_keeps_actual_endpoint_counts(self):
        source = helpers.BASE.replace('\n}', '''
 DataRegister logical[3:0] {ReadCallBack Chains reader <D>; ReadDataSource p;}
}''')
        self.continued(source,'''iProc reader {data} {
 iRead q $data; iApply; iNote [iGetStatus -clear]
}
iWrite p 15; iApply; iRead logical 0; iApply
''','''iNote [iGetReadData q -dec]; iNote [iGetMiscompares q -bin]; iNote [iGetStatus]
''',[b'4',b'15',b'0b1111',b'0'])

    def test_namespaced_clear_and_local_reset_keep_procedure_text(self):
        definition = 'iProc access {value} {iScan client 4 -si $value -so 0; iApply; iNote [iGetStatus -clear]}'
        prefix = 'iProcsForModule Chains -iProcNameSpace ops\n'+definition+'''
iReset; iWrite r 9; iApply; iCall ops::access 6
'''
        _,reference = self.continued(raw.ROOT,prefix,'iNote [iGetStatus]\n',[b'2',b'0'])
        reset = self.root/'reset.pdl'
        rewritten = self.run_tool(self.root/'source.icl',self.root/'full.pdl','reset-result.pdl',
                                  extra=('--local_reset',str(reset)))
        self.assertEqual(reference,rewritten)
        self.assertIn(definition,reset.read_text())
        self.assertEqual(reference,self.run_tool(self.root/'source.icl',reset,'reset/replay.pdl'))

    def test_bound_tap_clear_retains_ir_history(self):
        delegate = tap.ModeledTapReadback(methodName='runTest');delegate.root=self.root
        prefix = '''iReset; iRead tap.INSTRUCTION 0; iApply; iNote [iGetStatus -clear]
'''
        suffix = '''iNote [iGetStatus]; iNote [iGetMiscompares tap.INSTRUCTION -bin]
iRead tap.INSTRUCTION 0; iApply; iNote [iGetStatus]
'''
        reference = delegate.replay(prefix+suffix)
        first,tail = self.root/'tap-first.pdl',self.root/'tap-tail.pdl'
        first.write_text('iPDLLevel 1\n'+prefix);tail.write_text('iPDLLevel 1\n'+suffix)
        delegate.run_tool(self.root/'source.icl',first,'tap-prefix.pdl')
        replay = delegate.run_tool(self.root/'source.icl',self.root/'tap-prefix.pdl','tap/replay.pdl',
                                   extra=('--pdl',str(tail)))
        self.assertEqual(reference,replay)
        self.assertEqual(self.notes(replay),[b'1',b'0',b'0b0001',b'1'])


if __name__ == '__main__':
    unittest.main(argv=[__file__])
