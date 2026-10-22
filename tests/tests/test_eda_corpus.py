#!/usr/bin/env python3
"""Guard maintained EDA fixtures against loss, duplication and provenance drift."""
import hashlib
import json
from pathlib import Path
import subprocess
import sys
import unittest

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'tests'))
from import_eda_corpus import module_id, category


class CorpusTest(unittest.TestCase):
    def test_canonical_definitions_and_all_source_mappings(self):
        cases = ROOT / 'tests/cases'
        manifest_path = cases / 'eda_sources.json'
        self.assertTrue(manifest_path.is_file(), 'EDA source manifest missing')
        manifest = json.loads(manifest_path.read_text())
        modules = manifest['modules']
        paths = [cases / m['path'] for m in modules]
        self.assertEqual(set(paths), set(cases.glob('pass/*/eda/*.icl')))
        self.assertEqual(len(paths), len(set(paths)), 'two canonical objects share one file')
        self.assertEqual(len(modules), len({m['id'] for m in modules}))
        result = subprocess.run([str(ROOT / 'build/icl_corpus_inventory'), *map(str, paths)],
                                capture_output=True, check=True)
        inventory = json.loads(result.stdout)
        self.assertEqual(len(inventory), len(modules))
        file_hashes = set()
        for m, parsed, path in zip(modules, inventory, paths):
            with self.subTest(path=m['path']):
                self.assertEqual(len(parsed['modules']), 1)
                self.assertEqual(module_id(parsed['modules'][0]), m['id'])
                sha = hashlib.sha256(path.read_bytes()).hexdigest()
                self.assertEqual(sha, m['sha256'])
                self.assertNotIn(sha, file_hashes, 'byte-identical canonical fixtures')
                file_hashes.add(sha)
        ids = {m['id'] for m in modules}
        referenced = set()
        origins = {}
        for source in manifest['sources']:
            self.assertNotIn(source['path'], origins)
            origins[source['path']] = source
            self.assertEqual(category(Path(source['path'])), source['category'])
            self.assertTrue(set(source['modules']) <= ids)
            referenced.update(source['modules'])
            for comment in source['comments']:
                self.assertGreaterEqual(comment, 0)
                self.assertLess(comment, len(manifest['comments']))
        self.assertEqual(ids, referenced)
        for m in modules:
            self.assertIn(m['id'], origins[m['source']]['modules'])
            self.assertEqual(Path(m['path']).parts[1], origins[m['source']]['category'])


if __name__ == '__main__':
    unittest.main()
