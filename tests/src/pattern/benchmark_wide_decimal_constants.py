#!/usr/bin/env python3
"""Measure checked native decimal decoding against the same hexadecimal fixtures."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import platform
import subprocess
import tempfile
import time

ROOT = Path(__file__).resolve().parents[3]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('binary', 'parent', 'sanitized', 'output'):
        parser.add_argument('--'+name, type=Path, required=True)
    args = parser.parse_args()
    binaries = {'parent': args.parent.resolve(), 'current': args.binary.resolve(),
                'sanitized': args.sanitized.resolve()}
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=True)
    digest = lambda p: hashlib.sha256(p.read_bytes()).hexdigest()
    fixture = ROOT/'tests/data/pattern/wide_decimal_constants.txt'
    fixture_sha = digest(fixture)
    report = dict(environment=platform.platform(), fixture=str(fixture),
                  fixture_sha256=fixture_sha, widths=[64,65,96,127,128,256,1024,4096,16384,65536],
                  binaries={k:dict(path=str(v),sha256=digest(v)) for k,v in binaries.items()}, runs=[])
    modes = [('parent','hex'), ('current','hex'), ('current','decimal'),
             ('sanitized','decimal')]
    failed = False
    for owner, mode in modes:
        for repeat in range(3):
            folder = Path(tempfile.mkdtemp(prefix=owner+'-'+mode+'-', dir=output))
            metric = folder/'time.txt'
            command = [str(binaries[owner]), '--wide-'+mode]
            env = dict(os.environ, ASAN_OPTIONS='detect_leaks=1', UBSAN_OPTIONS='halt_on_error=1')
            start = time.monotonic()
            with (folder/'stdout.log').open('w') as stdout, (folder/'stderr.log').open('w') as stderr:
                process = subprocess.run(['/usr/bin/time','-f','%e %M','-o',str(metric),
                    'timeout','--kill-after=2s','60s',*command], cwd=ROOT, env=env,
                    stdout=stdout, stderr=stderr)
            seconds = time.monotonic()-start
            measured = metric.read_text().splitlines()[-1].split() if metric.exists() else []
            rss = int(measured[1]) if len(measured)==2 and measured[1].isdigit() else None
            verified = process.returncode==0 and (folder/'stdout.log').read_text()==(
                'wide constants: '+mode+' cases=10\n')
            unchanged = digest(fixture)==fixture_sha and digest(binaries[owner])==report['binaries'][owner]['sha256']
            within = seconds<=10 and rss is not None and rss<=512*1024
            row = dict(owner=owner,mode=mode,repeat=repeat,command=command,directory=str(folder),
                       exit=process.returncode,seconds=seconds,peak_rss_kib=rss,
                       values_verified=verified and unchanged,within_bounds=within,checked_values=10 if verified else 0)
            report['runs'].append(row)
            failed |= not (verified and unchanged and within)
            (output/'results.json').write_text(json.dumps(report,indent=2)+'\n')
    complete = len(report['runs'])==3*len(modes)
    print(json.dumps(dict(runs=len(report['runs']),matrix_complete=complete,
                         all_values_verified=not failed)))
    return bool(failed) or not complete


if __name__ == '__main__':
    raise SystemExit(main())
