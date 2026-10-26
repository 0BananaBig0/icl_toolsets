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
        paths = sorted({cases / m['path'] for m in modules})
        self.assertEqual({p for p in paths if p.parent.name == 'eda'},
                         set(cases.glob('pass/*/eda/*.icl')))
        self.assertEqual(len(modules), len({m['id'] for m in modules}))
        result = subprocess.run([str(ROOT / 'build/icl_corpus_inventory'), *map(str, paths)],
                                capture_output=True, check=True)
        inventory = json.loads(result.stdout)
        self.assertEqual(len(inventory), len(paths))
        inventory = dict(zip(paths, inventory))
        hashes = {p: hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
        file_hashes = set()
        for sha in hashes.values():
            self.assertNotIn(sha, file_hashes, 'byte-identical canonical fixtures')
            file_hashes.add(sha)
        for m in modules:
            path = cases / m['path']
            with self.subTest(path=m['path']):
                matches = [item for item in inventory[path]['modules'] if module_id(item) == m['id']]
                self.assertEqual(len(matches), 1, 'canonical module must occur exactly once in its file')
                for key in ('name', 'namespace', 'use_namespace'):
                    self.assertEqual(matches[0][key], m[key])
                self.assertEqual(hashes[path], m['sha256'])
        ids = {m['id'] for m in modules}
        referenced = set()
        origins = {}
        for source in manifest['sources']:
            self.assertNotIn(source['path'], origins)
            origins[source['path']] = source
            self.assertEqual(category(Path(source['path'])), source['category'])
            versions = [source, *source.get('previous_versions', [])]
            self.assertEqual(len(versions), len({v['sha256'] for v in versions}))
            for version in versions:
                self.assertTrue(set(version['modules']) <= ids)
                referenced.update(version['modules'])
                for comment in version['comments']:
                    self.assertGreaterEqual(comment, 0)
                    self.assertLess(comment, len(manifest['comments']))
        self.assertEqual(ids, referenced)
        for m in modules:
            source = origins[m['source']]
            self.assertTrue(any(m['id'] in v['modules'] for v in
                                [source, *source.get('previous_versions', [])]))
            if Path(m['path']).parent.name == 'eda':
                self.assertEqual(Path(m['path']).parts[1], origins[m['source']]['category'])


if __name__ == '__main__':
    unittest.main()
