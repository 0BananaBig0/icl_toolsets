#!/usr/bin/env python3
"""Directional certification tests; only the licensed process is simulated."""
import hashlib
import json
import copy
import shutil
from pathlib import Path
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parent))
from test_tessent_golden import Goldens
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import tessent_roundtrip as rt


class RoundTrips(unittest.TestCase):
    setUp = Goldens.setUp
    prepared = Goldens.prepared
    vendor_evidence = Goldens.vendor_evidence
    def test_directions_certify_separately_and_replay_offline(self):
        self.vendor_evidence(self.prepared())
        for direction in ('forward', 'reverse'):
            snapshot = self.work / direction
            cert = rt.record(self.root, self.bundle, 'Example', direction,
                             snapshot, self.trace, self.binaries['icl-pattern-generation'])
            self.assertEqual(cert['direction'], direction)
            rt.check(self.root, snapshot, self.binaries['icl-pattern-generation'], self.trace)
            with self.assertRaises(FileExistsError):
                rt.record(self.root, self.bundle, 'Example', direction,
                          snapshot, self.trace, self.binaries['icl-pattern-generation'])
            (snapshot / 'vendor.pdl.expected').write_text('iApply\n')
            with self.assertRaisesRegex(ValueError, 'hash|changed'):
                rt.check(self.root, snapshot, self.binaries['icl-pattern-generation'], self.trace)

    def alter_export(self, job_id, data):
        path = self.bundle / 'results.json'
        report = json.loads(path.read_text())
        job = next(j for j in report['cases'] if j['id'] == job_id)
        (self.bundle / ('test_' + job_id) / 'result.pdl').write_bytes(data)
        job['result_sha256'] = hashlib.sha256(data).hexdigest()
        path.write_text(json.dumps(report))

    def test_forward_does_not_require_independent_source_schedule(self):
        self.vendor_evidence(self.prepared())
        self.alter_export('Example', b'iProcsForModule M\niProc oracle {} {iWrite data 0;iApply}\n')
        rt.record(self.root, self.bundle, 'Example', 'forward', self.work / 'forward',
                  self.trace, self.binaries['icl-pattern-generation'])

    def read_evidence(self, value='X'):
        (self.root / 'input.icl').write_text(
            'Module M{DataInPort data;DataInPort enable;DataRegister value{'
            'WriteDataSource data;WriteEnSource enable;ResetValue 0;}'
            'DataOutPort observed{Source value;}}\n')
        (self.root / 'input.pdl').write_text(f'iProcsForModule M\niRead value 0b{value};iApply\n')
        self.vendor_evidence(self.prepared())
        data = f'iProcsForModule M\niProc oracle {{}} {{iRead observed 0b{value};iApply}}\n'.encode()
        for job in ('Example', 'Example-icl-pattern-generation'):
            self.alter_export(job, data)

    def test_source_vendor_equivalence_preserves_distinct_logical_read_trace(self):
        self.read_evidence()
        snapshot = self.work / 'source-equivalence'
        try:
            cert = rt.record(self.root, self.bundle, 'Example', 'forward', snapshot,
                             self.trace, self.binaries['icl-pattern-generation'],
                             source_bundle=self.bundle)
        except (TypeError, ValueError) as error:
            self.fail('equivalent vendor readback must certify: ' + str(error))
        self.assertNotEqual(cert['native_trace'], cert['trace'])
        self.assertIn('iRead value', (snapshot / 'flat.pdl.expected').read_text())
        rt.check(self.root, snapshot, self.binaries['icl-pattern-generation'], self.trace)

    def test_source_vendor_equivalence_rejects_different_apply_groups(self):
        self.vendor_evidence(self.prepared())
        for body in ('iWrite data 1;iApply;iApply', 'iWrite data 0;iApply'):
            self.alter_export('Example',
                              ('iProcsForModule M\niProc oracle {} {' + body + '}\n').encode())
            with self.assertRaisesRegex(ValueError, 'differ'):
                rt.record(self.root, self.bundle, 'Example', 'forward', self.work / 'bad',
                          self.trace, self.binaries['icl-pattern-generation'],
                          source_bundle=self.bundle)
            self.assertFalse((self.work / 'bad').exists())

    def test_source_vendor_equivalence_rejects_changed_cared_read(self):
        self.read_evidence('1')
        self.alter_export('Example',
                          b'iProcsForModule M\niProc oracle {} {iRead observed 0b0;iApply}\n')
        with self.assertRaisesRegex(ValueError, 'differ'):
            rt.record(self.root, self.bundle, 'Example', 'forward', self.work / 'bad',
                      self.trace, self.binaries['icl-pattern-generation'], source_bundle=self.bundle)
        self.assertFalse((self.work / 'bad').exists())

    def reference_evidence(self):
        self.vendor_evidence(self.prepared())
        previous = rt.record(self.root, self.bundle, 'Example', 'forward', self.work / 'previous',
                             self.trace, self.binaries['icl-pattern-generation'])
        current_bundle = self.bundle
        self.bundle = self.work / 'reference'
        report = rt.prepare(self.root, [self.case], self.bundle,
                            self.binaries['icl-pattern-generation'], 'forward')
        report['reference_certificates'] = {'Example': previous}
        (self.bundle / 'bundle.json').write_text(json.dumps(report))
        self.vendor_evidence(report)
        reference_bundle = self.bundle
        self.bundle = current_bundle
        return reference_bundle, report, previous

    def test_certified_generated_reference_retains_its_provenance(self):
        reference_bundle, report, previous = self.reference_evidence()
        current_bundle = self.bundle
        snapshot = self.work / 'reference-equivalence'
        cert = rt.record(self.root, self.bundle, 'Example', 'forward', snapshot, self.trace,
                         self.binaries['icl-pattern-generation'], source_bundle=reference_bundle)
        self.assertEqual(cert['source_reference'], previous)
        rt.check(self.root, snapshot, self.binaries['icl-pattern-generation'], self.trace)
        cert['source_reference']['artifacts']['flat.pdl'] = 'forged'
        (snapshot / 'snapshot.json').write_text(json.dumps(cert))
        with self.assertRaisesRegex(ValueError, 'reference'):
            rt.check(self.root, snapshot, self.binaries['icl-pattern-generation'], self.trace)
        report['reference_certificates']['Example']['artifacts']['flat.pdl'] = 'forged'
        (reference_bundle / 'bundle.json').write_text(json.dumps(report))
        (reference_bundle / 'results.json').write_text(json.dumps(report))
        # Preserve actual successful results while changing the certificate in
        # both prepared/result metadata, so provenance validation must catch it.
        self.bundle = reference_bundle
        self.vendor_evidence(report)
        self.bundle = current_bundle
        with self.assertRaisesRegex(ValueError, 'reference'):
            rt.record(self.root, self.bundle, 'Example', 'forward', self.work / 'bad-reference',
                      self.trace, self.binaries['icl-pattern-generation'], source_bundle=reference_bundle)

    def test_record_rejects_false_execution_and_incomplete_prior_provenance(self):
        bundle, _, _ = self.reference_evidence()
        prepared = json.loads((bundle / 'bundle.json').read_text())
        results = json.loads((bundle / 'results.json').read_text())
        for field in ('false_source', 'binary_sha256', 'vendor_console_sha256', 'prior_input'):
            with self.subTest(field=field):
                before, after = copy.deepcopy(prepared), copy.deepcopy(results)
                for report in (before, after):
                    reference = report['reference_certificates']['Example']
                    if field == 'false_source':
                        del report['reference_certificates']
                        report['cases'][0]['tessent_pdl_inputs'] = ['source_0.pdl']
                    elif field == 'prior_input':
                        reference['job']['inputs'][reference['job']['tessent_pdl_inputs'][0]] = '0' * 64
                    else:
                        del reference[field]
                (bundle / 'bundle.json').write_text(json.dumps(before))
                (bundle / 'results.json').write_text(json.dumps(after))
                with self.assertRaises(ValueError):
                    rt.record(self.root, self.bundle, 'Example', 'forward', self.work / field,
                              self.trace, self.binaries['icl-pattern-generation'], source_bundle=bundle)

    def test_offline_rejects_false_execution_and_incomplete_prior_provenance(self):
        snapshot = self.work / 'actual-queued-state'
        shutil.copytree(rt.ROOT / 'tests/golden/tessent_roundtrip/queued_state-forward', snapshot)
        path = snapshot / 'snapshot.json'
        original = json.loads(path.read_text())
        rt.check(rt.ROOT, snapshot, self.binaries['icl-pattern-generation'], self.trace)
        for field in ('false_source', 'binary_sha256', 'vendor_console_sha256',
                      'prior_top', 'prior_input', 'prior_result'):
            with self.subTest(field=field):
                cert = copy.deepcopy(original)
                reference = cert['source_reference']
                if field == 'false_source':
                    del cert['source_reference']
                    cert['source_job']['tessent_pdl_inputs'] = ['source_0.pdl']
                elif field == 'prior_top':
                    reference['job']['top'] = 'Wrong'
                elif field == 'prior_input':
                    reference['job']['inputs'][reference['job']['tessent_pdl_inputs'][0]] = '0' * 64
                elif field == 'prior_result':
                    reference['job']['result_sha256'] = '0' * 64
                else:
                    del reference[field]
                path.write_text(json.dumps(cert))
                with self.assertRaises(ValueError):
                    rt.check(rt.ROOT, snapshot, self.binaries['icl-pattern-generation'], self.trace)

    def test_source_vendor_equivalence_rejects_stale_or_mutated_evidence(self):
        self.read_evidence()
        snapshot = self.work / 'source-equivalence'
        rt.record(self.root, self.bundle, 'Example', 'forward', snapshot,
                  self.trace, self.binaries['icl-pattern-generation'], source_bundle=self.bundle)
        path = snapshot / 'snapshot.json'
        original = path.read_text()
        for field in ('source_top', 'source_inputs', 'native_trace', 'source_trace', 'direction'):
            cert = json.loads(original)
            if field == 'source_top':
                cert['source_job']['top'] = 'Wrong'
            elif field == 'source_inputs':
                cert['source_job']['tessent_pdl_inputs'] = ['flat.pdl']
            elif field == 'native_trace':
                cert['native_trace']['operations'] = []
            elif field == 'source_trace':
                p = snapshot / 'source_vendor.pdl.expected'
                p.write_bytes(p.read_bytes().replace(b'iApply', b'iApply;iApply'))
                cert['artifacts']['source_vendor.pdl'] = hashlib.sha256(p.read_bytes()).hexdigest()
                cert['source_job']['result_sha256'] = cert['artifacts']['source_vendor.pdl']
            else:
                cert['direction'] = 'reverse'
            path.write_text(json.dumps(cert))
            with self.assertRaises(ValueError, msg=field):
                rt.check(self.root, snapshot, self.binaries['icl-pattern-generation'], self.trace)
            if field == 'source_trace':
                p.write_bytes(p.read_bytes().replace(b'iApply;iApply', b'iApply'))
        path.write_text(original)
        staged = self.bundle / 'test_Example/source_0.pdl'
        staged.write_bytes(staged.read_bytes() + b'\n')
        with self.assertRaisesRegex(ValueError, 'hash|changed'):
            rt.record(self.root, self.bundle, 'Example', 'forward', self.work / 'stale',
                      self.trace, self.binaries['icl-pattern-generation'], source_bundle=self.bundle)

    def test_forward_rejects_extra_apply_and_changed_value(self):
        self.vendor_evidence(self.prepared())
        for body in ('iWrite data 1;iApply;iApply', 'iWrite data 0;iApply'):
            self.alter_export('Example-icl-pattern-generation',
                              ('iProcsForModule M\niProc oracle {} {' + body + '}\n').encode())
            with self.assertRaisesRegex(ValueError, 'differ'):
                rt.record(self.root, self.bundle, 'Example', 'forward', self.work / 'bad',
                          self.trace, self.binaries['icl-pattern-generation'])
            self.assertFalse((self.work / 'bad').exists())

    def test_reverse_does_not_require_forward_success(self):
        self.vendor_evidence(self.prepared())
        path = self.bundle / 'results.json'
        report = json.loads(path.read_text())
        report['cases'][1]['status'] = 'candidate_failed'
        path.write_text(json.dumps(report))
        rt.record(self.root, self.bundle, 'Example', 'reverse', self.work / 'reverse',
                  self.trace, self.binaries['icl-pattern-generation'])

    def test_mutated_staging_and_stale_source_rejected(self):
        self.vendor_evidence(self.prepared())
        path = self.bundle / 'test_Example/design_0.icl'
        path.write_bytes(path.read_bytes() + b'\n')
        with self.assertRaisesRegex(ValueError, 'hash|changed'):
            rt.record(self.root, self.bundle, 'Example', 'reverse', self.work / 'bad',
                      self.trace, self.binaries['icl-pattern-generation'])
        (self.root / 'input.pdl').write_text('iApply\n')
        with self.assertRaisesRegex(ValueError, 'source|hash|changed'):
            rt.record(self.root, self.bundle, 'Example', 'forward', self.work / 'bad',
                      self.trace, self.binaries['icl-pattern-generation'])

    def test_offline_rejects_changed_context_and_trace(self):
        self.vendor_evidence(self.prepared())
        snapshot = self.work / 'forward'
        rt.record(self.root, self.bundle, 'Example', 'forward', snapshot,
                  self.trace, self.binaries['icl-pattern-generation'])
        path = snapshot / 'snapshot.json'
        original = path.read_text()
        for field in ('top', 'input', 'trace', 'policy'):
            cert = json.loads(original)
            if field == 'top':
                cert['job']['top'] = 'Wrong'
            elif field == 'input':
                cert['job']['tessent_pdl_inputs'] = ['source_0.pdl']
            elif field == 'trace':
                cert['trace']['operations'] = []
            else:
                cert['policy'] = 'ignore-applies'
            path.write_text(json.dumps(cert))
            with self.assertRaises(ValueError, msg=field):
                rt.check(self.root, snapshot, self.binaries['icl-pattern-generation'], self.trace)

    def test_forward_preparation_uses_flat_pattern_even_for_reset_cases(self):
        self.case['requires_local_reset'] = True
        report = rt.prepare(self.root, [self.case], self.bundle,
                            self.binaries['icl-pattern-generation'], 'forward')
        job = report['cases'][0]
        self.assertEqual(job['tessent_pdl_inputs'], ['ours.pdl'])
        folder = rt.oracle.case_directory(self.bundle, report, job)
        self.assertNotIn('local_reset', (folder / 'run.tcl').read_text())
        self.assertEqual(job['status'], 'ready_reference_only')


del Goldens

if __name__ == '__main__':
    unittest.main()
