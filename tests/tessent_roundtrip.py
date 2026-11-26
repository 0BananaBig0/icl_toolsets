#!/usr/bin/env python3
"""Certify either ordered-apply round trip; replay stored evidence without Tessent."""
import argparse
import copy
import hashlib
import json
import re
from pathlib import Path
import subprocess
import tempfile

import tessent_golden as golden
import tessent_oracle as oracle

ROOT = golden.ROOT
POLICY = 'resolved-bits-ordered-applies-v1'
SOURCE_POLICY = 'reference-vendor-equivalence-ordered-applies-v1'
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


def verify_execution(job, directory, suffix=''):
    icl = (['design.icl'] if job.get('icl_origin') else
           [f'design_{i}.icl' for i in range(len(job['icl']))])
    expected = oracle.tessent_script(job['top'], icl, job['tessent_pdl_inputs'],
                                    'model.bsdl' if job.get('bsdl') else None,
                                    job.get('export_testbench', False))
    if oracle.input_path(directory, 'run.tcl' + suffix).read_text() != expected:
        raise ValueError('changed certified vendor execution inputs')


def verify_reference(reference, case, job, vendor):
    try:
        prior, artifacts = reference['job'], reference['artifacts']
        current_pdl, prior_pdl = job['tessent_pdl_inputs'], prior['tessent_pdl_inputs']
        valid = (reference['schema'] == 3 and reference['policy'] == POLICY
                 and reference['direction'] == 'forward' and reference['case'] == case
                 and reference['source_hashes'] == job['source_hashes'] == prior['source_hashes']
                 and prior['top'] == case['top'] and prior['status'] == 'reference_pass'
                 and prior['outcome'] == 'ok' and prior['exit_code'] == 0
                 and 'Tessent Shell ' in prior['version']
                 and set(artifacts) == {'flat.pdl', 'vendor.pdl'}
                 and prior['result_sha256'] == artifacts['vendor.pdl']
                 and len(current_pdl) == len(prior_pdl) == 1
                 and job['inputs'][current_pdl[0]] == artifacts['flat.pdl']
                 and prior['inputs'][prior_pdl[0]] == artifacts['flat.pdl']
                 and reference['trace'] == vendor and vendor['binding'] == case['top']
                 and all(re.fullmatch(r'[0-9a-f]{64}', value) for value in (
                     reference['binary_sha256'], reference['vendor_console_sha256'],
                     artifacts['flat.pdl'], artifacts['vendor.pdl'])))
    except (KeyError, TypeError):
        valid = False
    if not valid:
        raise ValueError('changed previously certified reference input/provenance/trace')


def record(root, bundle, case_id, direction, output, trace, generator, source_bundle=None):
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
    source_job = source_directory = source_reference = None
    if source_bundle is not None:
        if direction != 'forward':
            raise ValueError('source-vendor equivalence requires forward direction')
        source_prepared = json.loads((source_bundle / 'bundle.json').read_text())
        source_report = json.loads((source_bundle / 'results.json').read_text())
        if (source_prepared.get('golden_cases') != source_report.get('golden_cases')
                or case not in source_prepared.get('golden_cases', [])):
            raise ValueError('changed source case definitions')
        source_job, source_directory = golden.verified_job(
            root, source_bundle, source_report, source_prepared, case, case_id)
        verify_execution(job, directory)
        verify_execution(source_job, source_directory)
        source_reference = source_prepared.get('reference_certificates', {}).get(case_id)
        if source_reference is not None:
            if source_report.get('reference_certificates') != source_prepared['reference_certificates']:
                raise ValueError('changed previously certified reference input/trace')
            verify_reference(source_reference, case, source_job, vendor)
        if (source_job['top'] != job['top']
                or (source_reference is None and source_job['tessent_pdl_inputs'] != [
                    f'source_{i}.pdl' for i in range(len(case['pdl']))])):
            raise ValueError('source requires original context and PDL sequence')
        if grouped(trace, source_directory, source_job,
                   source_directory / 'result.pdl', 'oracle') != vendor:
            raise ValueError('source and generated vendor apply groups differ')
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
        native_trace = grouped(trace, work, job, generated, '')
        if source_job is None and native_trace != vendor:
            raise ValueError(f'{direction} round-trip apply groups differ')
        artifact = generated.read_bytes()
    certificate = dict(schema=3, policy=POLICY, direction=direction, case=case,
                       source_hashes=golden.source_hashes(root, case), job=job,
                       trace=vendor, binary_sha256=golden.digest(generator),
                       vendor_console_sha256=golden.digest(directory / 'console.log'),
                       artifacts={'flat.pdl': hashlib.sha256(artifact).hexdigest(),
                                  'vendor.pdl': job['result_sha256']})
    if source_job is not None:
        certificate.update(schema=4, policy=SOURCE_POLICY, source_job=source_job,
                           native_trace=native_trace,
                           source_vendor_console_sha256=golden.digest(source_directory / 'console.log'))
        certificate['artifacts']['source_vendor.pdl'] = source_job['result_sha256']
        if source_reference is not None:
            certificate['source_reference'] = source_reference
    output.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='.roundtrip-', dir=output.parent) as temp:
        staged = Path(temp) / 'snapshot'
        (staged / 'inputs').mkdir(parents=True)
        for name in job['inputs']:
            (staged / 'inputs' / (name + '.expected')).write_bytes(
                oracle.input_path(directory, name).read_bytes())
        (staged / 'flat.pdl.expected').write_bytes(artifact)
        (staged / 'vendor.pdl.expected').write_bytes((directory / 'result.pdl').read_bytes())
        if source_job is not None:
            (staged / 'source_inputs').mkdir()
            for name in source_job['inputs']:
                (staged / 'source_inputs' / (name + '.expected')).write_bytes(
                    oracle.input_path(source_directory, name).read_bytes())
            (staged / 'source_vendor.pdl.expected').write_bytes(
                (source_directory / 'result.pdl').read_bytes())
        (staged / 'snapshot.json').write_text(json.dumps(certificate, indent=2) + '\n')
        if output.exists() or output.is_symlink():
            raise FileExistsError(f'snapshot already exists: {output}')
        staged.rename(output)
    return certificate


def check(root, snapshot, generator, trace):
    cert = json.loads((snapshot / 'snapshot.json').read_text())
    source_equivalence = (cert.get('schema') == 4 and cert.get('policy') == SOURCE_POLICY
                          and cert.get('direction') == 'forward')
    if not (source_equivalence or (cert.get('schema') == 3 and cert.get('policy') == POLICY
                                  and cert.get('direction') in DIRECTIONS)):
        raise ValueError('unsupported round-trip certificate')
    case, job = cert['case'], cert['job']
    oracle.validate(root, [case])
    if case['top'] != job['top'] or cert['trace']['binding'] != case['top']:
        raise ValueError('changed certified context/binding')
    if golden.source_hashes(root, case) != cert['source_hashes']:
        raise ValueError('changed source: explicit vendor recertification required')
    artifacts = {'flat.pdl', 'vendor.pdl'}
    if source_equivalence:
        artifacts.add('source_vendor.pdl')
        source_job = cert['source_job']
        source_reference = cert.get('source_reference')
        if source_reference is not None:
            verify_reference(source_reference, case, source_job, cert['trace'])
        if (source_job['top'] != case['top']
                or source_job['source_hashes'] != cert['source_hashes']
                or source_job['result_sha256'] != cert['artifacts']['source_vendor.pdl']
                or (source_reference is None and source_job['tessent_pdl_inputs'] != [
                    f'source_{i}.pdl' for i in range(len(case['pdl']))])):
            raise ValueError('changed certified source context/input')
        golden.verify(snapshot / 'source_inputs', {
            n + '.expected': h for n, h in source_job['inputs'].items()})
    if (set(cert['artifacts']) != artifacts
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
    if source_equivalence:
        verify_execution(job, snapshot / 'inputs', '.expected')
        verify_execution(source_job, snapshot / 'source_inputs', '.expected')
    with tempfile.TemporaryDirectory(prefix='icl-roundtrip-check-') as temp:
        work = Path(temp)
        for name in job['inputs']:
            (work / name).write_bytes(oracle.input_path(snapshot / 'inputs', name + '.expected').read_bytes())
        (work / 'vendor.pdl').write_bytes((snapshot / 'vendor.pdl.expected').read_bytes())
        if grouped(trace, work, job, work / 'vendor.pdl', 'oracle') != cert['trace']:
            raise ValueError('changed certified vendor trace')
        if source_equivalence:
            source_work = work / 'source'
            source_work.mkdir()
            for name in source_job['inputs']:
                (source_work / name).write_bytes(oracle.input_path(
                    snapshot / 'source_inputs', name + '.expected').read_bytes())
            (source_work / 'vendor.pdl').write_bytes(
                (snapshot / 'source_vendor.pdl.expected').read_bytes())
            if grouped(trace, source_work, source_job,
                       source_work / 'vendor.pdl', 'oracle') != cert['trace']:
                raise ValueError('source and generated vendor apply groups differ')
        generated = regenerate(work, job, case, cert['direction'], generator)
        if golden.digest(generated) != cert['artifacts']['flat.pdl']:
            raise ValueError('changed golden output')
        if grouped(trace, work, job, generated, '') != cert[
                'native_trace' if source_equivalence else 'trace']:
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
    parser.add_argument('--source-bundle', type=Path,
                        help='fresh source or previously certified program export for forward equivalence')
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
            record(args.root, args.bundle, args.case, args.direction, args.output, trace, generator,
                   source_bundle=args.source_bundle)
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
