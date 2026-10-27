#!/usr/bin/env python3
"""Directional certification tests; only the licensed process is simulated."""
import hashlib
import json
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
