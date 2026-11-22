#!/usr/bin/env python3
"""Bound wide-alias callback temporary storage at a demonstrated input size."""
import argparse
import hashlib
import json
from pathlib import Path
import platform
import subprocess
import time


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--binary", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--repeats", type=int, default=3)
    args = parser.parse_args()
    if args.repeats < 1:
        parser.error("--repeats must be positive")
    binary = args.binary.resolve()
    report = {"environment": platform.platform(),
              "binary_sha256": hashlib.sha256(binary.read_bytes()).hexdigest(),
              "runs": []}
    for count in (4096, 16384):
        root = args.output.resolve() / str(count)
        root.mkdir(parents=True, exist_ok=True)
        icl, pdl = root / "model.icl", root / "source.pdl"
        source = "Module Top {" + "".join(
            "DataRegister r%d {WriteCallBack Top store <D> <R>;}" % i
            for i in range(count))
        source += ("Alias all[%d:0] = " % (count - 1) +
                   ",".join("r%d" % i for i in range(count)) + ";}")
        icl.write_text(source)
        pdl.write_text("iPDLLevel 1; iProcsForModule Top; "
                       "iProc store {data reg} {}; iWrite all 0b" +
                       "1" * count + "; iApply")
        for repeat in range(args.repeats):
            output, metric = root / "result.pdl", root / "time.txt"
            command = [str(binary), "--icl", str(icl), "--pdl", str(pdl),
                       "--top_module", "Top", "--output", str(output)]
            started = time.monotonic()
            result = subprocess.run(
                ["/usr/bin/time", "-f", "%e %M", "-o", str(metric), *command],
                capture_output=True, text=True, timeout=180)
            if result.returncode:
                raise RuntimeError(result.stdout + result.stderr)
            seconds, rss = metric.read_text().split()
            data = output.read_bytes()
            if data.count(b"iState r") != count:
                raise RuntimeError("wide alias omitted callback targets")
            row = {"targets": count, "operand_bits": count, "applies": 1,
                   "repeat": repeat, "wall_seconds": time.monotonic() - started,
                   "time_seconds": float(seconds), "rss_kib": int(rss),
                   "command": command,
                   "icl_sha256": hashlib.sha256(icl.read_bytes()).hexdigest(),
                   "pdl_sha256": hashlib.sha256(pdl.read_bytes()).hexdigest(),
                   "output_sha256": hashlib.sha256(data).hexdigest()}
            report["runs"].append(row)
            (args.output / "results.json").write_text(json.dumps(report, indent=2) + "\n")
            print(count, repeat, round(row["wall_seconds"], 4), row["rss_kib"], flush=True)
            if count == 16384 and int(rss) > 256 * 1024:
                raise RuntimeError("16384 callback alias exceeds 256 MiB RSS budget")
    print("wide callback alias memory budget passed", len(report["runs"]), "processes")


if __name__ == "__main__":
    main()
