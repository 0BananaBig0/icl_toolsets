#!/usr/bin/env python3
"""Measure explicit callbacks and matched callback-free/unused controls."""
import argparse
import hashlib
import json
from pathlib import Path
import platform
import statistics
import subprocess
import tempfile
import time


def inputs(targets, callbacks, applies):
    declaration = ("WriteCallBack Top store <D> <R>;" if callbacks else "")
    source = ("Module Top {DataInPort p[3:0];" + "".join(
        "DataRegister r%d[3:0] %s" % (i, "{" + declaration + "}" if callbacks else ";")
        for i in range(targets)) + "}")
    procs = ("iPDLLevel 1\niProcsForModule Top\n"
             "iProc store {data reg} {iWrite p $data}\n")
    control = procs + "".join(
        "iWrite p %d; iApply\n" % (j + 1) for j in range(applies))
    invoked = procs + "".join(
        "".join("iWrite r%d %d;\n" % (i, j + 1) for i in range(targets)) +
        "iApply\n" for j in range(applies))
    return source, control, invoked


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
                           for name, path in binaries.items()}, "runs": []}

    def run(root, owner, mode, targets, icl, pdl):
        output, metric = root / "result.pdl", root / "time.txt"
        command = [str(binaries[owner]), "--icl", str(icl), "--pdl", str(pdl),
                   "--top_module", "Top", "--output", str(output), "--debug"]
        started = time.monotonic()
        result = subprocess.run(
            ["/usr/bin/time", "-f", "%e %M", "-o", str(metric), *command],
            text=True, capture_output=True, timeout=180)
        if result.returncode:
            raise RuntimeError(mode + ": " + result.stdout + result.stderr)
        seconds, rss = metric.read_text().split()
        data = output.read_bytes()
        report["runs"].append({
            "owner": owner, "mode": mode, "targets": targets, "applies": 4,
            "operand_bits_per_apply": targets * 4,
            "wall_seconds": time.monotonic() - started,
            "time_seconds": float(seconds), "rss_kib": int(rss),
            "command": command, "output_bytes": len(data),
            "sha256": hashlib.sha256(data).hexdigest(),
            "icl_sha256": hashlib.sha256(icl.read_bytes()).hexdigest(),
            "pdl_sha256": hashlib.sha256(pdl.read_bytes()).hexdigest()})
        (args.output / "results.json").write_text(json.dumps(report, indent=2) + "\n")
        return data, output

    with tempfile.TemporaryDirectory(prefix="callback-execution-scale-") as temporary:
        root = Path(temporary)
        for targets in (64, 256, 1024):
            reference = None
            for owner, mode in (("parent", "plain"), ("current", "plain"),
                                ("parent", "unused"), ("current", "unused"),
                                ("current", "callbacks")):
                source, control, invoked = inputs(targets, mode != "plain", 4)
                directory = root / (str(targets) + "-" + owner + "-" + mode)
                directory.mkdir()
                icl, pdl = directory / "model.icl", directory / "source.pdl"
                icl.write_text(source)
                pdl.write_text(invoked if mode == "callbacks" else control)
                for repeat in range(args.repeats):
                    data, output = run(directory, owner, mode, targets, icl, pdl)
                    if mode == "callbacks":
                        if data.count(b"iState r") != targets * 4:
                            raise RuntimeError("missing callback write postconditions")
                        for value in range(1, 5):
                            if ("iWrite p 0b" + format(value, "04b")).encode() not in data:
                                raise RuntimeError("missing actual endpoint write")
                    elif reference is None:
                        reference = data
                    elif data != reference:
                        raise RuntimeError("parent/current control output differs")
                measurements = report["runs"][-args.repeats:]
                print(targets, owner, mode, "median",
                      round(statistics.median(v["wall_seconds"] for v in measurements), 4),
                      "rss_kib", max(v["rss_kib"] for v in measurements), flush=True)
                if mode == "callbacks":
                    for label, model, commands in (
                            ("flat", directory / "debug/Top.icl", pdl),
                            ("generated", icl, output)):
                        replay = directory / label
                        replay.mkdir()
                        replay_data, _ = run(replay, "current", label, targets, model, commands)
                        if replay_data != data:
                            raise RuntimeError(label + " callback replay differs")
    print("callback scale controls passed", len(report["runs"]), "processes")


if __name__ == "__main__":
    main()
