#!/usr/bin/env python3
"""Bound full-root string allocation work at a fixed 8200-bit route width."""
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import sys
import tempfile
from benchmark_multiple_opaque_children import model


def measure(binary, output):
    binary = binary.resolve()
    output.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='opaque-projection-allocations-') as folder:
        root = Path(folder)
        source, library = root/'counter.cc', root/'counter.so'
        source.write_text(r'''
#include <cstdio>
#include <cstdlib>
#include <new>
static unsigned long roots = 0;
void* operator new(std::size_t size) {
  if (size == 8203) ++roots;
  if (void* memory = std::malloc(size)) return memory;
  throw std::bad_alloc();
}
__attribute__((destructor)) static void Report() {
  std::fprintf(stderr, "opaque_root_allocations=%lu\n", roots);
}
''')
        subprocess.run(['c++','-std=c++17','-fPIC','-shared','-O2',str(source),
                        '-o',str(library)], check=True)
        rows = []
        reference = None
        for children, width in ((8,1024),(32,256)):
            case = output/str(children)
            case.mkdir(exist_ok=True)
            icl, pdl, result = case/'source.icl',case/'source.pdl',case/'result.pdl'
            icl.write_text(model([width]*children))
            pdl.write_text('iPDLLevel 1\niProcsForModule Top\niReset\n'
                           'iScan client 8200 -si 0 -stable; iApply\n')
            command = [str(binary),'--icl',str(icl),'--pdl',str(pdl),
                       '--top_module','Top','--output',str(result)]
            process = subprocess.run(command,env=dict(os.environ,LD_PRELOAD=str(library)),
                                     capture_output=True,text=True,timeout=10)
            (case/'stdout.log').write_text(process.stdout)
            (case/'stderr.log').write_text(process.stderr)
            if process.returncode:
                raise RuntimeError(process.stdout+process.stderr)
            match = re.search(r'^opaque_root_allocations=(\d+)$',process.stderr,re.M)
            if not match or not int(match[1]):
                raise RuntimeError('allocation interposer did not report actual root allocations')
            data = result.read_text()
            if ('iScan client 8200 -si 0b'+'0'*8200+' -so 0b'+'x'*8200+' -stable\n') not in data:
                raise RuntimeError('complete root payload differs')
            if reference is None:
                reference = data
            elif reference!=data:
                raise RuntimeError('fixed-width root replay differs')
            rows.append(dict(children=children,width=width,root_width=8200,
                             root_allocations=int(match[1]),command=command,
                             output_sha256=hashlib.sha256(result.read_bytes()).hexdigest()))
        increase = rows[1]['root_allocations']-rows[0]['root_allocations']
        report = dict(binary=str(binary),binary_sha256=hashlib.sha256(binary.read_bytes()).hexdigest(),
                      runs=rows,root_allocation_increase=increase,max_increase=4,
                      within_bound=increase<=4)
        (output/'results.json').write_text(json.dumps(report,indent=2)+'\n')
        print(json.dumps(report),flush=True)
        if increase>4:
            raise RuntimeError('full-root allocation work grows with child count')
        return report


if __name__=='__main__':
    measure(Path(sys.argv[1]),Path(sys.argv[2]))
