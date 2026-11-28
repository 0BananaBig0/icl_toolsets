#!/usr/bin/env python3
"""Audit actual model defaults and reject incomplete or stale evidence."""
import importlib.util
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
from types import SimpleNamespace
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[4]
sys.path.insert(0, str(ROOT / "tests"))
import audit_icl_corpus as parser_audit

SPEC = importlib.util.spec_from_file_location(
    "model_audit", ROOT / "tests/src/pattern/audit_model_corpus.py")
audit_module = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(audit_module)
INVENTORY = Path(os.environ.get("MODEL_INVENTORY", ROOT / "build/pattern_netlist_inventory"))
PARSER_INVENTORY = Path(os.environ.get("PARSER_INVENTORY", ROOT / "build/icl_corpus_inventory"))


class ModelAuditTest(unittest.TestCase):
    def setUp(self):
        temp = tempfile.TemporaryDirectory()
        self.addCleanup(temp.cleanup)
        self.root = Path(temp.name)
        sources = {
            "a.icl": "NameSpace N; Module A {} Module A {DataInPort p;} Module B {}",
            "b.icl": "NameSpace Q; Module A {ToSelectPort out;}",
            "c.icl": "Module Incomplete {Instance child Of Absent;}",
            "d.icl": "Module Opaque {AccessLink tap Of Generic {opaque;}}",
        }
        for name, source in sources.items():
            (self.root / name).write_text(source)
        self.corpus = self.root / "corpus.json"
        self.corpus.write_text(json.dumps(parser_audit.audit(
            self.root, PARSER_INVENTORY)))
        self.registry = self.root / "registry.json"
        self.policy = {
            "schema": 1,
            "required": ["a.icl::N::A", "a.icl::N::B", "b.icl::Q::A"],
            "unsupported": {
                "d.icl::Opaque": {
                    "stage": "serialize",
                    "diagnostic": "flat serialization is not implemented for AccessLink",
                }
            },
        }
        self.registry.write_text(json.dumps(self.policy))

    def audit(self):
        return audit_module.audit(self.root, self.corpus, INVENTORY, self.registry)

    def records(self):
        result = subprocess.run(
            [str(INVENTORY), *[str(self.root / name) for name in ("a.icl", "b.icl", "c.icl", "d.icl")]],
            capture_output=True, text=True, check=True)
        return [json.loads(line) for line in result.stdout.splitlines()]

    def fake(self, records, returncode=0):
        return SimpleNamespace(returncode=returncode,
                               stdout="\n".join(json.dumps(row) for row in records),
                               stderr="backend failure" if returncode else "")

    def test_real_batch_keeps_namespaces_overrides_and_boundaries(self):
        report = self.audit()
        self.assertTrue(report["ok"], report["errors"])
        self.assertEqual((report["files"], report["effective_modules"],
                          report["canonical"], report["unsupported"],
                          report["elaboration_failures"]), (4, 5, 3, 1, 1))
        self.assertEqual(len(report["inputs"]), 4)
        self.assertEqual(len(report["inventory_sha256"]), 64)

    @unittest.skipUnless(Path("/dev/full").exists(), "/dev/full is required")
    def test_short_inventory_detects_buffered_stdout_failure(self):
        with open("/dev/full", "wb") as output:
            result = subprocess.run(
                [str(INVENTORY), str(self.root / "b.icl")], stdout=output,
                stderr=subprocess.PIPE, text=True, timeout=45)
        self.assertEqual(result.returncode, 2, result.stderr)
        self.assertIn("cannot write inventory", result.stderr)

    def test_stale_missing_and_unlisted_input_are_rejected(self):
        (self.root / "a.icl").write_text("Module Different {}")
        with patch.object(audit_module.subprocess, "run") as backend:
            with self.assertRaisesRegex(ValueError, "stale corpus input"):
                self.audit()
            backend.assert_not_called()
        (self.root / "a.icl").unlink()
        with self.assertRaises((ValueError, OSError)):
            self.audit()

    def test_new_source_file_is_not_hidden_by_old_report(self):
        (self.root / "new.icl").write_text("Module Hidden {}")
        with patch.object(audit_module.subprocess, "run") as backend:
            with self.assertRaisesRegex(ValueError, "corpus file coverage"):
                self.audit()
            backend.assert_not_called()

    def test_missing_duplicate_or_wrong_module_coverage_rejected(self):
        records = self.records()
        variants = [
            records[:-1],
            records + [records[0]],
            [dict(records[0], modules=records[0]["modules"][:-1]), *records[1:]],
            [dict(records[0], path=str(self.root / "other.icl")), *records[1:]],
        ]
        for value in variants:
            with self.subTest(value=value):
                with patch.object(audit_module.subprocess, "run", return_value=self.fake(value)):
                    with self.assertRaisesRegex(ValueError, "coverage"):
                        self.audit()

    def test_required_missing_and_failed_canonical_are_not_success(self):
        self.policy["required"].append("absent.icl::Missing")
        self.registry.write_text(json.dumps(self.policy))
        report = self.audit()
        self.assertFalse(report["ok"])
        self.assertTrue(any("missing required" in error for error in report["errors"]))
        self.policy["required"].pop()
        self.registry.write_text(json.dumps(self.policy))
        records = self.records()
        records[0]["modules"][0].update(ok=False, stage="canonical", error="bytes differ")
        with patch.object(audit_module.subprocess, "run", return_value=self.fake(records)):
            report = self.audit()
        self.assertFalse(report["ok"])
        self.assertTrue(any("canonical" in error for error in report["errors"]))

    def test_unsupported_change_or_silent_promotion_requires_registry_update(self):
        self.policy["unsupported"]["d.icl::Opaque"]["diagnostic"] = "wrong boundary"
        self.registry.write_text(json.dumps(self.policy))
        self.assertFalse(self.audit()["ok"])
        self.policy["unsupported"]["d.icl::Opaque"]["diagnostic"] = (
            "flat serialization is not implemented for AccessLink")
        self.registry.write_text(json.dumps(self.policy))
        records = self.records()
        records[-1]["modules"][0] = {
            "name": "Opaque", "stage": "canonical", "ok": True, "nodes": 1, "flat_bytes": 20}
        with patch.object(audit_module.subprocess, "run", return_value=self.fake(records)):
            report = self.audit()
        self.assertFalse(report["ok"])
        self.assertTrue(any("unsupported" in error for error in report["errors"]))

    def test_internal_or_invalid_protocol_and_failed_backend_are_rejected(self):
        records = self.records()
        records[2]["modules"][0]["error"] = "map::at"
        with patch.object(audit_module.subprocess, "run", return_value=self.fake(records)):
            report = self.audit()
        self.assertFalse(report["ok"])
        self.assertTrue(any("map::at" in error for error in report["errors"]))
        records[2]["modules"][0]["stage"] = "invented"
        with patch.object(audit_module.subprocess, "run", return_value=self.fake(records)):
            with self.assertRaisesRegex(ValueError, "stage"):
                self.audit()
        with patch.object(audit_module.subprocess, "run", return_value=self.fake([], 2)):
            with self.assertRaisesRegex(ValueError, "inventory failed"):
                self.audit()

    def test_malformed_registry_object_replaces_old_success(self):
        report = self.root / "report.json"
        report.write_text('{"ok":true,"old":true}')
        self.policy["unsupported"]["d.icl::Opaque"] = None
        self.registry.write_text(json.dumps(self.policy))
        result = subprocess.run(
            [sys.executable, str(ROOT / "tests/src/pattern/audit_model_corpus.py"),
             "--root", str(self.root), "--corpus-report", str(self.corpus),
             "--registry", str(self.registry), "--inventory", str(INVENTORY),
             "--report", str(report)], capture_output=True, text=True)
        self.assertNotEqual(result.returncode, 0)
        self.assertFalse(json.loads(report.read_text())["ok"])
        self.assertNotIn("old", json.loads(report.read_text()))

    def test_parser_rejection_and_declaration_consistency(self):
        original = json.loads(self.corpus.read_text())
        variants = []
        value = json.loads(json.dumps(original))
        value["rejected"].append("c.icl")
        variants.append(value)
        value = json.loads(json.dumps(original))
        value["fixtures"]["a.icl"]["modules"].pop()
        variants.append(value)
        value = json.loads(json.dumps(original))
        value["modules"] += 1
        variants.append(value)
        value = json.loads(json.dumps(original))
        value["rejected"] = ["c.icl", "c.icl"]
        variants.append(value)
        for value in variants:
            with self.subTest(value=value):
                self.corpus.write_text(json.dumps(value))
                with self.assertRaises(ValueError):
                    self.audit()
        self.corpus.write_text(json.dumps(original))

    def test_contradictory_success_and_extended_boundary_fail(self):
        records = self.records()
        records[0]["modules"][0]["error"] = "map::at"
        with patch.object(audit_module.subprocess, "run", return_value=self.fake(records)):
            with self.assertRaises(ValueError):
                self.audit()
        records = self.records()
        records[-1]["modules"][0]["error"] += "Extended"
        with patch.object(audit_module.subprocess, "run", return_value=self.fake(records)):
            self.assertFalse(self.audit()["ok"])

    def test_source_drift_during_real_inventory_is_rejected(self):
        actual_run = subprocess.run
        path = self.root / "a.icl"
        def edited_before_run(*args, **kwargs):
            path.write_text(path.read_text().replace(
                "DataInPort p;", "DataInPort p; DataInPort added;"))
            return actual_run(*args, **kwargs)
        with patch.object(audit_module.subprocess, "run", side_effect=edited_before_run):
            with self.assertRaisesRegex(ValueError, "changed during"):
                self.audit()

    def test_helper_or_source_set_drift_after_execution_is_rejected(self):
        import shutil
        actual_run = subprocess.run
        helper = self.root / "inventory-copy"
        shutil.copy2(INVENTORY, helper)
        def rebuilt_after_run(*args, **kwargs):
            result = actual_run(*args, **kwargs)
            with helper.open("ab") as stream:
                stream.write(b"\n")
            return result
        with patch.object(audit_module.subprocess, "run", side_effect=rebuilt_after_run):
            with self.assertRaisesRegex(ValueError, "changed during"):
                audit_module.audit(self.root, self.corpus, helper, self.registry)
        def added_after_run(*args, **kwargs):
            result = actual_run(*args, **kwargs)
            (self.root / "new.icl").write_text("Module Added {}")
            return result
        with patch.object(audit_module.subprocess, "run", side_effect=added_after_run):
            with self.assertRaisesRegex(ValueError, "changed during"):
                self.audit()

    def test_failed_rerun_replaces_old_success_report(self):
        report = self.root / "report.json"
        command = [sys.executable, str(ROOT / "tests/src/pattern/audit_model_corpus.py"),
                   "--root", str(self.root), "--corpus-report", str(self.corpus),
                   "--registry", str(self.registry), "--inventory", str(INVENTORY),
                   "--report", str(report)]
        first = subprocess.run(command, capture_output=True, text=True)
        self.assertEqual(first.returncode, 0, first.stderr)
        self.assertTrue(json.loads(report.read_text())["ok"])
        command[command.index("--inventory") + 1] = "/usr/bin/true"
        second = subprocess.run(command, capture_output=True, text=True)
        self.assertNotEqual(second.returncode, 0)
        self.assertFalse(json.loads(report.read_text())["ok"])


if __name__ == "__main__":
    unittest.main()
