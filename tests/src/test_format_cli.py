#!/usr/bin/env python3
"""Both formatter CLIs must report I/O errors without damaging input files."""
import argparse
import os
from pathlib import Path
import resource
import signal
import stat
import subprocess
import tempfile
import unittest

BINARIES = {}
SOURCES = {
    'icl': 'Module Example { ScanInPort SI; ScanOutPort SO { Source SI; } }\n',
    'pdl': 'iPDLLevel 0\niProcsForModule Example\niProc run {} { iApply }\n',
}


def restrict_writes():
    # Size limits also truncate gcov counters; exclude these child runs.
    os.environ["GCOV_PREFIX"] = "/dev/null"
    signal.signal(signal.SIGXFSZ, signal.SIG_IGN)
    resource.setrlimit(resource.RLIMIT_FSIZE, (16, 16))
    resource.setrlimit(resource.RLIMIT_CORE, (0, 0))


class FileIO(unittest.TestCase):
    def test_failed_in_place_write_preserves_source(self):
        for language, binary in BINARIES.items():
            with self.subTest(language=language), tempfile.TemporaryDirectory() as tmp:
                path = Path(tmp) / ('input.' + language)
                original = SOURCES[language].encode()
                path.write_bytes(original)
                run = subprocess.run([binary, '-w', str(path)], capture_output=True,
                                     preexec_fn=restrict_writes, timeout=5)
                self.assertEqual(run.returncode, 2, run.stderr)
                self.assertEqual(path.read_bytes(), original)
                self.assertEqual(list(Path(tmp).iterdir()), [path])

    def test_failed_output_write_preserves_destination(self):
        for language, binary in BINARIES.items():
            for exists in (False, True):
                with self.subTest(language=language, exists=exists), tempfile.TemporaryDirectory() as tmp:
                    path = Path(tmp) / 'output'
                    if exists:
                        path.write_bytes(b'original destination\n')
                    run = subprocess.run([binary, '-o', str(path)],
                                         input=SOURCES[language].encode(), capture_output=True,
                                         preexec_fn=restrict_writes, timeout=5)
                    self.assertEqual(run.returncode, 2, run.stderr)
                    if exists:
                        self.assertEqual(path.read_bytes(), b'original destination\n')
                    else:
                        self.assertFalse(path.exists())
                    self.assertEqual(set(Path(tmp).iterdir()), {path} if exists else set())

    def test_directory_input_is_an_error(self):
        with tempfile.TemporaryDirectory() as tmp:
            for language, binary in BINARIES.items():
                with self.subTest(language=language):
                    run = subprocess.run([binary, '--check', tmp], capture_output=True, timeout=5)
                    self.assertEqual(run.returncode, 2)
                    self.assertTrue(run.stderr)
                    self.assertEqual(run.stdout, b'')

    def test_directory_stdin_is_an_error(self):
        with tempfile.TemporaryDirectory() as tmp:
            descriptor = os.open(tmp, os.O_RDONLY | os.O_DIRECTORY)
            try:
                for language, binary in BINARIES.items():
                    with self.subTest(language=language):
                        run = subprocess.run(
                            [binary], stdin=descriptor, capture_output=True, timeout=5,
                            preexec_fn=lambda: resource.setrlimit(resource.RLIMIT_CORE, (0, 0)))
                        self.assertEqual(run.returncode, 2, run.stderr)
                        self.assertTrue(run.stderr)
                        self.assertEqual(run.stdout, b'')
            finally:
                os.close(descriptor)

    def test_dangling_output_symlink_is_not_replaced(self):
        for language, binary in BINARIES.items():
            with self.subTest(language=language), tempfile.TemporaryDirectory() as tmp:
                path = Path(tmp) / 'output'
                path.symlink_to('missing')
                run = subprocess.run([binary, '-o', str(path)],
                                     input=SOURCES[language].encode(),
                                     capture_output=True, timeout=5)
                self.assertEqual(run.returncode, 2, run.stderr)
                self.assertTrue(path.is_symlink())
                self.assertEqual(os.readlink(path), 'missing')
                self.assertFalse((Path(tmp) / 'missing').exists())

    def test_empty_input_is_valid(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / 'empty'
            path.touch()
            for language, binary in BINARIES.items():
                with self.subTest(language=language):
                    run = subprocess.run([binary, str(path)], capture_output=True, timeout=5)
                    self.assertEqual(run.returncode, 0, run.stderr)
                    self.assertEqual(run.stdout, b'')

    def test_symlink_and_permissions_survive_replacement(self):
        for language, binary in BINARIES.items():
            with self.subTest(language=language), tempfile.TemporaryDirectory() as tmp:
                target = Path(tmp) / ('target.' + language)
                target.write_text(SOURCES[language])
                target.chmod(0o640)
                link = Path(tmp) / ('link.' + language)
                link.symlink_to(target.name)
                expected = subprocess.run([binary, str(target)], capture_output=True, check=True).stdout
                run = subprocess.run([binary, '-w', str(link)], capture_output=True, timeout=5)
                self.assertEqual(run.returncode, 0, run.stderr)
                self.assertTrue(link.is_symlink())
                self.assertEqual(os.readlink(link), target.name)
                self.assertEqual(target.read_bytes(), expected)
                self.assertEqual(stat.S_IMODE(target.stat().st_mode), 0o640)
                self.assertEqual(set(Path(tmp).iterdir()), {target, link})

    def test_failed_symlink_write_preserves_target(self):
        for language, binary in BINARIES.items():
            with self.subTest(language=language), tempfile.TemporaryDirectory() as tmp:
                target = Path(tmp) / 'target'
                original = SOURCES[language].encode()
                target.write_bytes(original)
                link = Path(tmp) / 'link'
                link.symlink_to(target.name)
                run = subprocess.run([binary, '-w', str(link)], capture_output=True,
                                     preexec_fn=restrict_writes, timeout=5)
                self.assertEqual(run.returncode, 2)
                self.assertTrue(link.is_symlink())
                self.assertEqual(target.read_bytes(), original)
                self.assertEqual(set(Path(tmp).iterdir()), {target, link})

    @unittest.skipIf(os.geteuid() == 0, 'root bypasses file write permissions')
    def test_readonly_destination_is_not_replaced(self):
        for language, binary in BINARIES.items():
            with self.subTest(language=language), tempfile.TemporaryDirectory() as tmp:
                path = Path(tmp) / 'output'
                path.write_bytes(b'original\n')
                path.chmod(0o444)
                run = subprocess.run([binary, '-o', str(path)],
                                     input=SOURCES[language].encode(), capture_output=True, timeout=5)
                self.assertEqual(run.returncode, 2, run.stderr)
                self.assertEqual(path.read_bytes(), b'original\n')
                self.assertEqual(stat.S_IMODE(path.stat().st_mode), 0o444)

    def test_new_output_obeys_umask(self):
        for language, binary in BINARIES.items():
            with self.subTest(language=language), tempfile.TemporaryDirectory() as tmp:
                path = Path(tmp) / 'output'
                run = subprocess.run([binary, '-o', str(path)],
                                     input=SOURCES[language].encode(), capture_output=True,
                                     preexec_fn=lambda: os.umask(0o027), timeout=5)
                self.assertEqual(run.returncode, 0, run.stderr)
                self.assertEqual(stat.S_IMODE(path.stat().st_mode), 0o640)

    def test_stdout_write_failure_is_reported(self):
        for language, binary in BINARIES.items():
            with self.subTest(language=language), tempfile.TemporaryFile() as output:
                run = subprocess.run([binary], input=SOURCES[language].encode(),
                                     stdout=output, stderr=subprocess.PIPE,
                                     preexec_fn=restrict_writes, timeout=5)
                self.assertEqual(run.returncode, 2, run.stderr)

    def test_replacing_one_hardlink_leaves_other_paths_unchanged(self):
        for language, binary in BINARIES.items():
            with self.subTest(language=language), tempfile.TemporaryDirectory() as tmp:
                path = Path(tmp) / 'input'
                original = SOURCES[language].encode()
                path.write_bytes(original)
                alias = Path(tmp) / 'alias'
                os.link(path, alias)
                run = subprocess.run([binary, '-w', str(path)], capture_output=True, timeout=5)
                self.assertEqual(run.returncode, 0, run.stderr)
                self.assertEqual(alias.read_bytes(), original)
                self.assertNotEqual(path.stat().st_ino, alias.stat().st_ino)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--icl', required=True)
    parser.add_argument('--pdl', required=True)
    args, rest = parser.parse_known_args()
    BINARIES = {'icl': str(Path(args.icl).resolve()), 'pdl': str(Path(args.pdl).resolve())}
    unittest.main(argv=['test_format_cli.py'] + rest)
