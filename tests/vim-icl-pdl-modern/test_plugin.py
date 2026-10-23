#!/usr/bin/env python3
"""Exercise the standalone runtime in real Vim/Neovim, optionally with vim-plug."""
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
PLUGIN = ROOT / 'vim-icl-pdl-modern'
SCRIPT = Path(__file__).with_name('test_syntax.vim')


class Plugin(unittest.TestCase):
    def run_editor(self, editor, plug=None, install=False, fallback_first=False):
        executable = shutil.which(editor)
        if not executable:
            self.skipTest(f'{editor} not installed')
        with tempfile.TemporaryDirectory() as directory:
            temp = Path(directory)
            isolated = temp / 'vim-icl-pdl-modern'
            if PLUGIN.exists():
                shutil.copytree(PLUGIN, isolated)
            else:
                isolated.mkdir()
            if install:
                for command in [['init', '-b', 'main'], ['add', '.'],
                                ['-c', 'user.name=Plugin Test',
                                 '-c', 'user.email=plugin-test@example.invalid',
                                 '-c', 'commit.gpgsign=false',
                                 '-c', 'core.hooksPath=/dev/null',
                                 'commit', '-m', 'Standalone plugin fixture']]:
                    subprocess.run(['git', '-C', str(isolated), *command],
                                   check=True, capture_output=True, timeout=10)
            # No source tree, compiler, language server or other plugin is present.
            for filename in ['example.icl', 'example.pdl', 'plain.tcl', 'plain.cpp']:
                (temp / filename).write_text('')
            icl = re.findall(r'^([A-Za-z][A-Za-z0-9_]*) \{ return icl::IclParser::make_',
                             (ROOT / 'src/icl_lexer.l').read_text(), re.MULTILINE)
            pdl = re.findall(r'\{"(i[A-Z][A-Za-z]+)", \{\d+,',
                             (ROOT / 'src/pdl_document.cc').read_text())
            self.assertGreater(len(icl), 70)
            self.assertEqual(len(pdl), 24)
            for language, words in [('icl', icl), ('pdl', pdl)]:
                (temp / ('keywords.' + language)).write_text(''.join(
                    word + ' sample;\n' for word in words))
            env = dict(os.environ, ICL_PDL_PLUGIN=isolated.as_uri() if install else str(isolated),
                       ICL_PDL_INSTALL='1' if install else '', ICL_PDL_TEMP=directory,
                       ICL_PDL_RESULT=str(temp / 'result'), ICL_PDL_PLUG=str(plug or ''),
                       ICL_PDL_FALLBACK_FIRST='1' if fallback_first else '',
                       NVIM_LOG_FILE=str(temp / 'nvim.log'))
            run = subprocess.run([executable, '-Nu', 'NONE', '-n', '-i', 'NONE',
                                  '-es', '-S', str(SCRIPT)],
                                 env=env, capture_output=True, text=True, timeout=30)
            result = temp / 'result'
            errors = result.read_text() if result.exists() else 'editor did not finish tests'
            self.assertEqual(run.returncode, 0, errors + run.stderr + run.stdout)
            self.assertEqual(errors, '')
            if install:
                checkout = temp / 'plugged/vim-icl-pdl-modern'
                tracked = subprocess.check_output(['git', '-C', str(checkout), 'ls-files'])
                expected = subprocess.check_output(['git', '-C', str(isolated), 'ls-files'])
                self.assertEqual(tracked, expected)
                for unrelated in ('src', 'tests', 'build', 'Makefile'):
                    self.assertFalse((checkout / unrelated).exists(), unrelated)
            print(editor, 'vim-plug' if plug else 'standalone',
                  (temp / 'timings').read_text().strip(), flush=True)

    def test_vim_standalone(self):
        self.run_editor('vim')

    def test_neovim_standalone(self):
        self.run_editor('nvim')

    def test_vim_plug_lazy_loading(self):
        plug = Path(os.environ.get('VIM_PLUG', Path.home() / '.vim/autoload/plug.vim'))
        if not plug.is_file():
            self.skipTest('vim-plug absent; set VIM_PLUG to its plug.vim')
        for editor in ('vim', 'nvim'):
            for fallback_first in (False, True):
                with self.subTest(editor=editor, fallback_first=fallback_first):
                    self.run_editor(editor, plug, fallback_first=fallback_first)

    def test_vim_plug_install_only_runtime(self):
        plug = Path(os.environ.get('VIM_PLUG', Path.home() / '.vim/autoload/plug.vim'))
        if not plug.is_file():
            self.skipTest('vim-plug absent; set VIM_PLUG to its plug.vim')
        self.run_editor('vim', plug, install=True)


if __name__ == '__main__':
    unittest.main()
