#!/usr/bin/env python3
"""Measure modeled raw lanes with scalar/opaque controls and exact replay."""
import argparse
import hashlib
import json
from pathlib import Path
import platform
import subprocess
import tempfile
import time


def inputs(widths, mode):
    if mode == "scalar":
        widths = [sum(widths)]
    model = ["Module Top {"]
    count = len(widths)
    model += ["ScanInPort si[" + str(count - 1) + ":0];",
              "ScanOutPort so[" + str(count - 1) + ":0] {"]
    if mode != "opaque":
        model += ["Source " + ", ".join("r" + str(i) + "[0]"
                                        for i in range(count)) + ";"]
    model += ["}"]
    if mode != "opaque":
        for i, width in enumerate(widths):
            model += [f"ScanRegister r{i}[{width - 1}:0] "
                      f"{{ScanInSource si[{count - 1 - i}]; "
                      f"CaptureSource r{i}; ResetValue 0;}}"]
    model += ["SelectPort sel; CaptureEnPort ce; ShiftEnPort se; "
              "UpdateEnPort ue; TCKPort ck;",
              "ScanInterface client {Port sel; Port ce; Port se; Port ue; Port ck;"]
    if mode == "scalar":
        model += ["Port si[0]; Port so[0];"]
    else:
        for i in range(count):
            default = f"DefaultLoadValue {widths[i]}'b0;" if mode == "opaque" else ""
            model += [f"Chain lane{i} {{Port si[{count - 1 - i}]; "
                      f"Port so[{count - 1 - i}];{default}}}"]
    model += ["}}"]
    program = ["iPDLLevel 1", "iReset"]
    payloads = [("10010110" * ((width + 7) // 8))[:width] for width in widths]
    for stage in range(2):
        for i, width in enumerate(widths):
            if mode == "implicit" and i:
                continue
            target = "client" + ("" if mode == "scalar" else f" -chain lane{i}")
            bits = payloads[i] if stage == 0 else "0" * width
            program += [f"iScan {target} {width} -si 0b{bits} -stable"]
        program += ["iApply"]
        for i, width in enumerate(widths):
            if mode == "implicit" and i:
                continue
            target = "client" + ("" if mode == "scalar" else f" -chain lane{i}")
            expected = "x" * width if mode == "opaque" else (
                "0" * width if stage == 0 else payloads[i])
            program += [f'if {{[iGetReadData {target} -bin] ne "0b{expected}"}} '
                        f'{{error {{actual return mismatch {stage}/{i}}}}}',
                        f"iNote [iGetReadData {target} -hex]"]
    return "\n".join(model) + "\n", "\n".join(program) + "\n"


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--binary", type=Path, required=True)
    parser.add_argument("--parent", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--smoke", action="store_true")
    args = parser.parse_args()
    binary, parent = args.binary.resolve(), args.parent.resolve()
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=True)

    def digest(path):
        return hashlib.sha256(path.read_bytes()).hexdigest()

    report = {"environment": platform.platform(),
              "binary": str(binary), "binary_sha256": digest(binary),
              "parent": str(parent), "parent_sha256": digest(parent), "runs": []}
    shapes = [(f"{count}x{width}", [width] * count)
              for count in (8, 32, 64) for width in (32, 512, 4096)]
    shapes += [("skewed", [262144] + [32] * 63)]
    if args.smoke:
        shapes = [("smoke", [4, 2])]

    def run(mode, form, label, widths, root, model, program):
        root.mkdir(parents=True, exist_ok=True)
        invocation = Path(tempfile.mkdtemp(prefix=form + "-", dir=root))
        result, metrics = invocation / "result.pdl", invocation / "time.txt"
        command = [str(binary), "--icl", str(model), "--pdl", str(program),
                   "--top_module", "Top", "--model_readback", "--debug",
                   "--output", str(result)]
        started = time.monotonic()
        process = subprocess.run(
            ["/usr/bin/time", "-f", "%e %M", "-o", str(metrics), *command],
            text=True, capture_output=True, timeout=60)
        (invocation / "process.log").write_text(process.stdout + process.stderr)
        if process.returncode:
            raise RuntimeError(process.stdout + process.stderr)
        flat = invocation / "debug/Top.icl"
        if not result.is_file() or not flat.is_file():
            raise RuntimeError("missing fresh result or flat model")
        elapsed, rss = metrics.read_text().split()
        wall = time.monotonic() - started
        if wall > 45 or int(rss) > 512 * 1024:
            raise RuntimeError("45s/512MiB representative replay budget exceeded")
        data = result.read_bytes()
        if data.count(b"iApply") != 2:
            raise RuntimeError("raw group did not keep one Apply per CSU")
        expected_scans = 2 if mode == "scalar" else 2 * len(widths)
        if data.count(b"iScan") != expected_scans:
            raise RuntimeError("raw chain fields lost or duplicated")
        report["runs"].append({
            "mode": mode, "form": form, "shape": label,
            "chains": 1 if mode == "scalar" else len(widths),
            "aggregate_bits": sum(widths), "maximum_chain_bits":
                sum(widths) if mode == "scalar" else max(widths),
            "wall_seconds": wall, "time_seconds": float(elapsed),
            "rss_kib": int(rss), "command": command,
            "icl_sha256": digest(model), "pdl_sha256": digest(program),
            "output_sha256": digest(result), "output_bytes": len(data)})
        (output / "results.json").write_text(json.dumps(report, indent=2) + "\n")
        return data, result, flat

    # The prior binary is a negative control, never a parallel passing result.
    root = Path(tempfile.mkdtemp(prefix="parent-", dir=output))
    source, commands = inputs([4, 2], "parallel")
    model, program = root / "model.icl", root / "source.pdl"
    model.write_text(source)
    program.write_text(commands)
    old = subprocess.run([str(parent), "--icl", str(model), "--pdl", str(program),
                          "--top_module", "Top", "--output", str(root / "result.pdl"),
                          "--model_readback"], capture_output=True, text=True, timeout=45)
    message = old.stdout + old.stderr
    (root / "process.log").write_text(message)
    if old.returncode == 0 or "raw scan" not in message or (root / "result.pdl").exists():
        raise RuntimeError("parent negative control did not reject modeled parallel raw")
    report["parent_negative_control"] = {
        "exit": old.returncode, "diagnostic": message, "artifact": str(root)}
    for label, widths in shapes:
        modes = ("parallel", "scalar", "opaque")
        if label == "skewed":
            modes += ("implicit",)
        for mode in modes:
            root = Path(tempfile.mkdtemp(prefix=label + "-" + mode + "-", dir=output))
            source, commands = inputs(widths, mode)
            model, program = root / "model.icl", root / "source.pdl"
            model.write_text(source)
            program.write_text(commands)
            data, result, flat = run(mode, "source", label, widths, root, model, program)
            for form, replay_model, replay_program in (
                    ("flat", flat, program), ("generated", model, result)):
                replay, _, _ = run(mode, form, label, widths, root,
                                   replay_model, replay_program)
                if replay != data:
                    raise RuntimeError(mode + "/" + form + " byte replay differs")
            rows = report["runs"][-3:]
            print(label, mode, "max_seconds",
                  round(max(row["wall_seconds"] for row in rows), 4),
                  "max_rss_kib", max(row["rss_kib"] for row in rows), flush=True)
    print("modeled parallel raw scale passed", len(report["runs"]), "processes")


if __name__ == "__main__":
    main()
