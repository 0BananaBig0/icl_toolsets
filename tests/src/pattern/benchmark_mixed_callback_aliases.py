#!/usr/bin/env python3
"""Measure mixed alias retargeting with separately queued parent controls."""
import argparse
import json
import os
from pathlib import Path
import platform
import subprocess
import tempfile
import time

from benchmark_mixed_callback_groups import counts, digest, fixture as separate_fixture


def fixture(targets, mode):
    source, separate = separate_fixture(targets, "unused" if mode == "unused" else "observed")
    if mode in ("separate", "unused"):
        return source, separate
    alias = "Alias mixed[%d:0] = spare,%s;\n" % (
        4 * (targets + 1) - 1, ",".join("r%d" % n for n in range(targets)))
    position = source.rfind("}")
    source = source[:position] + alias + source[position:]
    body = ("iPDLLevel 1\niProcsForModule Top\n"
            "iProc store {data reg} {iWrite p $data}\niReset\n")
    for value in range(1, 5):
        body += "iWrite spare %d\niApply\n" % value
        body += "iWrite mixed 0b%s\niRead spare_out 0\niApply\n" % (
            format(value, "04b") * (targets + 1))
        if mode != "observer-off":
            body += "iNote [iGetReadData spare_out -dec]\n"
    return source, body


def wide_fixture(width):
    source = """Module Top {
 DataInPort p[3:0]; DataInPort spare[3:0];
 DataOutPort spare_out[3:0] {Source spare;}
 DataRegister word[%d:0] {WriteCallBack Top store <D> <R>;}
 Alias mixed[%d:0]=spare,word;
}
""" % (width - 5, width - 1)
    body = ("iPDLLevel 1\niProcsForModule Top\n"
            "iProc store {data reg} {iWrite p [expr {$data & 15}]}\niReset\n")
    for value in range(1, 5):
        body += "iWrite spare %d\niApply\n" % value
        body += "iWrite mixed 0b%s%s\niRead spare_out 0\niApply\n" % (
            format(value, "04b"), format(value, "0%db" % (width - 4)))
        body += "iNote [iGetReadData spare_out -dec]\n"
    return source, body


def operand_bits(text, widths):
    total = 0
    for line in text.splitlines():
        words = line.split()
        if words and words[0] in ("iWrite", "iRead", "iState"):
            total += widths[words[1]]
    return total


def physical_counts(text, callback_widths):
    applies = pio_frames = endpoints = spare = postconditions = 0
    pending = False
    accesses = []
    for line in text.splitlines():
        words = line.split()
        if not words:
            continue
        command = words[0]
        if command == "iScan":
            raise RuntimeError("unexpected serial scan in the primary-only fixture")
        if command in ("iWrite", "iRead"):
            allowed = ("p", "spare") if command == "iWrite" else ("spare_out",)
            if len(words) != 3 or words[1] not in allowed or len(words[2]) != 6 or not words[2].startswith("0b"):
                raise RuntimeError("unexpected physical access: " + line)
            accesses.append(("pio", len(words[2]) - 2))
            pending = True
            endpoints += command == "iWrite" and words[1] == "p"
            spare += command == "iWrite" and words[1] == "spare"
        elif command == "iState":
            if (len(words) != 3 or words[1] not in callback_widths or
                    not words[2].startswith("0b") or
                    len(words[2]) - 2 != callback_widths[words[1]]):
                raise RuntimeError("unexpected abstract postcondition: " + line)
            postconditions += 1
        elif command == "iApply":
            applies += 1
            pio_frames += pending
            pending = False
        elif command not in ("iPDLLevel", "iProcsForModule", "iReset", "iNote"):
            raise RuntimeError("unexpected output command: " + line)
    if pending:
        raise RuntimeError("unapplied physical access")
    return {"apply_count": applies, "pio_frame_count": pio_frames,
            "empty_apply_count": applies - pio_frames,
            "endpoint_writes": endpoints, "ordinary_spare_writes": spare,
            "callback_postconditions": postconditions,
            "csu_count": sum(kind == "scan" for kind, _ in accesses),
            "shifted_bits": sum(width for kind, width in accesses if kind == "scan"),
            "pio_only_verified": True}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--binary", type=Path, required=True)
    parser.add_argument("--parent", type=Path, required=True)
    parser.add_argument("--sanitized", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--sizes", type=int, nargs="*", default=[64, 256, 1024])
    parser.add_argument("--wide-widths", type=int, nargs="*", default=[4096, 16384])
    parser.add_argument("--repeats", type=int, default=3)
    args = parser.parse_args()
    if (args.repeats < 1 or any(n < 1 for n in args.sizes) or
            any(n < 8 for n in args.wide_widths) or not (args.sizes or args.wide_widths)):
        parser.error("positive target sizes/repeats and alias widths >=8 required")
    binaries = {name: path.resolve() for name, path in (
        ("current", args.binary), ("parent", args.parent), ("sanitized", args.sanitized))}
    args.output = args.output.resolve()
    args.output.mkdir(parents=True, exist_ok=True)
    report = {"environment": platform.platform(),
              "runtime_environment": {name: os.environ[name] for name in ("ASAN_OPTIONS", "UBSAN_OPTIONS") if name in os.environ},
              "binaries": {name: {"path": str(path), "sha256": digest(path)} for name, path in binaries.items()},
              "runs": [], "wide_runs": []}
    controls = {}
    mixed_outputs = {}

    def run(owner, mode, targets, repeat, form, icl, pdl, original,
            callback_widths, alias_width, reference=None, wide=False):
        root = Path(tempfile.mkdtemp(prefix="%s-%s-%d-%s-" % (owner, mode, targets, form), dir=args.output))
        output, metric = root / "result.pdl", root / "time.txt"
        modeled = mode != "observer-off"
        command = [str(binaries[owner]), "--icl", str(icl), "--pdl", str(pdl),
                   "--top_module", "Top", "--output", str(output), "--debug",
                   *(["--model_readback"] if modeled else [])]
        started = time.monotonic()
        process = subprocess.run(["/usr/bin/time", "-f", "%e %M", "-o", str(metric), *command],
                                 capture_output=True, text=True, timeout=30)
        if process.returncode:
            raise RuntimeError(process.stdout + process.stderr)
        elapsed = time.monotonic() - started
        seconds, rss = metric.read_text().split()
        data = output.read_text()
        notes = [line[6:] for line in data.splitlines() if line.startswith("iNote ") and "modeled " not in line]
        if notes != (["1", "2", "3", "4"] if modeled else []):
            raise RuntimeError("actual ordinary observations differ: " + repr(notes))
        if reference is not None and data != reference:
            raise RuntimeError(form + " canonical replay differs")
        dimensions = physical_counts(data, callback_widths)
        invoked = mode != "unused"
        expected = (4 * (targets + 2), 4 * (targets + 1), 4, 4 * targets, 4 * targets) if invoked else (8, 8, 0, 4, 0)
        actual = tuple(dimensions[name] for name in (
            "apply_count", "pio_frame_count", "empty_apply_count", "endpoint_writes", "callback_postconditions"))
        if actual != expected or dimensions["ordinary_spare_writes"] != 8:
            raise RuntimeError("actual frame/endpoint dimensions differ: " + repr(actual))
        bound = 256 * 1024 if wide else 512 * 1024
        if elapsed > 10 or int(rss) > bound:
            raise RuntimeError("mixed alias %s/%s/%d/%s measured %.4fs/%dKiB; "
                               "bound10s/%dKiB; artifacts %s" %
                               (owner, mode, targets, form, elapsed, int(rss), bound, root))
        widths = {"p": 4, "spare": 4, "spare_out": 4, "mixed": alias_width, **callback_widths}
        row = {"owner": owner, "mode": mode, "targets": targets, "repeat": repeat,
               "form": form, "probe_groups": 4, "alias_width": alias_width,
               "declared_ordinary_bits": 4,
               "declared_callback_bits": sum(callback_widths.values()),
               "declared_callback_targets": len(callback_widths),
               "primary_input_bits": 8, "scan_storage_bits": 0,
               "declared_aliases": icl.read_text().count("Alias "),
               "original_callback_requests": 4 * targets if invoked else 0,
               "original_request_counts": counts(original),
               "original_operand_bits": operand_bits(original, widths),
               "input_request_counts": counts(pdl.read_text()),
               "input_operand_bits": operand_bits(pdl.read_text(), widths),
               "fresh_getters": pdl.read_text().count("iGetReadData spare_out -dec"),
               "command": command, "wall_seconds": elapsed, "time_seconds": float(seconds),
               "rss_kib": int(rss), "icl_sha256": digest(icl), "pdl_sha256": digest(pdl),
               "output_sha256": digest(output), "output_bytes": len(data.encode()), **dimensions}
        report["wide_runs" if wide else "runs"].append(row)
        (args.output / "results.json").write_text(json.dumps(report, indent=2) + "\n")
        return data, output

    for owner in ("current", "sanitized", "parent"):
        sizes = ([min(args.sizes)] if args.sizes else []) if owner == "sanitized" else args.sizes
        modes = ("mixed",) if owner == "sanitized" else (("mixed", "separate", "unused", "observer-off") if owner == "current" else ("separate", "unused"))
        for targets in sizes:
            callback_widths = {"r%d" % n: 4 for n in range(targets)}
            for mode in modes:
                source, original = fixture(targets, mode)
                root = Path(tempfile.mkdtemp(prefix="fixture-", dir=args.output))
                icl, pdl = root / "source.icl", root / "source.pdl"
                icl.write_text(source)
                pdl.write_text(original)
                reference = None
                for repeat in range(args.repeats):
                    data, output = run(owner, mode, targets, repeat, "source", icl, pdl,
                                       original, callback_widths, 4 * (targets + 1), reference)
                    reference = data
                    if owner == "current" and mode == "mixed":
                        mixed_outputs[targets] = data
                    if mode in ("separate", "unused"):
                        key = mode, targets
                        if key in controls and controls[key] != data:
                            raise RuntimeError("matched parent/current control differs")
                        controls[key] = data
                        if mode == "separate" and data != mixed_outputs[targets]:
                            raise RuntimeError("mixed and separately queued hardware patterns differ")
                    if mode == "mixed":
                        run(owner, mode, targets, repeat, "flat", output.parent / "debug/Top.icl",
                            pdl, original, callback_widths, 4 * (targets + 1), reference)
                        probe = data.replace("iPDLLevel 0", "iPDLLevel 1", 1)
                        for value in range(1, 5):
                            literal = "iNote %d\n" % value
                            if probe.count(literal) != 1:
                                raise RuntimeError("missing generated observation boundary")
                            probe = probe.replace(literal, "iNote [iGetReadData spare_out -dec]\n", 1)
                        generated = output.parent / "generated-observations.pdl"
                        generated.write_text(probe)
                        run(owner, mode, targets, repeat, "generated", icl, generated,
                            original, callback_widths, 4 * (targets + 1), reference)
    for width in args.wide_widths:
        source, original = wide_fixture(width)
        root = Path(tempfile.mkdtemp(prefix="wide-fixture-", dir=args.output))
        icl, pdl = root / "source.icl", root / "source.pdl"
        icl.write_text(source)
        pdl.write_text(original)
        reference = None
        for repeat in range(args.repeats):
            reference, _ = run("current", "mixed", 1, repeat, "source", icl, pdl,
                               original, {"word": width - 4}, width, reference, wide=True)
    rows = report["runs"] + report["wide_runs"]
    print(json.dumps({"runs": len(report["runs"]), "wide_runs": len(report["wide_runs"]),
                      "max_wall_seconds": max(row["wall_seconds"] for row in rows),
                      "max_rss_kib": max(row["rss_kib"] for row in rows)}))


if __name__ == "__main__":
    main()
