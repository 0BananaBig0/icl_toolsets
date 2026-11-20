#!/usr/bin/env python3
"""Closed modeled child scans emit compatible parent preparation."""
import unittest
import subprocess

import test_modeled_root_scan as modeled


class AutomaticChildRaw(unittest.TestCase):
    setUp=modeled.ModeledRootScan.setUp
    files=modeled.ModeledRootScan.files
    def run_tool(self,*args,**kwargs):
        try:return modeled.ModeledRootScan.run_tool(self,*args,**kwargs)
        except subprocess.TimeoutExpired:
            self.fail('automatic child preparation exceeded the existing 15-second workload target')

    def replay(self,commands,model=modeled.SIB):
        return modeled.opaque.OpaqueRootScan.replay(self,commands,model)

    def rejected(self,commands,diagnostic,model=modeled.SIB):
        return modeled.opaque.OpaqueRootScan.rejected(self,commands,diagnostic,model)

    def static_port_model(self):
        return (modeled.opaque.ROOT / 'tests/cases/ours/pattern_generation/static_child_scans/model.icl').read_text()

    def test_compatible_child_port_lock_and_followup(self):
        model = self.static_port_model()
        body = ('iReset; iTake sel; iState sel 1; '
                'iScan child.local 8 -si 0xa5 -so 0; iApply; '
                'iWrite child.value[0] 0; iApply; iRead child.value 0xa4; iApply\n')
        result = self.replay(body, model)
        self.assertIn('iWrite child.value 0b10100100', result)

    def test_conflicting_child_port_locks_remain_atomic(self):
        model = self.static_port_model().replace(
            ' ScanOutPort so { Source value[0]; }',
            ' DataOutPort observed { Source value[0]; }\n'
            ' ScanOutPort so { Source value[0]; }')
        model = model.replace('Module Top {',
            'Module Top { DataOutPort observed {Source child.observed;}')
        self.rejected('iReset; iTake sel; iState sel 0; '
            'iScan child.local 8 -si 0xa5; iApply\n', 'reserved', model)
        # A read in a single stream does not assign a controlling value.
        self.replay('iReset; iTake observed; iRead observed 0; iApply; '
                    'iScan child.local 8 -si 0xa5; iApply; '
                    'iRead observed 1; iApply\n', model)
        self.replay('iReset; iTake observed; iRead observed 0; iApply; '
                    'iRelease observed; iScan child.local 8 -si 0xa5; iApply\n', model)

    def test_scoped_child_port_owner_returns(self):
        model = self.static_port_model()
        result = self.replay('iProcsForModule Top\n'
            'iProc load {} { iTake sel; iState sel 1; '
            'iScan child.local 8 -si 0xa5; iApply }\n'
            'iReset; iCall load; iState sel 0; '
            'iWrite child.value[0] 0; iApply; iRead child.value 0xa4; iApply\n', model)
        self.assertIn('iWrite child.value 0b10100100', result)

    def test_closed_child_matches_explicit_setup_and_partial_history(self):
        scan='iScan child.local 8 -si 0x96 -so 0 -stable; iApply\n'
        tail='iWrite child.value[0] 1; iApply; iRead child.value 0x97; iRead open 1; iApply\n'
        manual=self.replay('iReset; iWrite open 1; iApply; '+scan+tail)
        automatic=self.replay('iReset; '+scan+tail)
        self.assertEqual(automatic,manual)
        self.assertIn('iWrite open 0b1\niApply\niScan client 9',automatic)
        self.assertEqual(automatic.count('iScan '),1)

    def test_nested_closed_child_has_only_parent_prerequisites(self):
        model=(modeled.opaque.ROOT/'tests/cases/ours/pattern_generation/dynamic_child_raw/model.icl').read_text()
        scan='iScan child.leaf.local 8 -si 0x96 -so 0 -stable; iApply\n'
        manual=self.replay('iReset; iWrite open 1; iWrite child.enable 1; iApply; '+scan,model)
        automatic=self.replay('iReset; '+scan,model)
        self.assertEqual(automatic,manual)
        self.assertEqual(automatic.count('iScan '),1)
        self.assertIn('iScan client 10',automatic)

    def test_compatible_closed_children_share_preparation(self):
        model=modeled.native_bench.many_model(2)
        scans='iScan r0.leaf.local 8 -si 0x96 -so 0; iScan r1.leaf.local 8 -si 0x3c -so 0; iApply\n'
        manual=self.replay('iReset; iWrite r0.enable 1; iWrite r1.enable 1; iApply; '+scans,model)
        automatic=self.replay('iReset; '+scans,model)
        self.assertEqual(automatic,manual)
        self.assertEqual(automatic.count('iScan '),1)

    def test_canceled_closed_child_does_not_prepare(self):
        result=self.replay('iScan child.local 7 -si 0; iReset; iApply\n')
        self.assertNotIn('iWrite',result)
        self.assertNotIn('iScan',result)

    def test_maximum_leaf_uses_existing_native_setup(self):
        width=1048575
        model=modeled.SIB.replace('[7:0]',f'[{width-1}:0]').replace("ResetValue 8'b0",f"ResetValue {width}'b0")
        scan=f'iScan child.local {width} -si 0x96 -so 0 -stable; iApply\n'
        manual=self.replay('iReset; iWrite open 1; iApply; '+scan,model)
        self.assertEqual(self.replay('iReset; '+scan,model),manual)

    def test_many_closed_parents_prepare_as_one_batch(self):
        count=1024
        model=modeled.native_bench.many_model(count)
        scans=''.join(f'iScan r{i}.leaf.local 8 -si 0x96;\n' for i in range(count))+'iApply\n'
        setup=''.join(f'iWrite r{i}.enable 1;\n' for i in range(count))+'iApply\n'
        manual=self.replay('iReset; '+setup+scans,model)
        self.assertEqual(self.replay('iReset; '+scans,model),manual)

    def test_parent_protection_and_child_unknown_mask_are_atomic(self):
        self.rejected('iReset; iTake open; iState open 0; '
            'iScan child.local 8 -si 0x96; iApply\n','reserved')
        self.rejected('iReset; iTake child.value[3]; iState child.value[3] 0; '
            'iScan child.local 8 -si 0b10x0xx01; iApply\n','reserved')
        result=self.replay('iReset; iScan child.local 8 -si 0b10x01x01; iApply; '
            'iWrite child.value[1] 1; iApply; iRead child.value 0x8b; iApply\n')
        self.assertIn('iWrite child.value 0b10001011',result)

    def test_owned_raw_refresh_and_owner_return(self):
        source='iProc load {} {iTake child.value; iState child.value 0; '
        source+='iScan child.local 8 -si 0x96 -so 0; iApply}\n'
        source+='iReset; iCall load; iWrite child.value[0] 1; iApply; '
        source+='iRead child.value 0x97; iRead open 1; iApply\n'
        result=self.replay(source)
        self.assertIn('iWrite child.value 0b10010111',result)

    def test_stable_failure_after_setup_and_following_valid_retry(self):
        model=(modeled.opaque.ROOT/'tests/cases/ours/pattern_generation/dynamic_child_raw/model.icl').read_text()
        self.rejected('iReset; iScan child.local 1 -si 1 -stable; iApply\n','stable',model)
        result=self.replay('iReset; iScan child.local 1 -si 1; iApply; '
            'iScan child.leaf.local 8 -si 0x96 -so 0 -stable; iApply; '
            'iRead child.leaf.value 0x96; iRead open 1; iRead child.enable 1; iApply\n',model)
        self.assertIn('iScan client 2 -si 0b11',result)
        self.assertIn('iScan client 10 -si 0b1001011011',result)

    def test_post_setup_overlap_unknown_local_route_and_scoped_call(self):
        self.rejected('iReset; iScan client 9 -si 1; '
            'iScan child.local 8 -si 0x96; iApply\n','overlap')
        dynamic=(modeled.opaque.ROOT/'tests/cases/ours/pattern_generation/dynamic_child_raw/model.icl').read_text()
        self.rejected('iReset; iState child.enable 0bx; '
            'iScan child.local 1 -si 0; iApply\n','known',dynamic)
        source='iProcsForModule RawSibLeaf\niProc load {} {iScan local 8 -si 0x96; iApply}\n'
        source+='iProcsForModule Top\niReset; iCall child.load; iRead child.value 0x96; iApply\n'
        self.assertIn('iScan client 9',self.replay(source))

    def test_parity_aliases_and_ascending_local_register(self):
        for flags in range(8):
            for ascending in (False,True):
                with self.subTest(flags=flags,ascending=ascending):
                    model=modeled.SIB
                    if flags&1:model=model.replace('InputPort si = si;','InputPort si = ~si;')
                    if flags&2:model=model.replace('Source value[0];','Source ~value[0];')
                    if flags&4:model=model.replace('ScanOutPort so { Source open; }',
                        'ScanOutPort so { Source ~open; }')
                    if ascending:model=model.replace('value[7:0]','value[0:7]').replace('value[0];','value[7];')
                    model=model.replace('ScanInSource si; CaptureSource value;',
                        'ScanInSource forwarding; CaptureSource value;')
                    model=model.replace(' ScanRegister value', ' Alias forwarding = si;\n ScanRegister value')
                    raw='iScan child.local 8 -si 0x96 -so 0bxxxxxxxx -stable; iApply\n'
                    manual=self.replay('iReset; iWrite open 1; iApply; '+raw,model)
                    self.assertEqual(self.replay('iReset; '+raw,model),manual)


if __name__=='__main__':
    unittest.main()
