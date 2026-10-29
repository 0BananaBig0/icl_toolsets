#!/usr/bin/env python3
"""IEEE 1687 shadow-register events and concrete TAP replay."""
import pathlib
import subprocess
import sys
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[3]
BIN = pathlib.Path(sys.argv.pop(1)).resolve() if len(sys.argv) > 1 else ROOT / 'build/icl-pattern-generation'
FIXTURE = ROOT / 'tests/cases/ours/pattern_generation/resources'
BSDL = FIXTURE.parent / 'access_link/simple.bsdl'


class HiddenEvents(unittest.TestCase):
    def setUp(self):
        temp = tempfile.TemporaryDirectory()
        self.addCleanup(temp.cleanup)
        self.root = pathlib.Path(temp.name)
        (self.root / 'source.icl').write_bytes((FIXTURE / 'slices.icl').read_bytes())

    def run_tool(self, pdl='source.pdl', icl='source.icl', output='result.pdl', ok=True, extra=()):
        result = subprocess.run([str(BIN), '--icl', icl, '--pdl', pdl,
                                 '--bsdl', 'AccessProbe', str(BSDL), '--output', output, *extra],
                                cwd=self.root, capture_output=True, text=True, timeout=60)
        self.assertEqual(result.returncode == 0, ok, result.stdout + result.stderr)
        return (self.root / output).read_text() if ok else result.stdout

    def test_original_vendor_slice_probes(self):
        for name in ('slice_high', 'slice_low', 'slice_release', 'slice_alias', 'slice_partial_write'):
            with self.subTest(name=name):
                (self.root / 'source.pdl').write_bytes((FIXTURE / f'{name}.pdl').read_bytes())
                if name == 'slice_low':
                    (self.root / 'result.pdl').write_text('previous output')
                    error = self.run_tool(ok=False)
                    self.assertIn('reserved core.gate', error)
                    self.assertEqual((self.root / 'result.pdl').read_text(), 'previous output')
                    continue
                text = self.run_tool(extra=('--debug',))
                self.assertIn('iWrite core.target 0b0101', text)
                self.assertEqual(text, self.run_tool(pdl='result.pdl', output='replay.pdl'))
                self.assertEqual(text, self.run_tool(icl='debug/SliceTop.icl', output='flat.pdl'))

    def test_suppressed_capture_and_known_shadow_capture(self):
        for suppressed in (False, True):
            with self.subTest(suppressed=suppressed):
                source = (FIXTURE / 'slices.icl').read_text().replace(
                    "ResetValue 2'b00;", "CaptureSource 2'b11; ResetValue 2'b00;").replace(
                    "ResetValue 4'b0;", "CaptureSource 4'b1010; ResetValue 4'b0;")
                if suppressed:
                    source = source.replace('Instance core Of SliceInstrument;',
                                            "Instance core Of SliceInstrument { InputPort capture = 1'b0; }")
                (self.root / 'source.icl').write_text(source)
                (self.root / 'source.pdl').write_text(
                    'iProcsForModule SliceTop\niReset\niWrite core.gate 2\niApply\n'
                    'iWrite core.target 5\niApply\n')
                text = self.run_tool(extra=('--debug',))
                self.assertEqual(text, self.run_tool(pdl='result.pdl', output='replay.pdl'))
                self.assertEqual(text, self.run_tool(icl='debug/SliceTop.icl', output='flat.pdl'))

    def test_wide_shadow_storage_and_resource_limit(self):
        for width in (32, 128, 1024):
            with self.subTest(width=width):
                source = (FIXTURE / 'slices.icl').read_text().replace(
                    'target[3:0]', f'target[{width - 1}:0]').replace(
                    "ResetValue 4'b0;", f"ResetValue {width}'b0;")
                (self.root / 'source.icl').write_text(source)
                (self.root / 'source.pdl').write_text(
                    'iProcsForModule SliceTop\niReset\niWrite core.target 5\niApply\n')
                if width == 1024:
                    (self.root / 'result.pdl').write_text('previous output')
                    self.assertIn('scan event resource limit', self.run_tool(ok=False))
                    self.assertEqual((self.root / 'result.pdl').read_text(), 'previous output')
                else:
                    text = self.run_tool(extra=('--debug',))
                    self.assertIn('iWrite core.target 0b' + format(5, f'0{width}b'), text)
                    self.assertEqual(text, self.run_tool(pdl='result.pdl', output='replay.pdl'))
                    self.assertEqual(text, self.run_tool(icl='debug/SliceTop.icl', output='flat.pdl'))

    def test_unknown_event_control_preserves_output(self):
        source = (FIXTURE / 'slices.icl').read_text().replace(
            'Instance core Of SliceInstrument;',
            "Instance core Of SliceInstrument { InputPort shift = 1'bx; }")
        (self.root / 'source.icl').write_text(source)
        (self.root / 'source.pdl').write_text(
            'iProcsForModule SliceTop\niReset\niWrite core.target 5\niApply\n')
        (self.root / 'result.pdl').write_text('previous output')
        self.run_tool(ok=False)
        self.assertEqual((self.root / 'result.pdl').read_text(), 'previous output')


if __name__ == '__main__':
    unittest.main()
