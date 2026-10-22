#!/usr/bin/env python3
"""Import a local snapshot of the three EDA trees into a NEW corpus directory.

Usage: make corpuscheck BISON=/usr/bin/bison
       python3 tests/import_eda_corpus.py --input SNAPSHOT --output NEW_DIRECTORY
Input contains test/tessent_ijtag, tessent_2026.2_usage_examples and/or syn_slm.
No network access, source mutation, or overwriting of existing output.
"""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess


def category(path):
    parts = path.parts
    if parts[:3] == ('test', 'tessent_ijtag', 'test_syn_slm_icl') or parts[0] == 'syn_slm':
        return 'syn_slm'
    if parts[:2] == ('test', 'tessent_ijtag') or parts[0] == 'tessent_2026.2_usage_examples':
        return 'tessent'
    raise ValueError(f'unknown source category: {path}')


def digest(data):
    return hashlib.sha256(data).hexdigest()


def module_id(module):
    # Keep literal spellings/order, strings and opaque text; normalize only trivia
    # excluded by the shared lexer. This is not semantic equivalence analysis.
    return digest(json.dumps([module[k] for k in
                             ('namespace', 'use_namespace', 'tokens', 'opaque')],
                             ensure_ascii=True, separators=(',', ':')).encode())


def collect(inputs, inventory):
    paths = sorted(p for p in inputs.rglob('*') if p.is_file() and p.suffix.lower() == '.icl')
    if not paths:
        raise ValueError('no ICL files found')
    contents, source_info = {}, []
    for path in paths:
        relative = path.relative_to(inputs)
        data = path.read_bytes()
        sha = digest(data)
        contents.setdefault(sha, (path, data))
        source_info.append({'path': relative.as_posix(), 'category': category(relative), 'sha256': sha})
    result = subprocess.run([str(inventory.resolve()), *[str(p) for p, _ in contents.values()]],
                            capture_output=True, check=True)
    parsed = dict(zip(contents, json.loads(result.stdout), strict=True))
    modules, comments, comment_ids, fixtures = {}, [], {}, {}
    for origin in source_info:
        info = parsed[origin['sha256']]
        data = contents[origin['sha256']][1]
        origin['modules'], origin['comments'] = [], []
        for comment in info['comments']:
            text = comment['text']
            if text not in comment_ids:
                comment_ids[text] = len(comments)
                comments.append(text)
            origin['comments'].append(comment_ids[text])
        outside = ''.join(c['text'] + '\n' for c in info['comments']
                          if not any(m['begin'] <= c['begin'] and c['end'] <= m['end']
                                     for m in info['modules']))
        for module in info['modules']:
            identity = module_id(module)
            origin['modules'].append(identity)
            if identity in modules:
                continue
            path = f"pass/{origin['category']}/eda/{module['name']}_{identity[:16]}.icl"
            if path in fixtures:
                raise ValueError(f'filename hash collision: {path}')
            prefix = (outside + f"NameSpace {module['namespace']};\n"
                      f"UseNameSpace {module['use_namespace']};\n").encode()
            fixtures[path] = prefix + data[module['begin']:module['end']] + b'\n'
            modules[identity] = {k: module[k] for k in ('name', 'namespace', 'use_namespace')}
            modules[identity].update(id=identity, path=path, source=origin['path'],
                                     sha256=digest(fixtures[path]))
    manifest = {'identity': 'ICL tokens + exact opaque source + definition/import namespaces; SHA-256',
                'sources': source_info, 'modules': list(modules.values()), 'comments': comments}
    return manifest, fixtures


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--input', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--inventory', type=Path, default=Path('build/icl_corpus_inventory'))
    args = parser.parse_args()
    try:
        if args.output.exists() or args.output.is_symlink():
            raise ValueError(f'output already exists: {args.output}')
        manifest, fixtures = collect(args.input, args.inventory)
        args.output.mkdir(parents=True, exist_ok=False)
        for name, data in fixtures.items():
            path = args.output / name
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(data)
        (args.output / 'eda_sources.json').write_text(json.dumps(manifest, indent=2) + '\n')
        print(f"{len(manifest['sources'])} source paths -> {len(fixtures)} unique modules")
    except (ValueError, OSError, subprocess.CalledProcessError) as error:
        if isinstance(error, subprocess.CalledProcessError):
            parser.exit(1, error.stderr.decode(errors='replace'))
        parser.exit(1, str(error) + '\n')


if __name__ == '__main__':
    main()
