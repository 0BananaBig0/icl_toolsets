#!/usr/bin/env python3
"""Measure merged callback calls against matched serial and unused controls."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import platform
import subprocess
import tempfile
import time

import benchmark_downstream_write_callbacks as base


def fixture(targets, mode, width=4):
    model, text = base.fixture(targets, "unused" if mode == "unused" else "automatic", width)
    if mode != "serial":
        prefix, body = text.split("iReset\n", 1)
        text = (prefix + "iProc access {} {\n" + body +
                "}\niReset\niMerge -begin\niCall access\niMerge -end\n")
    return model, text


def checked_frames(text, targets, width, invoked):
    dimensions, frames = base.physical_counts(text, targets, width, invoked)
    return dict(dimensions, ordered_postconditions_verified=True), frames


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ("binary", "parent", "sanitized", "output"):
        parser.add_argument("--" + name, type=Path, required=True)
    parser.add_argument("--sizes", type=int, nargs="*", default=[16, 64, 128])
    parser.add_argument("--wide-widths", type=int, nargs="*", default=[512, 1024])
    parser.add_argument("--repeats", type=int, default=3)
    args = parser.parse_args()
    if (args.repeats < 1 or any(n < 1 for n in args.sizes) or
            any(n < 4 for n in args.wide_widths) or not (args.sizes or args.wide_widths)):
        parser.error("positive sizes/repeats and widths >=4 required")
    binaries = {name: path.resolve() for name, path in (
        ("current", args.binary), ("parent", args.parent), ("sanitized", args.sanitized))}
    args.output = args.output.resolve()
    args.output.mkdir(parents=True, exist_ok=True)
    report = {"environment": platform.platform(),
              "runtime_environment": {key: os.environ[key] for key in
                                      ("ASAN_OPTIONS", "UBSAN_OPTIONS") if key in os.environ},
              "binaries": {name: {"path": str(path), "sha256": base.digest(path)}
                           for name, path in binaries.items()},
              "runs": [], "wide_runs": []}
    controls = {}

    def run(owner, mode, targets, width, repeat, form, icl, pdl, original,
            reference=None, wide=False):
        root = Path(tempfile.mkdtemp(prefix="%s-%s-%d-%s-" %
                                    (owner, mode, targets, form), dir=args.output))
        output, metric = root / "result.pdl", root / "time.txt"
        command = [str(binaries[owner]), "--icl", str(icl), "--pdl", str(pdl),
                   "--top_module", "Chains", "--output", str(output), "--debug"]
        started = time.monotonic()
        process = subprocess.run(["/usr/bin/time", "-f", "%e %M", "-o", str(metric), *command],
                                 capture_output=True, text=True, timeout=30)
        (root / "process.log").write_text(process.stdout + process.stderr)
        if process.returncode:
            raise RuntimeError(str(root) + "\n" + process.stdout + process.stderr)
        elapsed = time.monotonic() - started
        seconds, rss = metric.read_text().split()
        data = output.read_text()
        if reference is not None and data != reference:
            raise RuntimeError(form + " canonical replay differs")
        invoked = mode != "unused"
        dimensions, frames = checked_frames(data, targets, width, invoked)
        if dimensions["empty_apply_count"]:
            raise RuntimeError("unexpected empty Apply")
        key = not invoked, targets, width
        if key in controls and controls[key] != frames:
            raise RuntimeError("matched physical control differs")
        controls[key] = frames
        cap = (256 if wide else 512) * 1024
        if elapsed > 10 or int(rss) > cap:
            raise RuntimeError("%s measured %.4fs/%dKiB; bound10s/%dKiB" %
                               (root, elapsed, int(rss), cap))
        widths = {"en": 1, "p": 4, **{"r%d" % n: width for n in range(targets)},
                  **{"d%d" % n: width for n in range(targets)}}
        row = {"owner": owner, "mode": mode, "targets": targets, "width": width,
               "repeat": repeat, "form": form, "probe_groups": 4,
               "connected_callback_bits": targets * width,
               "connected_destination_bits": targets * width,
               "primary_input_bits": 5, "scan_storage_bits": 0,
               "original_callback_requests": 4 * targets if invoked else 0,
               "inferred_callback_requests": 4 * targets if invoked else 0,
               "original_destination_requests": 4 * targets if invoked else 0,
               "callback_invocations": 4 * targets if invoked and form != "generated" else 0,
               "original_merge_blocks": int(mode != "serial"),
               "input_merge_blocks": int(mode != "serial" and form != "generated"),
               "original_request_counts": base.counts(original),
               "input_request_counts": base.counts(pdl.read_text()),
               "original_operand_bits": base.operand_bits(original, widths),
               "input_operand_bits": base.operand_bits(pdl.read_text(), widths),
               "generated_notes_are_literal": form == "generated",
               "command": command, "wall_seconds": elapsed,
               "time_seconds": float(seconds), "rss_kib": int(rss),
               "icl_sha256": base.digest(icl), "pdl_sha256": base.digest(pdl),
               "output_sha256": base.digest(output),
               "physical_frames_sha256": hashlib.sha256(json.dumps(
                   frames, separators=(",", ":")).encode()).hexdigest(), **dimensions}
        report["wide_runs" if wide else "runs"].append(row)
        (args.output / "results.json").write_text(json.dumps(report, indent=2) + "\n")
        return data, output

    for owner in ("current", "sanitized", "parent"):
        sizes = ([min(args.sizes)] if args.sizes else []) if owner == "sanitized" else args.sizes
        modes = (("merged",) if owner == "sanitized" else
                 ("merged", "serial", "unused") if owner == "current" else ("serial", "unused"))
        for targets in sizes:
            for mode in modes:
                source, original = fixture(targets, mode)
                root = Path(tempfile.mkdtemp(prefix="fixture-", dir=args.output))
                icl, pdl = root / "source.icl", root / "source.pdl"
                icl.write_text(source)
                pdl.write_text(original)
                reference = None
                for repeat in range(args.repeats):
                    data, output = run(owner, mode, targets, 4, repeat, "source",
                                       icl, pdl, original, reference)
                    reference = data
                    if mode == "merged":
                        run(owner, mode, targets, 4, repeat, "flat",
                            output.parent / "debug/Chains.icl", pdl, original, reference)
                        run(owner, mode, targets, 4, repeat, "generated",
                            icl, output, original, reference)
    for width in args.wide_widths:
        source, original = fixture(1, "merged", width)
        root = Path(tempfile.mkdtemp(prefix="wide-fixture-", dir=args.output))
        icl, pdl = root / "source.icl", root / "source.pdl"
        icl.write_text(source)
        pdl.write_text(original)
        reference = None
        for repeat in range(args.repeats):
            reference, _ = run("current", "merged", 1, width, repeat, "source",
                               icl, pdl, original, reference, wide=True)
    rows = report["runs"] + report["wide_runs"]
    print(json.dumps({"runs": len(report["runs"]), "wide_runs": len(report["wide_runs"]),
                      "max_wall_seconds": max(row["wall_seconds"] for row in rows),
                      "max_rss_kib": max(row["rss_kib"] for row in rows)}))


if __name__ == "__main__":
    main()
