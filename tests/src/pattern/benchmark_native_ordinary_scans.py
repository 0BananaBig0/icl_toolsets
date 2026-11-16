#!/usr/bin/env python3
"""Shared large ordinary-scan fixtures and native-path measurements."""
import argparse
import hashlib
import json
from pathlib import Path
import platform
import subprocess

ROOT = Path(__file__).resolve().parents[3]


def many_model(count=1024):
    modules = (ROOT / 'tests/cases/ours/pattern_generation/dynamic_child_raw/model.icl').read_text().split('Module Top ', 1)[0]
    ports = 'ScanInPort si; SelectPort sel; CaptureEnPort ce; ShiftEnPort se; UpdateEnPort ue; TCKPort tck; ResetPort reset;\n'
    aliases = 'Alias a0 = sel;\n' + ''.join(f'Alias a{i} = a{i - 1};\n' for i in range(1, 512))
    instances = []
    for i in range(count):
        si = 'si' if i == 0 else f'r{i - 1}.so'
        instances.append(f'Instance r{i} Of DynamicRawBranch {{InputPort si = {si}; InputPort sel = a511; '
                         'InputPort ce = ce; InputPort se = se; InputPort ue = ue; InputPort tck = tck; InputPort reset = reset;}\n')
    model = modules + 'Module Top {\n' + ports + aliases + ''.join(instances)
    model += f'ScanOutPort so {{Source r{count - 1}.so;}}\n'
    model += 'ScanInterface client {Port si; Port so; Port sel; Port ce; Port se; Port ue; Port tck; Port reset; '
    model += f"DefaultLoadValue {count}'b0;}}\n}}\n"
    return model


def scenarios():
    fixtures = ROOT / 'tests/cases/ours/pattern_generation'
    sib = (fixtures / 'gated_sib_raw/model.icl').read_text()
    prefix = 'iPDLLevel 1\niProcsForModule Top\niReset\n'
    for width in [16384, 1048576]:
        model = sib.replace('[7:0]', f'[{width - 2}:0]')
        model = model.replace("ResetValue 8'b0", f"ResetValue {width - 1}'b0")
        raw = f'iScan client {width} -si 1; iApply\n'
        source = prefix + 'iWrite open 1; iRead open 0; ' + raw
        reference = source if width == 16384 else prefix + 'iScan client 1 -si 1 -so 0; iApply\n' + raw
        yield f'wide{width}', model, source, reference, width, 1, width == 16384
    count = 1024
    raw = f'iScan client {9 * count} -si 0b' + '101001011' * count + '; iApply\n'
    prepare = ''.join(f'iWrite r{i}.enable 1; ' for i in range(count)) + 'iRead r0.enable 0; '
    reference = prefix + f'iScan client {count} -si 0b' + '1' * count + ' -so 0b0' + 'x' * (count - 1) + '; iApply\n' + raw
    yield 'distinct1024', many_model(count), prefix + prepare + raw, reference, 9 * count, count, False
    # Same ordinary program on both binaries, including repeated close/reopen.
    body = 'iWrite open 1; iRead open 0; iScan client 9 -si 0b101001010; iApply\n'
    yield 'repeat128', sib, prefix + body * 128, prefix + body * 128, 9, 128, True
    model = (fixtures / 'static_child_scans/model.icl').read_text()
    width = 1048568
    model = model.replace('head[2:0]', f'head[{width - 1}:0]')
    model = model.replace("ResetValue 3'b0", f"ResetValue {width}'b0")
    model = model.replace(" ScanRegister tail[4:0] { ScanInSource child.so; CaptureSource tail; ResetValue 5'b0; }\n", '')
    model = model.replace('Source tail[0]', 'Source child.so').replace("DefaultLoadValue 16'b0", "DefaultLoadValue 1048576'b0")
    owner = 'iTake head[31:0]; iState head[31:0] 0\n'
    # Final raw probe exposes all million retained/output bits independently of
    # the different logical spelling of the ordinary/manual preparation.
    probe = 'iScan client 1048576 -si 0 -so 0xc3; iApply\n'
    source = prefix + owner + 'iWrite child.value 0xc3; iRead child.value 0; iApply\n' + probe
    reference = prefix + owner + 'iScan client 1048576 -si 0xc3 -so 0; iApply\n' + probe
    yield 'partial32', model, source, reference, 1048576, 1, False


def main(scenario_source=scenarios):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--binary', type=Path, required=True)
    parser.add_argument('--reference', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    binary, reference, out = args.binary.resolve(), args.reference.resolve(), args.output.resolve()
    out.mkdir(parents=True, exist_ok=False)
    digest = lambda data: hashlib.sha256(data).hexdigest()
    report = dict(platform=platform.platform(), scheduler_sha256=digest(
        (ROOT / 'src/pattern/pattern_scheduler.cc').read_bytes()),
        binary_sha256=digest(binary.read_bytes()), reference_binary_sha256=digest(reference.read_bytes()), rows=[])
    for name, model, source, manual, active, setups, equivalent in scenario_source():
        folder = out / name
        folder.mkdir()
        (folder / 'model.icl').write_text(model)
        (folder / 'source.pdl').write_text(source)
        (folder / 'reference.pdl').write_text(manual)
        reference_artifact = candidate_artifact = probe = None
        for form in ['reference0', 'reference1', 'reference2', 'source0', 'source1', 'source2', 'flat', 'generated']:
            old = form.startswith('reference')
            pdl = folder / ('reference.pdl' if old else 'source0.pdl' if form == 'generated' else 'source.pdl')
            output = folder / (form + '.pdl')
            command = [str(reference if old else binary), '--icl',
                       str(folder / 'debug/Top.icl' if form == 'flat' else folder / 'model.icl'),
                       '--pdl', str(pdl), '--top_module', 'Top', '--output', str(output)]
            if old or form.startswith('source'):
                command.append('--debug')
            timing = folder / (form + '.time')
            with (folder / (form + '.log')).open('w') as log:
                result = subprocess.run(['/usr/bin/time', '-f', '%e %M', '-o', str(timing)] + command,
                                        stdout=log, stderr=subprocess.STDOUT, timeout=120)
            values = timing.read_text().split()
            row = dict(scenario=name, active_bits=active, ordinary_setups=setups, form=form,
                       reference_kind='same ordinary program' if equivalent else 'manually authored raw preparation',
                       command=command, exit=result.returncode, seconds=float(values[-2]), max_rss_kib=int(values[-1]))
            if result.returncode == 0:
                data = output.read_bytes()
                row['artifact_sha256'] = digest(data)
                packet = [line for line in data.splitlines() if line.startswith(b'iScan ')][-1]
                row['final_raw_packet_sha256'] = digest(packet)
                if old:
                    reference_artifact = reference_artifact or row['artifact_sha256']
                    assert row['artifact_sha256'] == reference_artifact, (name, form, 'reference repeat mismatch')
                else:
                    candidate_artifact = candidate_artifact or row['artifact_sha256']
                    assert row['artifact_sha256'] == candidate_artifact, (name, form, 'source/flat/generated mismatch')
                    if equivalent:
                        assert row['artifact_sha256'] == reference_artifact, (name, form, 'ordinary reference mismatch')
                probe = probe or row['final_raw_packet_sha256']
                assert row['final_raw_packet_sha256'] == probe, (name, form, 'final raw packet mismatch')
            report['rows'].append(row)
            (out / 'results.json').write_text(json.dumps(report, indent=2) + '\n')
            print(json.dumps(row), flush=True)
            if result.returncode:
                raise RuntimeError((name, form, 'generator failure'))


if __name__ == '__main__':
    main()
