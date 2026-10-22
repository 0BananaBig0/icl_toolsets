#!/usr/bin/env python3
"""Distinct same-line reductions must never collapse into a single hit."""
import importlib.util
import contextlib
import io
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch
ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location('grammar_reader', ROOT/'tests/grammar_coverage.py')
grammar = importlib.util.module_from_spec(spec)
spec.loader.exec_module(grammar)

class GrammarCoverage(unittest.TestCase):
    def test_same_line_productions_are_distinct(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp)/'parser.cc'
            path.write_text('const int yyrline_[] = {0, 41, 41, 41, 47};')
            # Generator slot0 is unused; slot1 is $accept; printed IDs = slot-1.
            self.assertEqual(grammar.rule_lines(path, 'yyrline'), {1: 41, 2: 41, 3: 47})

    def test_trace_retains_production_id(self):
        self.assertEqual(grammar.REDUCE.findall('Reducing stack by rule 2 (line 41):'), [('2', '41')])

    def test_missing_table_is_error(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp)/'parser.cc'; path.write_text('// no table')
            with self.assertRaises(ValueError): grammar.rule_lines(path, 'yyrline')

    def test_empty_rule_tables_are_errors(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / 'tables.cc'
            path.write_text('int yyrline[] = {0, 1}; int yy_rule_linenum[] = {0};')
            for name in ('yyrline', 'yy_rule_linenum'):
                with self.subTest(name=name), self.assertRaises(ValueError):
                    grammar.rule_lines(path, name)

    def exercise(self, result, extra=(), accept=True, blocks=("enum",)):
        generated = ROOT / 'tests/generated/ours/coverage_reader'
        generated.mkdir(parents=True, exist_ok=True)
        with tempfile.TemporaryDirectory(dir=generated) as tmp:
            root = Path(tmp)
            case = root / 'case.icl'
            case.write_text('Module CoverageReader {}\n')
            (root / 'parser.cc').write_text('int yyrline[] = {0, 1, 10};')
            (root / 'lexer.cc').write_text('int yy_rule_linenum[] = {0, 20};')
            manifest = root / 'cases.json'
            manifest.write_text(json.dumps([{'path': str(case), 'block': block,
                                             'accept': accept} for block in blocks]))
            args = ['grammar_coverage.py', '--binary', 'traced-formatter',
                    '--parser', str(root / 'parser.cc'), '--lexer', str(root / 'lexer.cc'),
                    '--cases', str(case), *extra]
            if '--report' in extra:
                args[args.index('--report') + 1] = str(root / 'report.json')
                args += ['--manifest', str(manifest)]
            output = io.StringIO()
            def run(*args, **kwargs):
                if isinstance(result, Exception):
                    raise result
                kwargs['stderr'].write(result.stderr)
                return result

            with patch.object(sys, 'argv', args), patch.object(
                    grammar.subprocess, 'run', side_effect=run), contextlib.redirect_stdout(output), contextlib.redirect_stderr(output):
                status = grammar.main()
            report = root / 'report.json'
            return status, json.loads(report.read_text()) if report.exists() else None

    def trace(self, code):
        return subprocess.CompletedProcess([], code, stderr=(
            b'Reducing stack by rule 1 (line 10):\n--accepting rule at line 20\n'))

    def test_crash_after_full_trace_is_not_coverage_success(self):
        self.assertNotEqual(self.exercise(self.trace(-11))[0], 0)

    def test_timeout_is_reported_as_failure(self):
        self.assertNotEqual(self.exercise(subprocess.TimeoutExpired('formatter', 30))[0], 0)

    def test_empty_explicit_selection_does_not_fall_back_to_corpus(self):
        with self.assertRaises(SystemExit) as error:
            self.exercise(self.trace(0), ['--cases'])
        self.assertEqual(error.exception.code, 2)

    def test_block_report_attributes_distinct_rules(self):
        status, report = self.exercise(self.trace(0), ['--report', 'unused'])
        self.assertEqual(status, 0)
        self.assertEqual(report['blocks']['enum'], {
            'accepted': 1, 'rejected': 0, 'productions': [1], 'scanner_lines': [20]})

    def test_shared_fixture_is_attributed_to_each_manifest_block(self):
        status, report = self.exercise(self.trace(0), ['--report', 'unused'],
                                       blocks=('enum', 'data_out_port'))
        self.assertEqual(status, 0)
        self.assertEqual(report['files'], 1)
        self.assertEqual(set(report['blocks']), {'enum', 'data_out_port'})
        for block in report['blocks'].values():
            self.assertEqual(block['accepted'], 1)
            self.assertEqual(block['productions'], [1])

    def test_full_trace_does_not_override_negative_expectation(self):
        status, _ = self.exercise(self.trace(0), ['--report', 'unused'], accept=False)
        self.assertNotEqual(status, 0)

if __name__ == '__main__': unittest.main()
