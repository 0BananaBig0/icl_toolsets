#!/usr/bin/env python3
"""Command-line exits and malformed input must preserve LSP startup/framing."""
import argparse
import json
from pathlib import Path
import subprocess
import unittest
BINARY = None


def frame(body):
    if isinstance(body, str): body = body.encode()
    return b'Content-Length: '+str(len(body)).encode()+b'\r\n\r\n'+body


class CommandLine(unittest.TestCase):
    def run_without_input(self, *args):
        # Keep stdin open: communicate() alone sends EOF and hides the hang.
        with subprocess.Popen([BINARY, *args], stdin=subprocess.PIPE,
                              stdout=subprocess.PIPE, stderr=subprocess.PIPE) as process:
            try:
                process.wait(timeout=2)
            except subprocess.TimeoutExpired:
                process.kill()
                process.communicate()
                self.fail(f'{args} waited for LSP input instead of exiting')
            stdout, stderr = process.communicate()
            return process.returncode, stdout, stderr

    def test_help_exits_without_lsp_input(self):
        for option in ('--help', '-h'):
            with self.subTest(option=option):
                code, stdout, stderr = self.run_without_input(option)
                self.assertEqual(code, 0)
                for text in (b'icl-lsp', b'--help', b'--version', b'stdin', b'stdout'):
                    self.assertIn(text, stdout)
                self.assertEqual(stderr, b'')
                self.assertNotIn(b'Content-Length:', stdout)

    def test_version_matches_lsp_initialize(self):
        code, stdout, stderr = self.run_without_input('--version')
        self.assertEqual(code, 0)
        self.assertEqual(stderr, b'')
        run = subprocess.run([BINARY], capture_output=True, timeout=5,
                             input=frame('{"jsonrpc":"2.0","id":1,"method":"initialize"}'))
        self.assertEqual(run.returncode, 0, run.stderr)
        self.assertTrue(run.stdout.startswith(b'Content-Length:'), run.stdout)
        info = json.loads(run.stdout.split(b'\r\n\r\n', 1)[1])['result']['serverInfo']
        self.assertEqual(stdout.decode(), f"icl-lsp {info['version']}\n")

    def test_invalid_arguments_exit_without_lsp_input(self):
        for args in [('--unknown',), ('input.icl',), ('--help', '--unknown')]:
            with self.subTest(args=args):
                code, stdout, stderr = self.run_without_input(*args)
                self.assertEqual(code, 2)
                self.assertEqual(stdout, b'')
                self.assertIn(b'icl-lsp', stderr)
                self.assertIn(b'--help', stderr)


class Protocol(unittest.TestCase):
    def run_server(self, data):
        return subprocess.run([BINARY], input=data, capture_output=True, timeout=5)

    def test_invalid_lengths_end_stream(self):
        valid = frame('{"jsonrpc":"2.0","id":2,"method":"initialize"}')
        for length in [b'2junk', b'+2', b'2\r\nContent-Length: 2']:
            with self.subTest(length=length):
                run = self.run_server(b'Content-Length: '+length+b'\r\n\r\n{}'+valid)
                self.assertEqual(run.returncode, 0, run.stderr)
                self.assertEqual(run.stdout, b'')

    def test_invalid_json_then_valid_request(self):
        invalid = ['+1', '01', '1.', '1e', '1e+', '1+2', '1e400', '"raw\nnewline"', '"\\ud800"', '"\\udc00"', '"\\ud800\\u0041"']
        invalid += [b'"\xff"', b'"\xc0\x80"', b'"\xed\xa0\x80"', b'"\xf4\x90\x80\x80"']
        for value in invalid:
            with self.subTest(value=value):
                prefix = b'{"jsonrpc":"2.0","id":1,"method":"initialize","params":{"value":'
                bad = prefix + (value.encode() if isinstance(value, str) else value) + b'}}'
                run = self.run_server(frame(bad)+frame('{"jsonrpc":"2.0","id":2,"method":"shutdown"}'))
                self.assertEqual(run.returncode, 0, run.stderr)
                self.assertEqual(run.stdout.count(b'Content-Length:'), 1, run.stdout)
                reply = json.loads(run.stdout.split(b'\r\n\r\n', 1)[1])
                self.assertEqual(reply['id'], 2)
                self.assertIn(b'malformed', run.stderr)

    def test_deep_json_is_rejected_without_crash(self):
        body = '{"id":1,"method":"initialize","params":'+ '['*1000 + '0' + ']'*1000 + '}'
        run = self.run_server(frame(body))
        self.assertEqual(run.returncode, 0, run.stderr)
        self.assertEqual(run.stdout, b'')
        self.assertIn(b'malformed', run.stderr)


if __name__ == '__main__':
    ap = argparse.ArgumentParser(); ap.add_argument('--binary', required=True)
    args, rest = ap.parse_known_args(); BINARY = str(Path(args.binary).resolve())
    unittest.main(argv=['test_icl_protocol.py']+rest)
