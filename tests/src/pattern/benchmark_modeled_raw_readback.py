#!/usr/bin/env python3
"""Reproducible parent/default/unused/queried raw-history scale controls."""
import argparse
import hashlib
import json
from pathlib import Path
import platform
import statistics
import subprocess
import tempfile
import time


def inputs(width, tap=False, child=False):
    controls = ("SelectPort sel; CaptureEnPort ce; ShiftEnPort se; "
                "UpdateEnPort ue; TCKPort tck; ResetPort reset;")
    ports = "Port sel; Port ce; Port se; Port ue; Port tck; Port reset;"
    source = ("Module Top {ScanInPort si; ScanOutPort so {Source r[0];}" +
              controls + "ScanRegister r[" + str(width - 1) +
              ":0] {ScanInSource si; CaptureSource r; ResetValue 0;}"
              "ScanInterface client {Port si; Port so;" + ports + "}}")
    if child:
        leaf = source.replace("Module Top", "Module RawLeaf").replace(
            "ScanInterface client", "ScanInterface local")
        root = ("Module Top {ScanInPort si;" + controls +
                "ScanRegister head[2:0] {ScanInSource si; CaptureSource head; ResetValue 0;}"
                "Instance child Of RawLeaf {InputPort si = head[0]; InputPort sel = sel;"
                "InputPort ce = ce; InputPort se = se; InputPort ue = ue;"
                "InputPort tck = tck; InputPort reset = reset;}"
                "ScanRegister tail[4:0] {ScanInSource child.so; CaptureSource tail; ResetValue 0;}"
                "ScanOutPort so {Source tail[0];}"
                "ScanInterface client {Port si; Port so;" + ports + "}}")
        return leaf + root, "Top", "child.local", []
    if tap:
        root = Path(__file__).resolve().parents[3]
        fixture = root / "tests/cases/ours/pattern_generation/raw_tap_ir"
        source = (fixture / "model.icl").read_text().replace(
            "r[7:0]", "r[" + str(width - 1) + ":0]").replace("8'b0", "0").replace(
            "ScanInSource si; ResetValue", "ScanInSource si; CaptureSource r; ResetValue")
        return source, "RawIrTop", "jtag", [
            "--bsdl", "RawIrProbe", str(fixture / "model.bsdl")]
    return source, "Top", "client", []


def opaque_inputs(chains, width):
    ports = "ScanInPort si[" + str(chains - 1) + ":0]; ScanOutPort so[" + str(chains - 1) + ":0];"
    source = "Module Top {" + ports + "SelectPort sel; ScanInterface client {Port sel;"
    for i in range(chains):
        source += ("Chain c" + str(i) + " {Port si[" + str(i) + "];Port so[" +
                   str(i) + "];DefaultLoadValue " + str(width) + "'b0;}")
    return source + "}}", "Top", "client", []


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--binary", type=Path, required=True)
    parser.add_argument("--parent", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--repeats", type=int, default=3)
    args = parser.parse_args()
    if args.repeats < 1:
        parser.error("--repeats must be positive")
    args.output.mkdir(parents=True, exist_ok=True)
    binaries = {name: path.resolve() for name, path in
                (("parent", args.parent), ("current", args.binary))}
    report = {"environment": platform.platform(),
              "binaries": {name: {"path": str(path),
                                  "sha256": hashlib.sha256(path.read_bytes()).hexdigest()}
                           for name, path in binaries.items()},
              "runs": []}
    cases = [("root32", 32, 1, 20, False),
             ("root512", 512, 1, 12, False),
             ("child512", 512, 1, 12, False),
             ("tap512", 512, 1, 12, True),
             ("opaque32x4096", 4096, 32, 8, False)]
    with tempfile.TemporaryDirectory(prefix="modeled-raw-scale-") as temporary:
        root = Path(temporary)
        for case, width, chains, applies, tap in cases:
            opaque = chains > 1
            source, top, target, extra = (opaque_inputs(chains, width) if opaque
                                         else inputs(width, tap, case.startswith("child")))
            icl = root / "model.icl"
            icl.write_text(source)
            for owner, mode in [("parent", "default"), ("current", "default"),
                                ("current", "unused"), ("current", "queried")]:
                if opaque and owner == "parent":
                    continue  # Parent rejects this newly supported topology.
                body = "iPDLLevel 1; iReset;\n"
                if tap:
                    body += "iScan -ir jtag 4 -si 3; iApply;\n"
                for _ in range(applies):
                    for chain in range(chains):
                        select = " -chain c" + str(chain) if opaque else ""
                        body += ("iScan " + target + select + " " + str(width) +
                                 " -si 9 -stable;\n")
                    body += "iApply;\n"
                    if mode == "queried":
                        select = " -chain c0" if opaque else ""
                        body += "iNote [iGetReadData " + target + select + " -bin];\n"
                pdl = root / "source.pdl"
                pdl.write_text(body)
                for repeat in range(args.repeats):
                    output, metric = root / "result.pdl", root / "time.txt"
                    command = [str(binaries[owner]), "--icl", str(icl), "--pdl",
                               str(pdl), "--top_module", top, "--output", str(output), *extra]
                    if mode != "default":
                        command.append("--model_readback")
                    start = time.monotonic()
                    result = subprocess.run(["/usr/bin/time", "-f", "%e %M", "-o",
                                             str(metric), *command],
                                            text=True, capture_output=True, timeout=180)
                    if result.returncode:
                        raise RuntimeError(case + " " + owner + " " + mode + ": " +
                                           result.stdout + result.stderr)
                    seconds, rss = metric.read_text().split()
                    data = output.read_bytes()
                    if mode == "queried":
                        expected = b"iNote 0b" + (b"x" * width if opaque
                                  else format(9, "0" + str(width) + "b").encode())
                        if expected not in data:
                            raise RuntimeError("actual raw-history check failed: " + case)
                    report["runs"].append({
                        "case": case, "owner": owner, "mode": mode, "repeat": repeat,
                        "bits_per_apply": width * chains, "applies": applies,
                        "wall_seconds": time.monotonic() - start,
                        "time_seconds": float(seconds), "rss_kib": int(rss),
                        "command": command, "output_bytes": len(data),
                        "sha256": hashlib.sha256(data).hexdigest()})
                    (args.output / "results.json").write_text(json.dumps(report, indent=2) + "\n")
                values = report["runs"][-args.repeats:]
                print(case, owner, mode,
                      "median", round(statistics.median(v["wall_seconds"] for v in values), 4),
                      "rss_kib", max(v["rss_kib"] for v in values), flush=True)
    print("raw scale controls passed", len(report["runs"]), "processes")


if __name__ == "__main__":
    main()
