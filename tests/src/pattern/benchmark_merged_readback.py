#!/usr/bin/env python3
"""Measure actual merge observations against matched complete serial controls."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import platform
import subprocess
import tempfile
import time

ROOT = Path(__file__).resolve().parents[3]


def fixture(kind, calls, width, merge, observed=True, owner=None):
    if kind == 'raw':
        model = (ROOT/'tests/cases/ours/pattern_generation/modeled_root_scan/model.icl').read_text()
        model = model.replace('Module Modeled', 'Module Chains').replace('[7:0]', f'[{width-1}:0]')
        model = model.replace("ResetValue 8'b0", f"ResetValue {width}'b0")
        model = model.replace("DefaultLoadValue 8'b00000000", f"DefaultLoadValue {width}'b0")
        command, target, initialize = f'iScan client {width} -si $word -so 0', 'client', 'iReset'
    else:
        model = f'Module Chains {{DataInPort p[{width-1}:0]; DataOutPort q[{width-1}:0] {{Source p;}}}}'
        command, target, initialize = 'iWrite p $word; iRead q 0', 'q', 'iState p 0'
    text = 'iPDLLevel 1\niProcsForModule Chains\nset ::seen {}\n'
    text += 'iProc access {word expected} {'+command+'; iApply; '
    if observed:
        text += ('set actual [iGetReadData '+target+' -bin]; '
                 'if {$actual ne $expected} {error actual_capture}; lappend ::seen $actual; ')
    text += '}\n'+initialize+'\n'+('iMerge -begin\n' if merge else '')
    expected, previous = [], 0
    for index in range(calls):
        value = (int('10'*((width+1)//2), 2) >> (index % 2)) & ((1 << width)-1)
        word = lambda n: '0b'+format(n, f'0{width}b')
        actual = word(previous if kind == 'raw' else value)
        text += f'iCall access {word(value)} {actual}\n'
        expected.append(actual)
        previous = value
    text += ('iMerge -end\n' if merge else '')+'foreach value $::seen {iNote $value}\n'
    return model, text, expected if observed else []


def main(make_fixture=fixture, wide_calls=1, applies_per_call=1):
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('binary', 'parent', 'output'):
        parser.add_argument('--'+name, type=Path, required=True)
    parser.add_argument('--sanitized', type=Path)
    parser.add_argument('--sizes', type=int, nargs='+', default=[16, 64, 128])
    parser.add_argument('--wide-widths', type=int, nargs='*', default=[512, 1024])
    parser.add_argument('--repeats', type=int, default=3)
    args = parser.parse_args()
    if args.repeats < 1 or min(args.sizes) < 1 or any(n < 4 for n in args.wide_widths):
        parser.error('positive sizes/repeats and widths>=4 required')
    args.output = args.output.resolve()
    args.output.mkdir(parents=True, exist_ok=True)
    binaries = {'parent': args.parent.resolve(), 'current': args.binary.resolve()}
    if args.sanitized:
        binaries['sanitized'] = args.sanitized.resolve()
    digest = lambda path: hashlib.sha256(path.read_bytes()).hexdigest()
    report = {'environment': platform.platform(), 'binaries': {
        key: {'path': str(path), 'sha256': digest(path)} for key, path in binaries.items()}, 'runs': []}
    failures = []

    def run(owner, mode, kind, calls, width, repeat, form, model, pdl, notes, reference=None):
        folder = Path(tempfile.mkdtemp(prefix=f'{owner}-{mode}-{kind}-{calls}-{width}-{form}-', dir=args.output))
        output, metric = folder/'result.pdl', folder/'time.txt'
        command = [str(binaries[owner]), '--icl', str(model), '--pdl', str(pdl),
            '--top_module', 'Chains', '--output', str(output), '--debug', '--model_readback']
        env = os.environ.copy()
        env.update(ASAN_OPTIONS='detect_leaks=1', UBSAN_OPTIONS='halt_on_error=1')
        started, error, code = time.monotonic(), '', None
        with (folder/'stdout.log').open('w') as stdout, (folder/'stderr.log').open('w') as stderr:
            try:
                code = subprocess.run(['/usr/bin/time', '-f', '%e %M', '-o', str(metric),
                    'timeout', '--kill-after=2s', '60s', *command], env=env,
                    stdout=stdout, stderr=stderr, timeout=65).returncode
            except subprocess.TimeoutExpired:
                error = 'outer timeout'
        seconds = time.monotonic()-started
        measured = metric.read_text().splitlines()[-1].split() if metric.exists() else []
        rss = int(measured[1]) if len(measured) == 2 and measured[1].isdigit() else None
        data = output.read_bytes() if output.exists() else b''
        actual_notes = [line[6:].decode() for line in data.splitlines()
            if line.startswith(b'iNote ') and b'modeled ' not in line]
        raw_count = sum(line.startswith(b'iScan ') for line in data.splitlines())
        if (code != 0 or actual_notes != notes or
                data.count(b'iApply\n') != calls*applies_per_call or
                (kind == 'raw' and raw_count != calls) or
                (reference is not None and data != reference)):
            error = error or 'process, complete observation list or exact replay differs'
        memory_bound = (256 if width > 4 else 512)*1024
        within = seconds <= 10 and rss is not None and rss <= memory_bound
        row = dict(owner=owner, mode=mode, kind=kind, calls=calls, width=width, repeat=repeat,
            form=form, command=command, directory=str(folder), exit=code, seconds=seconds,
            peak_rss_kib=rss, time_bound_seconds=10, memory_bound_kib=memory_bound,
            within_bounds=within, observations_verified=not error, error=error,
            provider_queries=0 if form == 'generated' or mode == 'unobserved' else calls,
            literal_notes=len(notes), apply_groups=data.count(b'iApply\n'),
            raw_scan_operations=raw_count, shifted_bits=raw_count*width,
            output_sha256=hashlib.sha256(data).hexdigest())
        report['runs'].append(row)
        (args.output/'results.json').write_text(json.dumps(report, indent=2)+'\n')
        if error or not within:
            failures.append(row)
        return data, output, folder/'debug/Chains.icl'

    cases = [(kind, size, 4) for kind in ('pio', 'raw') for size in args.sizes]
    cases += [(kind, max(2,wide_calls) if kind=='raw' else wide_calls, width)
              for kind in ('pio', 'raw') for width in args.wide_widths]
    references = {}
    for kind, calls, width in cases:
        for owner, mode in [('parent', 'serial'), ('current', 'serial'),
                            ('current', 'merged'), ('current', 'unobserved')]:
            folder = Path(tempfile.mkdtemp(prefix='fixture-', dir=args.output))
            source, pdl = folder/'source.icl', folder/'source.pdl'
            model, text, notes = make_fixture(kind, calls, width, mode != 'serial',
                                              mode != 'unobserved', owner=owner)
            source.write_text(model)
            pdl.write_text(text)
            for repeat in range(args.repeats):
                key = (kind, calls, width, repeat)
                reference = references.get(key)
                if reference is not None and mode == 'unobserved':
                    reference = b''.join(line for line in reference.splitlines(keepends=True)
                        if not line.startswith(b'iNote ') or b'modeled ' in line)
                data, generated, flat = run(owner, mode, kind, calls, width, repeat, 'source', source, pdl, notes, reference)
                if owner == 'parent':
                    references[key] = data
                expected_output = reference if reference is not None else data
                run(owner, mode, kind, calls, width, repeat, 'flat', flat, pdl, notes, expected_output)
                run(owner, mode, kind, calls, width, repeat, 'generated', source, generated, notes, expected_output)
    if 'sanitized' in binaries:
        for kind in ('pio', 'raw'):
            folder = Path(tempfile.mkdtemp(prefix='san-fixture-', dir=args.output))
            source, pdl = folder/'source.icl', folder/'source.pdl'
            model, text, notes = make_fixture(kind, 16, 4, True, owner='sanitized')
            source.write_text(model)
            pdl.write_text(text)
            for repeat in range(args.repeats):
                key = (kind, 16, 4, repeat)
                if key not in references:
                    control = Path(tempfile.mkdtemp(prefix='san-parent-', dir=args.output))
                    parent_model, parent_text, parent_notes = make_fixture(kind, 16, 4, False, owner='parent')
                    parent_source, parent_pdl = control/'source.icl', control/'source.pdl'
                    parent_source.write_text(parent_model); parent_pdl.write_text(parent_text)
                    references[key], _, _ = run('parent', 'serial', kind, 16, 4, repeat, 'source', parent_source, parent_pdl, parent_notes)
                reference = references[key]
                data, generated, flat = run('sanitized', 'merged', kind, 16, 4, repeat, 'source', source, pdl, notes, reference)
                run('sanitized', 'merged', kind, 16, 4, repeat, 'flat', flat, pdl, notes, reference)
                run('sanitized', 'merged', kind, 16, 4, repeat, 'generated', source, generated, notes, reference)
    extra_controls = 2 if args.sanitized and 16 not in args.sizes else 0
    expected_runs = (len(cases)*12+(6 if args.sanitized else 0)+extra_controls)*args.repeats
    complete = len(report['runs']) == expected_runs
    print(json.dumps({'runs': len(report['runs']), 'expected_runs': expected_runs,
                      'matrix_complete': complete, 'failed_or_out_of_bounds': len(failures)}))
    return bool(failures) or not complete


if __name__ == '__main__':
    raise SystemExit(main())
