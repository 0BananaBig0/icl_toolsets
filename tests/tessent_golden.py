#!/usr/bin/env python3
"""Explicit Tessent certification; ordinary golden checks are local and immutable."""
import argparse
import copy
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile

import tessent_oracle as oracle

ROOT = Path(__file__).resolve().parents[1]
TOOLS = ('icl-pattern-generation', 'icl-format', 'pdl-format')


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def source_hashes(root, case):
    names = case.get('icl', []) + case['pdl']
    if case.get('icl_origin'):
        names += ['tests/cases/eda_sources.json'] + [
            'tests/cases/' + m['path'] for m in oracle.origin_files(root, case['icl_origin'])]
    if case.get('bsdl'):
        names.append(case['bsdl'])
    return {name: digest(oracle.input_path(root, name)) for name in names}


def verify(root, hashes):
    if not hashes:
        raise ValueError('missing integrity hashes')
    for name, expected in hashes.items():
        if digest(oracle.input_path(root, name)) != expected:
            raise ValueError(f'changed input/output hash: {name}')


def candidate(directory, case, tool, binary):
    icl = [name for name in case['inputs'] if name.endswith('.icl')]
    pdl = case['tessent_pdl_inputs']
    if tool == 'icl-pattern-generation':
        command = [str(binary.resolve()), '--icl', *icl, '--pdl', *pdl,
                   '--top_module', case['top'], '--output', 'flat.pdl']
        if case.get('bsdl'):
            command += ['--bsdl', case['bsdl_entity'], 'model.bsdl']
        code, _ = oracle.run_process(command, directory, directory / 'generator.log', 120)
        if code:
            raise ValueError(f'generator failed ({code}): {directory / "generator.log"}')
        artifacts, pdl = ['flat.pdl'], ['flat.pdl']
    else:
        artifacts = icl if tool == 'icl-format' else pdl
        for name in artifacts:
            run = subprocess.run([str(binary.resolve()), name], cwd=directory,
                                 capture_output=True, timeout=120, check=True)
            (directory / name).write_bytes(run.stdout)
    return artifacts, icl, pdl


def prepare(root, cases, output, binaries, selected=TOOLS):
    root, output = root.resolve(), output.resolve()
    if any(tool not in TOOLS for tool in selected):
        raise ValueError('unknown golden tool')
    if any(case['expected'] != 'ok' or case['requires_local_reset'] for case in cases):
        raise ValueError('golden cases need successful, explicitly reset-safe source PDL')
    report = oracle.prepare(root, cases, output, reference_only=True)
    report['mode'] = 'vendor_goldens'
    report['golden_cases'] = copy.deepcopy(cases)
    originals = list(report['cases'])
    ids = {case['id'] for case in originals}
    for source, original in zip(cases, originals):
        original['source_hashes'] = source_hashes(root, source)
        folder = oracle.case_directory(output, report, original)
        for tool in selected:
            job = copy.deepcopy(original)
            job['id'] += '-' + tool
            if job['id'] in ids:
                raise ValueError('colliding golden job id')
            ids.add(job['id'])
            job['golden_tool'] = tool
            job['original_id'] = original['id']
            report['cases'].append(job)
            destination = oracle.case_directory(output, report, job)
            destination.mkdir()
            for name in original['inputs']:
                if name != 'run.tcl':
                    (destination / name).write_bytes(oracle.input_path(folder, name).read_bytes())
            try:
                artifacts, icl, pdl = candidate(destination, original, tool, binaries[tool])
                job['artifacts'] = artifacts
                job['binary_sha256'] = digest(binaries[tool])
                job['tessent_pdl_inputs'] = pdl
                (destination / 'run.tcl').write_text(oracle.tessent_script(
                    job['top'], icl, pdl, 'model.bsdl' if job.get('bsdl') else None))
                names = list(dict.fromkeys(list(original['inputs']) + artifacts))
                job['inputs'] = {name: digest(destination / name) for name in names}
                job['status'] = 'ready_reference_only'
            except (OSError, ValueError, subprocess.SubprocessError) as error:
                job.update(status='candidate_failed', error=str(error))
    (output / 'bundle.json').write_text(json.dumps(report, indent=2) + '\n')
    return report


def trace_file(binary, path, entry):
    run = subprocess.run([str(binary.resolve()), str(path.resolve()), entry],
                         capture_output=True, check=True, timeout=30)
    trace = json.loads(run.stdout)
    if not trace.get('operations'):
        raise ValueError('empty vendor trace cannot certify a semantic test')
    return trace


def record(root, bundle, case_id, tool, output, trace):
    if output.exists() or output.is_symlink():
        raise FileExistsError(f'snapshot already exists: {output}')
    report = json.loads((bundle / 'results.json').read_text())
    prepared = json.loads((bundle / 'bundle.json').read_text())
    if report.get('mode') != 'vendor_goldens' or tool not in TOOLS:
        raise ValueError('not a golden certification bundle/tool')
    if report['golden_cases'] != prepared['golden_cases']:
        raise ValueError('changed source case definitions')
    cases = [case for case in report['golden_cases'] if case['id'] == case_id]
    if len(cases) != 1:
        raise ValueError('missing or ambiguous source case')
    case = cases[0]
    oracle.validate(root, [case])
    jobs = []
    for job_id in (case_id, case_id + '-' + tool):
        matches = [job for job in report['cases'] if job['id'] == job_id]
        originals = [job for job in prepared['cases'] if job['id'] == job_id]
        if len(matches) != 1 or len(originals) != 1:
            raise ValueError('missing or ambiguous vendor job')
        job, original = matches[0], originals[0]
        if (job.get('status') != 'reference_pass' or job.get('outcome') != 'ok'
                or job.get('exit_code') != 0 or 'Tessent Shell ' not in job.get('version', '')):
            raise ValueError('vendor job did not successfully export a pattern')
        for key, value in original.items():
            if key != 'status' and job.get(key) != value:
                raise ValueError(f'changed prepared {key}')
        if job['source_hashes'] != source_hashes(root, case):
            raise ValueError('changed source hashes: regenerate vendor evidence')
        directory = oracle.case_directory(bundle, report, job)
        verify(directory, dict(job['inputs'], **{'result.pdl': job['result_sha256']}))
        console = oracle.input_path(directory, 'console.log').read_text(errors='replace')
        observed_version = next((line.strip() for line in console.splitlines()
                                 if 'Tessent Shell ' in line), 'not reported')
        if (oracle.result_status(job['exit_code'], console, directory / 'result.pdl') != 'ok'
                or observed_version != job['version']):
            raise ValueError('vendor console does not confirm successful export/version')
        jobs.append((job, directory))
    baseline, variant = jobs
    before = trace_file(trace, baseline[1] / 'result.pdl', 'oracle')
    after = trace_file(trace, variant[1] / 'result.pdl', 'oracle')
    if before != after or before['binding'] != case['top']:
        raise ValueError('original and candidate vendor patterns differ')
    if tool == 'icl-pattern-generation':
        if trace_file(trace, variant[1] / 'flat.pdl', '') != after:
            raise ValueError('flat pattern is not a vendor fixed point: operations differ')
    artifacts = {name: variant[0]['inputs'][name] for name in variant[0]['artifacts']}
    certificate = dict(schema=1, case=case, tool=tool, source_hashes=source_hashes(root, case),
                       artifacts=artifacts, trace=after,
                       binary_sha256=variant[0]['binary_sha256'],
                       vendor_version=[job['version'] for job, _ in jobs],
                       vendor_console_sha256=[digest(directory / 'console.log') for _, directory in jobs],
                       vendor_inputs=[job['inputs'] for job, _ in jobs],
                       vendor_outputs={'original_vendor.pdl': baseline[0]['result_sha256'],
                                       'vendor.pdl': variant[0]['result_sha256']})
    output.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='.golden-', dir=output.parent) as temp:
        staged = Path(temp) / 'snapshot'
        staged.mkdir()
        for name in artifacts:
            (staged / (name + '.expected')).write_bytes(oracle.input_path(variant[1], name).read_bytes())
        for name, (_, directory) in zip(('original_vendor.pdl', 'vendor.pdl'), jobs):
            (staged / (name + '.expected')).write_bytes((directory / 'result.pdl').read_bytes())
        (staged / 'snapshot.json').write_text(json.dumps(certificate, indent=2) + '\n')
        if output.exists() or output.is_symlink():
            raise FileExistsError(f'snapshot already exists: {output}')
        staged.rename(output)
    return certificate


def check(root, snapshot, binaries):
    certificate = json.loads((snapshot / 'snapshot.json').read_text())
    if certificate.get('schema') != 1 or certificate.get('tool') not in TOOLS:
        raise ValueError('unsupported golden schema/tool')
    case, tool = certificate['case'], certificate['tool']
    if certificate['trace']['binding'] != case['top']:
        raise ValueError('changed certified module binding/context')
    if source_hashes(root, case) != certificate['source_hashes']:
        raise ValueError('changed source: explicit vendor recertification required')
    for key in ('artifacts', 'vendor_outputs'):
        verify(snapshot, {name + '.expected': sha for name, sha in certificate[key].items()})
    with tempfile.TemporaryDirectory(prefix='icl-toolsets-golden-') as temp:
        bundle = Path(temp) / 'bundle'
        report = prepare(root, [case], bundle, binaries, (tool,))
        job = report['cases'][-1]
        if not job['status'].startswith('ready_'):
            raise ValueError(job.get('error', 'candidate failed'))
        directory = oracle.case_directory(bundle, report, job)
        if [item['inputs'] for item in report['cases']] != certificate['vendor_inputs']:
            raise ValueError('changed certified vendor input/context')
        actual = {name: digest(directory / name) for name in job['artifacts']}
        if actual != certificate['artifacts']:
            raise ValueError(f'changed golden output: {case["id"]}/{tool}')
        # Reuse the same source binding; test F(F(source)) == F(source).
        if tool == 'icl-pattern-generation':
            (directory / 'generator.log').unlink()
            (directory / 'source_replay.pdl').write_bytes((directory / 'flat.pdl').read_bytes())
            job['tessent_pdl_inputs'] = ['source_replay.pdl']
        candidate(directory, job, tool, binaries[tool])
        if {name: digest(directory / name) for name in job['artifacts']} != actual:
            raise ValueError('second output differs: idempotency failure')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('action', choices=('prepare', 'record', 'check'))
    parser.add_argument('--root', type=Path, default=ROOT)
    parser.add_argument('--build', type=Path, default=ROOT / 'build')
    parser.add_argument('--manifest', type=Path, default=ROOT / 'tests/cases/ours/tessent_golden/cases.json')
    parser.add_argument('--bundle', type=Path)
    parser.add_argument('--output', type=Path)
    parser.add_argument('--case')
    parser.add_argument('--tool', choices=TOOLS)
    parser.add_argument('--goldens', type=Path, default=ROOT / 'tests/golden/tessent')
    args = parser.parse_args()
    binaries = {name: args.build.resolve() / name for name in TOOLS}
    try:
        if args.action == 'prepare':
            if args.output is None:
                raise ValueError('prepare requires --output (new bundle directory)')
            cases = json.loads(args.manifest.read_text())['cases']
            report = prepare(args.root, cases, args.output, binaries)
            failed = [job['id'] for job in report['cases'] if not job['status'].startswith('ready_')]
            print(f'prepared {len(report["cases"])} jobs; failed: {failed}')
            return int(bool(failed))
        if args.action == 'record':
            if not all((args.bundle, args.case, args.tool, args.output)):
                raise ValueError('record requires --bundle --case --tool --output')
            record(args.root, args.bundle, args.case, args.tool, args.output,
                   args.build / 'pdl_pattern_trace')
            print(f'recorded {args.case}/{args.tool}: {args.output}')
        else:
            snapshots = sorted(args.goldens.glob('*/snapshot.json'))
            if not snapshots:
                raise ValueError('no certified snapshots found')
            cases = json.loads(args.manifest.read_text())['cases']
            oracle.validate(args.root, cases)
            configured = {case['id']: case for case in cases}
            for snapshot in snapshots:
                certified = json.loads(snapshot.read_text())['case']
                if configured.get(certified['id']) != certified:
                    raise ValueError('changed manifest case: explicit vendor recertification required')
                check(args.root, snapshot.parent, binaries)
            print(f'{len(snapshots)} Tessent-certified goldens passed (offline)')
        return 0
    except (OSError, ValueError, KeyError, subprocess.SubprocessError) as error:
        parser.exit(2, str(error) + '\n')


if __name__ == '__main__':
    raise SystemExit(main())
