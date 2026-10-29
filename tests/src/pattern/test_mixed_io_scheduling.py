#!/usr/bin/env python3
"""Self-constructed mixed serial/parallel scheduling and replay regressions."""
import pathlib
import subprocess
import sys
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[3]
BIN = pathlib.Path(sys.argv.pop(1)).resolve() if len(sys.argv) > 1 else ROOT / 'build/icl-pattern-generation'
FIXTURES = ROOT / 'tests/cases/ours/pattern_generation/mixed_io'


class MixedIo(unittest.TestCase):
    def exercise(self, fixture, top, pin, *, tap=False, body=None,
                 transform=lambda source: source, ok=True, width=4):
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            model = transform((FIXTURES / fixture).read_text())
            arguments = []
            if tap:
                model = model.replace('ScanInPort si;', """ScanInPort si;
  SelectPort select; CaptureEnPort capture; ShiftEnPort shift;
  UpdateEnPort update; TCKPort clock;
  ScanInterface client {
    Port si; Port so; Port select; Port capture; Port shift; Port update; Port clock;
  }""")
                port = f'{pin}[{width - 1}:0]' if pin == 'data' else pin
                model += f"""
Module {top}Probe {{
  DataInPort {port};
  Instance core Of {top} {{ InputPort {pin} = {pin}; }}
  DataOutPort observed[{width - 1}:0] {{ Source core.observed; }}
  AccessLink tap Of STD_1149_1_2001 {{
    BSDLEntity AccessProbe;
    IJTAG_ON {{ ScanInterface {{ core.client; }} }}
    IJTAG_OFF {{ ScanInterface {{ core.client; }} }}
  }}
}}
"""
                top += 'Probe'
                arguments = ['--bsdl', 'AccessProbe', str(FIXTURES.parent / 'access_link/simple.bsdl')]
            icl = root / 'model.icl'
            icl.write_text(model)
            default_body = body is None
            if default_body:
                body = 'iWrite value 5; iApply; iRead value 5; iWrite value 9; iApply'
            if tap:
                body = body.replace('value', 'core.value')
            source = root / 'source.pdl'
            source.write_text(f'iProcsForModule {top}\niReset\n' + body)


            def run(icl, pdl, output):
                destination = root / output / 'result.pdl'
                destination.parent.mkdir()
                if not ok:
                    destination.write_text('previous output')
                result = subprocess.run(
                    [str(BIN), '--icl', str(icl), '--pdl', str(pdl),
                     '--output', str(destination), '--debug', *arguments], cwd=root,
                    capture_output=True, text=True, timeout=45)
                self.assertEqual(result.returncode == 0, ok, result.stdout + result.stderr)
                if not ok:
                    self.assertEqual(destination.read_text(), 'previous output')
                    return result.stdout + result.stderr
                return destination.read_text()

            original = run(icl, source, 'first')
            if not ok:
                return original
            if default_body:
                self.assertIn(f'iWrite {pin} ', original)
                self.assertIn(f'iRead observed 0b{5:0{width}b}', original)
            flat = root / f'first/debug/{top}.icl'
            flat_text = flat.read_text()
            self.assertEqual(original, run(flat, source, 'flat'))
            self.assertEqual(flat_text, (root / f'flat/debug/{top}.icl').read_text())
            self.assertEqual(original, run(icl, root / 'first/result.pdl', 'replay'))

            return original

    def test_primary_data_with_scan_enable(self):
        self.exercise('primary_data.icl', 'MixedPrimaryData', 'data')

    def test_scan_data_with_primary_enable(self):
        self.exercise('scan_data.icl', 'MixedScanData', 'enable')

    def test_tap_primary_data(self):
        self.exercise('primary_data.icl', 'MixedPrimaryData', 'data', tap=True)

    def test_tap_scan_data(self):
        self.exercise('scan_data.icl', 'MixedScanData', 'enable', tap=True)

    def test_tap_parallel_disable_after_scan(self):
        self.exercise('scan_data.icl', 'MixedScanData', 'enable', tap=True,
                      body='iWrite value 9; iWrite enable 0; iApply')

    def test_taken_primary_driver_rejects_implicit_changes(self):
        error = self.exercise('primary_data.icl', 'MixedPrimaryData', 'data',
                              body='iTake data; iWrite data 5; iApply; iWrite value 9; iApply', ok=False)
        self.assertIn('reserved data', error)

    def test_explicit_replacement_of_taken_primary_driver(self):
        self.exercise('primary_data.icl', 'MixedPrimaryData', 'data',
                      body='iTake data; iWrite data 5; iApply; iWrite data 9; iWrite value 9; iApply')

    def test_together_cannot_merge_primary_and_scan_cycles(self):
        error = self.exercise('scan_data.icl', 'MixedScanData', 'enable',
                              body='iWrite enable 1; iWrite data 5; iApply -together', ok=False)
        self.assertIn('iApply -together', error)

    def test_access_together_register_group(self):
        result = self.exercise('scan_data.icl', 'MixedScanData', 'enable',
                               transform=lambda text: text.replace('  ScanOutPort so',
                                   '  Alias paired[4:0] = data, value[0] { AccessTogether; }\n  ScanOutPort so'),
                               body='iWrite paired 3; iApply')
        self.assertIn('iApply -together', result)

    def test_wide_mixed_data_replay(self):
        self.exercise('primary_data.icl', 'MixedPrimaryData', 'data', width=128,
                      transform=lambda text: text.replace('[3:0]', '[127:0]').replace("4'b0", "128'b0"))

    def test_primary_width_limit_is_explicit(self):
        error = self.exercise('primary_data.icl', 'MixedPrimaryData', 'data', ok=False,
                              transform=lambda text: text.replace('[3:0]', '[4096:0]').replace("4'b0", "4097'b0"))
        self.assertIn('primary-input resource limit', error)

    def test_primary_end_state_restoration_preserves_stored_goal(self):
        self.exercise('primary_data.icl', 'MixedPrimaryData', 'data',
                      transform=lambda text: text.replace('  ScanOutPort so',
                          "  Alias idle = data[0] { iApplyEndState 1'b0; }\n  ScanOutPort so"),
                      body='iWrite value 5; iApply')

    def test_addressed_primary_read_enable_in_scan_module(self):
        result = self.exercise('addressed.icl', 'MixedAddressed', 'data',
                               body='iWrite value 5; iApply; iRead value 5; iApply')
        self.assertIn('iWrite re 0b1', result)

    def test_observe_scan_storage_before_overwriting_it(self):
        result = self.exercise('scan_observation.icl', 'MixedScanObservation', 'data',
                               body='iRead observed 0; iWrite data 9; iApply')
        self.assertLess(result.index('iRead observed 0b0000'), result.index('iWrite data 0b1001'))

    def test_mixed_tap_with_hidden_storage_events(self):
        self.exercise('scan_data.icl', 'MixedScanData', 'enable', tap=True,
                      transform=lambda text: text.replace('  ScanOutPort so',
                          "  ScanRegister shadow[3:0] { ScanInSource si; CaptureSource 4'b1010; ResetValue 4'b0; }\n  ScanOutPort so"))


if __name__ == '__main__':
    unittest.main()
