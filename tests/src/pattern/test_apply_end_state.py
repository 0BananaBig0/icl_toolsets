#!/usr/bin/env python3
"""IEEE 1687-2014 6.5.2(h): default loads and temporary overrides."""
import pathlib
import subprocess
import sys
import tempfile
import time
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[3]
BIN = pathlib.Path(sys.argv.pop(1)).resolve() if len(sys.argv) > 1 else ROOT / 'build/icl-pattern-generation'
MODEL = '''Module EndState {
  ScanInPort si;
  ScanRegister trigger[1:0] { ScanInSource si; ResetValue 2'b00; }
  ScanOutPort so { Source trigger[0]; }
  Alias idle_trigger = trigger[0] { iApplyEndState 1'b0; }
}'''


class ApplyEndState(unittest.TestCase):
    def setUp(self):
        temp = tempfile.TemporaryDirectory()
        self.addCleanup(temp.cleanup)
        self.root = pathlib.Path(temp.name)
        (self.root / 'source.icl').write_text(MODEL)

    def run_tool(self, body=None, source='source.pdl', output='result.pdl',
                 icl='source.icl', ok=True, extra=()):
        if body is not None:
            (self.root / source).write_text('iProcsForModule EndState\n' + body)
        result = subprocess.run([str(BIN), '--icl', icl, '--pdl', source,
                                 '--output', output, *extra], cwd=self.root,
                                capture_output=True, text=True, timeout=40)
        self.assertEqual(result.returncode == 0, ok, result.stdout + result.stderr)
        return (self.root / output).read_text() if ok else result.stdout

    def replay(self, text):
        self.assertEqual(text, self.run_tool(source='result.pdl', output='replay.pdl'))
        self.assertEqual(text, self.run_tool(icl='debug/EndState.icl', output='flat.pdl'))
        self.assertEqual(text, self.run_tool(icl='debug/EndState.icl', source='result.pdl', output='both.pdl'))

    def test_final_override_restored_and_unrelated_bits_preserved(self):
        text = self.run_tool('iReset\niWrite trigger 3\niApply\n', extra=('--debug',))
        self.assertEqual(text.count('iApply\n'), 2)
        self.assertIn('iWrite trigger 0b11\niApply\niWrite trigger 0b10\niApply\n', text)
        self.replay(text)

    def test_override_expires_on_following_read_or_empty_apply(self):
        for next_group in ('iRead trigger 3\niApply\n', 'iApply\n'):
            with self.subTest(next_group=next_group):
                text = self.run_tool('iReset\niWrite idle_trigger 1\niApply\n' + next_group,
                                     extra=('--debug',))
                self.assertEqual(text.count('iApply\n'), 2)
                self.assertEqual(text.count('iWrite trigger 0b00\n'), 1)
                if next_group.startswith('iRead'):
                    self.assertIn('iRead trigger 0b11', text)
                self.replay(text)

    def test_repeated_override_lasts_each_apply(self):
        text = self.run_tool('iReset\niWrite idle_trigger 1\niApply\n'
                             'iWrite trigger[0] 1\niApply\n', extra=('--debug',))
        self.assertEqual(text.count('iApply\n'), 3)
        self.assertEqual(text.count('iWrite trigger 0b01\n'), 2)
        self.replay(text)

    def test_inverted_alias_default_and_x_write(self):
        (self.root / 'source.icl').write_text(MODEL.replace(
            '= trigger[0] { iApplyEndState 1\'b0;', '= ~trigger[0] { iApplyEndState 1\'b1;'))
        text = self.run_tool('iReset\niWrite idle_trigger 0\niApply\n'
                             'iWrite trigger 0b1x\niApply\n', extra=('--debug',))
        self.assertIn('iWrite trigger 0b01\niApply\niWrite trigger 0b10\niApply\n', text)
        self.assertEqual(text.count('iApply\n'), 2)
        self.replay(text)

    def test_primary_input_default(self):
        (self.root / 'source.icl').write_text('''Module EndState {
          DataInPort pin;
          Alias idle = pin { iApplyEndState 1'b0; }
        }''')
        text = self.run_tool('iWrite pin 1\niApply\n', extra=('--debug',))
        self.assertIn('iWrite pin 0b1\niApply\niWrite pin 0b0\niApply\n', text)
        self.replay(text)

    def test_forwarded_primary_x_uses_source_default(self):
        for source, value, initial, final in (('pin', "1'b0", '1', '0'),
                                             ('~pin', "1'b1", '0', '0')):
            with self.subTest(source=source):
                (self.root / 'source.icl').write_text(f"""Module EndState {{
                  DataInPort pin;
                  DataOutPort out {{ Source {source}; }}
                  Alias idle = out {{ iApplyEndState {value}; }}
                }}""")
                text = self.run_tool(f'iWrite out {initial}\niApply\n'
                                     'iWrite out 0bx\niApply\n', extra=('--debug',))
                self.assertEqual(text.count('iApply\n'), 2)
                self.assertTrue(text.endswith(f'iWrite pin 0b{final}\niApply\n'))
                self.replay(text)

    def test_sib_default_closes_with_target_update(self):
        (self.root / 'source.icl').write_text("""Module EndState {
          ScanInPort si;
          ScanRegister gate { ScanInSource si; ResetValue 1'b0; }
          ScanRegister target[3:0] { ScanInSource gate; ResetValue 4'b0000; }
          ScanMux route SelectedBy gate { 1'b0: gate; 1'b1: target[0]; }
          ScanOutPort so { Source route; }
          Alias idle = gate { iApplyEndState 1'b0; }
        }""")
        text = self.run_tool('iReset\niWrite target 9\niApply\n', extra=('--debug',))
        self.assertEqual(text.count('iApply\n'), 2)
        self.assertIn('iWrite gate 0b0\niWrite target 0b1001\niApply\n', text)
        self.replay(text)

    def test_reserved_override_cannot_be_restored_implicitly(self):
        error = self.run_tool('iReset\niTake trigger\niWrite trigger 1\niApply\n'
                              'iApply\n', ok=False)
        self.assertIn('reserved trigger', error)
        self.assertFalse((self.root / 'result.pdl').exists())

    def test_final_restoration_failure_reports_source(self):
        error = self.run_tool('iReset\niTake trigger\niWrite trigger 1\niApply\n', ok=False)
        self.assertIn('source.pdl:5:', error)
        self.assertIn('final iApplyEndState restoration', error)
        self.assertIn('iApplyEndState conflicts with reserved', error)
        self.assertFalse((self.root / 'result.pdl').exists())

    def test_wide_primary_default_round_trip(self):
        width = 1024
        (self.root / 'source.icl').write_text(f"""Module EndState {{
          DataInPort pins[{width - 1}:0];
          Alias idle[{width - 1}:0] = pins {{ iApplyEndState {width}'b0; }}
        }}""")
        start = time.monotonic()
        text = self.run_tool('iWrite pins 0b' + '1' * width + '\niApply\n',
                             extra=('--debug',))
        self.assertEqual(text.count('iApply\n'), 2)
        self.assertTrue(text.endswith('iWrite pins 0b' + '0' * width + '\niApply\n'))
        elapsed = time.monotonic() - start
        self.replay(text)
        print(f'end-state primary bits={width} source_seconds={elapsed:.3f}')

    def test_invalid_directives_do_not_publish(self):
        models = [
            "Module EndState { DataInPort pin; Alias a = pin { iApplyEndState 1'b0; } Alias b = pin { iApplyEndState 1'b1; } }",
            "Module EndState { DataOutPort tie { Source 1'b0; } Alias a = tie { iApplyEndState 1'b1; } }",
            MODEL.replace("iApplyEndState 1'b0", "iApplyEndState 1'bx"),
            MODEL.replace("iApplyEndState 1'b0", "iApplyEndState 1'b1"),
            MODEL.replace("ResetValue 2'b00;", ''),
            MODEL.replace('ScanOutPort so', "Alias conflict = trigger[0] { iApplyEndState 1'b1; } ScanOutPort so"),
            '''Module EndState { DataInPort pin; DataInPort en;
               DataOutPort out { Source pin; Enable en; }
               Alias idle = out { iApplyEndState 1'b0; } }''',
        ]
        for model in models:
            with self.subTest(model=model):
                (self.root / 'source.icl').write_text(model)
                (self.root / 'result.pdl').write_text('previous output\n')
                error = self.run_tool('iApply\n', ok=False, extra=('--debug',))
                self.assertIn('iApplyEndState', error)
                self.assertEqual((self.root / 'result.pdl').read_text(), 'previous output\n')
                self.assertFalse((self.root / 'debug/EndState.icl').exists())


if __name__ == '__main__':
    unittest.main()
