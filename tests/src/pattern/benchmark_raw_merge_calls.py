#!/usr/bin/env python3
"""Measure merged raw calls against identical complete serial controls."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import platform
import subprocess
import tempfile
import time

from benchmark_downstream_write_callbacks import digest

ROOT = Path(__file__).resolve().parents[3]


def fixture(calls, mode, width=4):
    model = (ROOT/'tests/cases/ours/pattern_generation/modeled_root_scan/model.icl').read_text()
    model = model.replace('Module Modeled', 'Module Chains').replace('[7:0]', '[%d:0]' % (width-1))
    model = model.replace("ResetValue 8'b0", "ResetValue %d'b0" % width)
    model = model.replace("DefaultLoadValue 8'b00000000", "DefaultLoadValue %d'b0" % width)
    text = ('iPDLLevel 1\niProcsForModule Chains\nset ::calls 0\n'
            'iProc access {before first second} {\n incr ::calls\n'
            ' iScan client %d -si $first -so $before; iApply\n'
            ' iScan client %d -si $second -so $first; iApply\n}\niReset\n' % (width, width))
    if mode == 'merged':
        text += 'iMerge -begin\n'
    expected, previous = [], 0
    for index in range(calls):
        first, second = (index*3+1) % 16, (index*5+2) % 16
        word = lambda value: '0b'+format(value, '0%db' % width)
        text += 'iCall access %s %s %s\n' % (word(previous), word(first), word(second))
        expected.extend([(word(first), word(previous)), (word(second), word(first))])
        previous = second
    if mode == 'merged':
        text += 'iMerge -end\n'
    text += 'if {$::calls != %d} {error call_count}\n' % calls
    return model, text, expected


def checked_frames(text, expected, width):
    frames, pending = [], []
    for line in text.splitlines():
        words = line.split()
        if not words:
            continue
        if words[0] == 'iScan':
            if (len(words) not in (7, 8) or words[1:4] != ['client', str(width), '-si'] or
                    words[5] != '-so' or (len(words) == 8 and words[7] != '-stable')):
                raise RuntimeError('unexpected raw frame: '+line)
            for value in (words[4], words[6]):
                if len(value) != width+2 or not value.startswith('0b') or set(value[2:])-set('01'):
                    raise RuntimeError('invalid raw payload: '+line)
            pending.append((words[4], words[6]))
        elif words[0] == 'iApply':
            if words != ['iApply'] or len(pending) != 1:
                raise RuntimeError('raw Apply boundary differs')
            frames.extend(pending)
            pending = []
        elif words[0] not in ('iPDLLevel', 'iProcsForModule', 'iReset'):
            raise RuntimeError('unexpected physical command: '+line)
    if pending or frames != expected:
        raise RuntimeError('complete ordered input/compare frames differ')
    return dict(csu_count=len(frames), scan_operations=len(frames),
                shifted_bits=len(frames)*width, apply_groups=len(frames),
                exact_frames_verified=True,
                physical_frames_sha256=hashlib.sha256(json.dumps(frames).encode()).hexdigest())


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('binary', 'parent', 'output'):
        parser.add_argument('--'+name, type=Path, required=True)
    parser.add_argument('--sanitized', type=Path)
    parser.add_argument('--sizes', type=int, nargs='+', default=[16, 64, 256])
    parser.add_argument('--wide-widths', type=int, nargs='*', default=[512, 1024])
    parser.add_argument('--repeats', type=int, default=3)
    args = parser.parse_args()
    if args.repeats < 1 or min(args.sizes) < 1 or any(width < 4 for width in args.wide_widths):
        parser.error('positive sizes/repeats and widths>=4 required')
    args.output = args.output.resolve()
    args.output.mkdir(parents=True, exist_ok=True)
    binaries = dict(parent=args.parent.resolve(), current=args.binary.resolve())
    if args.sanitized:
        binaries['sanitized'] = args.sanitized.resolve()
    report = dict(environment=platform.platform(), binaries={
        owner: dict(path=str(path), sha256=digest(path)) for owner, path in binaries.items()}, runs=[])
    failures = []

    def run(owner, mode, calls, width, repeat, form, icl, pdl, expected, reference=None):
        folder = Path(tempfile.mkdtemp(prefix='%s-%s-%d-%d-%s-' %
                                      (owner, mode, calls, width, form), dir=args.output))
        output, metric = folder/'result.pdl', folder/'time.txt'
        command = [str(binaries[owner]), '--icl', str(icl), '--pdl', str(pdl),
                   '--top_module', 'Chains', '--output', str(output), '--debug']
        measured = ['/usr/bin/time', '-f', '%e %M', '-o', str(metric),
                    'timeout', '--kill-after=2s', '60s', *command]
        env = os.environ.copy()
        if owner == 'sanitized':
            env.update(ASAN_OPTIONS='detect_leaks=1', UBSAN_OPTIONS='halt_on_error=1')
        started = time.monotonic()
        process = subprocess.run(measured, capture_output=True, text=True, env=env, timeout=65)
        elapsed = time.monotonic()-started
        (folder/'process.log').write_text(process.stdout+process.stderr)
        if process.returncode:
            raise RuntimeError(str(folder)+'\n'+process.stdout+process.stderr)
        data = output.read_bytes()
        if reference is not None and data != reference:
            raise RuntimeError('complete matched serial output differs: '+str(folder))
        dimensions = checked_frames(data.decode(), expected, width)
        seconds, rss = metric.read_text().split()
        cap = (256 if width > 4 else 512)*1024
        within_bound = elapsed <= 10 and int(rss) <= cap
        row = dict(owner=owner, mode=mode, calls=calls, width=width, repeat=repeat,
                   form=form, wall_seconds=elapsed, time_seconds=float(seconds), rss_kib=int(rss),
                   within_bound=within_bound, command=command, measurement_command=measured,
                   environment={key: env[key] for key in ('ASAN_OPTIONS', 'UBSAN_OPTIONS') if key in env},
                   icl_sha256=digest(icl), pdl_sha256=digest(pdl), output_sha256=digest(output),
                   original_calls=calls, executed_calls=0 if form == 'generated' else calls,
                   input_merge_blocks=int(mode == 'merged' and form != 'generated'),
                   original_merge_blocks=int(mode == 'merged'), scan_storage_bits=width,
                   matched_serial_output_verified=reference is not None, **dimensions)
        report['runs'].append(row)
        (args.output/'results.json').write_text(json.dumps(report, indent=2)+'\n')
        print('%s %s %d/%d %s %.4fs %dKiB' %
              (owner, mode, calls, width, form, elapsed, int(rss)), flush=True)
        if not within_bound:
            failures.append(str(folder))
        return data, output

    for calls, width in [(size, 4) for size in args.sizes]+[(1, width) for width in args.wide_widths]:
        folder = Path(tempfile.mkdtemp(prefix='fixture-', dir=args.output))
        icl, merged, serial = folder/'source.icl', folder/'merged.pdl', folder/'serial.pdl'
        model, commands, expected = fixture(calls, 'merged', width)
        icl.write_text(model)
        merged.write_text(commands)
        serial.write_text(fixture(calls, 'serial', width)[1])
        for repeat in range(args.repeats):
            reference, _ = run('parent', 'serial', calls, width, repeat, 'source', icl, serial, expected)
            run('current', 'serial', calls, width, repeat, 'source', icl, serial, expected, reference)
            _, generated = run('current', 'merged', calls, width, repeat, 'source', icl, merged, expected, reference)
            run('current', 'merged', calls, width, repeat, 'flat',
                generated.parent/'debug/Chains.icl', merged, expected, reference)
            run('current', 'merged', calls, width, repeat, 'generated', icl, generated, expected, reference)
            if args.sanitized and calls == min(args.sizes) and width == 4:
                _, generated = run('sanitized', 'merged', calls, width, repeat, 'source', icl, merged, expected, reference)
                run('sanitized', 'merged', calls, width, repeat, 'flat',
                    generated.parent/'debug/Chains.icl', merged, expected, reference)
                run('sanitized', 'merged', calls, width, repeat, 'generated', icl, generated, expected, reference)
    print(json.dumps(dict(runs=len(report['runs']), matched_outputs=True, bound_failures=failures)))
    if failures:
        raise RuntimeError('timing/memory bounds failed: '+', '.join(failures))


if __name__ == '__main__':
    main()
