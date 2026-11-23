#!/usr/bin/env python3
"""Measure mixed caller queues with actual ordinary observations and controls."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import platform
import subprocess
import tempfile
import time

from benchmark_callback_execution import inputs


def fixture(targets, mode):
    source, _, _ = inputs(targets, mode != "ordinary", 4)
    source = source.replace("DataInPort p[3:0];", """DataInPort p[3:0];
DataInPort spare[3:0]; DataOutPort spare_out[3:0] {Source spare;}""")
    body = ("iPDLLevel 1\niProcsForModule Top\n"
            "iProc store {data reg} {iWrite p $data}\niReset\n")
    invoked = mode in ("observed", "observer-off")
    for value in range(1, 5):
        body += "iWrite spare %d\niApply\n" % value
        body += "iWrite spare %d\niRead spare_out 0\n" % value
        if invoked:
            body += "".join("iWrite r%d %d\n" % (index, value) for index in range(targets))
        else:
            body += "iWrite p %d\n" % value
        body += "iApply\n"
        if mode != "observer-off":
            body += "iNote [iGetReadData spare_out -dec]\n"
    return source, body


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def counts(text):
    return {name: sum(line.startswith(name + " ") or line == name
                      for line in text.splitlines())
            for name in ("iWrite", "iRead", "iState", "iApply")}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--binary", type=Path, required=True)
    parser.add_argument("--parent", type=Path, required=True)
    parser.add_argument("--sanitized", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--sizes", type=int, nargs="+", default=[64, 256, 1024])
    parser.add_argument("--repeats", type=int, default=3)
    args = parser.parse_args()
    if args.repeats < 1 or any(n < 1 for n in args.sizes):
        parser.error("positive sizes and repeats required")
    binaries = {name: path.resolve() for name, path in (
        ("current", args.binary), ("parent", args.parent), ("sanitized", args.sanitized))}
    args.output = args.output.resolve()
    args.output.mkdir(parents=True, exist_ok=True)
    report = {"environment": platform.platform(),
              "runtime_environment": {k: os.environ[k] for k in ("ASAN_OPTIONS", "UBSAN_OPTIONS") if k in os.environ},
              "binaries": {name: {"path": str(path), "sha256": digest(path)} for name, path in binaries.items()},
              "runs": []}
    controls = {}

    def run(owner, mode, targets, repeat, form, icl, pdl, original, reference=None):
        root = Path(tempfile.mkdtemp(prefix="%s-%s-%d-%s-" % (owner, mode, targets, form), dir=args.output))
        output, metric = root / "result.pdl", root / "time.txt"
        modeled = mode != "observer-off"
        command = [str(binaries[owner]), "--icl", str(icl), "--pdl", str(pdl),
                   "--top_module", "Top", "--output", str(output), "--debug",
                   *(["--model_readback"] if modeled else [])]
        start = time.monotonic()
        process = subprocess.run(["/usr/bin/time", "-f", "%e %M", "-o", str(metric), *command],
                                 capture_output=True, text=True, timeout=30)
        if process.returncode:
            raise RuntimeError(process.stdout + process.stderr)
        elapsed = time.monotonic() - start
        seconds, rss = metric.read_text().split()
        data = output.read_text()
        notes = [line[6:] for line in data.splitlines() if line.startswith("iNote ") and "modeled " not in line]
        if notes != (["1", "2", "3", "4"] if modeled else []):
            raise RuntimeError("ordinary observation differs: " + repr(notes))
        if reference is not None and data != reference:
            raise RuntimeError(form + " canonical replay differs")
        output_counts = counts(data)
        endpoint = sum(line.startswith("iWrite p ") for line in data.splitlines())
        spare = sum(line.startswith("iWrite spare ") for line in data.splitlines())
        postconditions = sum(line.startswith("iState r") for line in data.splitlines())
        invoked = mode in ("observed", "observer-off")
        applies = 4 * (targets + 2) if invoked else 8
        endpoint_count = 4 * targets if invoked else 4
        pending = False
        pio_frames = 0
        for line in data.splitlines():
            if line.startswith(("iWrite ", "iRead ")):
                pending = True
            elif line.startswith("iApply"):
                pio_frames += pending
                pending = False
        expected_frames = 4 * (targets + 1) if invoked else 8
        if (output_counts["iApply"] != applies or endpoint != endpoint_count or
                spare != 8 or postconditions != (targets * 4 if invoked else 0) or
                pio_frames != expected_frames):
            raise RuntimeError("mixed endpoint/goal/Apply dimensions differ: " +
                               repr((output_counts["iApply"], endpoint, spare, postconditions, pio_frames)))
        if elapsed > 10 or int(rss) > 512 * 1024:
            raise RuntimeError("mixed caller queue exceeds10s/512MiB")
        actual_input = counts(pdl.read_text())
        original_counts = counts(original)
        report["runs"].append({
            "owner": owner, "mode": mode, "targets": targets, "repeat": repeat,
            "form": form, "probe_groups": 4,
            "declared_callback_targets": 0 if mode == "ordinary" else targets,
            "declared_data_register_bits": 4 * targets,
            "primary_input_bits": 8, "scan_storage_bits": 0,
            "original_callback_requests": targets * 4 if invoked else 0,
            "original_request_counts": original_counts,
            "original_operand_bits": 4 * (original_counts["iWrite"] + original_counts["iRead"]),
            "input_request_counts": actual_input,
            "input_operand_bits": 4 * (actual_input["iWrite"] + actual_input["iRead"] + actual_input["iState"]),
            "apply_count": output_counts["iApply"],
            "pio_frame_count": pio_frames,
            "empty_apply_count": output_counts["iApply"] - pio_frames,
            "csu_count": 0, "shifted_bits": 0,
            "endpoint_writes": endpoint, "ordinary_spare_writes": spare,
            "callback_postconditions": postconditions,
            "fresh_getters": pdl.read_text().count("iGetReadData spare_out -dec"),
            "command": command, "wall_seconds": elapsed, "time_seconds": float(seconds),
            "rss_kib": int(rss), "icl_sha256": digest(icl), "pdl_sha256": digest(pdl),
            "output_sha256": digest(output), "output_bytes": len(data.encode())})
        (args.output / "results.json").write_text(json.dumps(report, indent=2) + "\n")
        return data, output

    for owner in ("current", "sanitized", "parent"):
        sizes = [64] if owner == "sanitized" and 64 in args.sizes else ([] if owner == "sanitized" else args.sizes)
        modes = ("observed",) if owner == "sanitized" else (("observed", "ordinary", "unused", "observer-off") if owner == "current" else ("ordinary", "unused"))
        for targets in sizes:
            for mode in modes:
                source, original = fixture(targets, mode)
                root = Path(tempfile.mkdtemp(prefix="fixture-", dir=args.output))
                icl, pdl = root / "source.icl", root / "source.pdl"
                icl.write_text(source)
                pdl.write_text(original)
                reference = None
                for repeat in range(args.repeats):
                    data, output = run(owner, mode, targets, repeat, "source", icl, pdl, original, reference)
                    reference = data
                    if mode in ("ordinary", "unused"):
                        if targets in controls and controls[targets] != data:
                            raise RuntimeError("matched parent/current control differs")
                        controls[targets] = data
                    if mode == "observed":
                        run(owner, mode, targets, repeat, "flat", output.parent / "debug/Top.icl", pdl, original, reference)
                        probe = data.replace("iPDLLevel 0", "iPDLLevel 1", 1)
                        for value in range(1, 5):
                            literal = "iNote %d\n" % value
                            if probe.count(literal) != 1:
                                raise RuntimeError("missing generated observation boundary")
                            probe = probe.replace(literal, "iNote [iGetReadData spare_out -dec]\n", 1)
                        generated = output.parent / "generated-observations.pdl"
                        generated.write_text(probe)
                        run(owner, mode, targets, repeat, "generated", icl, generated, original, reference)
    print(json.dumps({"runs": len(report["runs"]),
                      "max_wall_seconds": max(x["wall_seconds"] for x in report["runs"]),
                      "max_rss_kib": max(x["rss_kib"] for x in report["runs"])}))


if __name__ == "__main__":
    main()
