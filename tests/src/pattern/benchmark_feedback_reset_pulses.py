#!/usr/bin/env python3
"""Retain feedback-reset matrix inputs, actual readback, replay and resources."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import platform
import signal
import subprocess
import sys
import time


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("binary", type=Path)
    parser.add_argument("parent", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()
    binary, parent, output = (p.resolve() for p in (args.binary, args.parent, args.output))
    output.mkdir(parents=True, exist_ok=True)
    # The existing CLI fixture module consumes its binary argument on import.
    sys.argv = [sys.argv[0], str(binary)]
    from test_data_register_feedback import reset_network

    rows = []
    manifest = {"environment": platform.platform(), "python": sys.version,
                "binary": str(binary), "binary_sha256": sha(binary),
                "parent": str(parent), "parent_sha256": sha(parent),
                "driver_sha256": sha(Path(__file__)), "points": rows}

    def run_point(round_, groups, width, depth, variant):
        masked = variant != "candidate"
        executable = parent if variant == "parent-control" else binary
        folder = output / f"r{round_}-g{groups}-w{width}-d{depth}-{variant}"
        folder.mkdir(exist_ok=True)
        source, commands = reset_network(groups, width, depth, masked)
        commands = "iPDLLevel 1; " + commands + "iNote [iGetReadData q -bin];\n"
        icl, pdl, reset = (folder / name for name in ("input.icl", "input.pdl", "reset.pdl"))
        icl.write_text(source)
        pdl.write_text(commands)
        row = dict(round=round_, groups=groups, width=width, depth=depth,
                   variant=variant, forms=[])
        forms = [(icl, pdl, "source"), (folder / "source/debug/Chains.icl", pdl, "flat"),
                 (icl, folder / "source/result.pdl", "generated"),
                 (folder / "source/debug/Chains.icl", folder / "source/result.pdl", "flat-generated"),
                 (folder / "source/debug/Chains.icl", reset, "reset-flat")]
        expected = b"iNote 0b" + (b"1" if masked else b"0") * (groups * depth) + b"\n"
        reference = None
        for model, program, form in forms:
            result = folder / form / "result.pdl"
            metrics = folder / (form + ".time")
            command = ["/usr/bin/time", "-f", "%e %M", "-o", str(metrics),
                       "timeout", "--signal=KILL", "10s", str(executable),
                       "--icl", str(model), "--pdl", str(program), "--top_module", "Chains",
                       "--output", str(result), "--model_readback", "--debug"]
            if form == "source":
                command += ["--local_reset", str(reset)]
            started = time.monotonic()
            proc = subprocess.Popen(command, cwd=folder, stdout=subprocess.PIPE,
                                    stderr=subprocess.STDOUT, start_new_session=True)
            try:
                log, _ = proc.communicate(timeout=15)
            except subprocess.TimeoutExpired:
                os.killpg(proc.pid, signal.SIGKILL)
                log, _ = proc.communicate()
            (folder / (form + ".log")).write_bytes(log)
            values = metrics.read_text().splitlines()[-1].split() if metrics.exists() else []
            seconds, rss = (float(values[0]), int(values[1])) if len(values) == 2 else (None, None)
            item = dict(form=form, command=command, exit=proc.returncode,
                        wall_seconds=time.monotonic() - started, time_seconds=seconds, rss_kib=rss)
            row["forms"].append(item)
            if proc.returncode:
                if variant == "parent-control":
                    assert proc.returncode in (124, 137, -9) or (
                        proc.returncode == 2 and b"resource limit" in log), (folder, log)
                break
            payload = result.read_bytes()
            assert expected in payload, (folder, form, "actual readback differs")
            if reference is None:
                reference = payload
            assert payload == reference, (folder, form, "replay differs")
            item["within_budget"] = seconds is not None and seconds <= 10 and rss <= 512 * 1024
            if not item["within_budget"]:
                if variant == "parent-control":
                    break
                raise AssertionError((folder, item))
        row["complete"] = len(row["forms"]) == 5 and all(
            f["exit"] == 0 and f.get("within_budget", False) for f in row["forms"])
        row["files"] = {str(p.relative_to(folder)): sha(p) for p in folder.rglob("*") if p.is_file()}
        rows.append(row)
        (output / "receipt.json").write_text(json.dumps(manifest, indent=2) + "\n")
        print(folder.name, "complete" if row["complete"] else ("baseline limit" if variant == "parent-control" else "failed"), flush=True)
        if variant != "parent-control":
            assert row["complete"], (folder, row["forms"][-1])
        return reference

    for round_ in range(3):
        for groups in (16, 128):
            for width in (2, 8):
                for depth in (1, 4):
                    run_point(round_, groups, width, depth, "candidate")
                    before = run_point(round_, groups, width, depth, "parent-control")
                    after = run_point(round_, groups, width, depth, "current-control")
                    if before is not None:
                        assert before == after, "completed masked controls differ"
    manifest["complete"] = True
    (output / "receipt.json").write_text(json.dumps(manifest, indent=2) + "\n")


if __name__ == "__main__":
    main()
