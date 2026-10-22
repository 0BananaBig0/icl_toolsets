#!/usr/bin/env python3
"""The corpus runner must actually check content and a second formatting pass."""
import contextlib
import io
from pathlib import Path
import subprocess
import sys
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import regression


class RegressionOracle(unittest.TestCase):
    def exercise(self, first, second, source=None):
        source = source or 'Module M { Attribute a = "a b"; }'
        result = lambda text: subprocess.CompletedProcess([], 0, stdout=text, stderr='')
        with patch.object(sys, 'argv', ['regression.py', '--binary', __file__]), \
                patch.object(regression, 'cases', side_effect=lambda kind: [('case', source)] if kind == 'pass' else []), \
                patch.object(regression, 'run_as_file', return_value=result(first)), \
                patch.object(regression, 'run_stdin', side_effect=lambda binary, text: result(first if text == source else second)), \
                contextlib.redirect_stdout(io.StringIO()):
            return regression.main()

    def test_content_loss_fails(self):
        self.assertEqual(self.exercise('Module M {}', 'Module M {}'), 1)

    def test_string_whitespace_loss_fails(self):
        changed = 'Module M { Attribute a = "ab"; }'
        self.assertEqual(self.exercise(changed, changed), 1)

    def test_non_idempotent_output_fails(self):
        self.assertEqual(self.exercise('Module M {\n Attribute a = "a b";\n}', 'Module M {}'), 1)

    def test_leading_bom_is_consumed(self):
        output = 'Module M { Attribute a = "a b"; }'
        self.assertEqual(self.exercise(output, output, "\ufeff" + output), 0)

    def test_stable_output_passes(self):
        output = 'Module M {\n Attribute a = "a b";\n}'
        self.assertEqual(self.exercise(output, output), 0)


if __name__ == '__main__':
    unittest.main()
