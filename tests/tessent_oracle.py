#!/usr/bin/env python3
"""Prepare isolated Tessent PDL runs; account for unpaired and blocked cases.

Preparation/comparison run locally. Run a prepared bundle on the EDA host.
No implicit SSH, source changes, golden refresh or license-parallel launches.
"""
import argparse
from collections import Counter
import hashlib
import json
import os
from pathlib import Path
import re
import signal
import subprocess
import sys
import time

ROOT = Path(__file__).resolve().parents[1]
MARKER = 'ICL_TOOLSETS_ORACLE_COMPLETE'


def tcl_word(value):
    return '"' + ''.join({'\\': '\\\\', '"': '\\"', '$': '\\$',
                          '[': '\\[', ']': '\\]', '\n': '\\n',
                          '\r': '\\r'}.get(c, c) for c in value) + '"'


def input_path(root, name):
    path = (root / name).resolve()
    if Path(name).is_absolute() or root.resolve() not in path.parents:
        raise ValueError(f'input outside repository: {name}')
    if not path.is_file():
        raise ValueError(f'missing input: {name}')
    return path


def validate(root, cases):
    ids = set()
    for case in cases:
        name = case['id']
        if not re.fullmatch(r'[A-Za-z][A-Za-z0-9_-]*', name) or name in ids:
            raise ValueError(f'invalid or duplicate case id: {name}')
        ids.add(name)
        if not isinstance(case.get('requires_local_reset'), bool):
            raise ValueError(f'{name}: explicit requires_local_reset boolean required')
        if not re.fullmatch(r'[A-Za-z][A-Za-z0-9_]*(?:::[A-Za-z][A-Za-z0-9_]*)?',
                            case['top']):
            raise ValueError(f'{name}: invalid top module')
        if case.get('expected') not in ('ok', 'error'):
            raise ValueError(f'{name}: expected must be ok or error')
        if case['expected'] == 'error' and not case.get('diagnostic'):
            raise ValueError(f'{name}: expected error requires a diagnostic')
        if not case.get('pdl') or bool(case.get('icl')) == bool(case.get('icl_origin')):
            raise ValueError(f'{name}: PDL and exactly one ICL source form required')
        for source in case.get('icl', []) + case['pdl']:
            input_path(root, source)
        if 'bsdl' in case:
            if not isinstance(case['bsdl'], str) or not case['bsdl']:
                raise ValueError(f'{name}: bsdl must name a repository file')
            input_path(root, case['bsdl'])
            entity = case.get('bsdl_entity')
            if not isinstance(entity, str) or not re.fullmatch(r'[A-Za-z][A-Za-z0-9_]*', entity):
                raise ValueError(f'{name}: bsdl_entity must name the BSDL entity explicitly')


def origin_files(root, origin):
    manifest = json.loads((root / 'tests/cases/eda_sources.json').read_text())
    sources = {source['path']: source for source in manifest['sources']}
    if origin not in sources:
        raise ValueError(f'unknown EDA source: {origin}')
    modules = {item['id']: item for item in manifest['modules']}
    return [modules[key] for key in sources[origin]['modules']]


def materialize_origin(root, origin, inventory):
    from import_eda_corpus import module_id
    modules = origin_files(root, origin)
    names = list(dict.fromkeys('tests/cases/' + m['path'] for m in modules))
    paths = [input_path(root, name) for name in names]
    run = subprocess.run([str(inventory), *map(str, paths)],
                         capture_output=True, check=True, timeout=120)
    parsed = json.loads(run.stdout)
    if len(parsed) != len(paths) or any(not item['ok'] for item in parsed):
        raise ValueError('invalid canonical ICL inventory')
    fragments = {}
    for path, record in zip(paths, parsed):
        data = path.read_bytes()
        for module in record['modules']:
            fragments[module_id(module)] = (module, data[module['begin']:module['end']])
    output = []
    context = {'namespace': '', 'use_namespace': ''}
    for item in modules:
        module, data = fragments[item['id']]
        # Preserve transitions, but don't add namespace syntax to global-only files:
        # Tessent 2026.2 rejects even an unnecessary empty namespace directive.
        for key, keyword in [('namespace', 'NameSpace'), ('use_namespace', 'UseNameSpace')]:
            if module[key] != context[key]:
                output.append((keyword + ' ' + module[key] + ';').encode())
                context[key] = module[key]
        output.append(data)
    return b'\n'.join(output) + b'\n'


def catalog(root, cases):
    assigned = {}
    for case in cases:
        names = case.get('icl', []) + case['pdl']
        if case.get('bsdl'):
            names.append(case['bsdl'])
        if case.get('icl_origin'):
            names += ['tests/cases/' + m['path'] for m in origin_files(root, case['icl_origin'])]
        for name in names:
            assigned.setdefault(name, set()).add(case['id'])
    report = {}
    for path in sorted((root / 'tests').rglob('*')):
        if not path.is_file():
            continue
        name = path.relative_to(root).as_posix()
        if name not in assigned and path.suffix.lower() not in ('.icl', '.pdl', '.bsdl', '.bsd'):
            continue
        status = ('manifest_case' if name in assigned else
                  'generated_syntax_case' if name.startswith('tests/generated/') else
                  'syntax_negative' if '/fail/' in name else 'unpaired')
        report[name] = dict(status=status, cases=sorted(assigned.get(name, [])),
                            sha256=hashlib.sha256(path.read_bytes()).hexdigest())
    return report


def run_process(argv, cwd, console, timeout):
    start = time.monotonic()
    with console.open('xb') as stream:
        process = subprocess.Popen(argv, cwd=cwd, stdin=subprocess.DEVNULL,
                                   stdout=stream, stderr=subprocess.STDOUT,
                                   start_new_session=True)
        try:
            code = process.wait(timeout=timeout)
        except subprocess.TimeoutExpired:
            os.killpg(process.pid, signal.SIGKILL)
            process.wait()
            code = 124
    return code, time.monotonic() - start


def tessent_script(top, icl_names, pdl_names, bsdl_name=None, testbench=False):
    lines = ['# patterns -ijtag: setup for loading; analysis for PDL/export.',
             'set here [file dirname [info script]]', 'if {[catch {',
             '  set_context patterns -ijtag']
    for name in icl_names:
        command = '  read_icl [file join $here ' + tcl_word(name) + '] -force'
        if bsdl_name:
            command += ' -bsdl_file [file join $here ' + tcl_word(bsdl_name) + ']'
        lines.append(command)
    lines += ['  set_current_design ' + tcl_word(top),
              '  set_system_mode analysis',
              '  open_pattern_set oracle -no_initial_ireset']
    for name in pdl_names:
        lines.append('  dofile [file join $here ' + tcl_word(name) + ']')
    lines += ['  close_pattern_set',
              '  write_patterns [file join $here result.pdl] -pdl -pattern_sets oracle']
    if testbench:
        lines.append('  write_patterns result.v -verilog -pattern_sets oracle')
    lines += ['  puts "' + MARKER + '"', '} message]} {',
              '  puts stderr "ICL_TOOLSETS_ORACLE_ERROR: $message"',
              '  exit 1', '}', 'exit 0', '']
    return '\n'.join(lines)


def case_directory(bundle, report, case):
    # Markerless bundles retain their historical directory names.
    prefix = report.get('case_directory_prefix', '')
    if prefix not in ('', 'test_'):
        raise ValueError('unsupported case directory prefix')
    name = case['id']
    if not isinstance(name, str) or not re.fullmatch(r'[A-Za-z][A-Za-z0-9_-]*', name):
        raise ValueError(f'invalid case id: {name}')
    directory = bundle / (prefix + name)
    if directory.is_symlink():
        raise ValueError(f'{name}: invalid case directory')
    return directory


def prepare(root, cases, output, generator=None, reference_only=False, inventory=None):
    root, output = root.resolve(), output.resolve()
    validate(root, cases)
    # Never truncate an existing run or replace test inputs with generated artifacts.
    output.mkdir(parents=True, exist_ok=False)
    report = dict(mode='reference_only' if reference_only else 'comparison',
                  case_directory_prefix='test_', cases=[])
    inventory = inventory or root / 'build/icl_corpus_inventory'
    for case in cases:
        item = dict(case)
        report['cases'].append(item)
        if reference_only and case['requires_local_reset']:
            item['status'] = 'blocked_local_reset'
            continue
        if not reference_only and (generator is None or not generator.is_file()):
            item['status'] = 'blocked_generator'
            continue
        directory = case_directory(output, report, case)
        directory.mkdir()
        icl_names = []
        if case.get('icl_origin'):
            data = materialize_origin(root, case['icl_origin'], inventory)
            (directory / 'design.icl').write_bytes(data)
            icl_names.append('design.icl')
        else:
            for i, name in enumerate(case['icl']):
                dest = f'design_{i}.icl'
                (directory / dest).write_bytes(input_path(root, name).read_bytes())
                icl_names.append(dest)
        pdl_names = []
        for i, name in enumerate(case['pdl']):
            dest = f'source_{i}.pdl'
            (directory / dest).write_bytes(input_path(root, name).read_bytes())
            pdl_names.append(dest)
        bsdl_name = 'model.bsdl' if case.get('bsdl') else None
        bsdl_names = [bsdl_name] if bsdl_name else []
        if bsdl_name:
            (directory / bsdl_name).write_bytes(input_path(root, case['bsdl']).read_bytes())
        item['inputs'] = {name: hashlib.sha256((directory / name).read_bytes()).hexdigest()
                          for name in icl_names + pdl_names + bsdl_names}
        if not reference_only:
            command = [str(generator.resolve()), '--icl',
                       *[str(directory / n) for n in icl_names], '--pdl',
                       *[str(directory / n) for n in pdl_names],
                       '--top_module', case['top'], '--output', str(directory / 'ours.pdl'),
                       '--local_reset', str(directory / 'local_reset.pdl')]
            if bsdl_name:
                command += ['--bsdl', case['bsdl_entity'], str(directory / bsdl_name)]
            item['generator_command'] = command
            code, elapsed = run_process(command, directory, directory / 'generator.log', 120)
            item['generator_exit'], item['generator_seconds'] = code, elapsed
            if code or any(not (directory / name).is_file() or not (directory / name).stat().st_size
                           for name in ('ours.pdl', 'local_reset.pdl')):
                item['status'] = 'generator_failed'
                continue
            pdl_names = ['local_reset.pdl']
        item['tessent_pdl_inputs'] = pdl_names
        item['status'] = 'ready_reference_only' if reference_only else 'ready_comparison'
        (directory / 'run.tcl').write_text(tessent_script(case['top'], icl_names, pdl_names, bsdl_name))
        for name in ['run.tcl'] + pdl_names + ([] if reference_only else ['ours.pdl']):
            item['inputs'][name] = hashlib.sha256((directory / name).read_bytes()).hexdigest()
    (output / 'bundle.json').write_text(json.dumps(report, indent=2) + '\n')
    return report


def result_status(code, console, output):
    if code == 124:
        return 'timeout'
    if code or re.search(r'^//\s*Error:|^ICL_TOOLSETS_ORACLE_ERROR:', console, re.M):
        return 'tool_error'
    if MARKER not in console.splitlines():
        return 'incomplete'
    if not output.is_file() or not output.stat().st_size:
        return 'missing_output'
    return 'ok'


def testbench_hashes(directory):
    paths = sorted(directory.glob('result.v*'))
    main = directory / 'result.v'
    if not main.is_file() or not main.stat().st_size:
        raise ValueError('missing testbench result.v')
    result = {}
    for path in paths:
        if path.is_symlink() or not path.is_file():
            raise ValueError('invalid testbench artifact: ' + path.name)
        hasher = hashlib.sha256()
        with path.open('rb') as stream:
            for block in iter(lambda: stream.read(1024 * 1024), b''):
                hasher.update(block)
        result[path.name] = hasher.hexdigest()
    return result


def run_bundle(bundle, tessent, timeout):
    report = json.loads((bundle / 'bundle.json').read_text())
    if (bundle / 'results.json').exists():
        raise FileExistsError('results already exist; prepare a fresh bundle')
    seen = set()
    for case in report['cases']:
        name = case['id']
        if not re.fullmatch(r'[A-Za-z][A-Za-z0-9_-]*', name) or name in seen:
            raise ValueError(f'invalid or duplicate case id: {name}')
        seen.add(name)
        directory = case_directory(bundle, report, case)
        if not case['status'].startswith('ready_'):
            continue
        if not directory.is_dir():
            raise ValueError(f'{name}: invalid case directory')
        if any((directory / name).exists() or (directory / name).is_symlink()
               for name in ('result.pdl', 'console.log', 'tessent.log')):
            raise FileExistsError(f'{case["id"]}: stale run artifacts')
        if case.get('export_testbench') and any(directory.glob('result.v*')):
            raise FileExistsError(f'{case["id"]}: stale testbench artifacts')
        required = ['run.tcl'] + case['tessent_pdl_inputs']
        if case.get('bsdl'):
            required.append('model.bsdl')
        if any(name not in case['inputs'] for name in required):
            raise ValueError(f'{case["id"]}: missing script/input integrity hash')
        for name, digest in case['inputs'].items():
            if hashlib.sha256(input_path(directory, name).read_bytes()).hexdigest() != digest:
                raise ValueError(f'{case["id"]}: changed input {name}')
        command = [tessent, '-shell', '-dofile', 'run.tcl', '-logfile', 'tessent.log']
        code, elapsed = run_process(command, directory, directory / 'console.log', timeout)
        console = (directory / 'console.log').read_text(errors='replace')
        case.update(tessent_command=command, exit_code=code, seconds=elapsed,
                    outcome=result_status(code, console, directory / 'result.pdl'))
        if case['outcome'] == 'ok' and case.get('export_testbench'):
            try:
                case['testbench_hashes'] = testbench_hashes(directory)
            except ValueError as error:
                case.update(outcome='missing_testbench', testbench_error=str(error))
        if case['outcome'] == 'ok':
            case['result_sha256'] = hashlib.sha256((directory / 'result.pdl').read_bytes()).hexdigest()
        case['version'] = next((line.strip() for line in console.splitlines()
                                if 'Tessent Shell ' in line), 'not reported')
        case['expectation_met'] = (case['outcome'] == 'ok' if case['expected'] == 'ok' else
                                   case['outcome'] == 'tool_error' and
                                   case['diagnostic'] in console)
        case['status'] = 'reference_pass' if case['expectation_met'] else 'reference_fail'
        print(case['id'], case['status'], case['outcome'], flush=True)
    (bundle / 'results.json').write_text(json.dumps(report, indent=2) + '\n')
    return report


def compare(bundle, trace):
    report = json.loads((bundle / 'results.json').read_text())
    for case in report['cases']:
        directory = case_directory(bundle, report, case)
        if report['mode'] != 'comparison' or not (directory / 'ours.pdl').is_file():
            case['comparison'] = 'blocked_generator'
            continue
        if case.get('outcome') != 'ok' or case.get('status') != 'reference_pass':
            case['comparison'] = 'blocked_reference'
            continue
        hashes = dict(case['inputs'], **{'result.pdl': case['result_sha256']})
        if 'ours.pdl' not in hashes or 'local_reset.pdl' not in hashes:
            raise ValueError(f'{case["id"]}: missing comparison input hash')
        for name, digest in hashes.items():
            if hashlib.sha256(input_path(directory, name).read_bytes()).hexdigest() != digest:
                raise ValueError(f'{case["id"]}: changed comparison input {name}')
        outputs = []
        for name, entry in [('ours.pdl', ''), ('result.pdl', 'oracle')]:
            run = subprocess.run([str(trace), str(directory / name), entry],
                                 capture_output=True, text=True, timeout=30)
            if run.returncode:
                case['comparison'] = 'unsupported_trace'
                case['trace_error'] = run.stderr
                break
            outputs.append(json.loads(run.stdout))
        else:
            case['comparison'] = ('same_operations' if outputs[0] == outputs[1]
                                   else 'different_operations_requires_review')
            (directory / 'operation_comparison.json').write_text(json.dumps(outputs, indent=2) + '\n')
    (bundle / 'comparison.json').write_text(json.dumps(report, indent=2) + '\n')
    return report


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('action', choices=['inventory', 'prepare', 'run', 'compare'])
    parser.add_argument('--root', type=Path, default=ROOT)
    parser.add_argument('--manifest', type=Path, default=ROOT / 'tests/cases/ours/pattern_generation/oracle_cases.json')
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--generator', type=Path)
    parser.add_argument('--reference-only', action='store_true')
    parser.add_argument('--tessent', default='tessent')
    parser.add_argument('--trace', type=Path, default=ROOT / 'build/pdl_pattern_trace')
    parser.add_argument('--timeout', type=int, default=120)
    args = parser.parse_args()
    try:
        if args.action in ('prepare', 'inventory'):
            cases = json.loads(args.manifest.read_text())['cases']
            validate(args.root, cases)
        if args.action == 'inventory':
            report = catalog(args.root, cases)
            args.output.parent.mkdir(parents=True, exist_ok=True)
            with args.output.open('x') as stream:
                json.dump(report, stream, indent=2)
            print(dict(Counter(item['status'] for item in report.values())))
            return 0
        if args.action == 'prepare':
            report = prepare(args.root, cases, args.output, args.generator, args.reference_only)
        elif args.action == 'run':
            if args.timeout <= 0:
                raise ValueError('timeout must be positive')
            report = run_bundle(args.output.resolve(), args.tessent, args.timeout)
        else:
            report = compare(args.output.resolve(), args.trace.resolve())
        key = 'comparison' if args.action == 'compare' else 'status'
        counts = Counter(item.get(key, 'unrun') for item in report['cases'])
        print(dict(counts))
        accepted = {'ready_reference_only', 'ready_comparison', 'reference_pass', 'same_operations'}
        return int(any(name not in accepted for name in counts))
    except (OSError, ValueError, KeyError, subprocess.SubprocessError) as error:
        parser.exit(2, str(error) + '\n')


if __name__ == '__main__':
    sys.exit(main())
