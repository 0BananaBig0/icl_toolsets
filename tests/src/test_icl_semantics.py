#!/usr/bin/env python3
"""Bounded dotted-path regression and reproducible parse/RSS measurements."""
import argparse
import json
from pathlib import Path
import resource
import subprocess
import tempfile
import time


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--binary', type=Path, required=True)
    parser.add_argument('--benchmark', action='store_true')
    args = parser.parse_args()

    def limits():
        resource.setrlimit(resource.RLIMIT_CORE, (0, 0))
        # 128 MiB leaves ample headroom for the shared parser on this 8 KiB
        # input, but catches retained quadratic prefix copies.
        if not args.benchmark:
            resource.setrlimit(resource.RLIMIT_AS, (128 << 20, 128 << 20))

    results = []
    with tempfile.TemporaryDirectory(prefix='icl-path-') as directory:
        stats = Path(directory) / 'usage.txt'
        for size in ((500, 1000, 2000, 4000) if args.benchmark else (4000,)):
            source = 'Module M { Alias a = ' + '.'.join(['x'] * size) + '; }\n'
            start = time.perf_counter()
            result = subprocess.run(
                ['/usr/bin/time', '-f', '%M', '-o', str(stats),
                 str(args.binary.resolve())], input=source, text=True,
                stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                preexec_fn=limits, timeout=15)
            elapsed = time.perf_counter() - start
            assert result.returncode == 0, (size, result.returncode, result.stderr)
            assert '.'.join(['x'] * size) in result.stdout
            results.append(dict(components=size, bytes=len(source),
                                wall_seconds=elapsed,
                                max_rss_kib=int(stats.read_text().strip())))
    print(json.dumps(results, indent=2))


if __name__ == '__main__':
    main()
