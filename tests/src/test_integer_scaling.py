#!/usr/bin/env python3
"""Guard against quadratic copying of long parameter-dependent integer expressions."""
import argparse
from pathlib import Path
import statistics
import subprocess
import time


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--binary', type=Path, required=True)
    args = ap.parse_args()
    root = Path(__file__).resolve().parents[1]/'generated/ours/performance'
    root.mkdir(parents=True, exist_ok=True)
    elapsed = []
    for count in (4000, 16000):
        path = root/f'integer_terms_{count}.icl'
        path.write_text('Module M { Parameter P = 1; DataInPort p[' + '+'.join(['$P']*count) + ':0]; }\n')
        timings = []
        for _ in range(3):
            start = time.perf_counter()
            run = subprocess.run([str(args.binary.resolve()), str(path)], capture_output=True, timeout=15)
            timings.append(time.perf_counter() - start)
            assert run.returncode == 0, run.stderr
            assert run.stdout.count(b'$P') == count, 'expression terms were lost'
        elapsed.append(statistics.median(timings))
    ratio = elapsed[1] / elapsed[0]
    print(f'integer expression 4k/16k: {elapsed[0]:.4f}s/{elapsed[1]:.4f}s, ratio={ratio:.2f}')
    assert ratio < 6, '4x input must not approach quadratic (16x) growth'
    return 0

if __name__ == '__main__': raise SystemExit(main())
