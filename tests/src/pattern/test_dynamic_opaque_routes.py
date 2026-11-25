#!/usr/bin/env python3
"""Opaque scalar routes execute only their active pre-CSU intervals."""
import unittest

import test_modeled_child_prefix as helpers
from benchmark_dynamic_opaque_routes import model

CASES = helpers.helpers.helpers.ROOT/'tests/cases/ours/pattern_generation'


def single():
    source = (CASES/'active_child_raw/model.icl').read_text()
    source = source.replace(" ScanRegister value[7:0] { ScanInSource si; CaptureSource value; ResetValue 8'b0; }\n",'')
    source = source.replace('ScanOutPort so { Source value[0]; }','ScanOutPort so;')
    return source.replace("Port reset; }","Port reset; DefaultLoadValue 8'b0; }",1).replace('Module Top','Module Chains')


def mixed():
    visible = helpers.model().split('Module Chains {',1)[0].replace('Module Leaf','Module Visible')
    return visible+model(2,8).replace('Instance child1 Of Leaf','Instance child1 Of Visible')


class DynamicOpaqueRoutes(unittest.TestCase):
    setUp = helpers.ModeledChildPrefix.setUp
    run_tool = helpers.ModeledChildPrefix.run_tool
    notes = helpers.ModeledChildPrefix.notes
    continued = helpers.ModeledChildPrefix.continued

    def test_root_opening_then_child_returns_unknown_without_root_overwrite(self):
        generated,_ = self.continued(single(),'''iReset
iScan client 1 -si 1 -so 0bx; iApply
iScan child.local 8 -si 165 -so 0bx; iApply
''','''iNote [iGetReadData child.local -bin]; iNote [iGetMiscompares child.local -bin]
iNote [iGetReadData client -bin]; iNote [iGetStatus]
''',[b'0bxxxxxxxx',b'0b00000000',b'0b0',b'0'])
        self.assertEqual(sum(line.startswith(b'iScan ') for line in generated.splitlines()),2)
        self.assertNotIn(b'__opaque_scan_state',generated)

    def test_closure_keeps_cared_unknown_child_history_and_unknown_status(self):
        self.continued(single(),'''iReset; iScan client 1 -si 1 -so 0bx; iApply
iScan child.local 8 -si 165 -so 0b0xxxxxxx; iApply
iScan client 9 -si 0 -so 0bx; iApply
''','''iNote [iGetReadData child.local -bin]; iNote [iGetMiscompares child.local -bin]
if {![catch {iGetStatus -clear}]} {error {unknown status returned}}
if {![catch {iGetStatus}]} {error {unknown status cleared}}
iNote unknown
''',[b'0bxxxxxxxx',b'0bx0000000',b'unknown'])

    def test_disjoint_children_preserve_actual_old_root_geometry(self):
        generated,_ = self.continued(model(2,8),'''iReset
iScan client 10 -si 0b1011110011 -so 0bx; iApply
iScan child0.local 8 -si 165 -so 0bx; iScan child1.local 8 -si 60 -so 0bx; iApply
iScan client 26 -si 0b10100000000000000000010011 -so 0bx; iApply
''','''iNote [iGetReadData child0.local -bin]; iNote [iGetReadData child1.local -bin]
iNote [iGetMiscompares child0.local -bin]; iNote [iGetMiscompares child1.local -bin]
iNote [iGetReadData client -bin]; iNote [iGetStatus]
''',[b'0bxxxxxxxx',b'0bxxxxxxxx',b'0b00000000',b'0b00000000',
     b'0b101xxxxxxxx1xxxxxxxx110011',b'0'])
        self.assertIn(b'iScan client 26 -si 0b10110100101100111100110011 -so ',generated)
        self.assertEqual(sum(line.startswith(b'iScan ') for line in generated.splitlines()),3)

    def test_root_only_scans_do_not_invent_child_history(self):
        self.continued(single(),'''iReset; iScan client 1 -si 1 -so 0bx; iApply
iScan client 9 -si 0b101001010 -so 0bx; iApply
''','''if {![catch {iGetReadData child.local}]} {error {unrequested child history invented}}
iNote [iGetReadData client -bin]; iNote [iGetStatus]
''',[b'0bxxxxxxxx1',b'0'])

    def test_inactive_sibling_keeps_history_without_requiring_implicit_length(self):
        close = '0b101'+'0'*8+'1'+'0'*8+'0'+'10011'
        generated,_ = self.continued(model(2,8),'''iReset
iScan client 10 -si 0b1011110011 -so 0bx; iApply
iScan child1.local 8 -si 90 -so 0b0xxxxxxx; iApply
iScan client 26 -si '''+close+''' -so 0bx; iApply
iScan child0.local 8 -si 165 -so 0bx -stable; iApply
''','''iNote [iGetReadData child0.local -bin]; iNote [iGetReadData child1.local -bin]
iNote [iGetMiscompares child0.local -bin]; iNote [iGetMiscompares child1.local -bin]
if {![catch {iGetStatus -clear}]} {error {unknown status returned}}
iNote unknown
''',[b'0bxxxxxxxx',b'0bxxxxxxxx',b'0b00000000',b'0bx0000000',b'unknown'])
        self.assertIn(b'iScan client 18 -si ',generated)

    def test_old_history_survives_closed_route_cancel_reset_and_reopen(self):
        self.continued(single(),'''iReset; iScan client 1 -si 1 -so 0bx; iApply
iScan child.local 8 -si 165 -so 0b10xxxxxx -stable; iApply
iScan client 9 -si 0 -so 0bx; iApply
''','''iScan child.local 9 -si 0; iReset
iScan client 1 -si 1 -so 0bx; iApply
iNote [iGetReadData child.local -bin]; iNote [iGetMiscompares child.local -bin]
iScan child.local 8 -si 60 -so 0bx; iApply
iNote [iGetMiscompares child.local -bin]
if {![catch {iGetStatus}]} {error {unknown status lost}}
iNote unknown
''',[b'0bxxxxxxxx',b'0bxx000000',b'0b00000000',b'unknown'])

    def test_default_regeneration_retains_opaque_intent_without_observations(self):
        icl,script,tail = (self.root/n for n in ('source.icl','source.pdl','tail.pdl'))
        icl.write_text(single())
        script.write_text('''iPDLLevel 1
iReset; iScan client 1 -si 1 -so 0bx; iApply
iScan child.local 8 -si 165 -so 0bx; iApply
''')
        prefix = self.run_tool(icl,script,'prefix.pdl',modeled=False)
        self.assertIn(b'modeled opaque child raw history ',prefix)
        self.assertEqual(self.run_tool(icl,self.root/'prefix.pdl','again.pdl',modeled=False),prefix)
        tail.write_text('iPDLLevel 1\niGetReadData child.local\n')
        self.assertIn('requires hardware readback',self.run_tool(icl,self.root/'again.pdl',
            'unavailable.pdl',modeled=False,ok=False,extra=('--pdl',str(tail))))
        tail.write_text('''iPDLLevel 1
iNote [iGetReadData child.local -bin]; iNote [iGetMiscompares child.local -bin]
iNote [iGetStatus -clear]; iNote [iGetStatus]
''')
        output = self.run_tool(icl,self.root/'again.pdl','observed.pdl',extra=('--pdl',str(tail)))
        self.assertEqual(self.notes(output),[b'0bxxxxxxxx',b'0b00000000',b'0',b'0'])

    def test_local_reset_preserves_namespaced_unknown_history_getters(self):
        icl,script,tail = (self.root/n for n in ('source.icl','source.pdl','tail.pdl'))
        icl.write_text(single())
        script.write_text('''iPDLLevel 1
iReset; iScan client 1 -si 1 -so 0bx; iApply
iScan child.local 8 -si 165 -so 0bx; iApply
iScan client 9 -si 0 -so 0bx; iApply
''')
        self.run_tool(icl,script,'prefix.pdl')
        definition = 'iProc result {} {iNote [iGetReadData child.local -bin]; iNote [iGetStatus -clear]}'
        tail.write_text('iPDLLevel 1\niProcsForModule Chains -iProcNameSpace ops\n'+definition+
                        '\niCall ops::result; iNote [iGetStatus]\n')
        reference = self.run_tool(icl,self.root/'prefix.pdl','reference.pdl',extra=('--pdl',str(tail)))
        self.assertEqual(self.notes(reference),[b'0bxxxxxxxx',b'0',b'0'])
        for source_icl in (icl,self.root/'debug/Chains.icl'):
            reset = self.root/'reset.pdl'
            output = self.run_tool(source_icl,self.root/'prefix.pdl','reset/result.pdl',
                extra=('--pdl',str(tail),'--local_reset',str(reset)))
            self.assertEqual(output,reference)
            self.assertIn(definition,reset.read_text())
            self.assertEqual(self.run_tool(source_icl,reset,'reset/replayed.pdl'),reference)

    def test_inverted_bypass_data_keeps_opaque_actual_bits_unknown(self):
        source = single().replace("1'b1: child.so;","1'b1: ~child.so;")
        self.continued(source,'''iReset; iScan client 1 -si 1 -so 0bx; iApply
iScan child.local 8 -si 165 -so 0bx; iApply
iScan client 9 -si 0 -so 0bx; iApply
''','''iNote [iGetReadData child.local -bin]; iNote [iGetMiscompares child.local -bin]
iNote [iGetReadData client -bin]; iNote [iGetStatus]
''',[b'0bxxxxxxxx',b'0b00000000',b'0bxxxxxxxx1',b'0'])

    def test_closed_root_note_cannot_replace_older_care_mask(self):
        generated,_ = self.continued(single(),'''iReset
iScan client 1 -si 1 -so 0bx; iApply
iScan child.local 8 -si 165 -so 0b0xxxxxxx; iApply
iScan client 9 -si 0 -so 0bx; iApply
iScan client 1 -si 0 -so 0bx; iApply
iNote -comment {modeled opaque child raw history client child.local 0bxxxxxxxx}
''','''iNote [iGetReadData child.local -bin]; iNote [iGetMiscompares child.local -bin]
if {![catch {iGetStatus}]} {error {unknown status lost}}
iNote unknown
''',[b'0bxxxxxxxx',b'0bx0000000',b'unknown'])
        self.assertIn(b'iNote -comment "modeled opaque child raw history client child.local 0bxxxxxxxx"',generated)

    def test_shared_deep_mux_arms_do_not_expand_exponentially(self):
        arms = ''
        for i in range(512):
            source = 'si' if not i else 'bypass%d'%(i-1)
            arms += " ScanMux bypass%d SelectedBy open {1'b0: %s; 1'b1: %s;}\n"%(i,source,source)
        source = single().replace(' ScanMux route',arms+' ScanMux route')
        source = source.replace("1'b0: si; 1'b1: child.so;","1'b0: bypass511; 1'b1: child.so;")
        generated,_ = self.continued(source,'''iReset
iScan client 1 -si 1 -so 0bx; iApply
iScan child.local 8 -si 165 -so 0bx; iApply
''','iNote [iGetReadData child.local -bin]; iNote [iGetStatus]\n',
            [b'0bxxxxxxxx',b'0'])
        self.assertEqual(sum(line.startswith(b'iScan ') for line in generated.splitlines()),2)

    def test_unselected_cycle_is_rejected_before_opening(self):
        source = single().replace('InputPort si = si;','InputPort si = open;')
        icl,pdl = self.root/'source.icl',self.root/'source.pdl'
        icl.write_text(source)
        pdl.write_text('iPDLLevel 1\niReset; iScan client 1 -si 1; iApply\n')
        self.assertIn('raw scan',self.run_tool(icl,pdl,'bad.pdl',ok=False))

    def test_held_selector_aliases_and_typed_duplicate_notes(self):
        source = single().replace(' ScanMux route',' Alias held = open;\n ScanMux route')
        source = source.replace('SelectedBy open','SelectedBy held')
        source = source.replace('InputPort sel = open;','InputPort sel = held;')
        generated,_ = self.continued(source,'''iReset
iScan client 1 -si 1 -so 0bx; iApply
iScan child.local 8 -si 60 -so 0bx; iApply
iNote -status {modeled opaque child raw history client child.local 0bxxxxxxxx}
iNote -status {modeled opaque child raw history client child.local 0bxxxxxxxx}
''','''iNote [iGetReadData child.local -bin]; iNote [iGetMiscompares child.local -bin]
iNote [iGetStatus]
''',[b'0bxxxxxxxx',b'0b00000000',b'0'])
        self.assertEqual(sum(line.startswith(b'iScan ') for line in generated.splitlines()),2)

    def test_general_opaque_merge_still_requires_its_existing_contract(self):
        icl,pdl = self.root/'source.icl',self.root/'source.pdl'
        icl.write_text(single())
        pdl.write_text('''iPDLLevel 1
iReset; iScan client 1 -si 1 -so 0bx; iApply
iMerge -begin; iMerge -end
''')
        self.assertIn('opaque',self.run_tool(icl,pdl,'bad.pdl',ok=False))

    def test_ordinary_parent_access_does_not_activate_an_inactive_opaque_leaf(self):
        for reclosed in (False,True):
            for command in ('iWrite head 5','iRead head 0'):
                with self.subTest(reclosed=reclosed,command=command):
                    prefix = 'iReset; iScan client 9 -si '+('0b000100000' if reclosed else '0')+' -so 0bx; iApply\n'
                    if reclosed:
                        prefix += '''iScan child0.local 8 -si 165 -so 0bx; iApply
iScan client 17 -si 0 -so 0bx; iApply
'''
                    prefix += command+'; iApply\n'
                    old_root = b'0b000xxxxxxxx100000' if reclosed else b'0b000000000'
                    generated,reference = self.continued(model(),prefix,
                        'iNote [iGetReadData client -bin]; iNote [iGetStatus]\n',[old_root,b'0'])
                    self.assertEqual(generated.count(b'\niApply\n'),4 if reclosed else 2)
                    twin = self.root/'twin.icl';twin.write_text(model(visible=True))
                    control = self.run_tool(twin,self.root/'full.pdl','twin.pdl')
                    self.assertEqual(reference.replace(b'modeled opaque child raw history ',
                                                       b'modeled dynamic child raw history '),control)

    def test_modeled_child_history_survives_an_inactive_opaque_sibling(self):
        generated,_ = self.continued(mixed(),'''iReset
iScan client 10 -si 0b0000100000 -so 0bx; iApply
iScan child1.local 8 -si 165 -so 0; iApply
''','''iNote [iGetReadData child1.local -bin]; iNote [iGetMiscompares child1.local -bin]
iNote [iGetReadData client -bin]; iNote [iGetStatus]
if {![catch {iGetReadData child0.local}]} {error {inactive opaque history invented}}
''',[b'0b00000000',b'0b00000000',b'0b0000000000',b'0'])
        self.assertIn(b'modeled dynamic child raw history client child1.local 0b00000000',generated)

    def test_mixed_requests_keep_both_history_families_and_default_regeneration(self):
        generated,_ = self.continued(mixed(),'''iReset
iScan client 10 -si 0b0001100000 -so 0bx; iApply
iScan child0.local 8 -si 60 -so 0bx
iScan child1.local 8 -si 165 -so 0; iApply
''','''iNote [iGetReadData child1.local -bin]; iNote [iGetMiscompares child1.local -bin]
iNote [iGetReadData child0.local -bin]; iNote [iGetMiscompares child0.local -bin]
iNote [iGetStatus]
''',[b'0b00000000',b'0b00000000',b'0bxxxxxxxx',b'0b00000000',b'0'])
        self.assertIn(b'modeled dynamic child raw history client child1.local 0b00000000',generated)
        self.assertIn(b'modeled opaque child raw history client child0.local 0bxxxxxxxx',generated)
        self.assertEqual(self.run_tool(self.root/'source.icl',self.root/'prefix.pdl',
                                      'default.pdl',modeled=False),generated)

    def test_identical_mux_arms_do_not_qualify_as_an_opaque_bypass(self):
        for upstream in (False,True):
            with self.subTest(upstream=upstream):
                source = model().replace("CaptureSource enable0; ResetValue 0;",
                                         "CaptureSource enable0; ResetValue 1;")
                source = source.replace("DefaultLoadValue 9'b0;","DefaultLoadValue 17'b0;")
                if upstream:
                    source = source.replace('InputPort si = head[0];','InputPort si = route0;')
                    source = source.replace("1'b1: child0.so;","1'b1: head[0];")
                    source = source.replace('ScanInSource route0;','ScanInSource child0.so;')
                else:
                    source = source.replace("1'b0: head[0];","1'b0: child0.so;")
                icl,pdl = self.root/'source.icl',self.root/'source.pdl'
                icl.write_text(source)
                pdl.write_text('iPDLLevel 1\niReset; iScan child0.local 8 -si 1 -so 0bx; iApply\n')
                self.assertIn('raw scan',self.run_tool(icl,pdl,'bad.pdl',ok=False))

    def test_real_bypass_is_independent_of_mux_choice_order(self):
        source = single().replace("1'b0: si; 1'b1: child.so;",
                                  "1'b1: child.so; 1'b0: si;")
        self.continued(source,'''iReset
iScan client 1 -si 1 -so 0bx; iApply
iScan child.local 8 -si 165 -so 0bx; iApply
''','iNote [iGetReadData child.local -bin]; iNote [iGetStatus]\n',
            [b'0bxxxxxxxx',b'0'])

    def test_static_modeled_child_with_opaque_sibling_keeps_history(self):
        visible = helpers.model().split('Module Chains {',1)[0].replace('Module Leaf','Module Visible')
        opaque = model().split('Module Chains {',1)[0]
        root = helpers.model(2,8).split('Module Chains {',1)[1]
        source = opaque+visible+'Module Chains {'+root.replace('Instance child1 Of Leaf','Instance child1 Of Visible')
        generated,_ = self.continued(source,'''iReset
iScan child1.local 8 -si 165 -so 0; iApply
''','''iNote [iGetReadData child1.local -bin]; iNote [iGetMiscompares child1.local -bin]
iNote [iGetStatus]
if {![catch {iGetReadData child0.local}]} {error {implicit opaque history invented}}
''',[b'0b00000000',b'0b00000000',b'0'])
        self.assertIn(b'modeled child raw history client child1.local 0b00000000',generated)

    def test_selector_correlations_cannot_create_a_fictitious_bypass(self):
        source = model().replace("CaptureSource enable0; ResetValue 0;",
                                 "CaptureSource enable0; ResetValue 1;")
        source = source.replace("DefaultLoadValue 9'b0;","DefaultLoadValue 17'b0;")
        source = source.replace(
            "ScanMux route0 SelectedBy enable0 {1'b0: head[0]; 1'b1: child0.so;}",
            "ScanMux route0 SelectedBy enable0 {1'b0: child0.so; 1'b1: alternate;}\n"
            " ScanMux alternate SelectedBy enable0 {1'b0: head[0]; 1'b1: child0.so;}")
        icl,pdl = self.root/'source.icl',self.root/'source.pdl'
        icl.write_text(source)
        pdl.write_text('iPDLLevel 1\niReset; iScan child0.local 8 -si 1 -so 0bx; iApply\n')
        self.assertIn('raw scan',self.run_tool(icl,pdl,'bad.pdl',ok=False))

    def test_bypass_proof_batches_more_than_sixty_four_held_selectors(self):
        opened = '0b101'+'1'*65+'10011'
        generated,_ = self.continued(model(65,1),
            'iReset; iScan client 73 -si '+opened+''' -so 0bx; iApply
iScan child0.local 1 -si 1 -so 0bx
iScan child64.local 1 -si 0 -so 0bx; iApply
''','''iNote [iGetReadData child0.local -bin]; iNote [iGetReadData child64.local -bin]
iNote [iGetMiscompares child0.local -bin]; iNote [iGetMiscompares child64.local -bin]
iNote [iGetStatus]
''',[b'0bx',b'0bx',b'0b0',b'0b0',b'0'])
        self.assertEqual(sum(line.startswith(b'iScan ') for line in generated.splitlines()),2)


if __name__=='__main__':
    unittest.main(argv=[__file__])
