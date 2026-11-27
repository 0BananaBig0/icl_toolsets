#!/usr/bin/env python3
"""Automatic callback intent stays separate from actual primary observations."""
import unittest

import benchmark_automatic_primary_readback as scale
import test_modeled_child_prefix as helpers
from test_downstream_write_callbacks import MODEL

PROCS = '''iProc store {data reg} {
 incr ::calls; iWrite en 0; iWrite p $data
}
set ::calls 0
'''


class AutomaticPrimaryReadback(unittest.TestCase):
    setUp = helpers.ModeledChildPrefix.setUp
    run_tool = helpers.ModeledChildPrefix.run_tool
    notes = helpers.ModeledChildPrefix.notes
    continued = helpers.ModeledChildPrefix.continued

    def test_data_objective_order_precedes_drive_count(self):
        for first_width, second_width in ((1, 64), (33, 1), (31, 32)):
            with self.subTest(first_width=first_width, second_width=second_width):
                source = f'''Module Chains {{
 DataInPort en; DataInPort p;
 DataRegister first[{first_width-1}:0] {{WriteDataSource {first_width}'b{'1'*first_width}; WriteEnSource en; ResetValue 0;}}
 DataRegister second[{second_width-1}:0] {{WriteDataSource {second_width}'b{'1'*second_width}; WriteEnSource ~en; ResetValue 0;}}
 DataOutPort qfirst[{first_width-1}:0] {{Source first;}}
 DataOutPort qsecond[{second_width-1}:0] {{Source second;}}
}}'''
                _, reference = self.continued(
                    source, 'iReset; iState en 1; iWrite p 0; iApply; '
                    'iRead qfirst 0; iRead qsecond 0bx; iApply;\n',
                    'iNote first=[iGetReadData qfirst -bin]; '
                    'iNote second=[iGetReadData qsecond -bin];\n',
                    [b'first=0b' + b'0'*first_width,
                     b'second=0b' + b'1'*second_width])
                self.assertIn(b'iWrite en 0b0\n', reference.split(b'iApply\n')[0])

    def test_larger_primary_callbacks_preserve_actual_replay(self):
        for count, width in ((32, 8), (1, 1024)):
            with self.subTest(lanes=count, width=width):
                icl, pdl, default, generated = (
                    self.root / name for name in
                    ('source.icl', 'source.pdl', 'default.pdl', 'generated.pdl'))
                icl.write_text(scale.model(count, width))
                pdl.write_text(scale.commands(count, width)[0])
                default.write_text(scale.commands(count, width, False)[0])
                hardware = self.run_tool(icl, default, 'hardware.pdl', modeled=False)
                reference = self.run_tool(icl, pdl, 'reference.pdl')
                self.assertEqual(scale.hardware_output(reference, count, width), hardware)
                for index in range(count):
                    self.assertEqual(hardware.count(('iNote cb%d\n' % index).encode()), 1)
                generated.write_text(scale.replay_observations(reference, count, width))
                self.assertEqual(self.run_tool(self.root / 'debug/Chains.icl', pdl,
                                               'flat/result.pdl'), reference)
                self.assertEqual(self.run_tool(icl, generated, 'generated/result.pdl'),
                                 reference)

    def test_automatic_intent_does_not_become_actual_downstream_data(self):
        self.continued(MODEL,PROCS+'''iReset; iWrite dest 9; iApply
if {$::calls != 1} {error {callback repeated}}
''','''iRead endpoint 9; iRead q 9; iApply
iNote [iGetReadData endpoint -dec]; iNote [iGetReadData q -bin]
iNote [iGetMiscompares endpoint -bin]; iNote [iGetMiscompares q -bin]
if {![catch {iGetStatus -clear}]} {error {abstract data became known}}
if {![catch {iGetStatus}]} {error {unknown status cleared}}
iNote unknown
''',[b'9',b'0bxxxx',b'0b0000',b'0bxxxx',b'unknown'])

    def test_generated_default_automatic_frames_keep_actual_unknown_data(self):
        icl,pdl,tail = (self.root/n for n in ('source.icl','source.pdl','tail.pdl'))
        icl.write_text(MODEL)
        pdl.write_text('iPDLLevel 1\niProcsForModule Chains\n'+PROCS+'iReset; iWrite dest 9; iApply\n')
        self.run_tool(icl,pdl,'prefix.pdl',modeled=False)
        tail.write_text('''iPDLLevel 1
iRead endpoint 9; iRead q 0bx; iApply
iNote [iGetReadData endpoint -dec]; iNote [iGetReadData q -bin]
iNote [iGetMiscompares q -bin]; iNote [iGetStatus -clear]; iNote [iGetStatus]
''')
        output=self.run_tool(icl,self.root/'prefix.pdl','observed.pdl',extra=('--pdl',str(tail)))
        self.assertEqual(self.notes(output),[b'9',b'0bxxxx',b'0b0000',b'0',b'0'])

    def test_callback_local_queries_keep_real_old_reset_and_endpoint_data(self):
        source=MODEL.replace('WriteEnSource en;}','WriteEnSource en; ResetValue 0;}')
        self.continued(source,'''iProc store {data reg} {
 iWrite en 0; iWrite p $data; iApply
 iRead endpoint 0; iRead q 0; iApply
 iNote [iGetReadData endpoint -bin]; iNote [iGetReadData q -bin]
 iNote [iGetMiscompares endpoint -bin]
 iNote [iGetStatus -clear]; iNote [iGetStatus]
}
iReset; iWrite dest 9; iApply
''','''iRead q 0; iApply
iNote [iGetReadData q -bin]; iNote [iGetMiscompares q -bin]
if {![catch {iGetStatus}]} {error {unknown downstream comparison became known}}
iNote unknown
''',[b'0b1001',b'0b0000',b'0b1001',b'2',b'0',b'0bxxxx',b'0bxxxx',b'unknown'])


    def test_sparse_inverted_outputs_and_logical_destination_are_actual(self):
        self.continued(MODEL,PROCS+'iReset; iWrite dest 13; iApply\n','''
iRead endpoint[3:2] 0; iRead endpoint[0] 0; iRead inverse 0bx; iRead dest 0bx; iApply
iNote [iGetReadData endpoint -bin]; iNote [iGetMiscompares endpoint -bin]
iNote [iGetReadData inverse -bin]; iNote [iGetReadData dest -bin]
iNote [iGetStatus -clear]; iNote [iGetStatus]
''',[b'0b11x1',b'0b11x1',b'0bxxxx',b'0bxxxx',b'3',b'0'])

    def test_reset_cancels_requests_and_preserves_actual_history(self):
        source=MODEL.replace('WriteEnSource en;}','WriteEnSource en; ResetValue 0;}')
        self.continued(source,PROCS+'''iReset; iState p 3
iRead endpoint 0; iApply
iWrite dest 9; iReset
if {$::calls != 0} {error {canceled callback invoked}}
''','''iNote [iGetReadData endpoint -dec]
iRead q 0; iApply; iNote [iGetReadData q -bin]
iNote [iGetStatus -clear]; iNote [iGetStatus]
''',[b'3',b'0b0000',b'2',b'0'])

    def test_repeated_overwritten_and_dont_care_writes_keep_real_endpoint(self):
        self.continued(MODEL,PROCS+'''iReset
iWrite dest 3; iWrite dest 13; iApply
iWrite dest 13; iApply
iWrite dest 0bx; iApply
if {$::calls != 1} {error {callback invocation count}}
''','''iRead endpoint 13; iRead q 0bx; iApply
iNote [iGetReadData endpoint -dec]; iNote [iGetReadData q -bin]
iNote [iGetStatus -clear]; iNote [iGetStatus]
''',[b'13',b'0bxxxx',b'0',b'0'])

    def test_merged_automatic_calls_remain_serial_and_preserve_last_actual_data(self):
        self.continued(MODEL,PROCS+'''iProc request {data} {iWrite dest $data; iApply}
iReset; iMerge -begin; iCall request 13; iCall request 6; iMerge -end
if {$::calls != 2} {error {merged callbacks repeated}}
''','''iRead endpoint 6; iRead q 0bx; iApply
iNote [iGetReadData endpoint -dec]; iNote [iGetReadData q -bin]
iNote [iGetStatus -clear]; iNote [iGetStatus]
''',[b'6',b'0bxxxx',b'0',b'0'])

    def test_namespaced_callback_arguments_execute_once(self):
        source=MODEL.replace('WriteCallBack Chains store','WriteCallBack Chains::api store')
        self.continued(source,'''iProcsForModule Chains -iProcNameSpace api
iProc store {data reg} {incr ::calls; iWrite en 0; iWrite p $data}
iProcsForModule Chains
set ::calls 0; set ::args 0
iReset; iWrite dest [expr {[incr ::args]+12}]; iApply
if {$::calls != 1 || $::args != 1} {error {argument evaluation repeated}}
''','''iRead endpoint 13; iRead q 0bx; iApply
iNote [iGetReadData endpoint -dec]; iNote [iGetReadData q -bin]
iNote [iGetStatus]
''',[b'13',b'0bxxxx',b'0'])


    def test_unknown_actual_selector_rejects_logical_route_without_publishing(self):
        source=MODEL.replace(' DataOutPort q[3:0] {Source dest;}', '''
 DataMux choice[3:0] SelectedBy value[0] {1'b0: dest; 1'b1: other;}
 DataOutPort q[3:0] {Source choice;}''')
        icl,pdl,out=(self.root/n for n in ('source.icl','source.pdl','protected.pdl'))
        icl.write_text(source)
        pdl.write_text('iPDLLevel 1\niProcsForModule Chains\n'+PROCS+'''
iReset; iWrite dest 6; iApply
iRead dest 0bx; iApply; iGetReadData dest -bin
''')
        out.write_bytes(b'keep output\n')
        self.assertIn('actual primary read route',self.run_tool(icl,pdl,'protected.pdl',ok=False))
        self.assertEqual(out.read_bytes(),b'keep output\n')

    def test_held_actual_storage_cannot_inherit_callback_planning_proof(self):
        icl,pdl,out=(self.root/n for n in ('source.icl','source.pdl','protected.pdl'))
        icl.write_text(MODEL)
        pdl.write_text('iPDLLevel 1\niProcsForModule Chains\n'+PROCS+'''
iReset; iTake other -persistent; iState other 9
iWrite dest 9; iApply
iRead q 0bx; iApply; iGetReadData q -bin
''')
        out.write_bytes(b'keep output\n')
        self.assertIn('reserved',self.run_tool(icl,pdl,'protected.pdl',ok=False))
        self.assertEqual(out.read_bytes(),b'keep output\n')

    def test_local_reset_keeps_procedure_bytes_and_actual_observations(self):
        icl,pdl=(self.root/n for n in ('source.icl','source.pdl'))
        icl.write_text(MODEL)
        pdl.write_text('iPDLLevel 1\niProcsForModule Chains\n'+PROCS+'''
iReset; iWrite dest 13; iApply
iRead endpoint 13; iRead q 0bx; iApply
iNote [iGetReadData endpoint -dec]; iNote [iGetReadData q -bin]
iNote [iGetStatus -clear]; iNote [iGetStatus]
''')
        reference=self.run_tool(icl,pdl,'reference.pdl')
        for model in (icl,self.root/'debug/Chains.icl'):
            reset=self.root/'reset.pdl'
            self.assertEqual(self.run_tool(model,pdl,'reset/result.pdl',extra=('--local_reset',str(reset))),reference)
            self.assertEqual(reset.read_bytes(),pdl.read_bytes())
            self.assertEqual(self.run_tool(model,reset,'reset/replayed.pdl'),reference)

    def test_clock_and_physical_boundaries_reject_without_getters(self):
        cases=((MODEL.replace(' DataInPort en;',' TCKPort tck; DataInPort en;'),()),
               (MODEL,('--physical_pdl',)))
        for source,extra in cases:
            with self.subTest(extra=extra):
                icl,pdl,out=(self.root/n for n in ('source.icl','source.pdl','protected.pdl'))
                icl.write_text(source)
                pdl.write_text('iPDLLevel 1\niProcsForModule Chains\n'+PROCS+'iReset; iWrite dest 9; iApply\n')
                out.write_bytes(b'keep output\n')
                self.assertIn('callback',self.run_tool(icl,pdl,'protected.pdl',ok=False,extra=extra))
                self.assertEqual(out.read_bytes(),b'keep output\n')

    def test_caught_cycle_reset_and_retry_keep_completed_actual_endpoint(self):
        self.continued(MODEL,'''set ::calls 0
iProc store {data reg} {incr ::calls; iWrite dest $data; iApply}
iWrite dest 9
if {![catch {iApply}]} {error {cycle accepted}}
iReset
iProc store {data reg} {incr ::calls; iWrite en 0; iWrite p $data}
iWrite dest 13; iApply
if {$::calls != 2} {error {retry invoked twice}}
''','''iRead endpoint 13; iRead q 0bx; iApply
iNote [iGetReadData endpoint -dec]; iNote [iGetReadData q -bin]
iNote [iGetStatus]
''',[b'13',b'0bxxxx',b'0'])

    def test_nested_automatic_prerequisites_keep_each_actual_storage_unknown(self):
        source=MODEL.replace(' DataInPort en;',' DataInPort go; DataInPort en;').replace(
            ' Alias inverse', '''
 DataRegister peer[3:0] {WriteCallBack Chains store <D> <R>;}
 DataRegister remote[3:0] {WriteDataSource peer; WriteEnSource go;}
 DataOutPort remote_out[3:0] {Source remote;}
 Alias inverse''')
        self.continued(source,'''set ::calls 0
iProc store {data reg} {
 incr ::calls
 if {$reg eq "value"} {iWrite en 0; iApply; iWrite remote 6; iApply}
 iWrite en 0; iWrite go 0; iWrite p $data
}
iReset; iWrite dest 9; iApply
if {$::calls != 2} {error {nested callback count}}
''','''iRead endpoint 9; iRead q 0bx; iRead remote_out 0bx; iApply
iNote [iGetReadData endpoint -dec]; iNote [iGetReadData q -bin]
iNote [iGetReadData remote_out -bin]; iNote [iGetStatus -clear]; iNote [iGetStatus]
''',[b'9',b'0bxxxx',b'0bxxxx',b'0',b'0'])

    def rejected_suffix(self,source,prefix,suffix,diagnostic):
        for observed in (False,True):
            with self.subTest(observed_prefix=observed):
                icl,pdl,out=(self.root/n for n in ('source.icl','source.pdl','protected.pdl'))
                icl.write_text(source)
                query='iRead endpoint 0bx; iApply; iNote [iGetReadData endpoint -bin]\n' if observed else ''
                pdl.write_text('iPDLLevel 1\niProcsForModule Chains\n'+PROCS+prefix+query+suffix)
                artifacts=(out,self.root/'debug/Chains.icl',self.root/'reset.pdl')
                for path in artifacts:
                    path.parent.mkdir(parents=True,exist_ok=True);path.write_bytes(b'keep output\n')
                self.assertIn(diagnostic,self.run_tool(icl,pdl,'protected.pdl',ok=False,
                    extra=('--local_reset',str(artifacts[-1]))))
                for path in artifacts:self.assertEqual(path.read_bytes(),b'keep output\n')

    def test_final_history_rejects_timed_suffix_without_a_getter(self):
        self.rejected_suffix(MODEL,'iReset; iWrite dest 9; iApply\n',
            'iRunLoop 1 -tck; iRead endpoint 9; iApply\n','iRunLoop histories')

    def test_final_history_rejects_unknown_route_without_a_getter(self):
        source=MODEL.replace(' DataOutPort q[3:0] {Source dest;}', '''
 DataMux choice[3:0] SelectedBy value[0] {1'b0: dest; 1'b1: other;}
 DataOutPort q[3:0] {Source choice;}''')
        self.rejected_suffix(source,'iReset; iWrite dest 6; iApply\n',
            'iRead dest 0bx; iApply\n','actual primary read route')

    def test_access_together_preserves_existing_and_automatic_histories(self):
        for automatic in (False,True):
            for inverted in (False,True):
                with self.subTest(automatic=automatic,inverted=inverted):
                    source=MODEL.replace(' Alias inverse',
                        ' Alias pair[7:0] = '+('~' if inverted else '')+'endpoint, spare_out {AccessTogether;}\n Alias inverse')
                    prefix='iReset; iWrite spare 5; '+('iWrite dest 9' if automatic else 'iWrite p 9')+'; iApply\n'
                    prefix+='if {$::calls != %d} {error {callback count}}\n'%automatic
                    word='01100101' if inverted else '10010101'
                    self.continued(source,PROCS+prefix,'''iRead pair 0b%s; iRead q 0bx; iApply
iNote [iGetReadData pair -bin]; iNote [iGetMiscompares pair -bin]
iNote [iGetReadData q -bin]; iNote [iGetStatus -clear]; iNote [iGetStatus]
'''%word,[('0b'+word).encode(),b'0b00000000',b'0bxxxx',b'0',b'0'])

    def test_grouped_logical_route_uses_actual_callback_selector(self):
        source=MODEL.replace(' DataOutPort q[3:0] {Source dest;}', '''
 DataMux choice[3:0] SelectedBy value[0] {1'b0: dest; 1'b1: other;}
 DataOutPort q[3:0] {Source choice;}
 Alias grouped[7:0] = dest, spare_out {AccessTogether;}''')
        icl,pdl,out=(self.root/n for n in ('source.icl','source.pdl','protected.pdl'))
        icl.write_text(source)
        pdl.write_text('iPDLLevel 1\niProcsForModule Chains\n'+PROCS+'''
iReset; iWrite spare 5; iWrite dest 6; iApply
iRead grouped 0bx; iApply; iGetReadData grouped -bin
''')
        out.write_bytes(b'keep output\n')
        self.assertIn('actual primary read route',self.run_tool(icl,pdl,'protected.pdl',ok=False))
        self.assertEqual(out.read_bytes(),b'keep output\n')

    def test_independent_known_care_counts_clear_once_with_unknown_dont_care_data(self):
        self.continued(MODEL,PROCS+'iReset; iWrite dest 13; iApply\n','''
iRead endpoint 0; iRead q 0bx; iApply
iNote [iGetReadData endpoint -bin]; iNote [iGetMiscompares endpoint -bin]
iNote [iGetReadData q -bin]; iNote [iGetMiscompares q -bin]
iNote [iGetStatus]; iNote [iGetStatus -clear]; iNote [iGetStatus]
''',[b'0b1101',b'0b1101',b'0bxxxx',b'0b0000',b'3',b'3',b'0'])


if __name__=='__main__':unittest.main(argv=[__file__])
