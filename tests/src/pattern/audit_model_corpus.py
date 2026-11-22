#!/usr/bin/env python3
"""Audit actual default-model roundtrips against explicit corpus acceptance."""
import argparse
import json
from pathlib import Path
import subprocess
import sys
import time

ROOT = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(ROOT / "tests"))
from audit_icl_corpus import source_paths
from import_eda_corpus import digest

STAGES = {"elaborate", "serialize", "flat_load", "flat_elaborate", "canonical"}


def audit(root, corpus_report, inventory, registry):
    started = time.monotonic()
    root = Path(root).resolve()
    inventory = Path(inventory).resolve()
    corpus_bytes, registry_bytes = Path(corpus_report).read_bytes(), Path(registry).read_bytes()
    corpus, policy = json.loads(corpus_bytes), json.loads(registry_bytes)
    fixtures = corpus["fixtures"]
    paths = {path.relative_to(root).as_posix(): path for path in source_paths(root)}
    if set(paths) != set(fixtures) or corpus["files"] != len(paths):
        raise ValueError("corpus file coverage differs; regenerate parser report")
    inputs = {}
    for name, path in paths.items():
        inputs[name] = digest(path.read_bytes())
        if inputs[name] != fixtures[name]["sha256"]:
            raise ValueError("stale corpus input: " + name)
    rejected = set(corpus["rejected"])
    if not rejected <= set(fixtures):
        raise ValueError("rejected corpus file coverage differs")
    names = sorted(set(paths) - rejected)
    expected = {}
    for name in names:
        keys = fixtures[name].get("module_keys")
        if not isinstance(keys, list) or any(not isinstance(key, str) or not key for key in keys):
            raise ValueError("missing module keys; regenerate parser report: " + name)
        expected[name] = set(keys)
    required, unsupported = policy["required"], policy["unsupported"]
    if (policy["schema"] != 1 or not isinstance(required, list) or
            any(not isinstance(key, str) for key in required) or
            len(set(required)) != len(required) or not isinstance(unsupported, dict) or
            set(required) & set(unsupported)):
        raise ValueError("invalid model acceptance registry")
    for boundary in unsupported.values():
        if (boundary.get("stage") != "serialize" or
                not isinstance(boundary.get("diagnostic"), str) or not boundary["diagnostic"]):
            raise ValueError("unsupported registry entries require explicit serialization diagnostics")

    command = [str(inventory), *[str(paths[name]) for name in names]]
    process = subprocess.run(command, capture_output=True, text=True,
                             encoding="utf-8", errors="surrogateescape", timeout=45)
    if process.returncode:
        raise ValueError("inventory failed: " + process.stderr)
    lines = process.stdout.splitlines()
    if len(lines) != len(names):
        raise ValueError("inventory file coverage is incomplete")
    records = [json.loads(line) for line in lines]
    results, errors = {}, []
    for name, record in zip(names, records):
        if not isinstance(record, dict) or record.get("path") != str(paths[name]):
            raise ValueError("inventory file coverage/order differs: " + name)
        modules = record.get("modules")
        if not isinstance(modules, list):
            raise ValueError("inventory module coverage missing: " + name)
        if "load_error" in record:
            if modules or not isinstance(record["load_error"], str) or not record["load_error"]:
                raise ValueError("invalid load-error protocol: " + name)
            errors.append("file load failed: " + name + ": " + record["load_error"])
            continue
        keys = [module.get("name") for module in modules if isinstance(module, dict)]
        if (len(keys) != len(modules) or any(not isinstance(key, str) for key in keys) or
                len(set(keys)) != len(keys) or set(keys) != expected[name]):
            raise ValueError("inventory module coverage differs: " + name)
        for module in modules:
            stage, ok = module.get("stage"), module.get("ok")
            if stage not in STAGES or type(ok) is not bool or (ok and stage != "canonical"):
                raise ValueError("invalid inventory stage/result: " + name)
            if ok:
                if (type(module.get("nodes")) is not int or module["nodes"] < 0 or
                        type(module.get("flat_bytes")) is not int or module["flat_bytes"] < 1):
                    raise ValueError("invalid canonical inventory counts: " + name)
            elif not isinstance(module.get("error"), str) or not module["error"]:
                raise ValueError("invalid inventory failure: " + name)
            results[name + "::" + module["name"]] = module

    for key in required:
        if key not in results:
            errors.append("missing required canonical model: " + key)
        elif not results[key]["ok"]:
            errors.append("required canonical model failed: " + key)
    known = 0
    for key, boundary in unsupported.items():
        actual = results.get(key)
        if (actual is None or actual["ok"] or actual["stage"] != boundary["stage"] or
                boundary["diagnostic"] not in actual["error"]):
            errors.append("unsupported boundary changed or missing: " + key)
        else:
            known += 1
    for key, actual in results.items():
        if actual["ok"]:
            continue
        if "map::at" in actual["error"]:
            errors.append("internal map::at diagnostic: " + key + ": " + actual["error"])
        if actual["stage"] != "elaborate" and key not in unsupported:
            errors.append("unexpected " + actual["stage"] + ": " + key + ": " + actual["error"])
    return {
        "ok": not errors, "files": len(names), "parser_rejected_files": len(rejected),
        "effective_modules": len(results),
        "canonical": sum(module["ok"] for module in results.values()),
        "unsupported": known,
        "elaboration_failures": sum(not module["ok"] and module["stage"] == "elaborate"
                                    for module in results.values()),
        "errors": errors, "inputs": inputs, "results": results,
        "inventory": str(inventory), "inventory_sha256": digest(inventory.read_bytes()),
        "corpus_report_sha256": digest(corpus_bytes), "registry_sha256": digest(registry_bytes),
        "command": command, "wall_seconds": time.monotonic() - started,
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=ROOT / "tests")
    parser.add_argument("--corpus-report", type=Path, required=True)
    parser.add_argument("--inventory", type=Path, default=ROOT / "build/pattern_netlist_inventory")
    parser.add_argument("--registry", type=Path, default=ROOT / "tests/cases/model_source_acceptance.json")
    parser.add_argument("--report", type=Path, required=True)
    args = parser.parse_args()
    try:
        report = audit(args.root, args.corpus_report, args.inventory, args.registry)
    except (OSError, ValueError, KeyError, TypeError, subprocess.SubprocessError) as error:
        report = {"ok": False, "errors": [str(error)]}
    args.report.parent.mkdir(parents=True, exist_ok=True)
    args.report.write_text(json.dumps(report, indent=2) + "\n")
    for error in report["errors"]:
        print(error, file=sys.stderr)
    print("model corpus:", "PASS" if report["ok"] else "FAIL",
          "files", report.get("files", 0), "modules", report.get("effective_modules", 0),
          "canonical", report.get("canonical", 0), "unsupported", report.get("unsupported", 0),
          "original elaboration failures", report.get("elaboration_failures", 0))
    return 0 if report["ok"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
