#!/usr/bin/env python3
"""Measure opaque child traversal and actual source/flat/generated observations."""
import argparse
import hashlib
import json
from pathlib import Path
import platform
import subprocess
import tempfile
import time

ROOT = Path(__file__).resolve().parents[3]


def model(width=8, kind="opaque"):
    source = (ROOT / "tests/cases/ours/pattern_generation/static_child_scans/model.icl").read_text()
    source = source.replace("value[7:0]", "value[%d:0]" % (width - 1))
    source = source.replace("ResetValue 8'b0", "ResetValue %d'b0" % width)
    source = source.replace("DefaultLoadValue 16'b0", "DefaultLoadValue %d'b0" % (width + 8))
    source = source.replace("Port reset; }", "Port reset; DefaultLoadValue %d'b0; }" % width, 1)
    if kind == "opaque":
        source = source.replace(" ScanRegister value[%d:0] { ScanInSource si; CaptureSource value; ResetValue %d'b0; }\n" % (width - 1, width), "")
        source = source.replace("ScanOutPort so { Source value[0]; }", "ScanOutPort so;")
    elif kind == "visible":
        source = source.replace("CaptureSource value;", "CaptureSource %d'bx; DefaultLoadValue %d'b0;" % (width, width))
        source = source.replace("ResetValue %d'b0" % width, "ResetValue %d'bx" % width)
    elif kind == "root":
        source = """Module Top {
 ScanInPort si; ScanOutPort so; SelectPort sel; CaptureEnPort ce;
 ShiftEnPort se; UpdateEnPort ue; TCKPort tck; ResetPort reset;
 ScanInterface client {Port si; Port so; Port sel; Port ce; Port se;
 Port ue; Port tck; Port reset; DefaultLoadValue %d'b0;}
}""" % (width + 8)
    else:
        raise ValueError(kind)
    return source


def inputs(width, kind="opaque"):
    body = "iPDLLevel 1\niProcsForModule Top\niReset\n"
    if kind != "root":
        body += "iWrite head 5; iWrite tail 19; iApply\n"
    expected = "0b" + ("x" * (width + 8) if kind == "root" else "101" + "x" * width + "10011")
    for group in range(4):
        payload = ("10100101" if group % 2 == 0 else "00111100") * ((width + 7) // 8)
        payload = payload[:width]
        if kind != "root":
            body += "iScan child.local %d -si 0b%s -stable; iApply\n" % (width, payload)
        body += "iScan client %d -si 0b101%s10011 -stable; iApply\n" % (width + 8, payload)
        body += "iNote [iGetReadData client -bin]\n"
    return model(width, kind), body, expected


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--binary", type=Path, required=True)
    parser.add_argument("--parent", type=Path, required=True)
    parser.add_argument("--sanitized", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--repeats", type=int, default=3)
    parser.add_argument("--sizes", type=int, nargs="+", default=[8, 64, 256, 1024])
    args = parser.parse_args()
    if args.repeats < 1 or any(n < 1 for n in args.sizes):
        parser.error("positive sizes and repeats required")
    binaries = {k: p.resolve() for k, p in (("current", args.binary), ("parent", args.parent), ("sanitized", args.sanitized))}
    args.output.mkdir(parents=True, exist_ok=True)
    report = {"environment": platform.platform(), "binaries": {k: {"path": str(p), "sha256": digest(p)} for k, p in binaries.items()}, "runs": []}

    def run(owner, kind, width, repeat, form, icl, pdl, expected, reference=None):
        root = Path(tempfile.mkdtemp(prefix="%s-%s-%d-%s-" % (owner, kind, width, form), dir=args.output))
        output, metric = root / "result.pdl", root / "time.txt"
        command = [str(binaries[owner]), "--icl", str(icl), "--pdl", str(pdl), "--top_module", "Top", "--output", str(output), "--debug", "--model_readback"]
        start = time.monotonic()
        result = subprocess.run(["/usr/bin/time", "-f", "%e %M", "-o", str(metric), *command], capture_output=True, text=True, timeout=30)
        if result.returncode:
            raise RuntimeError(result.stdout + result.stderr)
        elapsed = time.monotonic() - start
        seconds, rss = metric.read_text().split()
        data = output.read_text()
        notes = [line[6:] for line in data.splitlines() if line.startswith("iNote ") and "modeled " not in line]
        if notes != [expected] * 4:
            raise RuntimeError("four actual full-root observations differ: " + repr(notes))
        if "__opaque_scan_state" in data or "__opaque_scan_state" in (root / "debug/Top.icl").read_text():
            raise RuntimeError("private storage escaped")
        if reference is not None and data != reference:
            raise RuntimeError(form + " canonical replay differs")
        if elapsed > 10 or int(rss) > 512 * 1024:
            raise RuntimeError("opaque child exceeds10s/512MiB")
        report["runs"].append({"owner": owner, "kind": kind, "opaque_width": width, "root_width": width + 8, "visible_parent_bits": 0 if kind == "root" else 8, "outer_probe_groups": 4, "repeat": repeat, "form": form, "command": command, "wall_seconds": elapsed, "time_seconds": float(seconds), "rss_kib": int(rss), "icl_sha256": digest(icl), "pdl_sha256": digest(pdl), "output_sha256": digest(output), "output_bytes": len(data.encode()), "fresh_getters": pdl.read_text().count("iGetReadData client -bin")})
        (args.output / "results.json").write_text(json.dumps(report, indent=2) + "\n")
        return data, output

    for owner in ("current", "sanitized", "parent"):
        sizes = [n for n in args.sizes if n in (8, 64)] if owner == "sanitized" else args.sizes
        kinds = ["opaque"] if owner == "sanitized" else (["opaque", "visible", "root"] if owner == "current" else ["visible", "root"])
        for width in sizes:
            for kind in kinds:
                source, body, expected = inputs(width, kind)
                fixture = Path(tempfile.mkdtemp(prefix="fixture-", dir=args.output))
                icl, pdl = fixture / "source.icl", fixture / "source.pdl"
                icl.write_text(source)
                pdl.write_text(body)
                reference = None
                for repeat in range(args.repeats):
                    data, output = run(owner, kind, width, repeat, "source", icl, pdl, expected, reference)
                    reference = data
                    if kind == "opaque":
                        run(owner, kind, width, repeat, "flat", output.parent / "debug/Top.icl", pdl, expected, reference)
                        probe = data.replace("iPDLLevel 0", "iPDLLevel 1", 1)
                        literal = "iNote " + expected + "\n"
                        if probe.count(literal) != 4:
                            raise RuntimeError("missing four generated probe boundaries")
                        probe = probe.replace(literal, "iNote [iGetReadData client -bin]\n")
                        generated = output.parent / "generated-observations.pdl"
                        generated.write_text(probe)
                        run(owner, kind, width, repeat, "generated", icl, generated, expected, reference)
    print(json.dumps({"runs": len(report["runs"]), "max_wall_seconds": max(r["wall_seconds"] for r in report["runs"]), "max_rss_kib": max(r["rss_kib"] for r in report["runs"])}))


if __name__ == "__main__":
    main()
