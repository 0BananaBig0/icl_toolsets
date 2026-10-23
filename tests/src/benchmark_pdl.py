#!/usr/bin/env python3
"""Measure PDL CLI parsing/formatting time and peak RSS on generated inputs."""
import argparse
import json
import platform
from pathlib import Path
import statistics
import subprocess
import tempfile
import time


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--binary', default='build/pdl-format')
    parser.add_argument('--sizes', type=int, nargs='+', default=[100_000, 1_000_000])
    parser.add_argument('--repeat', type=int, default=3)
    args = parser.parse_args()
    if args.repeat < 1 or any(size < 1 for size in args.sizes):
        parser.error('sizes and repeat must be positive')
    binary = str(Path(args.binary).resolve())
    print(json.dumps({'platform': platform.platform(), 'binary': binary}))
    with tempfile.TemporaryDirectory() as directory:
        rss_file = Path(directory) / 'rss.txt'
        for size in args.sizes:
            header = b'iPDLLevel 1 -version STD_1687_2014\niProcsForModule Demo\niProc run {} {\n'
            command = b'iWrite reg[7:0] 0xA5; iApply\n'
            source = header + command * max(1, size // len(command)) + b'}\n'
            times, rss = [], []
            for _ in range(args.repeat):
                start = time.perf_counter()
                run = subprocess.run(
                    ['/usr/bin/time', '-f', '%M', '-o', str(rss_file), binary],
                    input=source, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE,
                    timeout=60)
                elapsed = time.perf_counter() - start
                if run.returncode:
                    raise RuntimeError(run.stderr.decode(errors='replace'))
                times.append(elapsed)
                rss.append(int(rss_file.read_text()))
            print(json.dumps({'input_bytes': len(source), 'runs': args.repeat,
                              'wall_seconds_median': round(statistics.median(times), 4),
                              'peak_rss_kib_max': max(rss)}))


if __name__ == '__main__':
    main()
