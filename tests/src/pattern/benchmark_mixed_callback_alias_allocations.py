#!/usr/bin/env python3
# Copyright 2026 The ICL Toolsets Authors.
# SPDX-License-Identifier: MIT
"""Check payload-copy work while canceling dense and sparse mixed writes."""
import json
import os
from pathlib import Path
import re
import subprocess
import sys
import tempfile


def measure(binary):
    with tempfile.TemporaryDirectory(prefix="mixed-alias-allocations-") as folder:
        root = Path(folder)
        source = root / "counter.cc"
        library = root / "counter.so"
        source.write_text("""
#include <cstdio>
#include <cstdlib>
#include <new>
static unsigned long copies = 0;
void* operator new(std::size_t size) {
  if (size == 515) ++copies;
  if (void* memory = std::malloc(size)) return memory;
  throw std::bad_alloc();
}
__attribute__((destructor)) static void Report() {
  std::fprintf(stderr, "payload_allocations=%lu\\n", copies);
}
""")
        subprocess.run(["c++", "-fPIC", "-shared", "-O2", str(source),
                        "-o", str(library)], check=True)
        rows = []
        for sparse in (False, True):
            pins = (",".join("p[%d]" % (2 * i) for i in range(256))
                    if sparse else "p[255:0]")
            (root / "model.icl").write_text(
                "Module Top {DataInPort p[511:0]; DataRegister cb[255:0] "
                "{WriteCallBack Top store <D>;} Alias mixed[511:0]="
                + pins + ",cb;}")
            for access in (False, True):
                (root / "source.pdl").write_text(
                    "iPDLLevel 1; iProcsForModule Top; "
                    + ("iWrite mixed 0; " if access else "") + "iReset")
                process = subprocess.run(
                    [str(binary), "--top_module", "Top", "--icl", "model.icl",
                     "--pdl", "source.pdl", "--output", "out.pdl"], cwd=root,
                    env=dict(os.environ, LD_PRELOAD=str(library)),
                    capture_output=True, text=True, timeout=10)
                assert process.returncode == 0, process.stdout + process.stderr
                match = re.search(r"payload_allocations=(\d+)", process.stderr)
                assert match, process.stderr
                rows.append({"sparse": sparse, "access": access,
                             "payload_allocations": int(match[1])})
        print(json.dumps(rows, indent=2), flush=True)
        assert rows[0]["payload_allocations"] == rows[2]["payload_allocations"] == 0
        assert rows[3]["payload_allocations"] <= rows[1]["payload_allocations"] + 1, (
            "full alias payload copies must not grow with ordinary run count")
        return rows


if __name__ == "__main__":
    measure(Path(sys.argv[1]).resolve())
