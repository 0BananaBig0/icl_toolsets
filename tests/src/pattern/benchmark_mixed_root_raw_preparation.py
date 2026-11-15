#!/usr/bin/env python3
"""Compare mixed root preparation with explicit source Apply boundaries."""
import argparse
import hashlib
import json
from pathlib import Path
import platform
import subprocess

ROOT = Path(__file__).resolve().parents[3]


def scenarios():
    fixtures = ROOT / 'tests/cases/ours/pattern_generation'
    model = (fixtures / 'gated_sib_raw/model.icl').read_text()
    prefix = 'iPDLLevel 1\niProcsForModule Top\niReset\n'
    # The inherited ordinary solver supports this measured width. Larger
    # rejected/timed-out workloads are retained in the qualification evidence.
    active = 16384
    wide = model.replace('[7:0]', f'[{active - 2}:0]').replace("ResetValue 8'b0", f"ResetValue {active - 1}'b0")
    payload = '10' * ((active - 2) // 2) + '11'
    prepare = 'iWrite open 1\n'
    raw = f'iScan client {active} -si 0b' + payload + '\niApply\n'
    yield 'wide', wide, prefix + prepare + raw, prefix + prepare + 'iApply\n' + raw, active, 1
    raw = 'iScan client 9 -si 0b101001010\niApply\n'
    yield 'repeat128', model, prefix + (prepare + raw) * 128, prefix + (prepare + 'iApply\n' + raw) * 128, 9, 128
    count = 1024
    prepare = 'iWrite open 1\n' * count
    raw = 'iScan client 9 -si 0b101001011\niApply\n'
    yield 'queued1024', model, prefix + prepare + raw, prefix + prepare + 'iApply\n' + raw, 9, count


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--binary', type=Path, required=True)
    parser.add_argument('--reference', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    binary, reference, out = args.binary.resolve(), args.reference.resolve(), args.output.resolve()
    out.mkdir(parents=True, exist_ok=False)
    report = dict(platform=platform.platform(), scheduler_sha256=hashlib.sha256(
        (ROOT / 'src/pattern/pattern_scheduler.cc').read_bytes()).hexdigest(),
        reference_binary_sha256=hashlib.sha256(reference.read_bytes()).hexdigest(), rows=[])
    for name, model, mixed, explicit, active, setups in scenarios():
        folder = out / name
        folder.mkdir()
        (folder / 'model.icl').write_text(model)
        (folder / 'source.pdl').write_text(mixed)
        (folder / 'explicit.pdl').write_text(explicit)
        expected = None
        for form in ['reference0', 'reference1', 'reference2', 'explicit0', 'explicit1', 'explicit2',
                     'source0', 'source1', 'source2', 'flat', 'generated']:
            old = form.startswith('reference')
            pdl = folder / ('explicit.pdl' if old or form.startswith('explicit') else
                            'source0.pdl' if form == 'generated' else 'source.pdl')
            output = folder / (form + '.pdl')
            command = [str(reference if old else binary), '--icl',
                       str(folder / 'debug/Top.icl' if form == 'flat' else folder / 'model.icl'),
                       '--pdl', str(pdl), '--top_module', 'Top', '--output', str(output)]
            if form.startswith('source'):
                command.append('--debug')
            timing = folder / (form + '.time')
            with (folder / (form + '.log')).open('w') as log:
                result = subprocess.run(['/usr/bin/time', '-f', '%e %M', '-o', str(timing)] + command,
                                        stdout=log, stderr=subprocess.STDOUT, timeout=120)
            values = timing.read_text().split()
            row = dict(scenario=name, active_bits=active, ordinary_setups=setups, form=form,
                       command=command, exit=result.returncode, seconds=float(values[-2]), max_rss_kib=int(values[-1]))
            if result.returncode == 0:
                row['sha256'] = hashlib.sha256(output.read_bytes()).hexdigest()
                if expected is None:
                    expected = row['sha256']
                if row['sha256'] != expected:
                    raise AssertionError((name, form, 'complete generated pattern mismatch'))
            report['rows'].append(row)
            (out / 'results.json').write_text(json.dumps(report, indent=2) + '\n')
            print(json.dumps(row), flush=True)
            if result.returncode:
                raise RuntimeError((name, form, 'generator failure'))


if __name__ == '__main__':
    main()
