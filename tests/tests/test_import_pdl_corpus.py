#!/usr/bin/env python3
"""Executable-boundary checks for the maintained PDL snapshot importer."""
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]


class ImportPdlCorpus(unittest.TestCase):
    def run_import(self, source, output):
        return subprocess.run([sys.executable, str(ROOT / 'tests/import_pdl_corpus.py'),
                               '--input', str(source), '--output', str(output)],
                              text=True, capture_output=True, timeout=10)

    def test_deduplicates_files_and_preserves_entire_stream_and_origins(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source, output = root / 'source', root / 'output'
            content = b'# notice\niProcsForModule A\niProc x {} {iApply}\niProcsForModule B\niProc x {} {iApply}\n'
            originals = {
                'syn_slm/original.pdl': content,
                'syn_slm/copy.PDL': content,
                'syn_slm/tessent_example_copy/tessent.pdl': b'# Tessent example\niApply\n',
                'test/tessent_ijtag/test_syn_slm_icl/example.pdl': content,
                'tessent_2026.2_usage_examples/demo/other.pdl': b'iApply\n',
            }
            for relative, data in originals.items():
                path = source / relative
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_bytes(data)
            run = self.run_import(source, output)
            self.assertEqual(run.returncode, 0, run.stderr)
            manifest = json.loads((output / 'sources.json').read_text())
            self.assertEqual(len(manifest['sources']), 5)
            self.assertEqual(len(manifest['fixtures']), 3)
            self.assertEqual({p.read_bytes() for p in output.rglob('*.pdl')}, set(originals.values()))
            for origin in manifest['sources']:
                self.assertEqual((output / origin['fixture']).read_bytes(), originals[origin['path']])
                self.assertEqual((source / origin['path']).read_bytes(), originals[origin['path']])
            copied_tessent = next(s for s in manifest['sources'] if 'tessent_example_copy' in s['path'])
            self.assertEqual(copied_tessent['category'], 'tessent')

    def test_refuses_existing_output_without_overwriting(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            output = root / 'existing'
            output.mkdir()
            marker = output / 'keep'
            marker.write_text('original')
            run = self.run_import(root / 'input', output)
            self.assertEqual(run.returncode, 1, run.stderr)
            self.assertIn('already exists', run.stderr)
            self.assertEqual(marker.read_text(), 'original')

    def test_empty_input_reports_error(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            run = self.run_import(root, root / 'output')
            self.assertEqual(run.returncode, 1, run.stderr)
            self.assertIn('no PDL', run.stderr)
            self.assertFalse((root / 'output').exists())


if __name__ == '__main__':
    unittest.main()
