#!/usr/bin/env python3
# Copyright 2026 The ICL Toolsets Authors.
# SPDX-License-Identifier: MIT
"""Count real model evaluations for the connected 256-target callback case."""
import json
import os
from pathlib import Path
import re
import shlex
import subprocess
import tempfile
import time

from benchmark_downstream_write_callbacks import fixture
from benchmark_merged_callbacks import checked_frames


def measure():
    project = Path(__file__).resolve().parents[3]
    commands = subprocess.check_output(
        ['make', '-n', '-B', 'build/icl-pattern-generation'], cwd=project,
        text=True).splitlines()
    link = next(shlex.split(line) for line in commands
                if line.endswith('-o build/icl-pattern-generation'))
    with tempfile.TemporaryDirectory(prefix='primary-decode-calls-') as folder:
        root = Path(folder)
        source, obj, binary = root/'counter.cc', root/'counter.o', root/'generator'
        source.write_text('''#include <cstdio>
#include <z3.h>
static unsigned long calls = 0;
extern "C" bool __real_Z3_model_eval(Z3_context, Z3_model, Z3_ast, bool, Z3_ast*);
extern "C" bool __wrap_Z3_model_eval(Z3_context c, Z3_model m, Z3_ast t,
                                    bool complete, Z3_ast* result) {
  ++calls;
  return __real_Z3_model_eval(c, m, t, complete, result);
}
__attribute__((destructor)) static void Report() {
  std::fprintf(stderr, "model_eval_calls=%lu\\n", calls);
}
''')
        subprocess.run([link[0], '-std=c++17', '-O2',
                        '-I'+str(Path(os.environ['Z3_PREFIX'])/'include'),
                        '-c', str(source), '-o', str(obj)], check=True)
        link[link.index('-o')+1] = str(binary)
        link += [str(obj), '-Wl,--wrap=Z3_model_eval']
        subprocess.run(link, cwd=project, check=True)
        model, commands = fixture(256, 'automatic')
        (root/'source.icl').write_text(model)
        (root/'source.pdl').write_text(commands)
        started = time.monotonic()
        result = subprocess.run([str(binary), '--icl', 'source.icl', '--pdl',
            'source.pdl', '--top_module', 'Chains', '--output', 'result.pdl'],
            cwd=root, capture_output=True, text=True, timeout=60)
        assert result.returncode == 0, result.stdout+result.stderr
        dimensions, _ = checked_frames((root/'result.pdl').read_text(), 256, 4, True)
        match = re.search(r'model_eval_calls=(\d+)', result.stderr)
        assert match, result.stderr
        count = int(match[1])
        row = dict(model_eval_calls=count, bound=600000,
                   instrumented_wall_seconds=time.monotonic()-started,
                   targets=256, width=4, **dimensions)
        print(json.dumps(row), flush=True)
        assert count <= 600000, 'repeated primary decoding exceeds model-evaluation work budget'
        return row


if __name__ == '__main__':
    measure()
