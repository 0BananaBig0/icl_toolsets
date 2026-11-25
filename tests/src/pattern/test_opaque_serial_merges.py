#!/usr/bin/env python3
"""Native opaque-child iMerge calls retain actual serial frames and histories."""
import unittest

import test_modeled_child_prefix as helpers
from benchmark_dynamic_opaque_routes import model

DEFINITIONS = '''iProc first {} {iScan child0.local 8 -si 165 -so 0bx -stable; iApply}
iProc second {} {iScan child1.local 8 -si 60 -so 0bx -stable; iApply}
'''


class OpaqueSerialMerges(unittest.TestCase):
    setUp = helpers.ModeledChildPrefix.setUp
    run_tool = helpers.ModeledChildPrefix.run_tool
    notes = helpers.ModeledChildPrefix.notes
    continued = helpers.ModeledChildPrefix.continued

    def merge(self, automatic):
        start = DEFINITIONS+'iReset\n'
        if not automatic:
            start += 'iScan client 10 -si 0b1011110011 -so 0bx; iApply\n'
        suffix = '''iNote [iGetReadData child0.local -bin]; iNote [iGetReadData child1.local -bin]
iNote [iGetMiscompares child0.local -bin]; iNote [iGetMiscompares child1.local -bin]
'''
        expected = [b'0bxxxxxxxx',b'0bxxxxxxxx',b'0b00000000',b'0b00000000']
        if automatic:
            suffix += 'if {![catch {iGetReadData client}]} {error {root raw history invented}}\n'
        else:
            suffix += 'iNote [iGetReadData client -bin]\n'
            expected += [b'0b0000000000']
        suffix += 'iNote [iGetStatus]\n';expected += [b'0']
        icl,pdl = self.root/'source.icl',self.root/'sequential.pdl'
        icl.write_text(model(2,8))
        pdl.write_text('iPDLLevel 1\niProcsForModule Chains\n'+start+
                       'iCall first; iCall second\n'+suffix)
        control = self.run_tool(icl,pdl,'sequential-result.pdl')
        generated,reference = self.continued(model(2,8),start+
            'iMerge -begin\niCall first\niCall second\niMerge -end\n',suffix,expected)
        self.assertEqual(reference,control)
        self.assertEqual(generated.count(b'\niApply\n'),4 if automatic else 3)
        self.assertNotIn(b'iMerge ',generated)
        self.assertNotIn(b'__opaque_scan_state',generated)

    def test_opened_calls_match_actual_sequential_opaque_frames(self):
        self.merge(False)

    def test_closed_calls_keep_actual_automatic_preparation_and_history_scope(self):
        self.merge(True)

    def test_mixed_parent_and_raw_calls_keep_real_known_failure_counts(self):
        generated,_ = self.continued(model(2,8),'''iProc writer {} {
 iWrite head 5; iApply; iRead head 0; iApply
 iScan child0.local 8 -si 150 -so 0bx -stable; iApply
}
iProc tailer {} {iWrite tail 19; iApply; iRead tail 19; iApply}
iReset; iScan client 10 -si 0b0001100000 -so 0bx; iApply
iMerge -begin; iCall writer; iCall tailer; iMerge -end
''','''iNote [iGetReadData head -dec]; iNote [iGetReadData tail -dec]
iNote [iGetReadData child0.local -bin]; iNote [iGetMiscompares child0.local -bin]
if {![catch {iGetReadData child1.local}]} {error {implicit child history invented}}
iNote [iGetStatus -clear]; iNote [iGetStatus]
''',[b'5',b'19',b'0bxxxxxxxx',b'0b00000000',b'2',b'0'])
        self.assertEqual(generated.count(b'\niApply\n'),6)

    def test_nested_unequal_call_streams_retain_every_serial_apply(self):
        generated,_ = self.continued(model(2,8),'''iProc short {} {
 iScan child1.local 8 -si 60 -so 0bx -stable; iApply
}
iProc long {} {
 iScan child0.local 8 -si 150 -so 0bx -stable; iApply
 iScan child0.local 8 -si 105 -so 0bx -stable; iApply
}
iProc nested {} {iMerge -begin; iCall short; iCall long; iMerge -end}
iReset; iScan client 10 -si 0b1011110011 -so 0bx; iApply
iMerge -begin; iCall nested; iCall short; iMerge -end
''','''iNote [iGetReadData child0.local -bin]; iNote [iGetReadData child1.local -bin]
iNote [iGetMiscompares child0.local -bin]; iNote [iGetStatus]
''',[b'0bxxxxxxxx',b'0bxxxxxxxx',b'0b00000000',b'0'])
        self.assertEqual(generated.count(b'\niApply\n'),5)

    def test_direct_merge_notes_precede_call_hardware(self):
        generated,_ = self.continued(model(2,8),DEFINITIONS+'''iReset
iScan client 10 -si 0b1011110011 -so 0bx; iApply
iMerge -begin
iNote before
iCall first
iNote between
iCall second
iMerge -end
''','''iNote [iGetReadData child0.local -bin]; iNote [iGetReadData child1.local -bin]
iNote [iGetStatus]
''',[b'before',b'between',b'0bxxxxxxxx',b'0bxxxxxxxx',b'0'])
        self.assertLess(generated.index(b'iNote before'),generated.index(b'iNote between'))
        self.assertLess(generated.index(b'iNote between'),generated.index(b'iScan client 26'))
        self.assertEqual(generated.count(b'\niApply\n'),3)

    def test_cared_opaque_unknown_masks_and_status_survive_merge_end(self):
        definitions = DEFINITIONS.replace('165 -so 0bx','150 -so 0b10xxxx01')
        self.continued(model(2,8),definitions+'''iReset
iScan client 10 -si 0b1011110011 -so 0bx; iApply
iMerge -begin; iCall first; iCall second; iMerge -end
''','''iNote [iGetReadData child0.local -bin]; iNote [iGetReadData child1.local -bin]
iNote [iGetMiscompares child0.local -bin]; iNote [iGetMiscompares child1.local -bin]
if {![catch {iGetStatus -clear}]} {error {unknown status returned}}
if {![catch {iGetStatus}]} {error {unknown status cleared}}
iNote unknown
''',[b'0bxxxxxxxx',b'0bxxxxxxxx',b'0bxx0000xx',b'0b00000000',b'unknown'])

    def test_call_local_clear_counts_old_known_failure_once(self):
        definitions = DEFINITIONS.replace('165 -so 0bx -stable; iApply}',
            '165 -so 0bx -stable; iApply; iNote [iGetStatus -clear]}',1)
        generated,_ = self.continued(model(2,8),definitions+'''iReset
iScan client 10 -si 0b1011110011 -so 0b0000000001; iApply
iMerge -begin; iCall first; iCall second; iMerge -end
''','''iNote [iGetReadData child0.local -bin]; iNote [iGetReadData child1.local -bin]
iNote [iGetStatus]; iNote [iGetStatus]
''',[b'1',b'0bxxxxxxxx',b'0bxxxxxxxx',b'0',b'0'])
        self.assertEqual(generated.count(b'modeled readback status clear 1'),1)

    def test_namespaced_calls_preserve_parent_alias_reservation(self):
        source = model(2,8).replace(' ScanRegister tail',' Alias held[2:0] = head;\n ScanRegister tail')
        generated,_ = self.continued(source,
            'iProcsForModule Chains -iProcNameSpace ops\n'+DEFINITIONS+'''
iProcsForModule Chains
iReset; iScan client 10 -si 0b1011110011 -so 0bx; iApply
iTake held; iState held 5
iMerge -begin; iCall ops::first; iCall ops::second; iMerge -end
iRelease held
''','''iNote [iGetReadData child0.local -bin]; iNote [iGetReadData child1.local -bin]
iNote [iGetStatus]
''',[b'0bxxxxxxxx',b'0bxxxxxxxx',b'0'])
        self.assertNotIn(b'__opaque_scan_state',generated)

    def test_default_regeneration_retains_merge_histories_without_fabrication(self):
        icl,pdl,tail = (self.root/n for n in ('source.icl','source.pdl','tail.pdl'))
        icl.write_text(model(2,8))
        pdl.write_text('iPDLLevel 1\niProcsForModule Chains\n'+DEFINITIONS+'''iReset
iScan client 10 -si 0b1011110011 -so 0bx; iApply
iMerge -begin; iCall first; iCall second; iMerge -end
''')
        prefix = self.run_tool(icl,pdl,'prefix.pdl',modeled=False)
        self.assertEqual(self.run_tool(icl,self.root/'prefix.pdl','again.pdl',modeled=False),prefix)
        tail.write_text('iPDLLevel 1\niGetReadData child0.local\n')
        self.assertIn('requires hardware readback',self.run_tool(icl,self.root/'again.pdl',
                      'missing.pdl',modeled=False,ok=False,extra=('--pdl',str(tail))))
        tail.write_text('''iPDLLevel 1
iNote [iGetReadData child0.local -bin]; iNote [iGetReadData child1.local -bin]
iNote [iGetMiscompares child0.local -bin]; iNote [iGetMiscompares child1.local -bin]
iNote [iGetStatus -clear]; iNote [iGetStatus]
''')
        result = self.run_tool(icl,self.root/'again.pdl','observed.pdl',extra=('--pdl',str(tail)))
        self.assertEqual(self.notes(result),[b'0bxxxxxxxx',b'0bxxxxxxxx',b'0b00000000',b'0b00000000',b'0',b'0'])

    def test_original_local_reset_retains_merge_calls_and_procedure_definitions(self):
        icl,pdl = self.root/'source.icl',self.root/'source.pdl'
        icl.write_text(model(2,8))
        pdl.write_text('iPDLLevel 1\niProcsForModule Chains\n'+DEFINITIONS+'''iReset
iMerge -begin; iCall first; iCall second; iMerge -end
iNote [iGetReadData child0.local -bin]; iNote [iGetReadData child1.local -bin]
iNote [iGetStatus -clear]; iNote [iGetStatus]
''')
        reference = self.run_tool(icl,pdl,'reference.pdl')
        self.assertEqual(self.notes(reference),[b'0bxxxxxxxx',b'0bxxxxxxxx',b'0',b'0'])
        for source_icl in (icl,self.root/'debug/Chains.icl'):
            reset = self.root/'reset.pdl'
            result = self.run_tool(source_icl,pdl,'reset/result.pdl',extra=('--local_reset',str(reset)))
            self.assertEqual(result,reference)
            self.assertIn('iProc first',reset.read_text())
            self.assertIn('iMerge -begin',reset.read_text())
            self.assertEqual(self.run_tool(source_icl,reset,'reset/replayed.pdl'),reference)

    def test_standalone_root_physical_and_event_boundaries_remain_rejected(self):
        icl,pdl,out = (self.root/n for n in ('source.icl','source.pdl','protected.pdl'))
        leaf = model(2,8).split('Module Chains {',1)[0].replace('Module Leaf','Module Chains')
        cases = [
            (leaf,'iProc first {} {iScan local 8 -si 150 -so 0bx -stable; iApply}',(), 'opaque'),
            (model(2,8),DEFINITIONS,('--physical_pdl',),'physical'),
            (model(2,8),'iProc first {} {iRunLoop 1 -tck}',(),'opaque'),
        ]
        for source,definitions,extra,diagnostic in cases:
            with self.subTest(extra=extra,definitions=definitions):
                icl.write_text(source)
                pdl.write_text('iPDLLevel 1\niProcsForModule Chains\n'+definitions+
                    '\niReset; iMerge -begin; iCall first; iMerge -end\n')
                out.write_bytes(b'keep existing output\n')
                self.assertIn(diagnostic,self.run_tool(icl,pdl,'protected.pdl',ok=False,extra=extra))
                self.assertEqual(out.read_bytes(),b'keep existing output\n')

    def test_later_bad_call_length_cannot_replace_existing_output(self):
        icl,pdl,out = (self.root/n for n in ('source.icl','source.pdl','protected.pdl'))
        icl.write_text(model(2,8))
        pdl.write_text('iPDLLevel 1\niProcsForModule Chains\n'+
            DEFINITIONS.replace('child1.local 8','child1.local 9')+'''iReset
iScan client 10 -si 0b1011110011 -so 0bx; iApply
iMerge -begin; iCall first; iCall second; iMerge -end
''')
        out.write_bytes(b'keep existing output\n')
        self.assertIn('length',self.run_tool(icl,pdl,'protected.pdl',ok=False))
        self.assertEqual(out.read_bytes(),b'keep existing output\n')


if __name__=='__main__':
    unittest.main(argv=[__file__])
