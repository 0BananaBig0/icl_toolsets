#!/usr/bin/env python3
"""Contract tests for explicit vendor certification and offline-only goldens."""
import copy
import hashlib
import json
import os
from pathlib import Path
import sys
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'tests'))
import tessent_golden as golden


class Goldens(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.work = Path(self.temp.name)
        self.root = self.work / 'repo'
        self.root.mkdir()
        (self.root / 'input.icl').write_text(
            'Module M{DataInPort data;DataOutPort out{Source data;}}\n')
        (self.root / 'input.pdl').write_text(
            'iProcsForModule M\niWrite out 0b1;iApply\n')
        self.case = dict(id='Example', top='M', icl=['input.icl'],
                         pdl=['input.pdl'], expected='ok', requires_local_reset=False)
        build = Path(os.environ.get('GOLDEN_BUILD', ROOT / 'build')).resolve()
        self.binaries = {name: build / name for name in golden.TOOLS}
        self.trace = build / 'pdl_pattern_trace'
        self.bundle = self.work / 'bundle'

    def prepared(self):
        return golden.prepare(self.root, [self.case], self.bundle, self.binaries)

    def vendor_evidence(self, report):
        # Simulate only the licensed process boundary; production binaries and
        # the real static trace parser exercise all local transformations.
        report = copy.deepcopy(report)
        for job in report['cases']:
            folder = self.bundle / ('test_' + job['id'])
            data = b'iProcsForModule M\niProc oracle {} {\niWrite data 0b1\niApply\n}\n'
            (folder / 'result.pdl').write_bytes(data)
            (folder / 'console.log').write_text(
                '// Tessent Shell TEST FIXTURE\n' + golden.oracle.MARKER + '\n')
            job.update(status='reference_pass', outcome='ok', exit_code=0,
                       version='// Tessent Shell TEST FIXTURE',
                       result_sha256=hashlib.sha256(data).hexdigest())
        (self.bundle / 'results.json').write_text(json.dumps(report))

    def test_flat_pattern_is_vendor_input_and_formatter_jobs_are_separate(self):
        report = self.prepared()
        jobs = {job['id']: job for job in report['cases']}
        pattern = jobs['Example-icl-pattern-generation']
        self.assertEqual(pattern['tessent_pdl_inputs'], ['flat.pdl'])
        script = (self.bundle / 'test_Example-icl-pattern-generation/run.tcl').read_text()
        self.assertIn('"flat.pdl"', script)
        self.assertNotIn('local_reset', script)
        original = self.bundle / 'test_Example'
        for tool, unchanged in [('icl-format', 'source_0.pdl'), ('pdl-format', 'design_0.icl')]:
            self.assertEqual((original / unchanged).read_bytes(),
                             (self.bundle / f'test_Example-{tool}' / unchanged).read_bytes())

    def test_record_then_offline_check_and_no_implicit_overwrite(self):
        self.vendor_evidence(self.prepared())
        for tool in golden.TOOLS:
            snapshot = self.work / tool
            golden.record(self.root, self.bundle, 'Example', tool, snapshot, self.trace)
            golden.check(self.root, snapshot, self.binaries)
            self.assertTrue((snapshot / 'vendor.pdl.expected').is_file())
            with self.assertRaises(FileExistsError):
                golden.record(self.root, self.bundle, 'Example', tool, snapshot, self.trace)
            # Exact-byte checks must catch even a formatting-only output change.
            cert = json.loads((snapshot / 'snapshot.json').read_text())
            name = next(iter(cert['artifacts'])) + '.expected'
            (snapshot / name).write_bytes((snapshot / name).read_bytes() + b'\n')
            with self.assertRaisesRegex(ValueError, 'hash|changed'):
                golden.check(self.root, snapshot, self.binaries)

    def test_stale_source_and_mutated_vendor_input_cannot_be_certified(self):
        self.vendor_evidence(self.prepared())
        snapshot = self.work / 'snapshot'
        staged = self.bundle / 'test_Example-icl-format/design_0.icl'
        staged.write_bytes(staged.read_bytes() + b'\n')
        with self.assertRaisesRegex(ValueError, 'hash|changed'):
            golden.record(self.root, self.bundle, 'Example', 'icl-format', snapshot, self.trace)
        self.assertFalse(snapshot.exists())
        (self.root / 'input.pdl').write_text('iWrite out 0\niApply\n')
        with self.assertRaisesRegex(ValueError, 'source|hash|changed'):
            golden.record(self.root, self.bundle, 'Example', 'pdl-format', snapshot, self.trace)
        self.assertFalse(snapshot.exists())

    def test_fixed_point_alone_does_not_certify_wrong_original_semantics(self):
        report = self.prepared()
        self.vendor_evidence(report)
        results = json.loads((self.bundle / 'results.json').read_text())
        original = results['cases'][0]
        folder = self.bundle / ('test_' + original['id'])
        data = (folder / 'result.pdl').read_bytes().replace(b'0b1', b'0b0')
        (folder / 'result.pdl').write_bytes(data)
        original['result_sha256'] = hashlib.sha256(data).hexdigest()
        (self.bundle / 'results.json').write_text(json.dumps(results))
        with self.assertRaisesRegex(ValueError, 'differ'):
            golden.record(self.root, self.bundle, 'Example', 'icl-pattern-generation',
                          self.work / 'snapshot', self.trace)
        self.assertFalse((self.work / 'snapshot').exists())

    def test_offline_rejects_stale_source(self):
        self.vendor_evidence(self.prepared())
        snapshot = self.work / 'snapshot'
        golden.record(self.root, self.bundle, 'Example', 'icl-format', snapshot, self.trace)
        (self.root / 'input.icl').write_bytes(b'Module M {}\n')
        with self.assertRaisesRegex(ValueError, 'source|hash|changed'):
            golden.check(self.root, snapshot, self.binaries)

    def test_failed_console_cannot_be_certified_by_pass_status(self):
        self.vendor_evidence(self.prepared())
        (self.bundle / 'test_Example-icl-format/console.log').write_text(
            '// Error: failed to load design\n')
        with self.assertRaisesRegex(ValueError, 'console|vendor'):
            golden.record(self.root, self.bundle, 'Example', 'icl-format',
                          self.work / 'snapshot', self.trace)
        self.assertFalse((self.work / 'snapshot').exists())

    def test_changed_prepared_metadata_rejected(self):
        self.vendor_evidence(self.prepared())
        path = self.bundle / 'results.json'
        original = path.read_bytes()
        for key, value in [('top', 'UnrelatedTop'), ('binary_sha256', 'forged')]:
            with self.subTest(key=key):
                report = json.loads(original)
                report['cases'][2][key] = value
                path.write_text(json.dumps(report))
                snapshot = self.work / key
                with self.assertRaisesRegex(ValueError, 'changed prepared'):
                    golden.record(self.root, self.bundle, 'Example', 'icl-format',
                                  snapshot, self.trace)
                self.assertFalse(snapshot.exists())

    def test_changed_certificate_context_rejected_offline(self):
        self.vendor_evidence(self.prepared())
        snapshot = self.work / 'snapshot'
        golden.record(self.root, self.bundle, 'Example', 'icl-format', snapshot, self.trace)
        path = snapshot / 'snapshot.json'
        certificate = json.loads(path.read_text())
        certificate['case']['top'] = 'NeverCertified'
        path.write_text(json.dumps(certificate))
        with self.assertRaisesRegex(ValueError, 'context|binding|certified'):
            golden.check(self.root, snapshot, self.binaries)

    def test_pattern_must_be_a_fixed_point_without_operand_regrouping(self):
        self.vendor_evidence(self.prepared())
        path = self.bundle / 'results.json'
        report = json.loads(path.read_text())
        for job in report['cases']:
            folder = self.bundle / ('test_' + job['id'])
            data = (folder / 'result.pdl').read_bytes().replace(b'0b1', b'1')
            (folder / 'result.pdl').write_bytes(data)
            job['result_sha256'] = hashlib.sha256(data).hexdigest()
        path.write_text(json.dumps(report))
        with self.assertRaisesRegex(ValueError, 'fixed point'):
            golden.record(self.root, self.bundle, 'Example', 'icl-pattern-generation',
                          self.work / 'snapshot', self.trace)

    def test_snapshot_symlink_is_never_replaced(self):
        self.vendor_evidence(self.prepared())
        snapshot = self.work / 'snapshot'
        snapshot.symlink_to(self.work / 'missing')
        with self.assertRaises(FileExistsError):
            golden.record(self.root, self.bundle, 'Example', 'icl-format', snapshot, self.trace)
        self.assertTrue(snapshot.is_symlink())

    def test_cli_rejects_changed_manifest_context(self):
        self.vendor_evidence(self.prepared())
        snapshots = self.work / 'snapshots'
        golden.record(self.root, self.bundle, 'Example', 'icl-format',
                      snapshots / 'example', self.trace)
        manifest = self.work / 'cases.json'
        changed = dict(self.case, top='NeverCertified')
        manifest.write_text(json.dumps({'cases': [changed]}))
        run = subprocess.run(
            [sys.executable, str(ROOT / 'tests/tessent_golden.py'), 'check',
             '--root', str(self.root), '--goldens', str(snapshots),
             '--manifest', str(manifest)], capture_output=True, text=True)
        self.assertNotEqual(run.returncode, 0)
        self.assertIn('manifest', run.stderr)


if __name__ == '__main__':
    unittest.main()
