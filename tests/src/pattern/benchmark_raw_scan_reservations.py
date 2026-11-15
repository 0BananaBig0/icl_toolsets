#!/usr/bin/env python3
"""Measure reserved/raw CSU costs and compare equivalent physical packets."""
import argparse
import hashlib
import json
from pathlib import Path
import platform
import subprocess

ROOT = Path(__file__).resolve().parents[3]


def scenarios():
    fixture = ROOT / 'tests/cases/ours/pattern_generation'
    model = (fixture / 'static_child_scans/model.icl').read_text()
    width = 1048568
    model = model.replace('head[2:0]', f'head[{width - 1}:0]')
    model = model.replace("ResetValue 3'b0", f"ResetValue {width}'b0")
    model = model.replace(' ScanRegister tail[4:0] { ScanInSource child.so; CaptureSource tail; ResetValue 5\'b0; }\n', '')
    model = model.replace('Source tail[0]', 'Source child.so')
    model = model.replace("DefaultLoadValue 16'b0", "DefaultLoadValue 1048576'b0")
    bits = '10' * (width // 2)
    initial = f'iScan client 1048576 -si 0b{bits}00000000; iApply\n'
    scans = 'iScan child.local 8 -si 0xc3 -stable; iApply\n'
    owner = f'iTake head; iState head 0b{bits}\n'
    yield 'wide', model, initial, owner, scans, 1048576, 2, width
    direct = f'iScan client 1048576 -si 0b{bits[::-1]}11000011; iApply\n'
    yield 'wide_explicit', model, initial, owner, direct, 1048576, 2, width
    yield 'partial', model, initial, f'iTake head[31:0]; iState head[31:0] 0b{bits[-32:]}\n', scans, 1048576, 2, 32
    model = (fixture / 'gated_sib_raw/model.icl').read_text()
    start = model.index(' Instance child Of RawSibLeaf')
    end = model.index(' ScanMux route', start)
    block = model[start:end]
    count = 1024
    instances = []
    for i in range(count):
        item = block.replace('Instance child', f'Instance r{i}')
        if i:
            item = item.replace('InputPort si = si;', f'InputPort si = r{i - 1}.so;')
        instances.append(item)
    model = model[:start] + ''.join(instances) + model[end:]
    model = model.replace("1'b1: child.so", f"1'b1: r{count - 1}.so")
    aliases = ' Alias a0 = open;\n' + ''.join(f' Alias a{i} = a{i - 1};\n' for i in range(1, 512))
    model = model.replace(' ScanMux route', aliases + ' ScanMux route')
    model = model.replace('InputPort sel = open;', 'InputPort sel = a511;')
    initial = 'iScan client 1 -si 1; iApply\n'
    initial += f'iScan client {8 * count + 1} -si 0b' + '00000001' * count + '1; iApply\n'
    owner = ''.join(f'iTake r{i}.value[0]; iState r{i}.value[0] 1\n' for i in range(count))
    scans = ''.join(f'iScan r{i}.local 8 -si 0xa4 -stable\n' for i in range(count)) + 'iApply\n'
    yield 'many', model, initial, owner, scans, 8193, 1025, count


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--binary', type=Path, required=True)
    parser.add_argument('--reference', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    binary, reference, out = args.binary.resolve(), args.reference.resolve(), args.output.resolve()
    out.mkdir(parents=True, exist_ok=False)
    report = {'platform': platform.platform(), 'scheduler_sha256': hashlib.sha256(
        (ROOT / 'src/pattern/pattern_scheduler.cc').read_bytes()).hexdigest(),
        'reference_binary_sha256': hashlib.sha256(reference.read_bytes()).hexdigest(), 'rows': []}
    for name, model, initial, owner, scans, active, registers, protected in scenarios():
        folder = out / name
        folder.mkdir()
        icl = folder / 'model.icl'
        icl.write_text(model)
        prefix = 'iPDLLevel 1\niProcsForModule Top\niReset\n' + initial
        (folder / 'plain.pdl').write_text(prefix + scans)
        (folder / 'source.pdl').write_text(prefix + owner + scans)
        expected = None
        for form in ['reference0', 'reference1', 'reference2', 'plain0', 'plain1', 'plain2',
                     'source0', 'source1', 'source2', 'flat', 'generated']:
            old = form.startswith('reference')
            pdl = folder / ('plain.pdl' if old or form.startswith('plain') else
                            'source0.pdl' if form == 'generated' else 'source.pdl')
            output = folder / (form + '.pdl')
            command = [str(reference if old else binary), '--icl',
                       str(folder / 'debug/Top.icl' if form == 'flat' else icl),
                       '--pdl', str(pdl), '--top_module', 'Top', '--output', str(output)]
            if form.startswith('source'):
                command.append('--debug')
            timing = folder / (form + '.time')
            with (folder / (form + '.log')).open('w') as log:
                result = subprocess.run(['/usr/bin/time', '-f', '%e %M', '-o', str(timing)] + command,
                                        stdout=log, stderr=subprocess.STDOUT, timeout=120)
            values = timing.read_text().split()
            row = dict(scenario=name, active_bits=active, registers=registers, protected_bits=protected,
                       form=form, command=command, exit=result.returncode,
                       seconds=float(values[-2]), max_rss_kib=int(values[-1]))
            if result.returncode == 0:
                data = output.read_bytes()
                # iState is controlling metadata; compare every physical packet and Apply.
                physical = b'\n'.join(line for line in data.splitlines() if not line.startswith(b'iState '))
                row['artifact_sha256'] = hashlib.sha256(data).hexdigest()
                row['physical_sha256'] = hashlib.sha256(physical).hexdigest()
                if expected is None:
                    expected = row['physical_sha256']
                if row['physical_sha256'] != expected:
                    raise AssertionError((name, form, 'physical packet mismatch'))
            report['rows'].append(row)
            (out / 'results.json').write_text(json.dumps(report, indent=2) + '\n')
            print(json.dumps(row), flush=True)
            if result.returncode:
                raise RuntimeError((name, form, 'generator failure'))


if __name__ == '__main__':
    main()
