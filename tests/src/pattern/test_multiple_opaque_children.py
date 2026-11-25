#!/usr/bin/env python3
"""Independent opaque child queues share one exact native physical route."""
import unittest
import test_opaque_child_raw as base
from benchmark_multiple_opaque_children import model

MODEL = model()


class MultipleOpaqueChildren(unittest.TestCase):
    setUp = base.OpaqueChildRaw.setUp
    files = base.OpaqueChildRaw.files
    replay = base.OpaqueChildRaw.replay
    rejected = base.OpaqueChildRaw.rejected
    run_tool = base.OpaqueChildRaw.run_tool
    notes = base.OpaqueChildRaw.notes

    def test_disjoint_children_actual_histories_and_parent_state(self):
        output = self.replay("""iReset; iWrite head 5; iWrite tail 19; iApply
iScan child0.local 8 -si 0xa5 -so 0 -stable
iScan child1.local 8 -si 0x3c -so 0 -stable; iApply
iNote [iGetReadData child0.local -bin]; iNote [iGetReadData child1.local -bin]
iRead head 5; iRead tail 19; iApply
iNote [iGetReadData head -dec]; iNote [iGetReadData tail -dec]
""", MODEL)
        self.assertEqual(self.notes(output), ['0bxxxxxxxx', '0bxxxxxxxx', '5', '19'])
        self.assertIn('iScan client 24 -si 0b101101001010011110010011', output)
        self.assertNotIn('__opaque_scan_state', output)
        self.assertNotIn('__opaque_scan_state', (self.root/'debug/Top.icl').read_text())

    def test_generated_prefix_retains_explicit_opaque_histories(self):
        for count in (1, 2):
            with self.subTest(children=count):
                prefix = '''iReset; iWrite head 5; iWrite tail 19; iApply
iScan child0.local 8 -si 0xa5 -so 0b10xx0011 -stable
'''
                if count==2:
                    prefix += 'iScan child1.local 8 -si 0x3c -stable\n'
                prefix += 'iApply\n'
                suffix = '''iNote [iGetReadData child0.local -bin]
iNote [iGetMiscompares child0.local -bin]
if {![catch {iGetStatus}]} {error {opaque comparison became known}}
'''
                expected = ['0bxxxxxxxx', '0bxx00xxxx']
                if count==2:
                    suffix += '''iNote [iGetReadData child1.local -bin]
iNote [iGetMiscompares child1.local -bin]
'''
                    expected += ['0bxxxxxxxx', '0b00000000']
                icl, pdl = self.files(prefix, model([8]*count))
                self.run_tool(icl, pdl, 'prefix.pdl')
                source_icl, source = self.files(prefix+suffix, model([8]*count))
                reference = self.run_tool(source_icl, source, 'source-tail.pdl')
                tail = self.root/'tail.pdl'; tail.write_text('iPDLLevel 1\n'+suffix)
                replay = self.run_tool(icl, self.root/'prefix.pdl', 'prefix-tail.pdl',
                                       extra=('--pdl', str(tail)))
                self.assertEqual(reference, replay)
                self.assertEqual(self.notes(replay), expected)

    def test_child_only_prefix_does_not_invent_root_history(self):
        for count in (1, 2):
            with self.subTest(children=count):
                prefix = 'iReset; iWrite head 5; iWrite tail 19; iApply\n'
                for child in range(count):
                    prefix += 'iScan child%d.local 8 -si 165 -stable\n' % child
                prefix += 'iApply\n'
                suffix = '''foreach getter {iGetReadData iGetMiscompares} {
 if {![catch {$getter client}]} {error {physical root history invented}}
}
iNote absent; iNote [iGetStatus]
'''
                icl, first = self.files(prefix, model([8]*count))
                self.run_tool(icl, first, 'prefix.pdl')
                _, full = self.files(prefix+suffix, model([8]*count))
                reference = self.run_tool(icl, full, 'source-tail.pdl')
                tail = self.root/'tail.pdl'; tail.write_text('iPDLLevel 1\n'+suffix)
                replay = self.run_tool(icl, self.root/'prefix.pdl', 'prefix-tail.pdl',
                                       extra=('--pdl', str(tail)))
                self.assertEqual(reference, replay)
                self.assertEqual(self.notes(replay), ['absent','0'])

    def test_child_prefix_preserves_previous_root_masks_and_count(self):
        prefix = '''iReset; iWrite head 5; iWrite tail 19; iApply
iScan client 24 -si 0b101000000000000000010011 -so 0b000xxxxxxxxxxxxxxxx00000 -stable; iApply
iScan child0.local 8 -si 165 -stable; iApply
iScan child1.local 8 -si 60 -stable; iApply; iReset
'''
        suffix = '''iNote [iGetReadData client -bin]
iNote [iGetMiscompares client -bin]
iNote [iGetReadData child0.local -bin]; iNote [iGetReadData child1.local -bin]
iNote [iGetStatus -clear]; iNote [iGetStatus]
'''
        icl, first = self.files(prefix, MODEL)
        self.run_tool(icl, first, 'prefix.pdl')
        _, full = self.files(prefix+suffix, MODEL)
        reference = self.run_tool(icl, full, 'source-tail.pdl')
        tail = self.root/'tail.pdl'; tail.write_text('iPDLLevel 1\n'+suffix)
        replay = self.run_tool(icl, self.root/'prefix.pdl', 'prefix-tail.pdl',
                               extra=('--pdl', str(tail)))
        self.assertEqual(reference, replay)
        self.assertEqual(self.notes(replay), ['0b101'+'x'*16+'10011',
                         '0b101'+'0'*16+'10011','0bxxxxxxxx','0bxxxxxxxx','5','0'])

    def test_scope_comments_are_exact_typed_and_idempotent(self):
        prefix = '''iReset; iWrite head 5; iWrite tail 19; iApply
iScan child0.local 8 -si 165 -stable; iApply
'''
        suffix = '''if {![catch {iGetReadData client}]} {error {root history invented}}
iNote [iGetReadData child0.local -bin]; iNote [iGetStatus]
'''
        icl, first = self.files(prefix, MODEL)
        generated = self.run_tool(icl, first, 'prefix.pdl')
        marker = next(line for line in generated.splitlines()
                      if line.startswith('iNote -comment "modeled raw history omitted '))
        self.assertIn(marker+'\niScan client ', generated)
        _, full = self.files(prefix+suffix, MODEL)
        reference = self.run_tool(icl, full, 'reference.pdl')
        tail = self.root/'tail.pdl'; tail.write_text('iPDLLevel 1\n'+suffix)
        for tag in ('-comment','-status'):
            with self.subTest(tag=tag):
                note = marker.replace('-comment',tag,1)
                annotated = self.root/'annotated.pdl'
                annotated.write_text(generated.replace(marker,note+'\n'+note))
                replay = self.run_tool(icl, annotated, 'replayed.pdl', extra=('--pdl',str(tail)))
                self.assertEqual(reference,replay)
                self.assertEqual(replay.count('modeled raw history omitted '),1)
                self.assertEqual(self.notes(replay),['0bxxxxxxxx','0'])

    def test_bad_or_stale_scope_notes_cannot_hide_an_explicit_root_scan(self):
        scan = 'iScan client 24 -si 0 -so 0b000xxxxxxxxxxxxxxxx00000 -stable'
        marker = 'iNote -comment {modeled raw history omitted '+scan+'}'
        cases = [marker.replace('client 24','missing 24'),
                 marker.replace('-si 0','-si 1'),marker.replace(' -stable}',' -stable extra}'),
                 marker.replace('-comment','-bad'),marker+'; iReset',
                 marker+'; iNote intervening',marker+'; iWrite head 5; iApply']
        for note in cases:
            with self.subTest(note=note):
                result = self.replay('iReset; iWrite head 5; iWrite tail 19; iApply\n'+
                    note+'\n'+scan+'''; iApply
iNote [iGetReadData client -bin]; iNote [iGetMiscompares client -bin]
''',MODEL)
                self.assertEqual(self.notes(result)[-2:],
                    ['0b101'+'x'*16+'10011','0b101'+'0'*16+'10011']
                    if '; iReset' not in note else ['0b000'+'x'*16+'00000','0b'+'0'*24])

    def test_omitted_root_history_stays_absent_without_a_provider(self):
        prefix = '''iReset; iWrite head 5; iWrite tail 19; iApply
iScan child0.local 8 -si 165 -stable; iApply
'''
        icl, first = self.files(prefix,MODEL)
        self.run_tool(icl,first,'prefix.pdl',modeled=False)
        tail = self.root/'tail.pdl'
        tail.write_text('''iPDLLevel 1
if {![catch {iGetReadData client} message] || [string first {preceding applied iScan} $message] < 0} {
 error {physical root acquired unobserved history}
}
iNote absent
''')
        result = self.run_tool(icl,self.root/'prefix.pdl','unobserved.pdl',
                               modeled=False,extra=('--pdl',str(tail)))
        self.assertEqual(self.notes(result),['absent'])

    def test_prefix_history_persists_without_inventing_sibling_scope(self):
        prefix = '''iReset; iWrite head 5; iWrite tail 19; iApply
iScan child0.local 8 -si 0xa5 -stable; iApply
iScan client 24 -si 0 -stable; iApply; iReset
'''
        suffix = '''iNote [iGetReadData child0.local -bin]
iNote [iGetMiscompares child0.local -bin]
if {![catch {iGetReadData child1.local}]} {error {unrequested sibling history invented}}
iNote unread; iNote [iGetStatus -clear]; iNote [iGetStatus]
'''
        icl, first = self.files(prefix, MODEL)
        self.run_tool(icl, first, 'prefix.pdl')
        _, full = self.files(prefix+suffix, MODEL)
        reference = self.run_tool(icl, full, 'source-tail.pdl')
        tail = self.root/'tail.pdl'; tail.write_text('iPDLLevel 1\n'+suffix)
        replay = self.run_tool(icl, self.root/'prefix.pdl', 'prefix-tail.pdl',
                               extra=('--pdl', str(tail)))
        self.assertEqual(reference, replay)
        self.assertEqual(self.notes(replay), ['0bxxxxxxxx','0b00000000','unread','0','0'])

    def test_typed_duplicate_history_notes_add_no_hardware_or_status(self):
        prefix = '''iReset; iWrite head 5; iWrite tail 19; iApply
iScan child0.local 8 -si 0xa5 -stable; iApply
'''
        icl, first = self.files(prefix, MODEL)
        generated = self.run_tool(icl, first, 'prefix.pdl')
        marker = 'modeled opaque child raw history client child0.local 0bxxxxxxxx'
        annotation = 'iNote -comment "'+marker+'"'
        self.assertIn(annotation+'\n', generated)
        suffix = '''iNote [iGetReadData child0.local -bin]
iNote [iGetMiscompares child0.local -bin]
iNote [iGetStatus -clear]; iNote [iGetStatus]
'''
        for tag in ('-comment', '-status'):
            with self.subTest(tag=tag):
                note = 'iNote '+tag+' "'+marker+'"'
                text = generated.replace(annotation, note+'\n'+note)
                replay = self.root/'typed-prefix.pdl'; replay.write_text(text)
                tail = self.root/'tail.pdl'; tail.write_text('iPDLLevel 1\n'+suffix)
                output = self.run_tool(icl, replay, 'typed-tail.pdl', extra=('--pdl', str(tail)))
                self.assertEqual(self.notes(output), ['0bxxxxxxxx','0b00000000','0','0'])
                self.assertEqual(sum(line.startswith('iApply') for line in generated.splitlines()),
                                 sum(line.startswith('iApply') for line in output.splitlines()))

    def test_unmatched_history_notes_leave_existing_history_intact(self):
        marker = 'modeled opaque child raw history '
        for payload in ('client child0.local 0b00000000',
                        'client child0.local 0bxxx', 'client missing 0bxxxxxxxx',
                        'missing child0.local 0bxxxxxxxx', 'client child0.local'):
            with self.subTest(payload=payload):
                output = self.replay('''iReset; iWrite head 5; iWrite tail 19; iApply
iScan child0.local 8 -si 1 -stable; iApply
'''+ 'iNote -comment {'+marker+payload+'}\n'+'''
iNote [iGetReadData child0.local -bin]
iNote [iGetMiscompares child0.local -bin]; iNote [iGetStatus]
''', MODEL)
                self.assertEqual(self.notes(output), ['0bxxxxxxxx','0b00000000','0'])

    def test_history_annotation_requires_preceding_raw_scan(self):
        ordinary = 'iReset; iWrite head 5; iWrite tail 19; iApply\n'
        for prefix in (ordinary, ordinary+'iScan client 24 -si 0 -stable; iApply\n'+ordinary):
            with self.subTest(reset_after_raw=prefix!=ordinary):
                output = self.replay(prefix+'''
iNote -comment {modeled opaque child raw history client child0.local 0bxxxxxxxx}
if {![catch {iGetReadData child0.local}]} {error {unanchored history invented}}
iRead head 5; iRead tail 19; iApply
iNote [iGetReadData head -dec]; iNote [iGetReadData tail -dec]; iNote [iGetStatus]
''', MODEL)
                self.assertEqual(self.notes(output), ['5','19','0'])

    def test_history_note_identifies_an_interval_of_the_completed_raw_csu(self):
        for scan in ('iScan child0.local 8 -si 165 -stable',
                     'iScan client 32 -si 0 -stable'):
            with self.subTest(scan=scan):
                prefix = 'iReset; iWrite head 5; iWrite tail 19; iApply\n'+scan+'; iApply\n'
                suffix = '''iNote -comment {modeled opaque child raw history client child1.local 0bxxxxxxxxxxxxxxxx}
iNote [iGetReadData child1.local -bin]; iNote [iGetMiscompares child1.local -bin]
iNote [iGetStatus]
'''
                icl, first = self.files(prefix, model([8,16]))
                self.run_tool(icl, first, 'prefix.pdl')
                _, full = self.files(prefix+suffix, model([8,16]))
                reference = self.run_tool(icl, full, 'source-tail.pdl')
                tail = self.root/'tail.pdl'; tail.write_text('iPDLLevel 1\n'+suffix)
                replay = self.run_tool(icl, self.root/'prefix.pdl', 'prefix-tail.pdl',
                                       extra=('--pdl', str(tail)))
                self.assertEqual(reference, replay)
                self.assertEqual(self.notes(replay), ['0b'+'x'*16,'0b'+'0'*16,'0'])

    def test_generated_history_identity_requires_an_observation_provider(self):
        prefix = '''iReset; iWrite head 5; iWrite tail 19; iApply
iScan child0.local 8 -si 165 -stable; iApply
'''
        icl, first = self.files(prefix, MODEL)
        self.run_tool(icl, first, 'prefix.pdl', modeled=False)
        tail = self.root/'tail.pdl'
        tail.write_text('''iPDLLevel 1
if {![catch {iGetReadData child0.local} message] || [string first hardware $message] < 0} {
 error {history identity invented actual data without a provider}
}
iNote unavailable
''')
        output = self.run_tool(icl, self.root/'prefix.pdl', 'unobserved-tail.pdl',
                               modeled=False, extra=('--pdl', str(tail)))
        self.assertEqual(self.notes(output), ['unavailable'])
        tail.write_text('iPDLLevel 1\niNote [iGetReadData child0.local -bin]\n')
        output = self.run_tool(icl, self.root/'prefix.pdl', 'observed-tail.pdl',
                               extra=('--pdl', str(tail)))
        self.assertEqual(self.notes(output), ['0bxxxxxxxx'])

    def test_namespaced_split_and_local_reset_prefix_keep_actual_histories(self):
        definition = 'iProc shift {value} {iScan local 8 -si $value -stable; iApply}'
        icl, first = self.files('''iProcsForModule ChildLeaf -iProcNameSpace ops
'''+definition+'''
iProcsForModule Top
iReset; iWrite head 5; iWrite tail 19; iApply
''', MODEL)
        second = self.root/'second.pdl'
        second.write_text('''iPrefix child0; iCall ops::shift 165; iPrefix -
iPrefix child1; iCall ops::shift 60; iPrefix -
''')
        self.run_tool(icl, first, 'prefix.pdl', reset=True, extra=('--pdl', str(second)))
        reset = self.root/'reset.pdl'
        self.assertIn(definition, reset.read_text())
        tail = self.root/'tail.pdl'
        tail.write_text('''iPDLLevel 1
iPrefix child0; iNote [iGetReadData local -bin]; iPrefix -
iPrefix child1; iNote [iGetReadData local -bin]; iPrefix -
iNote [iGetStatus]
''')
        reference = self.run_tool(icl, reset, 'reset-tail.pdl', extra=('--pdl', str(tail)))
        for source in (icl, self.root/'debug/Top.icl'):
            replay = self.run_tool(source, self.root/'prefix.pdl', 'namespaced/result.pdl',
                                   extra=('--pdl', str(tail)))
            self.assertEqual(reference, replay)
            self.assertEqual(self.notes(replay), ['0bxxxxxxxx','0bxxxxxxxx','0'])

    def test_stable_children_complete_independently(self):
        output = self.replay("""iReset; iWrite head 5; iWrite tail 19; iApply
iScan child0.local 8 -si 0xa5 -stable; iApply
iScan child1.local 8 -si 0x3c -stable; iApply
iScan child0.local 8 -si 0xf0 -stable; iApply
""", model(defaults=(0x12, 0x34)))
        scans = [line for line in output.splitlines() if line.startswith('iScan ')]
        self.assertEqual([line.split()[4] for line in scans],
                         ['0b101101001010011010010011',
                          '0b101101001010011110010011',
                          '0b101111100000011110010011'])

    def test_root_actual_history_does_not_create_child_histories(self):
        output = self.replay("""iReset; iWrite head 5; iWrite tail 19; iApply
iScan client 24 -si 0b101101001010011110010011 -stable; iApply
iNote [iGetReadData client -bin]
foreach target {child0.local child1.local} {
 if {![catch {iGetReadData $target}]} {error {invented child history}}
}
iScan child0.local 8 -si 1 -stable; iApply
iNote [iGetReadData child0.local -bin]; iNote [iGetReadData client -bin]
""", MODEL)
        self.assertEqual(self.notes(output), ['0b101'+'x'*16+'10011',
                                             '0bxxxxxxxx', '0b101'+'x'*16+'10011'])

    def test_reset_and_canceled_width_preserve_independent_defaults(self):
        output = self.replay("""iScan child0.local 9; iScan child1.local 7; iReset
iWrite head 5; iWrite tail 19; iApply
iScan child0.local 8 -si 0xa5 -stable; iApply
iReset; iWrite head 5; iWrite tail 19; iApply
iScan child1.local 8 -si 0x3c -stable; iApply
""", model(defaults=(0x12, 0x34)))
        scans = [line.split()[4] for line in output.splitlines() if line.startswith('iScan ')]
        self.assertEqual(scans, ['0b101101001010011010010011',
                                 '0b101000100100011110010011'])

    def test_generated_prefix_retains_each_child_length_knowledge(self):
        prefix = """iReset; iWrite head 5; iWrite tail 19; iApply
iScan child1.local 8 -si 0x3c -stable; iApply
iScan child0.local 8 -si 0xa5; iApply
"""
        tail = """iPDLLevel 1
iScan child0.local 8 -si 0xf0 -stable; iApply
iRead head 5; iRead tail 19; iApply
iNote [iGetReadData head -dec]; iNote [iGetReadData tail -dec]
"""
        for modeled in (True, False):
            with self.subTest(modeled=modeled):
                suffix = tail if modeled else tail.split('iNote [iGetReadData', 1)[0]
                icl, pdl = self.files(prefix, MODEL)
                generated = self.run_tool(icl, pdl, 'prefix.pdl', modeled=modeled)
                pdl.write_text('iPDLLevel 1\niProcsForModule Top\n'+prefix+suffix)
                original = self.run_tool(icl, pdl, 'source-tail.pdl', modeled=modeled)
                continuation = self.root/'continuation.pdl'
                continuation.write_text(generated+suffix)
                self.assertEqual(original, self.run_tool(icl, continuation, 'generated-tail.pdl', modeled=modeled))
                flat = self.root/'debug/Top.icl'
                self.assertEqual(original, self.run_tool(flat, continuation, 'flat-tail/result.pdl', modeled=modeled))

    def test_unknown_length_requires_only_the_unstable_child_again(self):
        self.replay("""iReset; iWrite head 5; iWrite tail 19; iApply
iScan child1.local 8 -si 0x3c -stable; iApply
iScan child0.local 8 -si 0xa5; iApply
iScan child0.local 8 -si 0xf0 -stable; iApply
iWrite head 3; iApply
""", MODEL)
        self.rejected("""iReset; iScan child0.local 8 -si 1; iApply
iScan child1.local 8 -si 2 -stable; iApply
""", 'length', MODEL)

    def test_sparse_child_payloads_and_unknown_mismatch_status(self):
        output = self.replay("""iReset; iWrite head 5; iWrite tail 19; iApply
iScan child0.local 8 -si 0b1xx0x0xx -so 0 -stable
iScan child1.local 8 -si 0bxx1x01x0 -so 0 -stable; iApply
iNote [iGetReadData child0.local -bin]; iNote [iGetMiscompares child0.local -bin]
iNote [iGetReadData child1.local -bin]; iNote [iGetMiscompares child1.local -bin]
if {![catch {iGetStatus -clear}]} {error {invented known status}}
""", MODEL)
        self.assertEqual(self.notes(output), ['0bxxxxxxxx']*4)
        self.assertIn('iScan client 24 -si 0b1011xx0x0xxxx1x01x010011', output)

    def test_mixed_widths_keep_exact_serial_offsets(self):
        output = self.replay("""iReset; iWrite head 5; iWrite tail 19; iApply
iScan child0.local 4 -si 0b1010 -stable
iScan child1.local 9 -si 0b101001011 -stable
iScan child2.local 1 -si 1 -stable; iApply
iNote [iGetReadData child0.local -bin]
iNote [iGetReadData child1.local -bin]
iNote [iGetReadData child2.local -bin]
""", model((4, 9, 1)))
        self.assertEqual(self.notes(output), ['0bxxxx', '0bxxxxxxxxx', '0bx'])
        self.assertIn('iScan client 22 -si 0b1011010101001011110011', output)

    def test_wrong_lengths_and_overlapping_requests_reject_atomically(self):
        for commands, diagnostic in [
            ('iScan child0.local 9; iApply', 'length'),
            ('iScan child1.local 7; iApply', 'length'),
            ('iScan client 24; iScan child0.local 8; iApply', 'overlapping'),
            ('iScan child0.local 8 -si 256; iReset', 'scan')]:
            with self.subTest(commands=commands):
                self.rejected('iReset; '+commands+'\n', diagnostic, MODEL)

    def test_unmatched_annotations_remain_informational_without_inventing_length(self):
        prefix = 'modeled opaque child stable scan '
        for setup, payload in [
            ('iReset;', 'child1.local 0b00000000'),
            ('iReset; iScan client 24 -si 0 -stable; iApply;', 'missing 0b00000000'),
            ('iReset; iScan client 24 -si 0 -stable; iApply;', 'child1.local 0b00000001'),
            ('iReset; iScan client 24 -si 0 -stable; iApply;', 'child1.local 0b0000000'),
            ('iReset; iScan client 24 -si 0 -stable; iApply; iReset;', 'child1.local 0b00000000'),
            ('iReset;', 'child1.local')]:
            with self.subTest(payload=payload, setup=setup):
                self.replay(setup+'iNote -comment {'+prefix+payload+'}\n', MODEL)
        for tag in ('', '-comment ', '-status '):
            source = 'Module Top {DataInPort p; DataOutPort q {Source p;}}'
            output = self.replay('iNote '+tag+'{'+prefix+'test description}; '
                                 'iWrite p 1; iApply; iRead q 1; iApply; iNote [iGetReadData q -dec]\n',
                                 source)
            self.assertEqual(self.notes(output), ['1'])
        self.replay('iNote -comment {literal "$missing" [missing] \\path}; '
                    'iWrite p 1; iApply; iRead q 1; iApply\n', source)
        for payload in ('child1.local 0b00000001','missing 0b00000000',
                        'child1.local 0b0000000'):
            self.rejected('iReset; iScan client 24 -si 0; iApply; '
                          'iNote -comment {'+prefix+payload+'}; '
                          'iScan child0.local 8 -si 1 -stable; iApply\n', 'length', MODEL)

    def test_tagged_quoted_annotations_restore_generated_prefix(self):
        prefix = """iReset; iWrite head 5; iWrite tail 19; iApply
iScan child1.local 8 -si 0x3c -stable; iApply
iScan child0.local 8 -si 0xa5; iApply
"""
        suffix = 'iScan child0.local 8 -si 240 -stable; iApply\n'
        icl, pdl = self.files(prefix, MODEL)
        generated = self.run_tool(icl, pdl, 'prefix.pdl')
        lines = []
        for line in generated.splitlines():
            if line.startswith('iNote {modeled opaque child stable scan '):
                line = 'iNote -comment "'+line[len('iNote {'):-1]+'"'
            lines.append(line)
        icl, source = self.files(prefix+suffix, MODEL)
        expected = self.run_tool(icl, source, 'source-tail.pdl')
        for tag in ('-comment', '-status'):
            pdl = self.root/'tagged.pdl'
            pdl.write_text('\n'.join(lines).replace('iNote -comment ', 'iNote '+tag+' ')+'\n'+suffix)
            actual = self.run_tool(icl, pdl, 'tagged-tail.pdl')
            canonical = actual.replace('iNote -status ', 'iNote -comment ')
            self.assertEqual(expected, canonical)
        notes = [line for line in generated.splitlines()
                 if 'modeled opaque child stable scan ' in line]
        self.assertTrue(notes)
        self.assertTrue(all(line.startswith('iNote -comment "') and line.endswith('"')
                            for line in notes))

    def test_duplicate_matching_annotations_roundtrip_idempotently(self):
        for tag in ('', '-comment '):
            commands = """iReset; iWrite head 5; iWrite tail 19; iApply
iScan child1.local 8 -si 0x3c -stable; iApply
iScan child0.local 8 -si 0xa5; iApply
"""
            commands += ('iNote '+tag+'{modeled opaque child stable scan child1.local 0b00111100}\n')*2
            commands += 'iScan child0.local 8 -si 17 -stable; iApply\n'
            with self.subTest(tag=tag):
                self.replay(commands, MODEL)

    def test_route_inversions_and_bit_specific_aliases(self):
        for invert_input, invert_output in ((False, False), (True, False),
                                            (False, True), (True, True)):
            with self.subTest(invert_input=invert_input, invert_output=invert_output):
                source = MODEL.replace('Module Top {',
                                       'Module Top { Alias bridge[1:0] = child0.so, head[0];')
                source = source.replace('InputPort si = child0.so;',
                                        'InputPort si = '+('~' if invert_input else '')+'bridge[1];')
                if invert_output:
                    source = source.replace('Source tail[0]', 'Source ~tail[0]')
                output = self.replay("""iReset; iWrite head 5; iWrite tail 19; iApply
iScan child0.local 8 -si 0xa5 -stable; iScan child1.local 8 -si 0x3c -stable; iApply
iRead head 5; iRead tail 19; iApply
iNote [iGetReadData head -dec]; iNote [iGetReadData tail -dec]
""", source)
                self.assertEqual(self.notes(output), ['5', '19'])

    def test_namespace_prefix_split_files_and_unchanged_reset_source(self):
        icl, pdl = self.files("""iProcsForModule ChildLeaf -iProcNameSpace ops
iProc shift {value} {iScan local 8 -si $value -stable; iApply}
iProcsForModule Top
iReset; iWrite head 5; iWrite tail 19; iApply
""", MODEL)
        second = self.root/'second.pdl'
        second.write_text("""iPrefix child0; iCall ops::shift 0xa5
iNote [iGetReadData local -bin]; iPrefix -
iPrefix child1; iCall ops::shift 0x3c
iNote [iGetReadData local -bin]; iPrefix -
""")
        output = self.run_tool(icl, pdl, reset=True, extra=('--pdl', str(second)))
        self.assertEqual(self.notes(output), ['0bxxxxxxxx', '0bxxxxxxxx'])
        reset = self.root/'reset.pdl'
        self.assertIn(pdl.read_text(), reset.read_text())
        self.assertTrue(reset.read_text().endswith(second.read_text()))
        self.assertEqual(output, self.run_tool(icl, reset, 'reset-replay.pdl'))

    def test_private_side_channels_and_unsupported_modes_reject_atomically(self):
        leaking = MODEL.replace('CaptureSource tail;', 'CaptureSource child0.so, tail[3:0];')
        self.rejected('iReset; iScan child1.local 8; iApply\n', 'CaptureSource', leaking)
        for command in ('iRunLoop 1', 'iClock tck', 'iClockOverride tck -freqDivider 2',
                        'iOverrideScanInterface client -capture off',
                        'iMerge -begin; iMerge -end'):
            with self.subTest(command=command):
                self.rejected('iReset; '+command+'; iScan child0.local 8; iApply\n', 'opaque', MODEL)

    def test_modeled_sibling_retains_actual_value_between_opaque_children(self):
        source = base.VISIBLE + MODEL.replace(' Instance child1 Of', """ Instance vis Of Visible {
 InputPort si = child0.so; InputPort sel = sel; InputPort ce = ce;
 InputPort se = se; InputPort ue = ue; InputPort tck = tck; InputPort reset = reset;
 }
 Instance child1 Of""")
        position = source.index(' Instance child1 Of')
        source = source[:position]+source[position:].replace('InputPort si = child0.so;', 'InputPort si = vis.so;', 1)
        source = source.replace("DefaultLoadValue 24'b0", "DefaultLoadValue 32'b0")
        output = self.replay("""iReset; iWrite head 5; iWrite tail 19; iWrite vis.value 9; iApply
iScan child0.local 8 -si 0xa5 -stable; iScan child1.local 8 -si 0x3c -stable; iApply
iScan vis.local 8 -si 0x5a -stable; iApply
iRead vis.value 90; iRead head 5; iRead tail 19; iApply
iNote [iGetReadData vis.value -dec]
iNote [iGetReadData head -dec]; iNote [iGetReadData tail -dec]
""", source)
        self.assertEqual(self.notes(output), ['90', '5', '19'])
        self.assertIn('iScan client 32 -si 0b10110100101010110100011110010011', output)

    def test_private_targets_reservations_and_invalid_routes(self):
        self.replay("""iReset; iTake tail; iWrite tail 19; iApply
iWrite head 5; iApply
iScan child0.local 8 -si 0xa5 -stable; iScan child1.local 8 -si 0x3c -stable; iApply
iRelease tail
""", MODEL)
        self.rejected('iReset; iTake sel; iState sel 0; iScan child1.local 8; iApply\n',
                      'reserved', MODEL)
        for index in (0,1):
            self.rejected('iReset; iWrite child%d.__opaque_scan_state 1; iApply\n' % index,
                          'identifier', MODEL)
        for source in (MODEL.replace('InputPort si = child0.so;', 'InputPort si = head[0];'),
                       MODEL.replace('InputPort sel = sel;', 'InputPort sel = head[0];')):
            self.rejected('iReset; iScan child0.local 8; iApply\n', 'scan', source)


if __name__ == '__main__':
    unittest.main()
