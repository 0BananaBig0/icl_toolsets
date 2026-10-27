#!/usr/bin/env python3
"""Tests for the optional remote Tessent differential harness."""
import json
import hashlib
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'tests'))
import tessent_oracle as oracle


class Oracle(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        (self.root / 'tests').mkdir()
        (self.root / 'tests/a.icl').write_text('Module M {}')
        (self.root / 'tests/a.pdl').write_text('iWrite r 1\niApply\n')
        self.case = dict(id='simple', top='M', icl=['tests/a.icl'],
                         pdl=['tests/a.pdl'], requires_local_reset=False,
                         expected='ok')

    def test_new_cases_have_test_prefix_and_stable_ids(self):
        bundle = self.root / 'bundle'
        report = oracle.prepare(self.root, [self.case], bundle, reference_only=True)
        self.assertTrue((bundle / 'test_simple/design_0.icl').is_file())
        self.assertTrue((bundle / 'test_simple/run.tcl').is_file())
        self.assertFalse((bundle / 'simple').exists())
        self.assertEqual(report['cases'][0]['id'], 'simple')
        self.assertEqual(report['case_directory_prefix'], 'test_')

    def test_run_and_compare_reject_unsafe_directory_prefix(self):
        for index, prefix in enumerate(('../', '/tmp/', 'other_', None, ['test_'])):
            with self.subTest(prefix=prefix):
                bundle = self.root / f'bundle_{index}'
                report = oracle.prepare(self.root, [self.case], bundle, reference_only=True)
                # Rejection must happen even when there is no tool work to perform.
                report['cases'][0]['status'] = 'blocked_generator'
                report['case_directory_prefix'] = prefix
                (bundle / 'bundle.json').write_text(json.dumps(report))
                with self.assertRaisesRegex(ValueError, 'directory prefix'):
                    oracle.run_bundle(bundle, '/no/tool', 1)
                (bundle / 'results.json').write_text(json.dumps(report))
                with self.assertRaisesRegex(ValueError, 'directory prefix'):
                    oracle.compare(bundle, Path('/no/trace'))

    def test_run_and_compare_reject_case_directory_symlinks(self):
        bundle = self.root / 'bundle'
        report = oracle.prepare(self.root, [self.case], bundle, reference_only=True)
        directory = bundle / 'test_simple'
        moved = self.root / 'moved'
        directory.rename(moved)
        directory.symlink_to(moved, target_is_directory=True)
        report['cases'][0]['status'] = 'blocked_generator'
        (bundle / 'bundle.json').write_text(json.dumps(report))
        with self.assertRaisesRegex(ValueError, 'invalid case directory'):
            oracle.run_bundle(bundle, '/no/tool', 1)
        (bundle / 'results.json').write_text(json.dumps(report))
        with self.assertRaisesRegex(ValueError, 'invalid case directory'):
            oracle.compare(bundle, Path('/no/trace'))

    def test_compare_rejects_escaping_case_ids(self):
        bundle = self.root / 'bundle'
        report = oracle.prepare(self.root, [self.case], bundle, reference_only=True)
        report['cases'][0]['id'] = '../escape'
        (bundle / 'results.json').write_text(json.dumps(report))
        with self.assertRaisesRegex(ValueError, 'case id'):
            oracle.compare(bundle, Path('/no/trace'))

    def test_tcl_paths_are_literal(self):
        value = 'path with $var [error bad] "quotes" \\ and\nnewline'
        script = 'puts ' + oracle.tcl_word(value) + '\n'
        result = subprocess.run(['tclsh'], input=script, text=True,
                                capture_output=True, check=True)
        self.assertEqual(result.stdout, value + '\n')

    def test_bsdl_staging_script_inventory_and_integrity(self):
        source = self.root / 'tests/model with $ and [chars].vhd'
        source.write_text('entity Probe is end Probe;\n')
        self.case['bsdl'] = source.relative_to(self.root).as_posix()
        self.case['bsdl_entity'] = 'Probe'
        bundle = self.root / 'bundle'
        report = oracle.prepare(self.root, [self.case], bundle, reference_only=True)
        directory = bundle / 'test_simple'
        staged = directory / 'model.bsdl'
        self.assertTrue(staged.is_file(), 'BSDL must be staged with the ICL/PDL')
        self.assertEqual(staged.read_bytes(), source.read_bytes())
        self.assertEqual(report['cases'][0]['inputs']['model.bsdl'],
                         hashlib.sha256(source.read_bytes()).hexdigest())
        self.assertIn('-bsdl_file [file join $here "model.bsdl"]',
                      (directory / 'run.tcl').read_text())
        inventory = oracle.catalog(self.root, [self.case])
        self.assertEqual(inventory[self.case['bsdl']]['cases'], ['simple'])
        staged.write_text('modified')
        with self.assertRaisesRegex(ValueError, 'changed input model.bsdl'):
            oracle.run_bundle(bundle, '/no/tool', 1)

    def test_bsdl_entity_is_explicit_and_validated(self):
        self.case['bsdl'] = 'tests/model.bsdl'
        (self.root / self.case['bsdl']).write_text('entity Probe is end Probe;')
        for index, entity in enumerate([None, '', 'bad name', 'x/y', 17]):
            with self.subTest(entity=entity):
                self.case['bsdl_entity'] = entity
                output = self.root / f'entity_{index}'
                with self.assertRaisesRegex(ValueError, 'bsdl_entity'):
                    oracle.prepare(self.root, [self.case], output, reference_only=True)
                self.assertFalse(output.exists())
        del self.case['bsdl_entity']
        with self.assertRaisesRegex(ValueError, 'bsdl_entity'):
            oracle.validate(self.root, [self.case])

    def test_invalid_bsdl_inputs_are_rejected_before_staging(self):
        for index, invalid in enumerate(['../escape.bsdl', '/outside.bsdl',
                                         'tests/missing.bsdl', '', [], None]):
            with self.subTest(invalid=invalid):
                self.case['bsdl'] = invalid
                with self.assertRaises(ValueError):
                    oracle.prepare(self.root, [self.case], self.root / f'bundle_{index}',
                                   reference_only=True)
                self.assertFalse((self.root / f'bundle_{index}').exists())

    def test_missing_generator_is_blocked_not_reference_success(self):
        report = oracle.prepare(self.root, [self.case], self.root / 'bundle')
        self.assertEqual(report['cases'][0]['status'], 'blocked_generator')
        self.assertFalse((self.root / 'bundle/test_simple/run.tcl').exists())

    def test_reference_only_does_not_bypass_required_local_reset(self):
        self.case['requires_local_reset'] = True
        report = oracle.prepare(self.root, [self.case], self.root / 'bundle',
                                reference_only=True)
        self.assertEqual(report['cases'][0]['status'], 'blocked_local_reset')

    def test_inventory_accounts_for_all_files(self):
        (self.root / 'tests/orphan.pdl').write_text('iProc unused {} {}')
        (self.root / 'tests/generated').mkdir()
        (self.root / 'tests/generated/generated.icl').write_text('Module X {}')
        report = oracle.catalog(self.root, [self.case])
        self.assertEqual(len(report), 4)
        self.assertEqual(report['tests/a.icl']['cases'], ['simple'])
        self.assertEqual(report['tests/orphan.pdl']['status'], 'unpaired')
        self.assertEqual(report['tests/generated/generated.icl']['status'],
                         'generated_syntax_case')

    def test_reject_duplicate_ids_path_escape_and_existing_output(self):
        with self.assertRaises(ValueError):
            oracle.prepare(self.root, [self.case, self.case], self.root / 'dup')
        self.case['pdl'] = ['../escape.pdl']
        with self.assertRaises(ValueError):
            oracle.prepare(self.root, [self.case], self.root / 'escape')
        self.case['pdl'] = ['tests/a.pdl']
        out = self.root / 'existing'
        out.mkdir()
        (out / 'keep').write_text('unchanged')
        with self.assertRaises(FileExistsError):
            oracle.prepare(self.root, [self.case], out, reference_only=True)
        self.assertEqual((out / 'keep').read_text(), 'unchanged')

    def test_reference_script_has_explicit_initial_state_and_pdl_export(self):
        report = oracle.prepare(self.root, [self.case], self.root / 'bundle',
                                reference_only=True)
        self.assertEqual(report['cases'][0]['status'], 'ready_reference_only')
        # Exercise emitted Tcl with recording commands; no Tcl expressions in paths.
        script = (self.root / 'bundle/test_simple/run.tcl').read_text()
        capture = self.root / 'captured'
        shim = ('proc unknown {args} {puts $::capture $args}\n'
                'rename exit real_exit\nproc exit {args} {}\n'
                'set capture [open ' + oracle.tcl_word(str(capture)) + ' w]\n')
        subprocess.run(['tclsh'], input=shim + script + '\nclose $capture\n',
                       text=True, check=True, capture_output=True)
        calls = capture.read_text()
        self.assertIn('open_pattern_set oracle -no_initial_ireset', calls)
        self.assertIn('-pdl -pattern_sets oracle', calls)
        self.assertIn('source_0.pdl', calls)

    def test_zero_exit_with_tool_error_cannot_pass(self):
        output = self.root / 'result.pdl'
        output.write_text('iApply')
        self.assertEqual(oracle.result_status(0, '// Error: failed\n'
                         'ICL_TOOLSETS_ORACLE_COMPLETE\n', output), 'tool_error')
        self.assertEqual(oracle.result_status(0, '// puts "ICL_TOOLSETS_ORACLE_COMPLETE"',
                                              output), 'incomplete')
        self.assertEqual(oracle.result_status(0, 'ICL_TOOLSETS_ORACLE_COMPLETE\n',
                                              output), 'ok')
        output.unlink()
        self.assertEqual(oracle.result_status(0, 'ICL_TOOLSETS_ORACLE_COMPLETE\n',
                                              output), 'missing_output')

    def test_generator_original_input_and_tessent_rewrite_are_distinct(self):
        # An actual subprocess contract fixture: assert received paths and publish outputs.
        generator = self.root / 'generator'
        generator.write_text('#!/usr/bin/env python3\nimport pathlib,sys\n'
            'a=sys.argv\n'
            'p=pathlib.Path(a[a.index("--pdl")+1])\n'
            'assert "iWrite r 1" in p.read_text()\n'
            'assert a[a.index("--bsdl")+1] == "Probe"\n'
            'b=pathlib.Path(a[a.index("--bsdl")+2])\n'
            'assert b.read_text() == "entity Probe is end Probe;"\n'
            'pathlib.Path(a[a.index("--output")+1]).write_text("iWrite r 1\\niApply\\n")\n'
            'pathlib.Path(a[a.index("--local_reset")+1]).write_text('
            '"# reset rewrite\\niWrite reset 1\\niApply\\niWrite r 1\\niApply\\n")\n')
        generator.chmod(0o755)
        self.case['requires_local_reset'] = True
        self.case['bsdl'] = 'tests/source.bsdl'
        self.case['bsdl_entity'] = 'Probe'
        (self.root / self.case['bsdl']).write_text('entity Probe is end Probe;')
        report = oracle.prepare(self.root, [self.case], self.root / 'bundle',
                                generator=generator)
        self.assertEqual(report['cases'][0]['status'], 'ready_comparison')
        command = report['cases'][0]['generator_command']
        self.assertIn('--bsdl', command)
        self.assertEqual(command[command.index('--bsdl') + 1], 'Probe')
        self.assertEqual(Path(command[command.index('--bsdl') + 2]).read_text(),
                         'entity Probe is end Probe;')
        script = (self.root / 'bundle/test_simple/run.tcl').read_text()
        self.assertIn('local_reset.pdl', script)
        self.assertNotIn('source_0.pdl', script)
        self.assertIn('iWrite reset 1',
                      (self.root / 'bundle/test_simple/local_reset.pdl').read_text())

    def test_manifest_requires_explicit_reset_policy(self):
        del self.case['requires_local_reset']
        with self.assertRaises(ValueError):
            oracle.prepare(self.root, [self.case], self.root / 'bundle',
                           reference_only=True)

    def test_bundle_rejects_changed_scripts_and_escaping_ids(self):
        bundle = self.root / 'bundle'
        report = oracle.prepare(self.root, [self.case], bundle, reference_only=True)
        script = bundle / 'test_simple/run.tcl'
        script.write_text('error changed')
        with self.assertRaisesRegex(ValueError, 'changed input'):
            oracle.run_bundle(bundle, '/no/tool', 1)
        report['cases'][0]['id'] = '../escape'
        (bundle / 'bundle.json').write_text(json.dumps(report))
        with self.assertRaisesRegex(ValueError, 'case id'):
            oracle.run_bundle(bundle, '/no/tool', 1)

    def test_timeout_and_stale_outputs(self):
        code, _ = oracle.run_process(
            [sys.executable, '-c', 'import time; time.sleep(20)'],
            self.root, self.root / 'timeout.log', 0.05)
        self.assertEqual(code, 124)
        self.assertEqual(oracle.result_status(code, '', self.root / 'absent'), 'timeout')
        bundle = self.root / 'bundle'
        oracle.prepare(self.root, [self.case], bundle, reference_only=True)
        (bundle / 'test_simple/result.pdl').write_text('stale')
        with self.assertRaises(FileExistsError):
            oracle.run_bundle(bundle, '/no/tool', 1)

    def test_reconstruct_original_modules_with_shared_parser(self):
        inventory = ROOT / 'build/icl_corpus_inventory'
        origin = 'test/tessent_ijtag/test_iWrite/chip_top.icl'
        source = oracle.materialize_origin(ROOT, origin, inventory)
        self.assertNotIn(b'NameSpace', source)
        path = self.root / 'reconstructed.icl'
        path.write_bytes(source)
        result = subprocess.run([str(inventory), str(path)], check=True,
                                capture_output=True)
        modules = json.loads(result.stdout)[0]
        self.assertTrue(modules['ok'])
        self.assertEqual([item['name'] for item in modules['modules']],
                         ['Sensor', 'TDR', 'SIB', 'chip'])

    def test_comparison_cannot_pass_unexpected_vendor_success(self):
        bundle = self.root / 'bundle'
        (bundle / 'simple').mkdir(parents=True)
        (bundle / 'simple/ours.pdl').write_text('iApply')
        report = dict(mode='comparison', cases=[dict(
            self.case, status='reference_fail', outcome='ok', expectation_met=False)])
        (bundle / 'results.json').write_text(json.dumps(report))
        result = oracle.compare(bundle, Path('/no/trace'))
        self.assertEqual(result['cases'][0]['comparison'], 'blocked_reference')

    def test_reconstruction_resets_empty_namespaces(self):
        from import_eda_corpus import module_id
        path = self.root / 'tests/cases/modules.icl'
        path.parent.mkdir()
        path.write_text('NameSpace scoped; UseNameSpace scoped; Module A {}\n'
                        'NameSpace; UseNameSpace; Module B {}\n')
        inventory = ROOT / 'build/icl_corpus_inventory'
        run = subprocess.run([str(inventory), str(path)], capture_output=True, check=True)
        parsed = json.loads(run.stdout)[0]
        self.assertTrue(parsed['ok'])
        ids = [module_id(module) for module in parsed['modules']]
        manifest = dict(sources=[dict(path='original.icl', modules=ids)],
                        modules=[dict(id=key, path='modules.icl') for key in ids])
        (path.parent / 'eda_sources.json').write_text(json.dumps(manifest))
        output = self.root / 'reconstructed.icl'
        output.write_bytes(oracle.materialize_origin(self.root, 'original.icl', inventory))
        run = subprocess.run([str(inventory), str(output)], capture_output=True, check=True)
        actual = json.loads(run.stdout)[0]
        self.assertTrue(actual['ok'])
        self.assertEqual([(m['namespace'], m['use_namespace']) for m in actual['modules']],
                         [('scoped', 'scoped'), ('', '')])

    def test_run_compare_and_changed_rewrite(self):
        for prefix in ('test_', ''):
            with self.subTest(prefix=prefix):
                bundle = self.root / ('prefixed' if prefix else 'legacy')
                report = oracle.prepare(self.root, [self.case], bundle, reference_only=True)
                directory = bundle / 'test_simple'
                if not prefix:
                    report.pop('case_directory_prefix', None)
                    directory.rename(bundle / 'simple')
                    directory = bundle / 'simple'
                report['mode'] = 'comparison'
                for name in ['ours.pdl', 'local_reset.pdl']:
                    data = b'iProcsForModule M\niWrite r 1\niApply\n'
                    (directory / name).write_bytes(data)
                    report['cases'][0]['inputs'][name] = hashlib.sha256(data).hexdigest()
                report['cases'][0]['tessent_pdl_inputs'] = ['local_reset.pdl']
                (bundle / 'bundle.json').write_text(json.dumps(report))
                # Simulated vendor subprocess tests the runner protocol, not retargeting.
                vendor = self.root / 'vendor'
                vendor.write_text('#!/usr/bin/env python3\nfrom pathlib import Path\n'
                    'Path("result.pdl").write_text('
                    '"iProcsForModule M\\niProc oracle {} {iWrite r 1; iApply}\\n")\n'
                    'print("Tessent Shell fixture")\n'
                    'print("ICL_TOOLSETS_ORACLE_COMPLETE")\n')
                vendor.chmod(0o755)
                result = oracle.run_bundle(bundle, str(vendor), 10)
                self.assertEqual(result['cases'][0]['status'], 'reference_pass')
                compared = oracle.compare(bundle, ROOT / 'build/pdl_pattern_trace')
                self.assertEqual(compared['cases'][0]['comparison'], 'same_operations')
                (directory / 'local_reset.pdl').write_text('iReset')
                with self.assertRaisesRegex(ValueError, 'changed comparison input'):
                    oracle.compare(bundle, ROOT / 'build/pdl_pattern_trace')

    def test_required_testbench_missing_or_hashed(self):
        for exported in (False, True):
            with self.subTest(exported=exported):
                bundle = self.root / f'testbench_{exported}'
                report = oracle.prepare(self.root, [self.case], bundle, reference_only=True)
                report['cases'][0]['export_testbench'] = True
                (bundle / 'bundle.json').write_text(json.dumps(report))
                vendor = self.root / f'vendor_{exported}'
                body = ('#!/usr/bin/env python3\nfrom pathlib import Path\n'
                        'Path("result.pdl").write_text("iWrite r 1; iApply\\n")\n'
                        'print("ICL_TOOLSETS_ORACLE_COMPLETE")\n')
                if exported:
                    body += ('Path("result.v").write_text("module tb; endmodule\\n")\n'
                             'Path("result.v.0.vec").write_text("01\\n")\n')
                vendor.write_text(body)
                vendor.chmod(0o755)
                result = oracle.run_bundle(bundle, str(vendor), 10)['cases'][0]
                self.assertEqual(result['status'], 'reference_pass' if exported else 'reference_fail')
                if exported:
                    self.assertEqual(set(result['testbench_hashes']), {'result.v', 'result.v.0.vec'})
                else:
                    self.assertEqual(result['outcome'], 'missing_testbench')


if __name__ == '__main__':
    unittest.main()
