#!/usr/bin/env python3
"""Reproducible CLI throughput/RSS measurements, with persisted generated inputs."""
import argparse
import json
from pathlib import Path
import platform
import statistics
import subprocess
import tempfile
import time

ROOT = Path(__file__).resolve().parents[2]


def inputs():
    generated = ROOT / 'tests/generated/ours/performance'
    generated.mkdir(parents=True, exist_ok=True)
    merged = ROOT/'tests/cases/pass/tessent/tessent_merged_icl.icl'
    paths = [merged, merged.with_name('tessent_merged_icl_1.icl')]
    for scale in (4, 16):
        path = generated/f'merged_x{scale}.icl'
        path.write_bytes(merged.read_bytes() * scale)
        paths.append(path)
    for size in (1000, 4000, 16000):
        path = generated/f'integer_terms_{size}.icl'
        path.write_text('Module M { Parameter P = 1; DataInPort p[' + '+'.join(['$P']*size) + ':0]; }\n')
        paths.append(path)
    for size in (1000, 4000, 16000):
        path = generated / f'parameter_aliases_{size}.icl'
        lines = [f'Module BenchmarkParameterAliases{size} {{']
        lines.extend(f'Parameter W{i} = $W{i + 1};' for i in range(size - 1))
        lines.extend([f'LocalParameter W{size - 1} = 8;',
                      "Attribute value = $W0'hFF;", '}'])
        path.write_text('\n'.join(lines) + '\n')
        paths.append(path)
    for size in (1000, 4000, 16000):
        path = generated/f'decimal_digits_{size}.icl'
        # All9s is near a width boundary; avoids the generous-width fast path.
        path.write_text(f"Module M {{ Attribute value = {int(size*3.322)+1}'d" + '9'*size + '; }\n')
        paths.append(path)
    for depth in (64, 256, 1024):
        path = generated/f'parentheses_{depth}.icl'
        path.write_text('Module M { DataInPort p[' + '('*depth + '1' + ')'*depth + ':0]; }\n')
        paths.append(path)
    return paths


def measure(binary, path, repeats):
    runs = []
    with tempfile.TemporaryDirectory(prefix='icl-perf-') as tmp:
        for _ in range(repeats):
            memory = Path(tmp)/'rss.txt'
            start = time.perf_counter()
            proc = subprocess.run(['/usr/bin/time', '-f', '%M', '-o', str(memory), str(binary), str(path)], stdout=subprocess.DEVNULL, stderr=subprocess.PIPE, timeout=60)
            elapsed = time.perf_counter() - start
            runs.append(dict(seconds=elapsed, max_rss_kib=int(memory.read_text().splitlines()[-1]), exit=proc.returncode, error=proc.stderr.decode(errors='replace')[:200]))
    return dict(median_seconds=statistics.median(r['seconds'] for r in runs), max_rss_kib=max(r['max_rss_kib'] for r in runs), runs=runs)


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--binary', type=Path, required=True)
    ap.add_argument('--baseline', type=Path)
    ap.add_argument('--output', type=Path, required=True)
    ap.add_argument('--repeats', type=int, default=3)
    args = ap.parse_args()
    result = dict(environment=platform.platform(), repeats=args.repeats, cases=[])
    for path in inputs():
        row = dict(path=str(path.relative_to(ROOT)), bytes=path.stat().st_size)
        for label, binary in [('final', args.binary), ('baseline', args.baseline)]:
            if binary: row[label] = measure(binary.resolve(), path, args.repeats)
        result['cases'].append(row)
        print(path.name, {k: round(v['median_seconds'], 4) for k,v in row.items() if isinstance(v, dict)}, flush=True)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(result, indent=2)+'\n')
    return int(any(run['exit'] for row in result['cases'] for run in row['final']['runs']))

if __name__ == '__main__': raise SystemExit(main())
