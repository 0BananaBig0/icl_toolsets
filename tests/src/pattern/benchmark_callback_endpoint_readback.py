#!/usr/bin/env python3
"""Bound actual write-callback observation cost with matched controls."""
import argparse
import hashlib
import json
from pathlib import Path
import platform
import subprocess
import tempfile
import time

from benchmark_callback_execution import inputs


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--binary", type=Path, required=True)
    parser.add_argument("--parent", type=Path, required=True)
    parser.add_argument("--sanitized", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--repeats", type=int, default=3)
    parser.add_argument("--sizes", type=int, nargs="+", default=[64, 256, 1024])
    args = parser.parse_args()
    if args.repeats < 1 or any(n < 1 for n in args.sizes):
        parser.error("sizes and repeats must be positive")
    binaries = {name: path.resolve() for name, path in (
        ("current", args.binary), ("parent", args.parent),
        ("sanitized", args.sanitized))}
    args.output.mkdir(parents=True, exist_ok=True)
    report = {"environment": platform.platform(),
              "binaries": {name: {"path": str(path), "sha256": digest(path)}
                           for name, path in binaries.items()}, "runs": []}

    def run(owner, mode, count, repeat, form, model, commands, parent):
        root = Path(tempfile.mkdtemp(prefix=form + "-", dir=parent))
        output, metric = root / "result.pdl", root / "time.txt"
        modeled = mode != "observer-off"
        command = [str(binaries[owner]), "--icl", str(model), "--pdl", str(commands),
                   "--top_module", "Top", "--output", str(output), "--debug",
                   *(["--model_readback"] if modeled else [])]
        started = time.monotonic()
        process = subprocess.run(
            ["/usr/bin/time", "-f", "%e %M", "-o", str(metric), *command],
            capture_output=True, text=True, timeout=45)
        if process.returncode:
            raise RuntimeError(process.stdout + process.stderr)
        seconds, rss = metric.read_text().split()
        elapsed = time.monotonic() - started
        data = output.read_bytes()
        notes = [line for line in data.splitlines()
                 if line.startswith(b"iNote ") and b"modeled " not in line]
        if modeled and notes != [b"iNote 1", b"iNote 2", b"iNote 3", b"iNote 4"]:
            raise RuntimeError("actual four-group endpoint observation mismatch")
        if mode in ("observed", "observer-off"):
            if data.count(b"iState r") != count * 4:
                raise RuntimeError("callback postconditions omitted")
        if elapsed > 10 or int(rss) > 512 * 1024:
            raise RuntimeError("callback readback exceeds10s/512MiB")
        report["runs"].append({
            "owner": owner, "mode": mode, "targets": count, "repeat": repeat,
            "form": form, "outer_groups": 4, "command": command,
            "wall_seconds": elapsed, "time_seconds": float(seconds),
            "rss_kib": int(rss), "icl_sha256": digest(model),
            "pdl_sha256": digest(commands), "output_sha256": digest(output),
            "output_bytes": len(data)})
        (args.output / "results.json").write_text(
            json.dumps(report, indent=2) + "\n")
        return data, output

    controls = {}
    for owner in ("current", "sanitized", "parent"):
        sizes = [64] if owner == "sanitized" else args.sizes
        modes = ["observed"] if owner == "sanitized" else [
            "plain", "unused", "observer-off", *(["observed"] if owner == "current" else [])]
        for count in sizes:
            for mode in modes:
                source, control, invoked = inputs(count, mode != "plain", 4)
                source = source.replace("DataInPort p[3:0];",
                                        "DataInPort p[3:0];DataOutPort q[3:0]{Source p;}")
                commands = invoked if mode in ("observed", "observer-off") else control
                if mode != "observer-off":
                    commands = commands.replace(
                        "iApply\n", "iApply; iRead q 0; iApply; iNote [iGetReadData q -dec]\n")
                root = args.output / owner / str(count) / mode
                root.mkdir(parents=True, exist_ok=True)
                model, pdl = root / "source.icl", root / "source.pdl"
                model.write_text(source)
                pdl.write_text(commands)
                reference = None
                for repeat in range(args.repeats):
                    data, output = run(owner, mode, count, repeat,
                                       "source", model, pdl, root)
                    if reference is None:
                        reference = data
                    elif data != reference:
                        raise RuntimeError("repeat output changed")
                    if mode == "observed":
                        for form, flat, body in (
                                ("flat", output.parent / "debug/Top.icl", pdl),
                                ("generated", model, output)):
                            replay, _ = run(owner, mode, count, repeat,
                                            form, flat, body, root)
                            if replay != reference:
                                raise RuntimeError(form + " differs from source")
                if mode != "observed":
                    key = (count, mode)
                    if owner == "current":
                        controls[key] = reference
                    elif reference != controls[key]:
                        raise RuntimeError("parent/current control output changed")
                rows = [row for row in report["runs"]
                        if (row["owner"], row["targets"], row["mode"]) ==
                        (owner, count, mode)]
                print(owner, count, mode, "max seconds",
                      round(max(row["wall_seconds"] for row in rows), 4),
                      "rss_kib", max(row["rss_kib"] for row in rows), flush=True)
    print("callback endpoint scale passed", len(report["runs"]), "processes")


if __name__ == "__main__":
    main()
