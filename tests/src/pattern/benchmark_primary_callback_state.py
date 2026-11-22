#!/usr/bin/env python3
"""Matched primary-solver cost with callback, unused and plain storage."""
import argparse
import hashlib
import json
from pathlib import Path
import platform
import statistics
import subprocess
import tempfile
import time

from benchmark_callback_execution import inputs


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--binary", type=Path, required=True)
    parser.add_argument("--parent", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--repeats", type=int, default=3)
    parser.add_argument("--sizes", type=int, nargs="+", default=[64, 256, 1024])
    args = parser.parse_args()
    if args.repeats < 1 or any(size < 1 for size in args.sizes):
        parser.error("repeats and sizes must be positive")
    args.output = args.output.resolve()
    args.output.mkdir(parents=True, exist_ok=True)
    binaries = {"parent": args.parent.resolve(), "current": args.binary.resolve()}

    def digest(path):
        return hashlib.sha256(path.read_bytes()).hexdigest()

    report = {"environment": platform.platform(),
              "compiler": subprocess.check_output(["g++", "--version"], text=True).splitlines()[0],
              "binaries": {name: {"path": str(path), "sha256": digest(path)}
                           for name, path in binaries.items()}, "runs": []}

    def run(owner, mode, form, count, root, model, commands):
        root.mkdir(parents=True, exist_ok=True)
        invocation = Path(tempfile.mkdtemp(prefix=form + "-", dir=root))
        output, metric = invocation / "result.pdl", invocation / "time.txt"
        command = [str(binaries[owner]), "--icl", str(model), "--pdl", str(commands),
                   "--top_module", "Top", "--output", str(output), "--debug"]
        started = time.monotonic()
        result = subprocess.run(
            ["/usr/bin/time", "-f", "%e %M", "-o", str(metric), *command],
            text=True, capture_output=True, timeout=180)
        if result.returncode:
            raise RuntimeError(mode + ": " + result.stdout + result.stderr)
        if not output.is_file() or not (invocation / "debug/Top.icl").is_file():
            raise RuntimeError("benchmark process produced no fresh pattern or flat model")
        seconds, rss = metric.read_text().split()
        data = output.read_bytes()
        report["runs"].append({
            "owner": owner, "mode": mode, "form": form, "declared_targets": count,
            "original_request_groups": 4,
            "original_operand_bits_per_group": count * 4 if mode == "callbacks" else 4,
            "output_apply_count": sum(line.startswith(b"iApply") for line in data.splitlines()),
            "output_endpoint_writes": data.count(b"iWrite p "),
            "wall_seconds": time.monotonic() - started,
            "time_seconds": float(seconds), "rss_kib": int(rss), "command": command,
            "icl_sha256": digest(model), "pdl_sha256": digest(commands),
            "output_sha256": hashlib.sha256(data).hexdigest(),
            "output_bytes": len(data)})
        (args.output / "results.json").write_text(json.dumps(report, indent=2) + "\n")
        return data, output

    for count in args.sizes:
        reference = {}
        measurements = {}
        for owner in ("parent", "current"):
            for mode in ("plain", "unused", "callbacks"):
                source, control, invoked = inputs(count, mode != "plain", 4)
                root = args.output / str(count) / (owner + "-" + mode)
                root.mkdir(parents=True, exist_ok=True)
                model, commands = root / "model.icl", root / "source.pdl"
                model.write_text(source)
                commands.write_text(invoked if mode == "callbacks" else control)
                rows = []
                for repeat in range(args.repeats):
                    data, output = run(owner, mode, "source", count, root, model, commands)
                    key = "callbacks" if mode == "callbacks" else "control"
                    if key not in reference:
                        reference[key] = data
                    elif data != reference[key]:
                        raise RuntimeError("parent/current output differs: " + mode)
                    if mode == "callbacks" and data.count(b"iState r") != count * 4:
                        raise RuntimeError("callback postconditions were lost")
                    rows.append(report["runs"][-1])
                measurements[(owner, mode)] = statistics.median(row["wall_seconds"] for row in rows)
                print(count, owner, mode, "median",
                      round(measurements[(owner, mode)], 4),
                      "rss_kib", max(row["rss_kib"] for row in rows), flush=True)
                if owner == "current" and mode == "callbacks":
                    replay_input = model, commands, output, root
        if count == 1024:
            current, parent = (measurements[(owner, "callbacks")]
                               for owner in ("current", "parent"))
            callback_rows = [row for row in report["runs"]
                             if row["declared_targets"] == count and
                             row["owner"] == "current" and row["mode"] == "callbacks"]
            if current > 10 or parent < current * 2:
                raise RuntimeError("1024-target callback median exceeds10s or misses2x parent speed")
            if max(row["rss_kib"] for row in callback_rows) > 256 * 1024:
                raise RuntimeError("callback RSS exceeds256MiB")
        model, commands, output, root = replay_input
        for form, replay_model, replay_commands in (
                ("flat", output.parent / "debug/Top.icl", commands),
                ("generated", model, output)):
            data, _ = run("current", "callbacks", form, count, root / form,
                          replay_model, replay_commands)
            if data != reference["callbacks"]:
                raise RuntimeError(form + " output differs from parent")
    print("primary callback state scale passed", len(report["runs"]), "processes")


if __name__ == "__main__":
    main()
