#!/usr/bin/env python3
"""Compare equivalent connected callback runs and complete replay outputs."""
import argparse
import json
import os
from pathlib import Path
import platform
import subprocess
import tempfile
import time

import benchmark_downstream_write_callbacks as base
from benchmark_merged_callbacks import checked_frames


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('binary', 'parent', 'output'):
        parser.add_argument('--'+name, type=Path, required=True)
    parser.add_argument('--sanitized', type=Path)
    parser.add_argument('--sizes', type=int, nargs='+', default=[64, 128, 256])
    parser.add_argument('--wide-widths', type=int, nargs='*', default=[512, 1024])
    parser.add_argument('--repeats', type=int, default=3)
    parser.add_argument('--source-only', action='store_true')
    args = parser.parse_args()
    if args.repeats < 1 or min(args.sizes) < 1 or any(w < 4 for w in args.wide_widths):
        parser.error('positive sizes/repeats and widths >=4 required')
    args.output = args.output.resolve()
    args.output.mkdir(parents=True, exist_ok=True)
    binaries = {'parent': args.parent.resolve(), 'current': args.binary.resolve()}
    if args.sanitized:
        binaries['sanitized'] = args.sanitized.resolve()
    report = {'environment': platform.platform(), 'binaries': {
        owner: {'path': str(path), 'sha256': base.digest(path)}
        for owner, path in binaries.items()}, 'runs': []}
    bound_failures = []

    def run(owner, targets, width, repeat, form, icl, pdl, reference=None):
        folder = Path(tempfile.mkdtemp(prefix='%s-%d-%d-%s-' %
                                      (owner, targets, width, form), dir=args.output))
        output, metric = folder/'result.pdl', folder/'time.txt'
        command = [str(binaries[owner]), '--icl', str(icl), '--pdl', str(pdl),
                   '--top_module', 'Chains', '--output', str(output), '--debug']
        env = os.environ.copy()
        if owner == 'sanitized':
            env.update(ASAN_OPTIONS='detect_leaks=1', UBSAN_OPTIONS='halt_on_error=1')
        started = time.monotonic()
        process = subprocess.run(['/usr/bin/time', '-f', '%e %M', '-o', str(metric),
                                  *command], capture_output=True, text=True, env=env, timeout=60)
        elapsed = time.monotonic()-started
        (folder/'process.log').write_text(process.stdout+process.stderr)
        if process.returncode:
            raise RuntimeError(str(folder)+'\n'+process.stdout+process.stderr)
        data = output.read_bytes()
        if reference is not None and data != reference:
            raise RuntimeError('complete ordered output differs from matched parent: '+str(folder))
        dimensions, frames = checked_frames(data.decode(), targets, width, True)
        seconds, rss = metric.read_text().split()
        cap = (256 if width > 4 else 512)*1024
        within_bound = elapsed <= 10 and int(rss) <= cap
        row = {'owner': owner, 'targets': targets, 'width': width, 'repeat': repeat,
               'form': form, 'wall_seconds': elapsed, 'time_seconds': float(seconds),
               'rss_kib': int(rss), 'within_bound': within_bound, 'command': command,
               'environment': {key: env[key] for key in ('ASAN_OPTIONS', 'UBSAN_OPTIONS') if key in env},
               'icl_sha256': base.digest(icl), 'pdl_sha256': base.digest(pdl),
               'output_sha256': base.digest(output), 'actual_callback_invocations':
               0 if form == 'generated' else 4*targets,
               'generated_notes_are_literal': form == 'generated',
               'connected_callback_bits': targets*width,
               'connected_destination_bits': targets*width,
               'primary_input_bits': 5, 'probe_groups': 4,
               'full_parent_output_verified': reference is not None, **dimensions}
        report['runs'].append(row)
        (args.output/'results.json').write_text(json.dumps(report, indent=2)+'\n')
        print('%s %d/%d %s %.4fs %dKiB' %
              (owner, targets, width, form, elapsed, int(rss)), flush=True)
        if owner != 'parent' and not within_bound:
            bound_failures.append(str(folder))
        return data, output

    cases = ([(size, 4, False) for size in args.sizes] +
             [(1, width, False) for width in args.wide_widths])
    if args.sanitized:
        cases.append((min(16, min(args.sizes)), 4, True))
    for targets, width, sanitized in cases:
        fixture_dir = Path(tempfile.mkdtemp(prefix='fixture-', dir=args.output))
        icl, pdl = fixture_dir/'source.icl', fixture_dir/'source.pdl'
        model, commands = base.fixture(targets, 'automatic', width)
        icl.write_text(model)
        pdl.write_text(commands)
        for repeat in range(args.repeats):
            reference, _ = run('parent', targets, width, repeat, 'source', icl, pdl)
            owner = 'sanitized' if sanitized else 'current'
            _, generated = run(owner, targets, width, repeat, 'source', icl, pdl, reference)
            if not args.source_only:
                run(owner, targets, width, repeat, 'flat',
                    generated.parent/'debug/Chains.icl', pdl, reference)
                run(owner, targets, width, repeat, 'generated', icl, generated, reference)
    print(json.dumps({'runs': len(report['runs']), 'matched_outputs': True,
                      'bound_failures': bound_failures}))
    if bound_failures:
        raise RuntimeError('timing/memory bounds failed: '+', '.join(bound_failures))


if __name__ == '__main__':
    main()
