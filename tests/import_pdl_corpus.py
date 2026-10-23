#!/usr/bin/env python3
"""Import complete PDL streams from a local EDA snapshot without modifying it."""
import argparse
import json
from pathlib import Path

from import_eda_corpus import category, digest


def source_category(relative):
    if any(part.startswith('tessent_example_') for part in relative.parts):
        return 'tessent'
    return category(relative)


def collect(source):
    paths = sorted(p for p in source.rglob('*')
                   if p.is_file() and not p.is_symlink() and p.suffix.lower() == '.pdl')
    if not paths:
        raise ValueError('no PDL files found')
    sources, fixtures, contents = [], {}, {}
    for path in paths:
        relative = path.relative_to(source)
        origin = source_category(relative)
        data = path.read_bytes()
        sha = digest(data)
        if sha not in contents:
            fixture = f'{origin}/{path.stem}_{sha[:16]}.pdl'
            if fixture in fixtures:
                raise ValueError(f'filename hash collision: {fixture}')
            fixtures[fixture] = data
            contents[sha] = {'path': fixture, 'sha256': sha, 'bytes': len(data)}
        sources.append({'path': relative.as_posix(), 'category': origin,
                        'sha256': sha, 'fixture': contents[sha]['path']})
    return {'identity': 'SHA-256 of exact complete PDL bytes; no procedure extraction',
            'sources': sources, 'fixtures': list(contents.values())}, fixtures


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--input', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    try:
        if args.output.exists() or args.output.is_symlink():
            raise ValueError(f'output already exists: {args.output}')
        manifest, fixtures = collect(args.input)
        args.output.mkdir(parents=True, exist_ok=False)
        for name, data in fixtures.items():
            path = args.output / name
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(data)
        (args.output / 'sources.json').write_text(json.dumps(manifest, indent=2) + '\n')
        print(f"{len(manifest['sources'])} source paths -> {len(fixtures)} unique PDL streams")
    except (OSError, ValueError) as error:
        parser.exit(1, str(error) + '\n')


if __name__ == '__main__':
    main()
