#!/usr/bin/env python3
"""Certify either ordered-apply round trip; replay stored evidence without Tessent."""
import argparse
import copy
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile

import tessent_golden as golden
import tessent_oracle as oracle

ROOT = golden.ROOT
POLICY = 'resolved-bits-ordered-applies-v1'
DIRECTIONS = ('forward', 'reverse')


def prepare(root, cases, output, generator, direction):
    if direction not in DIRECTIONS:
        raise ValueError('invalid round-trip direction')
    report = oracle.prepare(root, cases, output, generator,
                            reference_only=direction == 'reverse',
                            flat_pattern=direction == 'forward')
    report.update(roundtrip_direction=direction, golden_cases=copy.deepcopy(cases))
    for case, job in zip(cases, report['cases']):
        job['source_hashes'] = golden.source_hashes(root, case)
        if job['status'].startswith('ready_'):
            job['status'] = 'ready_reference_only'
    (output / 'bundle.json').write_text(json.dumps(report, indent=2) + '\n')
    return report


def grouped(trace, directory, job, path, entry):
    args = [str(trace.resolve()), str(path.resolve()), entry,
            '--apply-groups', job['top']]
    for name in job['inputs']:
        if name.endswith('.icl'):
            args += ['--icl', str(oracle.input_path(directory, name))]
    if job.get('bsdl'):
        args += ['--bsdl', job['bsdl_entity'], str(directory / 'model.bsdl')]
    result = subprocess.run(args, capture_output=True, timeout=120)
    if result.returncode:
        raise ValueError('cannot compare static apply groups: ' + result.stderr.decode(errors='replace'))
    value = json.loads(result.stdout)
    if not value['operations']:
        raise ValueError('empty trace cannot qualify a round trip')
    return value


def regenerate(directory, job, case, direction, generator):
    candidate = copy.deepcopy(job)
    if direction == 'reverse':
        (directory / 'invoke.pdl').write_text('iCall oracle\n')
        candidate['tessent_pdl_inputs'] = ['vendor.pdl', 'invoke.pdl']
    else:
        candidate['tessent_pdl_inputs'] = [f'source_{i}.pdl' for i in range(len(case['pdl']))]
    golden.candidate(directory, candidate, 'icl-pattern-generation', generator)
    return directory / 'flat.pdl'


def record(root, bundle, case_id, direction, output, trace, generator):
    if direction not in DIRECTIONS:
        raise ValueError('invalid round-trip direction')
    if output.exists() or output.is_symlink():
        raise FileExistsError(f'snapshot already exists: {output}')
    prepared = json.loads((bundle / 'bundle.json').read_text())
    report = json.loads((bundle / 'results.json').read_text())
    if report.get('golden_cases') != prepared.get('golden_cases'):
        raise ValueError('changed source case definitions')
    if report.get('roundtrip_direction') != prepared.get('roundtrip_direction'):
        raise ValueError('changed prepared direction')
    if prepared.get('roundtrip_direction'):
        if direction != prepared['roundtrip_direction']:
            raise ValueError('bundle direction differs')
        job_id = case_id
    elif prepared.get('mode') == 'vendor_goldens':
        job_id = case_id + ('-icl-pattern-generation' if direction == 'forward' else '')
    else:
        raise ValueError('not a round-trip or golden bundle')
    cases = [c for c in prepared['golden_cases'] if c['id'] == case_id]
    if len(cases) != 1:
        raise ValueError('missing or ambiguous source case')
    case = cases[0]
    oracle.validate(root, [case])
    job, directory = golden.verified_job(root, bundle, report, prepared, case, job_id)
    if direction == 'reverse' and job['tessent_pdl_inputs'] != [
            f'source_{i}.pdl' for i in range(len(case['pdl']))]:
        raise ValueError('reverse requires the original source PDL sequence')
    vendor = grouped(trace, directory, job, directory / 'result.pdl', 'oracle')
    with tempfile.TemporaryDirectory(prefix='icl-roundtrip-') as temp:
        work = Path(temp)
        for name in job['inputs']:
            (work / name).write_bytes(oracle.input_path(directory, name).read_bytes())
        (work / 'vendor.pdl').write_bytes((directory / 'result.pdl').read_bytes())
        # Save forward B before candidate regeneration can overwrite flat.pdl.
        before = None
        if direction == 'forward':
            names = job['tessent_pdl_inputs']
            if len(names) != 1:
                raise ValueError('forward requires exactly one flat vendor input')
            before = oracle.input_path(directory, names[0]).read_bytes()
        generated = regenerate(work, job, case, direction, generator)
        if before is not None and generated.read_bytes() != before:
            raise ValueError('changed generator output: fresh vendor evidence required')
        if grouped(trace, work, job, generated, '') != vendor:
            raise ValueError(f'{direction} round-trip apply groups differ')
        artifact = generated.read_bytes()
    certificate = dict(schema=3, policy=POLICY, direction=direction, case=case,
                       source_hashes=golden.source_hashes(root, case), job=job,
                       trace=vendor, binary_sha256=golden.digest(generator),
                       vendor_console_sha256=golden.digest(directory / 'console.log'),
                       artifacts={'flat.pdl': hashlib.sha256(artifact).hexdigest(),
                                  'vendor.pdl': job['result_sha256']})
    output.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='.roundtrip-', dir=output.parent) as temp:
        staged = Path(temp) / 'snapshot'
        (staged / 'inputs').mkdir(parents=True)
        for name in job['inputs']:
            (staged / 'inputs' / (name + '.expected')).write_bytes(
                oracle.input_path(directory, name).read_bytes())
        (staged / 'flat.pdl.expected').write_bytes(artifact)
        (staged / 'vendor.pdl.expected').write_bytes((directory / 'result.pdl').read_bytes())
        (staged / 'snapshot.json').write_text(json.dumps(certificate, indent=2) + '\n')
        if output.exists() or output.is_symlink():
            raise FileExistsError(f'snapshot already exists: {output}')
        staged.rename(output)
    return certificate


def check(root, snapshot, generator, trace):
    cert = json.loads((snapshot / 'snapshot.json').read_text())
    if (cert.get('schema') != 3 or cert.get('policy') != POLICY
            or cert.get('direction') not in DIRECTIONS):
        raise ValueError('unsupported round-trip certificate')
    case, job = cert['case'], cert['job']
    oracle.validate(root, [case])
    if case['top'] != job['top'] or cert['trace']['binding'] != case['top']:
        raise ValueError('changed certified context/binding')
    if golden.source_hashes(root, case) != cert['source_hashes']:
        raise ValueError('changed source: explicit vendor recertification required')
    if (set(cert['artifacts']) != {'flat.pdl', 'vendor.pdl'}
            or cert['artifacts']['vendor.pdl'] != job['result_sha256']
            or cert['source_hashes'] != job['source_hashes']):
        raise ValueError('changed certified hashes/context')
    pdl_inputs = job['tessent_pdl_inputs']
    if cert['direction'] == 'forward':
        if len(pdl_inputs) != 1 or job['inputs'].get(pdl_inputs[0]) != cert['artifacts']['flat.pdl']:
            raise ValueError('changed certified forward input')
    elif pdl_inputs != [f'source_{i}.pdl' for i in range(len(case['pdl']))]:
        raise ValueError('changed certified reverse input')
    golden.verify(snapshot, {n + '.expected': h for n, h in cert['artifacts'].items()})
    golden.verify(snapshot / 'inputs', {n + '.expected': h for n, h in job['inputs'].items()})
    with tempfile.TemporaryDirectory(prefix='icl-roundtrip-check-') as temp:
        work = Path(temp)
        for name in job['inputs']:
            (work / name).write_bytes(oracle.input_path(snapshot / 'inputs', name + '.expected').read_bytes())
        (work / 'vendor.pdl').write_bytes((snapshot / 'vendor.pdl.expected').read_bytes())
        if grouped(trace, work, job, work / 'vendor.pdl', 'oracle') != cert['trace']:
            raise ValueError('changed certified vendor trace')
        generated = regenerate(work, job, case, cert['direction'], generator)
        if golden.digest(generated) != cert['artifacts']['flat.pdl']:
            raise ValueError('changed golden output')
        if grouped(trace, work, job, generated, '') != cert['trace']:
            raise ValueError('round-trip apply groups differ')
        # A stable directional result must also be stable under local replay.
        (work / 'generator.log').unlink()
        (work / 'replay.pdl').write_bytes(generated.read_bytes())
        replay = dict(job, tessent_pdl_inputs=['replay.pdl'])
        golden.candidate(work, replay, 'icl-pattern-generation', generator)
        if golden.digest(generated) != cert['artifacts']['flat.pdl']:
            raise ValueError('round-trip local idempotency failure')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('action', choices=('prepare', 'record', 'check'))
    parser.add_argument('--root', type=Path, default=ROOT)
    parser.add_argument('--build', type=Path, default=ROOT / 'build')
    parser.add_argument('--manifest', type=Path, default=ROOT / 'tests/cases/ours/tessent_golden/cases.json')
    parser.add_argument('--direction', choices=DIRECTIONS)
    parser.add_argument('--bundle', type=Path)
    parser.add_argument('--output', type=Path)
    parser.add_argument('--case')
    parser.add_argument('--goldens', type=Path, default=ROOT / 'tests/golden/tessent_roundtrip')
    args = parser.parse_args()
    generator, trace = args.build / 'icl-pattern-generation', args.build / 'pdl_pattern_trace'
    try:
        if args.action == 'prepare':
            if not args.output or not args.direction:
                raise ValueError('prepare requires --output and --direction')
            cases = json.loads(args.manifest.read_text())['cases']
            report = prepare(args.root, cases, args.output, generator, args.direction)
            failed = [j['id'] for j in report['cases'] if not j['status'].startswith('ready_')]
            print(f'prepared {len(report["cases"])} {args.direction} jobs; blocked: {failed}')
            return int(bool(failed))
        if args.action == 'record':
            if not all((args.bundle, args.case, args.direction, args.output)):
                raise ValueError('record requires --bundle --case --direction --output')
            record(args.root, args.bundle, args.case, args.direction, args.output, trace, generator)
            print(f'certified {args.case}/{args.direction}: {args.output}')
        else:
            snapshots = sorted(args.goldens.glob('*/snapshot.json'))
            if not snapshots:
                raise ValueError('no certified round trips')
            configured = {c['id']: c for c in json.loads(args.manifest.read_text())['cases']}
            for path in snapshots:
                case = json.loads(path.read_text())['case']
                if configured.get(case['id']) != case:
                    raise ValueError('changed manifest case')
                check(args.root, path.parent, generator, trace)
            print(f'{len(snapshots)} directional round-trip goldens passed (offline)')
        return 0
    except (OSError, ValueError, KeyError, subprocess.SubprocessError) as error:
        parser.exit(2, str(error) + '\n')


if __name__ == '__main__':
    raise SystemExit(main())
