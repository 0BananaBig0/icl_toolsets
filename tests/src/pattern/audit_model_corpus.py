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


def source_snapshot(root):
    paths = {path.relative_to(root).as_posix(): path for path in source_paths(root)}
    return paths, {name: digest(path.read_bytes()) for name, path in paths.items()}


def audit(root, corpus_report, inventory, registry):
    started = time.monotonic()
    root = Path(root).resolve()
    inventory = Path(inventory).resolve()
    corpus_bytes, registry_bytes = Path(corpus_report).read_bytes(), Path(registry).read_bytes()
    corpus, policy = json.loads(corpus_bytes), json.loads(registry_bytes)
    if not isinstance(corpus, dict) or not isinstance(policy, dict):
        raise ValueError("corpus report and registry must be JSON objects")
    fixtures = corpus["fixtures"]
    if not isinstance(fixtures, dict) or any(not isinstance(value, dict) for value in fixtures.values()):
        raise ValueError("corpus fixtures must be JSON objects")
    paths, inputs = source_snapshot(root)
    if set(paths) != set(fixtures) or corpus["files"] != len(paths):
        raise ValueError("corpus file coverage differs; regenerate parser report")
    for name in paths:
        if inputs[name] != fixtures[name]["sha256"]:
            raise ValueError("stale corpus input: " + name)
    rejected_names = corpus["rejected"]
    if (not isinstance(rejected_names, list) or
            any(not isinstance(name, str) for name in rejected_names) or
            len(set(rejected_names)) != len(rejected_names)):
        raise ValueError("invalid or duplicate rejected corpus names")
    rejected = set(rejected_names)
    if not rejected <= set(fixtures):
        raise ValueError("rejected corpus file coverage differs")
    expected, declarations = {}, 0
    for name, fixture in fixtures.items():
        keys, hashes = fixture.get("module_keys"), fixture.get("modules")
        if (not isinstance(keys, list) or not isinstance(hashes, list) or
                any(not isinstance(key, str) or not key for key in keys) or
                any(not isinstance(sha, str) for sha in hashes) or len(keys) != len(hashes)):
            raise ValueError("inconsistent corpus declarations: " + name)
        if name in rejected:
            if keys or hashes:
                raise ValueError("rejected corpus entry has declarations: " + name)
        else:
            expected[name] = set(keys)
            declarations += len(keys)
    if type(corpus["modules"]) is not int or corpus["modules"] != declarations:
        raise ValueError("corpus declaration total differs")
    names = sorted(expected)
    required, unsupported = policy["required"], policy["unsupported"]
    if (policy["schema"] != 1 or not isinstance(required, list) or
            any(not isinstance(key, str) for key in required) or
            len(set(required)) != len(required) or not isinstance(unsupported, dict) or
            set(required) & set(unsupported)):
        raise ValueError("invalid model acceptance registry")
    for boundary in unsupported.values():
        if (not isinstance(boundary, dict) or boundary.get("stage") != "serialize" or
                not isinstance(boundary.get("diagnostic"), str) or not boundary["diagnostic"]):
            raise ValueError("unsupported registry entries require explicit serialization diagnostics")

    command = [str(inventory), *[str(paths[name]) for name in names]]
    inventory_sha = digest(inventory.read_bytes())
    process = subprocess.run(command, capture_output=True, text=True,
                             encoding="utf-8", errors="surrogateescape", timeout=45)
    try:
        _, after = source_snapshot(root)
        inventory_after = digest(inventory.read_bytes())
    except OSError as error:
        raise ValueError("audit inputs changed during inventory") from error
    if after != inputs or inventory_after != inventory_sha:
        raise ValueError("audit inputs changed during inventory")
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
                if "error" in module:
                    raise ValueError("canonical success contains an error: " + name)
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
        diagnostic = boundary["diagnostic"]
        if (actual is None or actual["ok"] or actual["stage"] != boundary["stage"] or
                not (actual["error"] == diagnostic or actual["error"].endswith(": " + diagnostic))):
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
        "inventory": str(inventory), "inventory_sha256": inventory_sha,
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
