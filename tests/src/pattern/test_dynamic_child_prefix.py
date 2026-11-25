#!/usr/bin/env python3
"""Dynamic child histories use the route that produced the return word."""
import unittest

import test_modeled_child_prefix as static
from benchmark_modeled_child_prefix import model as static_model
from benchmark_dynamic_child_prefix import model as dynamic_model

CASES = static.helpers.helpers.ROOT/'tests/cases/ours/pattern_generation'
NESTED_PREFIX = '''
iReset; iScan client 1 -si 1; iApply
iScan child.local 1 -si 1 -so 0; iApply
iScan child.local 9 -si 0b101001011 -so 0; iApply
iScan child.local 9 -si 0b001111000 -so 0; iApply
'''


def fixture(name):
    return (CASES/name).read_text().replace('Module Top','Module Chains')


class DynamicChildPrefix(unittest.TestCase):
    setUp = static.ModeledChildPrefix.setUp
    run_tool = static.ModeledChildPrefix.run_tool
    notes = static.ModeledChildPrefix.notes
    continued = static.ModeledChildPrefix.continued

    def test_nested_variable_width_history_survives_local_closure(self):
        generated,_ = self.continued(fixture('dynamic_child_raw/model.icl'),NESTED_PREFIX,
'''iNote [iGetReadData child.local -bin]
iNote [iGetMiscompares child.local -bin]
iNote [iGetReadData client -bin]; iNote [iGetStatus]
''',[b'0b101001011',b'0b101001011',b'0b0',b'6'])
        self.assertEqual(generated.count(b'modeled dynamic child raw history '),3)
        self.assertEqual(sum(line.startswith(b'iScan ') for line in generated.splitlines()),4)

    def test_active_child_stable_route_keeps_actual_data(self):
        self.continued(fixture('active_child_raw/model.icl'),'''
iReset; iScan client 1 -si 1; iApply
iWrite child.value 165; iApply
iScan child.local 8 -si 60 -so 0 -stable; iApply
''','''iNote [iGetReadData child.local -dec]
iNote [iGetMiscompares child.local -dec]; iNote [iGetStatus]
''',[b'165',b'165',b'4'])

    def test_disjoint_same_type_children_keep_old_long_histories(self):
        self.continued(dynamic_model(2,8),'''iReset
iWrite head 5; iWrite tail 19; iWrite child0.enable 1; iWrite child1.enable 1; iApply
iWrite child0.leaf.value 165; iWrite child1.leaf.value 60; iApply
iScan child0.local 9 -si 0b001111000 -so 0
iScan child1.local 9 -si 0b101001010 -so 0; iApply
''','''iNote [iGetReadData child0.local -dec]; iNote [iGetReadData child1.local -dec]
iNote [iGetMiscompares child0.local -dec]; iNote [iGetMiscompares child1.local -dec]
iNote [iGetStatus]
if {![catch {iGetReadData child0.leaf.local}]} {error {unrequested leaf history invented}}
if {![catch {iGetReadData client}]} {error {unrequested root history invented}}
''',[b'331',b'121',b'331',b'121',b'10'])

    def test_unknown_old_child_history_survives_canceled_queue_and_reset(self):
        source = fixture('active_child_raw/model.icl').replace("ResetValue 8'b0",
                                                               "ResetValue 8'bxxxxxxxx")
        self.continued(source,'''iReset; iScan client 1 -si 1; iApply
iScan child.local 8 -si 60 -so 0b0xxxxxxx; iApply
''','''iScan child.local 8 -si 0 -so 0; iReset
iNote [iGetReadData child.local -bin]; iNote [iGetMiscompares child.local -bin]
if {![catch {iGetStatus -clear}]} {error {unknown status returned}}
if {![catch {iGetStatus}]} {error {unknown status cleared}}
''',[b'0bxxxxxxxx',b'0bx0000000'])

    def test_default_regeneration_keeps_dynamic_intent_without_readback(self):
        icl,script,tail = (self.root/n for n in ('source.icl','source.pdl','tail.pdl'))
        icl.write_text(fixture('dynamic_child_raw/model.icl'))
        script.write_text('iPDLLevel 1\n'+NESTED_PREFIX)
        prefix = self.run_tool(icl,script,'prefix.pdl',modeled=False)
        self.assertEqual(prefix.count(b'modeled dynamic child raw history '),3)
        self.assertEqual(self.run_tool(icl,self.root/'prefix.pdl','again.pdl',modeled=False),prefix)
        tail.write_text('iPDLLevel 1\niGetReadData child.local\n')
        self.assertIn('preceding applied iScan',self.run_tool(icl,self.root/'again.pdl',
            'unavailable.pdl',modeled=False,ok=False,extra=('--pdl',str(tail))))
        tail.write_text('''iPDLLevel 1
iNote [iGetReadData child.local -dec]; iNote [iGetMiscompares child.local -dec]
iNote [iGetStatus -clear]; iNote [iGetStatus]
''')
        output = self.run_tool(icl,self.root/'again.pdl','observed.pdl',extra=('--pdl',str(tail)))
        self.assertEqual(self.notes(output),[b'331',b'331',b'6',b'0'])

    def test_default_chain_spelling_keeps_dynamic_note_inert(self):
        generated,_ = self.continued(fixture('active_child_raw/model.icl'),'''
iReset; iScan client 1 -si 1; iApply; iWrite child.value 165; iApply
iScan child.local 8 -si 60 -so 0; iApply
iScan client -chain default 9 -si 0b000000000 -so 0b00000000x; iApply
iNote -comment {modeled dynamic child raw history client child.local 0b00000000}
''','''iNote [iGetReadData child.local -dec]; iNote [iGetMiscompares child.local -dec]
iNote [iGetStatus]
''',[b'165',b'165',b'8'])
        self.assertIn(b'informational modeled dynamic child raw history ',generated)

    def test_static_and_dynamic_note_families_do_not_cross_activate(self):
        for dynamic in (False,True):
            with self.subTest(dynamic=dynamic):
                source = fixture('active_child_raw/model.icl') if dynamic else static_model()
                width = 9 if dynamic else 16
                prefix = 'iReset\n'+('iScan client 1 -si 1; iApply\n' if dynamic else '')
                prefix += 'iScan client %d -si 0b%s -so 0b%s; iApply\n'%(width,'0'*width,'x'*width)
                family = 'modeled child raw history ' if dynamic else 'modeled dynamic child raw history '
                child = 'child.local' if dynamic else 'child0.local'
                prefix += 'iNote -comment {'+family+'client '+child+' 0bxxxxxxxx}\n'
                generated,_ = self.continued(source,prefix,
                    'if {![catch {iGetReadData '+child+'}]} {error {history invented}}\n'
                    'iNote absent; iNote [iGetStatus]\n',[b'absent',b'0'])
                self.assertIn(b'informational '+family.encode(),generated)

    def test_note_for_closed_pre_route_preserves_older_child_history(self):
        generated,_ = self.continued(fixture('active_child_raw/model.icl'),'''
iReset; iScan client 1 -si 1; iApply; iWrite child.value 165; iApply
iScan child.local 8 -si 60 -so 0; iApply
iScan client 9 -si 0b000000000 -so 0bxxxxxxxxx; iApply
iScan client 1 -si 0b0 -so 0bx; iApply
iNote -comment {modeled dynamic child raw history client child.local 0bxxxxxxxx}
''','''iNote [iGetReadData child.local -dec]; iNote [iGetMiscompares child.local -dec]
iNote [iGetReadData client -bin]; iNote [iGetStatus]
''',[b'165',b'165',b'0b0',b'4'])
        self.assertIn(b'informational modeled dynamic child raw history ',generated)

    def test_typed_duplicate_dynamic_intents_do_not_recount(self):
        self.continued(fixture('dynamic_child_raw/model.icl'),NESTED_PREFIX,
            'iNote [iGetReadData child.local -dec]; iNote [iGetStatus]\n',[b'331',b'6'])
        prefix = (self.root/'prefix.pdl').read_bytes()
        marker = b'iNote -status {modeled dynamic child raw history client child.local 0b000000000}\n'
        (self.root/'duplicate.pdl').write_bytes(prefix+marker+marker)
        output = self.run_tool(self.root/'source.icl',self.root/'duplicate.pdl','duplicated.pdl',
                               extra=('--pdl',str(self.root/'tail.pdl')))
        self.assertEqual(self.notes(output),[b'331',b'6'])
        self.assertEqual(self.run_tool(self.root/'source.icl',self.root/'duplicated.pdl',
            'again.pdl'),output)

    def test_elided_ownership_does_not_activate_unconsumed_dynamic_note(self):
        generated,_ = self.continued(fixture('dynamic_child_raw/model.icl'),
            NESTED_PREFIX+'''iTake open; iRelease open
iNote -comment {modeled dynamic child raw history client child.local 0b000000000}
''','iNote [iGetReadData child.local -dec]; iNote [iGetStatus]\n',[b'331',b'6'])
        self.assertIn(b'informational modeled dynamic child raw history ',generated)
        self.assertNotIn(b'iTake ',generated)

    def test_generated_dynamic_prefix_local_reset_keeps_namespaced_getters(self):
        icl,script,tail = (self.root/n for n in ('source.icl','source.pdl','tail.pdl'))
        icl.write_text(fixture('dynamic_child_raw/model.icl'))
        script.write_text('iPDLLevel 1\n'+NESTED_PREFIX)
        self.run_tool(icl,script,'prefix.pdl')
        definition = 'iProc result {} {iNote [iGetReadData child.local -dec]; iNote [iGetStatus -clear]}'
        tail.write_text('iPDLLevel 1\niProcsForModule Chains -iProcNameSpace ops\n'+definition+
                        '\niCall ops::result; iNote [iGetStatus]\n')
        reference = self.run_tool(icl,self.root/'prefix.pdl','reference.pdl',extra=('--pdl',str(tail)))
        self.assertEqual(self.notes(reference),[b'331',b'6',b'0'])
        for source_icl in (icl,self.root/'debug/Chains.icl'):
            reset = self.root/'reset.pdl'
            output = self.run_tool(source_icl,self.root/'prefix.pdl','reset/result.pdl',
                extra=('--pdl',str(tail),'--local_reset',str(reset)))
            self.assertEqual(output,reference)
            self.assertIn(definition,reset.read_text())
            self.assertEqual(self.run_tool(source_icl,reset,'reset/replayed.pdl'),reference)

    def test_called_dynamic_scan_keeps_old_history_after_later_root_closure(self):
        self.continued(fixture('active_child_raw/model.icl'),'''
iReset; iScan client 1 -si 1; iApply; iWrite child.value 165; iApply
iProc access {} {iScan child.local 8 -si 60 -so 0 -stable; iApply}
iProc close {} {iScan client 9 -si 0 -so 0bxxxxxxxxx; iApply}
iMerge -begin; iCall access; iCall close; iMerge -end
''','''iNote [iGetReadData child.local -dec]; iNote [iGetMiscompares child.local -dec]
iNote [iGetStatus]
''',[b'165',b'165',b'4'])

    def test_stable_child_uses_old_root_offset_after_parent_route_changes(self):
        self.continued(fixture('dynamic_child_raw/local_stability.icl'),'''
iReset
iScan child.local 8 -si 165 -so 0 -stable; iApply
iScan child.local 8 -si 60 -so 0 -stable; iApply
''','''iNote [iGetReadData child.local -dec]
iNote [iGetMiscompares child.local -dec]; iNote [iGetStatus]
if {![catch {iGetReadData client}]} {error {unrequested root history invented}}
''',[b'165',b'165',b'4'])

    def test_equal_width_root_parity_change_uses_old_output_polarity(self):
        source = static_model().replace(
            ' ScanOutPort so {Source value[0];}',
            ' DataOutPort gate {Source value[0];}\n ScanOutPort so {Source value[0];}')
        source = source.replace(' ScanOutPort so {Source tail[0];}', '''
 ScanMux output_route SelectedBy child0.gate {1'b0: tail[0]; 1'b1: ~tail[0];}
 ScanOutPort so {Source output_route;}''')
        self.continued(source,'''iReset
iScan child0.local 8 -si 165 -so 0 -stable; iApply
iScan child0.local 8 -si 60 -so 0 -stable; iApply
''','''iNote [iGetReadData child0.local -dec]
iNote [iGetMiscompares child0.local -dec]; iNote [iGetStatus]
''',[b'165',b'165',b'4'])


if __name__ == '__main__':
    unittest.main(argv=[__file__])
