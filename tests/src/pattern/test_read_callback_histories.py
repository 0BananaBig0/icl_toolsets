#!/usr/bin/env python3
"""Read callback arguments stay separate from actual native endpoint histories."""
import unittest

import test_callback_execution as callbacks
import test_callback_endpoint_readback as endpoints
import test_modeled_readback as history
import test_simultaneous_scan_chains as cli

MODEL = callbacks.MODEL
PROCS = endpoints.PROCS


class ReadCallbackHistories(unittest.TestCase):
    setUp = history.ModeledReadback.setUp
    run_tool = history.ModeledReadback.run_tool
    replay = history.ModeledReadback.replay
    notes = endpoints.CallbackEndpointReadback.notes

    def check(self, body, model=MODEL, procs=PROCS):
        return self.replay(model, procs+body)

    def test_actual_endpoint_and_status(self):
        output = self.check('''iState p 9; iRead observed 0; iApply
iNote [iGetReadData q -dec]; iNote [iGetMiscompares q -bin]
iNote [iGetStatus -clear]; iNote [iGetStatus]
if {![catch {iGetReadData observed}]} {error {logical callback data invented}}
iNote [iGetReadData q -dec]
''')
        self.assertEqual(self.notes(output), [b'9', b'0b1001', b'2', b'0', b'9'])

    def test_body_getter(self):
        procs = PROCS.replace(' iRead q $data', ''' iRead q $data; iApply
 iNote [iGetReadData q -dec]; iNote [iGetStatus -clear]''')
        output = self.check('''iState p 9; iRead observed 0; iApply
iNote [iGetReadData q -dec]; iNote [iGetStatus]
''', procs=procs)
        self.assertEqual(self.notes(output), [b'9', b'2', b'9', b'0'])

    def test_no_query(self):
        output = self.check('iState p 9; iRead observed 0; iApply\n')
        self.assertIn(b'iRead q 0b0000\n', output)
        self.assertEqual(self.notes(output), [])

    def test_unknown_endpoint(self):
        output = self.check('''iRead observed 0; iApply
iNote [iGetReadData q -bin]
if {![catch {iGetStatus}]} {error {unknown endpoint produced known status}}
''')
        self.assertEqual(self.notes(output), [b'0bxxxx'])

    def test_sparse_inverted_request(self):
        source = MODEL.replace(' Alias low', ' Alias inverse[3:0] = ~q;\n Alias low')
        output = self.check('''iState p 9; iRead observed[3] 0; iApply
iNote [iGetReadData q -dec]; iNote [iGetReadData inverse -dec]
iNote [iGetMiscompares q -bin]; iNote [iGetStatus]
''', model=source)
        self.assertEqual(self.notes(output), [b'9', b'6', b'0b1000', b'1'])

    def test_nested_write_callback(self):
        procs = PROCS.replace(' iRead q $data', ''' iWrite value 6
 iRead q $data; iApply; iNote [iGetReadData q -dec]''')
        output = self.check('iState p 9; iRead observed 0; iApply; iNote [iGetStatus]\n', procs=procs)
        self.assertEqual(self.notes(output), [b'6', b'2'])

    def test_namespaced_callback(self):
        source = '''Module Leaf {
 DataInPort p[3:0]; DataOutPort q[3:0] {Source p;}
 DataRegister observed[3:0] {
  ReadCallBack Chains::ops load <D> <R>; ReadDataSource p;
 }
}
Module Chains {
 DataInPort p[3:0]; Instance leaf Of Leaf {InputPort p = p;}
 DataOutPort out[3:0] {Source leaf.q;}
}'''
        procs = '''iPDLLevel 1
iProcsForModule Chains -iProcNameSpace ops
iProc load {data reg} {
 if {$reg ne "leaf.observed"} {error callback_register}
 iRead leaf.q $data; iApply; iNote [iGetReadData leaf.q -dec]
}
iProcsForModule Chains
'''
        output = self.check('iState p 6; iRead leaf.observed 0; iApply; iNote [iGetStatus]\n', source, procs)
        self.assertEqual(self.notes(output), [b'6', b'2'])

    def test_selectable_and_serial_endpoints(self):
        selectable = '''Module Chains {
 DataInPort p[3:0]; DataInPort enable;
 DataRegister wire[3:0] {WriteDataSource p; WriteEnSource enable; ResetValue 0;}
 DataOutPort q[3:0] {Source wire;}
 DataRegister observed[3:0] {ReadCallBack Chains load <D>; ReadDataSource wire;}
}'''
        procs = 'iPDLLevel 1\niProcsForModule Chains\niProc load {data} {iRead wire $data}\n'
        output = self.check('''iReset; iWrite wire 9; iApply
iRead observed 0; iApply; iNote [iGetReadData wire -dec]; iNote [iGetStatus]
''', selectable, procs)
        self.assertEqual(self.notes(output), [b'9', b'2'])
        serial = cli.NETWORK.replace('  SelectPort sel;', '''
 DataRegister observed[3:0] {ReadCallBack Chains load <D>; ReadDataSource a;}
  SelectPort sel;''')
        output = self.check('''iReset; iWrite a 9; iApply
iRead observed 0; iApply; iNote [iGetReadData a -dec]; iNote [iGetStatus]
''', serial, procs.replace('iRead wire', 'iRead a'))
        self.assertEqual(self.notes(output), [b'9', b'2'])

    def test_reset_and_retry(self):
        procs = PROCS.replace(' iRead q $data', ''' iReset; iState p 6
 iRead q $data; iApply; iNote [iGetReadData q -dec]''')
        output = self.check('iState p 9; iRead observed 0; iApply; iNote [iGetStatus]\n', procs=procs)
        self.assertEqual(self.notes(output), [b'6', b'2'])
        procs = PROCS.replace(' iRead q $data', ''' iRead q $data; iApply
 iNote [iGetReadData q -dec]
 if {$data == 9} {error stopped}''')
        output = self.check('''iState p 9
if {![catch {iRead observed 9; iApply}]} {error expected_failure}
iReset; iState p 6; iRead observed 6; iApply; iNote [iGetStatus]
''', procs=procs)
        self.assertEqual(self.notes(output), [b'9', b'6', b'0'])

    def test_split_and_local_reset(self):
        model, first, second = (self.root/name for name in ('source.icl', 'first.pdl', 'second.pdl'))
        model.write_text(MODEL)
        first.write_text(PROCS+'iState p 9; iRead observed 0\n')
        second.write_text('iApply; iNote [iGetReadData q -dec]; iNote [iGetStatus]\n')
        output = self.run_tool(model, first, extra=('--pdl', str(second)))
        self.assertEqual(self.notes(output), [b'9', b'2'])
        self.assertEqual(output, self.run_tool(self.root/'debug/Chains.icl', first,
            'flat/result.pdl', extra=('--pdl', str(second))))
        self.assertEqual(output, self.run_tool(model, self.root/'result.pdl', 'generated/result.pdl'))
        source = callbacks.RESET_MODEL.replace(' DataRegister value[3:0]', '''
 DataRegister observed[3:0] {ReadCallBack Chains load <D> <R>; ReadDataSource wire;}
 DataRegister value[3:0]''')
        load = '''iProc load {data reg} {
 iRead child.wire $data; iApply; iNote [iGetReadData child.wire -dec]
}
'''
        model.write_text(source)
        first.write_text(callbacks.RESET_PROCS+load+'''iReset; iCall request 9; iApply
iRead child.observed 0; iApply; iNote [iGetStatus]
''')
        reset = self.root/'reset.pdl'
        output = self.run_tool(model, first, extra=('--local_reset', str(reset)))
        self.assertEqual(self.notes(output), [b'9', b'2'])
        self.assertIn(load.encode(), reset.read_bytes())
        self.assertEqual(output, self.run_tool(model, reset, 'rewritten/result.pdl'))
        self.assertEqual(output, self.run_tool(self.root/'debug/Chains.icl', reset, 'flat-reset/result.pdl'))
        self.assertEqual(output, self.run_tool(model, self.root/'result.pdl', 'generated-reset/result.pdl'))

    def test_merged_calls(self):
        procs = PROCS+'''iProc sample {word} {
 iWrite p $word; iRead observed 0; iApply
 iNote [iGetReadData q -dec]
}
'''
        output = self.check('''iMerge -begin; iCall sample 9; iCall sample 6; iMerge -end
iNote [iGetStatus -clear]; iNote [iGetStatus]
''', procs=procs)
        self.assertEqual(self.notes(output), [b'9', b'6', b'4', b'0'])

    def test_unsupported_boundaries(self):
        automatic = MODEL.replace(' Alias low', '''
 DataRegister dest[3:0] {WriteDataSource value; WriteEnSource 1'b1;}
 Alias low''')
        timed = PROCS.replace(' iRead q $data', ' iRunLoop 1 -tck; iRead q $data')
        cases = [(automatic, PROCS+'iWrite dest 9; iApply\n', (), 'callback'),
                 (MODEL, timed+'iRead observed 0; iApply; iGetReadData q\n', (), 'histories'),
                 (MODEL, PROCS+'iRead observed 0; iApply\n', ('--physical_pdl',), 'physical')]
        for source, body, extra, expected in cases:
            with self.subTest(boundary=expected):
                model, pdl = self.root/'source.icl', self.root/'source.pdl'
                model.write_text(source); pdl.write_text(body)
                artifacts = [self.root/'result.pdl', self.root/'debug/Chains.icl', self.root/'reset.pdl']
                for path in artifacts:
                    path.parent.mkdir(parents=True, exist_ok=True); path.write_bytes(b'retained\n')
                diagnostic = self.run_tool(model, pdl, ok=False,
                    extra=(*extra, '--local_reset', str(self.root/'reset.pdl')))
                self.assertIn(expected, diagnostic)
                for path in artifacts:
                    self.assertEqual(path.read_bytes(), b'retained\n')

    def test_apply_in_another_frame_consumes_all_callback_requests(self):
        for getter in (False, True):
            for queue in ('iRead observed 0',
                          'iRead observed[1:0] 0; iCall high',
                          'iCall high; iRead observed[1:0] 0'):
                with self.subTest(getter=getter, queue=queue):
                    procs = PROCS+'''iProc high {} {iRead observed[3:2] 0}
iProc runapply {} {iApply}
'''
                    if getter:
                        procs = procs.replace(' iRead q $data', ' iRead q $data; iApply; iGetReadData q')
                    output = self.check('iState p 9; '+queue+'''; iCall runapply
iWrite p 6; iRead q 0; iApply; iNote [iGetReadData q -dec]
''', procs=procs)
                    self.assertEqual(self.notes(output), [b'6'])

    def test_generated_prefix_retains_partial_logical_history_invalidation(self):
        model, prefix, continuation = (self.root/name for name in ('prefix.icl', 'prefix.pdl', 'continuation.pdl'))
        model.write_text(MODEL)
        for seed in ('', 'iState observed 5 -LastReadValue; iState observed 10 -LastMiscompareValue;'):
            with self.subTest(seed=bool(seed)):
                prefix.write_text(PROCS+'iState p 9; '+seed+'''
iRead observed[1:0] 0; iApply; iNote [iGetReadData q -dec]
''')
                self.run_tool(model, prefix, 'prefix-result.pdl')
                continuation.write_text('''iPDLLevel 1
if {[catch {iGetReadData observed[1:0] -dec}]} {iWrite p 0} else {iWrite p 5}
iApply
if {[catch {iGetMiscompares observed[1:0] -dec}]} {iNote unavailable} else {iNote invented}
iNote [iGetReadData observed[3:2] -bin]
iNote [iGetMiscompares observed[3:2] -bin]
''')
                source = self.run_tool(model, prefix, 'source-continuation.pdl', extra=('--pdl', str(continuation)))
                generated = self.run_tool(model, self.root/'prefix-result.pdl', 'generated-continuation.pdl', extra=('--pdl', str(continuation)))
                self.assertEqual(source, generated)
                self.assertIn(b'iWrite p 0b0000\n', generated)
                self.assertEqual(self.notes(generated)[-3:], [b'unavailable', b'0b01' if seed else b'0bxx', b'0b10' if seed else b'0bxx'])

    def test_malformed_history_notes_preserve_seeded_histories(self):
        for payload in ('1111', '111 observed', '1x11 observed', '1111 p', '1111 missing'):
            for tag in ('', '-comment ', '-status '):
                with self.subTest(payload=payload, tag=tag):
                    output = self.check('''iState observed 5 -LastReadValue
iState observed 10 -LastMiscompareValue
iState p 9; iRead q 0; iApply
'''+ 'iNote '+tag+'{modeled callback read history unavailable '+payload+'}\n'+'''
iNote [iGetReadData observed -dec]; iNote [iGetMiscompares observed -dec]
iNote [iGetReadData q -dec]; iNote [iGetStatus]
''')
                    self.assertEqual(self.notes(output), [b'5', b'10', b'9', b'2'])

    def test_empty_and_extra_argument_notes_remain_informational(self):
        model, pdl = self.root/'source.icl', self.root/'source.pdl'
        model.write_text(MODEL)
        for command in ('iNote', 'iNote -comment', 'iNote -status',
                        'iNote {modeled callback read history unavailable 1111 observed} extra',
                        'iNote -unknown {modeled callback read history unavailable 1111 observed}',
                        'iNote -comment {modeled callback read history unavailable 1111 observed} extra'):
            with self.subTest(command=command):
                body = '''iState observed 5 -LastReadValue
iState observed 10 -LastMiscompareValue
'''+command+'\n'
                output = self.check(body+'''iState p 9; iRead q 0; iApply
iNote [iGetReadData observed -dec]; iNote [iGetMiscompares observed -dec]
iNote [iGetReadData q -dec]; iNote [iGetStatus]
''')
                self.assertEqual(self.notes(output)[-4:], [b'5', b'10', b'9', b'2'])
                pdl.write_text(PROCS+body+'iWrite p 9; iApply\n')
                self.assertIn(b'iWrite p 0b1001\n', self.run_tool(model, pdl, modeled=False))

    def test_standard_and_legacy_prefix_notes_restore_sparse_unavailable_bits(self):
        model, prefix, suffix = (self.root/name for name in ('source.icl', 'prefix.pdl', 'suffix.pdl'))
        model.write_text(MODEL)
        prefix.write_text(PROCS+'''iState observed 5 -LastReadValue
iState observed 10 -LastMiscompareValue
iState p 9; iRead observed[1:0] 0; iApply
''')
        generated = self.run_tool(model, prefix, 'prefix-out.pdl')
        marker = b'modeled callback read history unavailable 0011 observed'
        annotations = [line for line in generated.splitlines() if marker in line]
        self.assertEqual(len(annotations), 1)
        suffix.write_text('''iPDLLevel 1
if {![catch {iGetReadData observed[1:0]}]} {error invented_read}
if {![catch {iGetMiscompares observed[1:0]}]} {error invented_miscompare}
iNote [iGetReadData observed[3:2] -bin]
iNote [iGetMiscompares observed[3:2] -bin]
iNote [iGetReadData q -dec]; iNote [iGetStatus]
''')
        reference = self.run_tool(model, prefix, 'reference.pdl', extra=('--pdl', str(suffix)))
        for tag in ('-comment', '-status', 'legacy'):
            with self.subTest(tag=tag):
                annotation = (b'iNote {'+marker+b'}' if tag=='legacy' else
                              b'iNote '+tag.encode()+b' "'+marker+b'"')
                replay = self.root/'replay.pdl'
                replay.write_bytes(generated.replace(annotations[0], annotation+b'\n'+annotation))
                for index, source in enumerate((model, self.root/'debug/Chains.icl')):
                    output = self.run_tool(source, replay, tag+'/'+str(index)+'/result.pdl',
                                           extra=('--pdl', str(suffix)))
                    self.assertEqual(self.notes(output), [b'0b01', b'0b10', b'9', b'1'])
                    self.assertEqual([line for line in output.splitlines() if line.startswith(b'iRead ')],
                                     [line for line in reference.splitlines() if line.startswith(b'iRead ')])
        self.assertEqual(annotations, [b'iNote -comment "'+marker+b'"'])


if __name__ == '__main__':
    unittest.main()
