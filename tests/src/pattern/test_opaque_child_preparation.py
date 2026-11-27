#!/usr/bin/env python3
"""Closed opaque children use real modeled parent-only preparation CSUs."""
import unittest

import test_modeled_child_prefix as helpers
from test_dynamic_opaque_routes import mixed, single
from benchmark_dynamic_opaque_routes import model


class OpaqueChildPreparation(unittest.TestCase):
    setUp = helpers.ModeledChildPrefix.setUp
    run_tool = helpers.ModeledChildPrefix.run_tool
    notes = helpers.ModeledChildPrefix.notes
    continued = helpers.ModeledChildPrefix.continued

    def test_closed_child_opens_once_before_raw_scan_and_keeps_root_unrequested(self):
        generated,_ = self.continued(single(),'''iReset
iScan child.local 8 -si 165 -so 0bx; iApply
''','''iNote [iGetReadData child.local -bin]; iNote [iGetMiscompares child.local -bin]
iNote [iGetStatus]
if {![catch {iGetReadData client}]} {error {root raw history invented}}
''',[b'0bxxxxxxxx',b'0b00000000',b'0'])
        self.assertIn(b'iWrite open 0b1\niApply\n',generated)
        self.assertEqual(generated.count(b'\niApply\n'),2)
        self.assertEqual(sum(line.startswith(b'iScan ') for line in generated.splitlines()),1)
        self.assertNotIn(b'__opaque_scan_state',generated)

    def test_two_closed_children_share_one_real_parent_opening(self):
        generated,_ = self.continued(model(2,8),'''iReset
iScan child0.local 8 -si 165 -so 0bx
iScan child1.local 8 -si 60 -so 0bx; iApply
''','''iNote [iGetReadData child0.local -bin]; iNote [iGetReadData child1.local -bin]
iNote [iGetMiscompares child0.local -bin]; iNote [iGetMiscompares child1.local -bin]
iNote [iGetStatus]
if {![catch {iGetReadData client}]} {error {root raw history invented}}
''',[b'0bxxxxxxxx',b'0bxxxxxxxx',b'0b00000000',b'0b00000000',b'0'])
        self.assertEqual(generated.count(b'\niApply\n'),2)
        self.assertIn(b'iScan client 26 -si ',generated)
        self.assertNotIn(b'__opaque_scan_state',generated)

    def test_reopen_after_nonstable_closure_preserves_old_root_history(self):
        generated,_ = self.continued(single(),'''iReset
iScan child.local 8 -si 165 -so 0bx; iApply
iScan client 9 -si 0 -so 0bx; iApply
iScan child.local 8 -si 60 -so 0bx; iApply
''','''iNote [iGetReadData child.local -bin]; iNote [iGetMiscompares child.local -bin]
iNote [iGetReadData client -bin]; iNote [iGetStatus]
''',[b'0bxxxxxxxx',b'0b00000000',b'0bxxxxxxxx1',b'0'])
        self.assertEqual(generated.count(b'iWrite open 0b1\niApply\n'),2)
        self.assertEqual(generated.count(b'\niApply\n'),5)

    def test_cared_unknown_result_cannot_be_cleared_into_known_status(self):
        self.continued(single(),'''iReset
iScan child.local 8 -si 165 -so 0b0xxxxxxx; iApply
''','''iNote [iGetReadData child.local -bin]; iNote [iGetMiscompares child.local -bin]
if {![catch {iGetStatus -clear}]} {error {unknown status returned}}
if {![catch {iGetStatus}]} {error {unknown status cleared}}
iNote unknown
''',[b'0bxxxxxxxx',b'0bx0000000',b'unknown'])

    def test_winning_request_and_canceled_length_do_not_change_preparation(self):
        generated,_ = self.continued(single(),'''iScan child.local 9 -si 0; iReset
iScan child.local 8 -si 165 -so 0
iScan child.local 8 -si 60 -so 0bx -stable; iApply
iScan child.local 8 -si 90 -so 0bx -stable; iApply
''','''iNote [iGetReadData child.local -bin]; iNote [iGetMiscompares child.local -bin]
iNote [iGetStatus]
''',[b'0bxxxxxxxx',b'0b00000000',b'0'])
        self.assertEqual(generated.count(b'iWrite open 0b1\niApply\n'),1)
        self.assertIn(b'-si 0b001111001 ',generated)
        self.assertIn(b'-si 0b010110101 ',generated)

    def test_alias_and_inverted_output_keep_actual_unknown_history(self):
        source = single().replace(' ScanMux route',' Alias held = open;\n ScanMux route')
        source = source.replace('SelectedBy open','SelectedBy held')
        source = source.replace('InputPort sel = open;','InputPort sel = held;')
        source = source.replace("1'b1: child.so;","1'b1: ~child.so;")
        generated,_ = self.continued(source,'''iReset
iScan child.local 8 -si 165 -so 0b01x0x1x0; iApply
''','''iNote [iGetReadData child.local -bin]; iNote [iGetMiscompares child.local -bin]
if {![catch {iGetStatus -clear}]} {error {unknown status returned}}
if {![catch {iGetStatus}]} {error {unknown status cleared}}
iNote unknown
''',[b'0bxxxxxxxx',b'0bxx0x0x0x',b'unknown'])
        scans = [line.split() for line in generated.splitlines() if line.startswith(b'iScan ')]
        self.assertEqual(len(scans),1)
        self.assertEqual(scans[0][:3],[b'iScan',b'client',b'9'])
        self.assertEqual(scans[0][scans[0].index(b'-so')+1],b'0b10x1x0x1x')

    def test_modeled_child_prepares_without_activating_opaque_sibling(self):
        generated,_ = self.continued(mixed(),'''iReset
iScan child1.local 8 -si 165 -so 0; iApply
''','''iNote [iGetReadData child1.local -bin]; iNote [iGetMiscompares child1.local -bin]
iNote [iGetStatus]
if {![catch {iGetReadData child0.local}]} {error {opaque history invented}}
if {![catch {iGetReadData client}]} {error {root raw history invented}}
''',[b'0b00000000',b'0b00000000',b'0'])
        self.assertEqual(generated.count(b'\niApply\n'),2)
        self.assertIn(b'modeled dynamic child raw history client child1.local 0b00000000',generated)

    def test_mixed_children_prepare_together_and_keep_both_history_families(self):
        generated,_ = self.continued(mixed(),'''iReset
iScan child0.local 8 -si 60 -so 0bx
iScan child1.local 8 -si 165 -so 0; iApply
''','''iNote [iGetReadData child0.local -bin]; iNote [iGetReadData child1.local -bin]
iNote [iGetMiscompares child0.local -bin]; iNote [iGetMiscompares child1.local -bin]
iNote [iGetStatus]
''',[b'0bxxxxxxxx',b'0b00000000',b'0b00000000',b'0b00000000',b'0'])
        self.assertEqual(generated.count(b'\niApply\n'),2)
        self.assertIn(b'modeled dynamic child raw history client child1.local 0b00000000',generated)
        self.assertIn(b'modeled opaque child raw history client child0.local 0bxxxxxxxx',generated)

    def test_preparation_through_active_opaque_sibling_uses_native_completion(self):
        generated,_ = self.continued(model(2,8),'''iReset
iScan client 10 -si 0b1011010011 -so 0bx; iApply
iScan child1.local 8 -si 165 -so 0bx; iApply
''','''iNote [iGetReadData child1.local -bin]; iNote [iGetMiscompares child1.local -bin]
if {![catch {iGetReadData child0.local}]} {error {implicit child history invented}}
iNote [iGetStatus]
''',[b'0bxxxxxxxx',b'0b00000000',b'0'])
        self.assertEqual(generated.count(b'\niApply\n'),3)
        self.assertNotIn(b'__opaque_scan_state',generated)

    def test_locked_parent_and_reset_state_keep_existing_rejections(self):
        for setup,diagnostic in (('iTake open; iState open 0;','unreserved parent'),
                                 ('iState reset 1;','state assumption for reset')):
            with self.subTest(setup=setup):
                icl,pdl = self.root/'source.icl',self.root/'source.pdl'
                icl.write_text(single())
                pdl.write_text('iPDLLevel 1\niProcsForModule Chains\niReset; '+setup+
                               ' iScan child.local 8 -si 165 -so 0bx; iApply\n')
                self.assertIn(diagnostic,self.run_tool(icl,pdl,'bad.pdl',ok=False))

    def test_default_regeneration_retains_intent_and_requires_real_readback(self):
        icl,pdl,tail = (self.root/n for n in ('source.icl','source.pdl','tail.pdl'))
        icl.write_text(single())
        pdl.write_text('iPDLLevel 1\niProcsForModule Chains\niReset; '
                       'iScan child.local 8 -si 165 -so 0bx; iApply\n')
        prefix = self.run_tool(icl,pdl,'prefix.pdl',modeled=False)
        self.assertEqual(self.run_tool(icl,self.root/'prefix.pdl','again.pdl',modeled=False),prefix)
        tail.write_text('iPDLLevel 1\niGetReadData child.local\n')
        self.assertIn('requires hardware readback',self.run_tool(icl,self.root/'again.pdl',
                      'missing.pdl',modeled=False,ok=False,extra=('--pdl',str(tail))))
        tail.write_text('''iPDLLevel 1
iNote [iGetReadData child.local -bin]; iNote [iGetMiscompares child.local -bin]
iNote [iGetStatus -clear]; iNote [iGetStatus]
''')
        result = self.run_tool(icl,self.root/'again.pdl','observed.pdl',extra=('--pdl',str(tail)))
        self.assertEqual(self.notes(result),[b'0bxxxxxxxx',b'0b00000000',b'0',b'0'])

    def test_namespaced_local_reset_replays_preparation_and_unknown_history(self):
        icl,pdl,tail = (self.root/n for n in ('source.icl','source.pdl','tail.pdl'))
        icl.write_text(single())
        pdl.write_text('''iPDLLevel 1\niProcsForModule Chains\niReset
iScan child.local 8 -si 165 -so 0bx; iApply
''')
        self.run_tool(icl,pdl,'prefix.pdl')
        definition = 'iProc result {} {iNote [iGetReadData child.local -bin]; iNote [iGetStatus -clear]}'
        tail.write_text('iPDLLevel 1\niProcsForModule Chains -iProcNameSpace ops\n'+definition+
                        '\niCall ops::result; iNote [iGetStatus]\n')
        reference = self.run_tool(icl,self.root/'prefix.pdl','reference.pdl',extra=('--pdl',str(tail)))
        self.assertEqual(self.notes(reference),[b'0bxxxxxxxx',b'0',b'0'])
        for source_icl in (icl,self.root/'debug/Chains.icl'):
            reset = self.root/'reset.pdl'
            result = self.run_tool(source_icl,self.root/'prefix.pdl','reset/result.pdl',
                extra=('--pdl',str(tail),'--local_reset',str(reset)))
            self.assertEqual(result,reference)
            self.assertIn(definition,reset.read_text())
            self.assertEqual(self.run_tool(source_icl,reset,'reset/replayed.pdl'),reference)


if __name__=='__main__':
    unittest.main(argv=[__file__])
