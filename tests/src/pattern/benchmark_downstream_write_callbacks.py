#!/usr/bin/env python3
"""Measure transparent callback prerequisites against explicit-before controls."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import platform
import subprocess
import tempfile
import time

from benchmark_mixed_callback_groups import counts, digest
from benchmark_mixed_callback_aliases import operand_bits


def fixture(targets, mode, width=4):
    model = "Module Chains {DataInPort en; DataInPort p[3:0];\n"
    for index in range(targets):
        model += ("DataRegister r%d[%d:0] {WriteCallBack Chains store <D> <R>;}\n"
                  "DataRegister d%d[%d:0] {WriteDataSource r%d; WriteEnSource en;}\n"
                  % (index, width - 1, index, width - 1, index))
    model += "}\n"
    body = """iPDLLevel 1
iProcsForModule Chains
set ::calls 0
iProc store {data reg} {
 incr ::calls; iNote "$reg=$data"; iWrite en 0; iWrite p [expr {$data & 15}]
}
iReset
"""
    for value in range(1, 5):
        if mode == "unused":
            body += "iWrite en 0\niWrite p %d\niApply\n" % value
            continue
        if mode == "explicit":
            for index in range(targets):
                body += "iWrite r%d %d\n" % (index, value)
            body += "iApply\n"
        for index in range(targets):
            body += "iWrite d%d %d\n" % (index, value)
        body += "iApply\nif {$::calls != %d} {error callback_count}\n" % (
            value * targets)
    if mode == "unused":
        body += "if {$::calls != 0} {error unused_callback}\n"
    return model, body


def physical_counts(text, targets, width, invoked):
    frames, pending, notes = [], [], []
    postconditions = []
    low = high = endpoints = empty = 0
    callback_names = {"r%d" % index for index in range(targets)}
    for line in text.splitlines():
        words = line.split()
        if not words:
            continue
        command = words[0]
        if command == "iScan":
            raise RuntimeError("unexpected serial transaction in PIO fixture")
        if command == "iWrite":
            if len(words) != 3 or words[1] not in ("en", "p"):
                raise RuntimeError("unexpected physical write: " + line)
            bits = words[2]
            if (not bits.startswith("0b") or
                    len(bits) != (3 if words[1] == "en" else 6) or
                    set(bits[2:]) - set("01")):
                raise RuntimeError("invalid physical payload: " + line)
            pending.append(words[1:])
            low += words[1:] == ["en", "0b0"]
            high += words[1:] == ["en", "0b1"]
            endpoints += words[1] == "p"
        elif command == "iState":
            if (len(words) != 3 or words[1] not in callback_names or
                    not words[2].startswith("0b") or len(words[2]) != width + 2):
                raise RuntimeError("unexpected callback postcondition: " + line)
            postconditions.append((words[1], words[2], len(frames)))
        elif command == "iApply":
            if pending:
                frames.append(pending)
                pending = []
            else:
                empty += 1
        elif command == "iNote":
            notes.append(line[6:])
        elif command not in ("iPDLLevel", "iProcsForModule", "iReset"):
            raise RuntimeError("unexpected output command: " + line)
    if pending:
        raise RuntimeError("unapplied physical writes")
    expected_postconditions = [
        ("r%d" % index, "0b" + format(value, "0%db" % width),
         (value - 1) * (targets + 1) + index + 1)
        for value in range(1, 5) for index in range(targets)] if invoked else []
    expected_notes = [name + "=" + value for name, value, _ in expected_postconditions]
    if notes != expected_notes or postconditions != expected_postconditions:
        raise RuntimeError("callback arguments/order/postconditions differ")
    expected_frames = []
    for value in range(1, 5):
        callback_frame = [["en", "0b0"], ["p", "0b" + format(value, "04b")]]
        expected_frames.extend([callback_frame] * (targets if invoked else 1))
        if invoked:
            expected_frames.append([["en", "0b1"]])
    if frames != expected_frames:
        raise RuntimeError("physical frame sequence differs")
    return {"apply_count": len(frames) + empty,
            "pio_frame_count": len(frames), "empty_apply_count": empty,
            "ordinary_endpoint_writes": endpoints, "enable_low_writes": low,
            "enable_high_writes": high,
            "destination_writes": high * targets,
            "logical_callback_events": len(notes),
            "callback_postconditions": len(postconditions),
            "csu_count": 0, "shifted_bits": 0,
            "pio_only_verified": True}, frames


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ("binary", "parent", "sanitized", "output"):
        parser.add_argument("--" + name, type=Path, required=True)
    parser.add_argument("--sizes", type=int, nargs="*", default=[16, 64, 128])
    parser.add_argument("--wide-widths", type=int, nargs="*", default=[512, 1024])
    parser.add_argument("--repeats", type=int, default=3)
    args = parser.parse_args()
    if (args.repeats < 1 or any(n < 1 for n in args.sizes) or
            any(n < 4 for n in args.wide_widths) or
            not (args.sizes or args.wide_widths)):
        parser.error("positive sizes/repeats and widths >=4 required")
    binaries = {name: path.resolve() for name, path in (
        ("current", args.binary), ("parent", args.parent),
        ("sanitized", args.sanitized))}
    args.output = args.output.resolve()
    args.output.mkdir(parents=True, exist_ok=True)
    report = {"environment": platform.platform(),
              "runtime_environment": {name: os.environ[name] for name in (
                  "ASAN_OPTIONS", "UBSAN_OPTIONS") if name in os.environ},
              "binaries": {name: {"path": str(path), "sha256": digest(path)}
                           for name, path in binaries.items()},
              "runs": [], "wide_runs": []}
    controls = {}

    def run(owner, mode, targets, width, repeat, form, icl, pdl, original,
            reference=None, wide=False):
        root = Path(tempfile.mkdtemp(
            prefix="%s-%s-%d-%s-" % (owner, mode, targets, form), dir=args.output))
        output, metric = root / "result.pdl", root / "time.txt"
        command = [str(binaries[owner]), "--icl", str(icl), "--pdl", str(pdl),
                   "--top_module", "Chains", "--output", str(output), "--debug"]
        started = time.monotonic()
        process = subprocess.run(
            ["/usr/bin/time", "-f", "%e %M", "-o", str(metric), *command],
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
        dimensions, frames = physical_counts(data, targets, width, invoked)
        expected_empty = 4 if mode == "explicit" else 0
        if dimensions["empty_apply_count"] != expected_empty:
            raise RuntimeError("unexpected empty Apply count")
        key = mode == "unused", targets, width
        if key in controls and controls[key] != frames:
            raise RuntimeError("matched physical control differs")
        controls[key] = frames
        cap = (256 if wide else 512) * 1024
        if elapsed > 10 or int(rss) > cap:
            raise RuntimeError("%s measured %.4fs/%dKiB; bound10s/%dKiB" %
                               (root, elapsed, int(rss), cap))
        widths = {"en": 1, "p": 4,
                  **{"r%d" % n: width for n in range(targets)},
                  **{"d%d" % n: width for n in range(targets)}}
        row = {"owner": owner, "mode": mode, "targets": targets,
               "width": width, "repeat": repeat, "form": form, "probe_groups": 4,
               "connected_callback_bits": targets * width,
               "connected_destination_bits": targets * width,
               "primary_input_bits": 5, "scan_storage_bits": 0,
               "original_callback_requests": 4 * targets if invoked else 0,
               "explicit_callback_requests": 4 * targets if mode == "explicit" else 0,
               "inferred_callback_requests": 4 * targets if mode == "automatic" else 0,
               "original_destination_requests": 4 * targets if invoked else 0,
               "callback_invocations": (4 * targets if invoked and
                                        form != "generated" else 0),
               "original_request_counts": counts(original),
               "input_request_counts": counts(pdl.read_text()),
               "original_operand_bits": operand_bits(original, widths),
               "input_operand_bits": operand_bits(pdl.read_text(), widths),
               "generated_notes_are_literal": form == "generated",
               "command": command, "wall_seconds": elapsed,
               "time_seconds": float(seconds), "rss_kib": int(rss),
               "icl_sha256": digest(icl), "pdl_sha256": digest(pdl),
               "output_sha256": digest(output),
               "physical_frames_sha256": hashlib.sha256(
                   json.dumps(frames, separators=(",", ":")).encode()).hexdigest(),
               **dimensions}
        report["wide_runs" if wide else "runs"].append(row)
        (args.output / "results.json").write_text(json.dumps(report, indent=2) + "\n")
        return data, output

    for owner in ("current", "sanitized", "parent"):
        sizes = ([min(args.sizes)] if args.sizes else []) if owner == "sanitized" else args.sizes
        modes = (("automatic",) if owner == "sanitized" else
                 ("automatic", "explicit", "unused") if owner == "current" else
                 ("explicit", "unused"))
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
                    if mode == "automatic":
                        run(owner, mode, targets, 4, repeat, "flat",
                            output.parent / "debug/Chains.icl", pdl, original, reference)
                        run(owner, mode, targets, 4, repeat, "generated",
                            icl, output, original, reference)
    for width in args.wide_widths:
        source, original = fixture(1, "automatic", width)
        root = Path(tempfile.mkdtemp(prefix="wide-fixture-", dir=args.output))
        icl, pdl = root / "source.icl", root / "source.pdl"
        icl.write_text(source)
        pdl.write_text(original)
        reference = None
        for repeat in range(args.repeats):
            reference, _ = run("current", "automatic", 1, width, repeat, "source",
                               icl, pdl, original, reference, wide=True)
    rows = report["runs"] + report["wide_runs"]
    print(json.dumps({"runs": len(report["runs"]), "wide_runs": len(report["wide_runs"]),
                      "max_wall_seconds": max(x["wall_seconds"] for x in rows),
                      "max_rss_kib": max(x["rss_kib"] for x in rows)}))


if __name__ == "__main__":
    main()
