#!/usr/bin/env python3
"""Exercise duplicate detection using the real shared parser and source bytes."""
import json
import hashlib
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]


class AuditTest(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)

    def source(self, path, text):
        target = self.root / path
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(text.encode())

    def audit(self, policy=None):
        command = [sys.executable, str(ROOT / 'tests/audit_icl_corpus.py'),
                   '--root', str(self.root), '--inventory', str(ROOT / 'build/icl_corpus_inventory'),
                   '--report', str(self.root / 'report.json')]
        if policy is not None:
            path = self.root / 'policy.json'
            path.write_text(json.dumps(policy))
            command += ['--policy', str(path)]
        run = subprocess.run(command, capture_output=True, text=True)
        report = self.root / 'report.json'
        return run, json.loads(report.read_text()) if report.exists() else {}

    def test_identical_files_and_lexical_modules_are_separate_findings(self):
        self.source('a.icl', 'Module A { Attribute s = "} // not a comment"; }')
        self.source('copy.ICL', 'Module A { Attribute s = "} // not a comment"; }')
        self.source('layout.icl', '/* notice */ Module A{Attribute s="} // not a comment";}')
        run, report = self.audit()
        self.assertEqual(run.returncode, 1, run.stderr)
        self.assertEqual(report['file_duplicates'][0]['paths'], ['a.icl', 'copy.ICL'])
        self.assertEqual(len(report['module_duplicates']), 1)
        self.assertEqual(len(report['module_duplicates'][0]['occurrences']), 3)

    def test_context_and_opaque_content_distinguish_definitions(self):
        for path, text in {
            'a.icl': 'Module A {}', 'ns.icl': 'NameSpace N; Module A {}',
            'use.icl': 'UseNameSpace N; Module A {}',
            'opaque1.icl': 'Module G { AccessLink a Of Generic { x y } }',
            'opaque2.icl': 'Module G { AccessLink a Of Generic { x  y } }',
            'invalid.icl': 'Module Broken {',
        }.items():
            self.source(path, text)
        run, report = self.audit()
        self.assertEqual(run.returncode, 0, run.stderr)
        self.assertEqual(report['rejected'], ['invalid.icl'])
        self.assertEqual(report['module_duplicates'], [])
        strict = subprocess.run([str(ROOT / 'build/icl_corpus_inventory'),
                                 str(self.root / 'invalid.icl')], capture_output=True)
        self.assertNotEqual(strict.returncode, 0, 'normal importer must still reject invalid input')

    def test_reviewed_variants_are_exactly_pinned(self):
        self.source('a.icl', 'Module A {}')
        self.source('comment.icl', 'Module A { /* formatting regression */ }')
        run, report = self.audit()
        self.assertEqual(run.returncode, 1, run.stderr)
        self.assertIn('module_duplicates', report)
        policy = {'retained_module_duplicates': report['module_duplicates']}
        for group in policy['retained_module_duplicates']:
            group['reason'] = 'Distinct comment-preservation regression inputs.'
        run, _ = self.audit(policy)
        self.assertEqual(run.returncode, 0, run.stderr)
        self.source('new.icl', 'Module A { /* unreviewed copy */ }')
        run, _ = self.audit(policy)
        self.assertEqual(run.returncode, 1, 'policy must not silently allow more duplicates')
        (self.root / 'new.icl').unlink()
        (self.root / 'comment.icl').unlink()
        run, _ = self.audit(policy)
        self.assertEqual(run.returncode, 1, 'obsolete policy entry must be removed')

    def test_negative_cases_are_checked_for_file_duplicates(self):
        self.source('bad.icl', 'Module Broken {')
        self.source('copy.icl', 'Module Broken {')
        run, report = self.audit()
        self.assertEqual(run.returncode, 1, run.stderr)
        self.assertEqual(report['rejected'], ['bad.icl', 'copy.icl'])
        self.assertEqual(len(report['file_duplicates']), 1)

    def test_only_the_generated_benchmark_tree_is_excluded(self):
        self.source('a.icl', 'Module A {}')
        self.source('generated/performance.icl', 'Module A {}')
        run, report = self.audit()
        self.assertEqual(run.returncode, 0, run.stderr)
        self.assertEqual(report['files'], 1)
        self.source('cases/vendor/generated/sample.icl', 'Module A {}')
        run, report = self.audit()
        self.assertEqual(run.returncode, 1, 'nested source directories must not bypass audit')

    def test_empty_corpus_is_not_success(self):
        run, _ = self.audit()
        self.assertEqual(run.returncode, 2)
        self.assertIn('no maintained ICL', run.stderr)

    def test_removal_records_resolve_to_the_original_content(self):
        self.source('a.icl', 'Module A {}')
        record = {'path': 'old.icl', 'canonical_path': 'a.icl',
                  'kind': 'identical-file', 'sha256': 'wrong', 'modules': []}
        policy = {'removed_files': [record]}
        run, _ = self.audit(policy)
        self.assertEqual(run.returncode, 1, 'wrong canonical file must fail provenance check')
        record['sha256'] = hashlib.sha256(b'Module A {}').hexdigest()
        run, _ = self.audit(policy)
        self.assertEqual(run.returncode, 0, run.stderr)
        record['kind'] = 'module-already-in-baseline'
        record['modules'] = ['nonexistent-module']
        run, _ = self.audit(policy)
        self.assertEqual(run.returncode, 1, 'absent canonical module must fail')
        record['canonical_path'] = 'missing.icl'
        run, _ = self.audit(policy)
        self.assertEqual(run.returncode, 1, 'missing canonical file must fail')


if __name__ == '__main__':
    unittest.main()
