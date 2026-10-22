#!/usr/bin/env python3
"""Exercise real parser-backed import; catch lost modules and false deduplication."""
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]


class ImportTest(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)
        self.inputs = self.root / 'input'
        self.output = self.root / 'output'

    def source(self, name, text):
        path = self.inputs / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(text)
        return path

    def run_import(self):
        return subprocess.run([sys.executable, str(ROOT / 'tests/import_eda_corpus.py'),
                               '--inventory', str(ROOT / 'build/icl_corpus_inventory'),
                               '--input', str(self.inputs), '--output', str(self.output)],
                              capture_output=True, text=True)

    def test_duplicates_variants_context_and_notices(self):
        source = ('// Copyright sample\nModule A { Attribute text = "} // x"; '
                  "Enum E { OFF = 1'b0; } }\nModule B {}")
        self.source('test/tessent_ijtag/a.icl', source)
        self.source('tessent_2026.2_usage_examples/copy.ICL', source)
        self.source('syn_slm/variant.icl', 'Module A { Attribute text = "} // y"; }')
        self.source('test/tessent_ijtag/test_syn_slm_icl/partial.icl',
                    '// another notice\nModule B { /* braces } { */ }\nModule C {}')
        self.source('syn_slm/namespace.icl', 'NameSpace N; Module B {}')
        self.source('syn_slm/use.icl', 'UseNameSpace N; Module B {}')
        self.source('syn_slm/layout.icl',
                    'Module /* header notice */ A{Attribute text="} // x"; '
                    "Enum E{OFF=1'b0;}}")
        before = {str(p): p.read_bytes() for p in self.inputs.rglob('*') if p.is_file()}
        result = self.run_import()
        self.assertEqual(result.returncode, 0, result.stderr)
        manifest = json.loads((self.output / 'eda_sources.json').read_text())
        self.assertEqual(len(manifest['sources']), 7)
        self.assertEqual(len(manifest['modules']), 6)
        self.assertEqual(len(list(self.output.rglob('*.icl'))), 6)
        partial = next(s for s in manifest['sources'] if s['path'].endswith('partial.icl'))
        self.assertEqual(partial['category'], 'syn_slm')
        self.assertEqual(len(partial['modules']), 2)
        self.assertIn('another notice', json.dumps(manifest['comments']))
        self.assertIn('header notice', json.dumps(manifest['comments']))
        sources = {s['path']: s for s in manifest['sources']}
        self.assertEqual(sources['test/tessent_ijtag/a.icl']['modules'],
                         sources['tessent_2026.2_usage_examples/copy.ICL']['modules'])
        self.assertEqual(sources['syn_slm/layout.icl']['modules'],
                         sources['test/tessent_ijtag/a.icl']['modules'][:1])
        for m in manifest['modules']:
            result = subprocess.run([str(ROOT / 'build/icl-format'), str(self.output / m['path'])],
                                    capture_output=True)
            self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(before, {str(p): p.read_bytes() for p in self.inputs.rglob('*') if p.is_file()})
        self.assertNotEqual(self.run_import().returncode, 0, 'must not overwrite existing output')

    def test_generic_payload_and_string_whitespace_are_significant(self):
        for i, value in enumerate(['x y', 'x  y']):
            self.source(f'syn_slm/g{i}.icl', f'Module A {{ AccessLink a Of Generic {{ {value} }} }}')
            self.source(f'syn_slm/s{i}.icl', f'Module B {{ Attribute x = "{value}"; }}')
        result = self.run_import()
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(len(list(self.output.rglob('*.icl'))), 4)

    def test_invalid_input_leaves_no_partial_corpus(self):
        self.source('syn_slm/good.icl', 'Module A {}')
        self.source('syn_slm/bad.icl', 'Module Broken {')
        result = self.run_import()
        self.assertNotEqual(result.returncode, 0)
        self.assertIn('bad.icl', result.stderr)
        self.assertFalse(self.output.exists())

    def test_unknown_source_category_is_rejected(self):
        self.source('elsewhere/a.icl', 'Module A {}')
        result = self.run_import()
        self.assertNotEqual(result.returncode, 0)
        self.assertIn('unknown source category', result.stderr)
        self.assertFalse(self.output.exists())


if __name__ == '__main__':
    unittest.main()
