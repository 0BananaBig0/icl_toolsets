#!/usr/bin/env python3
"""Check all maintained ICL for duplicate bytes and namespace-aware modules.

Generated performance fixtures intentionally repeat definitions and are excluded.
Rejected inputs are compared as files only; partial ASTs are not trustworthy
module boundaries. Review-only JSON reports belong under build/, not in Git.
"""
import argparse
from collections import defaultdict
import json
from pathlib import Path
import subprocess

from import_eda_corpus import digest, module_id

ROOT = Path(__file__).resolve().parents[1]


def audit(root, inventory):
    paths = sorted(p for p in root.rglob('*') if p.is_file() and p.suffix.lower() == '.icl'
                   and p.relative_to(root).parts[0] != 'generated')
    if not paths:
        raise ValueError(f'no maintained ICL files under {root}')
    by_hash = defaultdict(list)
    for path in paths:
        by_hash[digest(path.read_bytes())].append(path)
    run = subprocess.run([str(inventory.resolve()), '--audit',
                          *[str(group[0]) for group in by_hash.values()]],
                         capture_output=True, check=True)
    # The formatter accepts arbitrary comment bytes; JSON surrogate escaping
    # keeps those bytes readable for audit without discarding a valid fixture.
    parsed = json.loads(run.stdout.decode('utf-8', errors='surrogateescape'))
    if len(parsed) != len(by_hash):
        raise ValueError('incomplete shared-parser inventory')
    file_duplicates, rejected, modules, fixtures = [], [], {}, {}
    module_count = 0
    for (sha, copies), info in zip(by_hash.items(), parsed):
        names = [p.relative_to(root).as_posix() for p in copies]
        identities = [module_id(m) for m in info.get('modules', [])]
        for name in names:
            fixtures[name] = {'sha256': sha, 'modules': identities}
        if len(copies) > 1:
            file_duplicates.append({'sha256': sha, 'paths': names})
        if not info['ok']:
            rejected.extend(names)
            continue
        for index, (module, identity) in enumerate(zip(info['modules'], identities)):
            group = modules.setdefault(identity, {'id': identity, 'name': module['name'],
                                                  'namespace': module['namespace'], 'occurrences': []})
            for name in names:
                group['occurrences'].append({'path': name, 'module_index': index, 'sha256': sha})
                module_count += 1
    groups = [g for _, g in sorted(modules.items()) if len(g['occurrences']) > 1]
    for group in groups:
        group['occurrences'].sort(key=lambda item: (item['path'], item['module_index']))
    return {'files': len(paths), 'rejected': sorted(rejected), 'modules': module_count,
            'unique_modules': len(modules), 'file_duplicates': file_duplicates,
            'module_duplicates': groups, 'fixtures': fixtures}


def violations(report, policy):
    problems = [f"identical files: {', '.join(g['paths'])}" for g in report['file_duplicates']]
    for removed in policy.get('removed_files', []):
        if removed['path'] in report['fixtures']:
            problems.append(f"removed copy reappeared: {removed['path']}")
        canonical = report['fixtures'].get(removed['canonical_path'])
        if canonical is None:
            problems.append(f"missing canonical fixture: {removed['canonical_path']}")
        elif removed['kind'] == 'identical-file':
            if canonical['sha256'] != removed['sha256']:
                problems.append(f"canonical bytes changed: {removed['canonical_path']}")
        elif not removed['modules'] or not set(removed['modules']) <= set(canonical['modules']):
            problems.append(f"missing canonical module: {removed['canonical_path']}")
    allowed = {}
    for entry in policy.get('retained_module_duplicates', []):
        group = dict(entry)
        if not group.pop('reason', '').strip():
            raise ValueError('retained duplicate has no reviewed reason')
        if group['id'] in allowed:
            raise ValueError('duplicate policy ID')
        allowed[group['id']] = group
    for group in report['module_duplicates']:
        if allowed.pop(group['id'], None) != group:
            paths = ', '.join(o['path'] for o in group['occurrences'])
            problems.append(f"unreviewed module repetition: {group['namespace']}::{group['name']}: {paths}")
    problems.extend(f'stale duplicate policy: {identity}' for identity in allowed)
    return problems


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', type=Path, default=ROOT / 'tests')
    parser.add_argument('--inventory', type=Path, default=ROOT / 'build/icl_corpus_inventory')
    parser.add_argument('--policy', type=Path)
    parser.add_argument('--report', type=Path)
    args = parser.parse_args()
    try:
        report = audit(args.root, args.inventory)
        if args.report:
            args.report.write_text(json.dumps(report, indent=2) + '\n')
        policy = json.loads(args.policy.read_text()) if args.policy else {}
        problems = violations(report, policy)
        print(f"{report['files']} files; {len(report['rejected'])} rejected inputs checked by bytes; "
              f"{report['modules']} module occurrences / {report['unique_modules']} definitions; "
              f"{len(report['file_duplicates'])} duplicate-file groups; "
              f"{len(report['module_duplicates'])} repeated-module groups")
        for problem in problems:
            print(problem)
        return bool(problems)
    except (OSError, ValueError, subprocess.CalledProcessError) as error:
        if isinstance(error, subprocess.CalledProcessError):
            parser.exit(2, error.stderr.decode(errors='replace'))
        parser.exit(2, str(error) + '\n')


if __name__ == '__main__':
    raise SystemExit(main())
