#!/usr/bin/env python3
"""Measure raw IR against the existing ordinary IR planner with equal flags."""
import argparse
import hashlib
import json
from pathlib import Path
import platform
import re
import subprocess

ROOT = Path(__file__).resolve().parents[3]
FIXTURE = ROOT / 'tests/cases/ours/pattern_generation/raw_tap_ir'


def scenarios():
    for width, registers, repeats in [(4, 1, 1), (32, 1, 1), (256, 1, 1),
                                      (4096, 1, 1), (4, 1024, 1), (4, 1, 128)]:
        model = (FIXTURE / 'model.icl').read_text()
        extra = ''.join(f'  ScanRegister unused{i} {{ ScanInSource si; }}\n'
                        for i in range(registers - 1))
        model = model.replace('  ScanMux mux', extra + '  ScanMux mux')
        bsdl = (FIXTURE / 'model.bsdl').read_text()
        bsdl = bsdl.replace('entity is 4;', f'entity is {width};')
        bsdl = re.sub(r'\(([01]+)\)', lambda m: '(' +
                      ('1' * width if set(m[1]) == {'1'} else m[1].zfill(width)) + ')', bsdl)
        bsdl = bsdl.replace('entity is "0001";', 'entity is "' + '1'.zfill(width) + '";')
        prefix = 'iPDLLevel 1\niProcsForModule RawIrTop\niReset\n'
        raw = f'iScan -ir jtag {width} -si 3 -so 1; iApply\n'
        ordinary = 'iWrite tap.INSTRUCTION 3; iRead tap.INSTRUCTION 1; iApply\n'
        yield f'w{width}-nodes{registers}-repeat{repeats}', model, bsdl, prefix + raw * repeats, prefix + ordinary * repeats


def canonical(data):
    # Controlled fixture output: expand one raw instruction to its equivalent
    # whole-register assignments, retaining every boundary and observer bit.
    return re.sub(rb'iScan -ir jtag [0-9]+ -si (0b[01]+) -so (0b[01]+)\n',
                  rb'iWrite tap.INSTRUCTION \1\niRead tap.INSTRUCTION \2\n', data)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--binary', type=Path, required=True)
    parser.add_argument('--reference', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    binary, reference, out = args.binary.resolve(), args.reference.resolve(), args.output.resolve()
    out.mkdir(parents=True, exist_ok=False)
    digest = lambda data: hashlib.sha256(data).hexdigest()
    report = dict(platform=platform.platform(), binary_sha256=digest(binary.read_bytes()),
                  reference_sha256=digest(reference.read_bytes()), rows=[])
    for name, model, bsdl, source, ordinary in scenarios():
        folder = out / name
        folder.mkdir()
        for filename, data in [('model.icl', model), ('model.bsdl', bsdl),
                               ('source.pdl', source), ('ordinary.pdl', ordinary)]:
            (folder / filename).write_text(data)
        expected = raw_bytes = None
        forms = ['reference0', 'source0'] if name.startswith('w4096-') else [
            'reference0', 'reference1', 'source0', 'source1', 'flat', 'generated']
        for form in forms:
            old = form.startswith('reference')
            command = [str(reference if old else binary), '--icl',
                       str(folder / ('debug/RawIrTop.icl' if form == 'flat' else 'model.icl'))]
            if form != 'flat':
                command += ['--bsdl', 'RawIrProbe', str(folder / 'model.bsdl')]
            pdl = 'ordinary.pdl' if old else 'source0.pdl' if form == 'generated' else 'source.pdl'
            output = folder / (form + '.pdl')
            command += ['--pdl', str(folder / pdl), '--output', str(output)]
            if old or form.startswith('source'):
                command.append('--debug')
            timing = folder / (form + '.time')
            with (folder / (form + '.log')).open('w') as log:
                result = subprocess.run(['/usr/bin/time', '-f', '%e %M', '-o', str(timing)] + command,
                                        cwd=folder, stdout=log, stderr=subprocess.STDOUT, timeout=120)
            values = timing.read_text().split()
            row = dict(scenario=name, form=form, command=command, exit=result.returncode,
                       seconds=float(values[-2]), max_rss_kib=int(values[-1]))
            report['rows'].append(row)
            (out / 'results.json').write_text(json.dumps(report, indent=2) + '\n')
            if name.startswith('w4096-'):
                assert result.returncode == 2, row
                assert 'scan event resource limit' in (folder / (form + '.log')).read_text()
                assert not output.exists()
                row['expected_rejection'] = 'inherited scan event resource limit'
                (out / 'results.json').write_text(json.dumps(report, indent=2) + '\n')
                continue
            assert result.returncode == 0, row
            data = output.read_bytes()
            row['artifact_sha256'] = digest(data)
            row['canonical_sha256'] = digest(canonical(data))
            expected = expected or canonical(data)
            assert canonical(data) == expected, (name, form, 'ordinary/raw mismatch')
            if not old:
                raw_bytes = raw_bytes or data
                assert data == raw_bytes, (name, form, 'raw replay mismatch')
            (out / 'results.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(report, indent=2))


if __name__ == '__main__':
    main()
