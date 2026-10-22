#!/usr/bin/env python3
"""Run the real Vim/coc test with a PTY, without the util-linux script command."""
import os
from pathlib import Path
import pty
import select
import signal
import time


def main():
    root = Path(os.environ['COC_ROOT'])
    result = root / 'result.txt'
    result.unlink(missing_ok=True)
    pid, master = pty.fork()
    if pid == 0:
        os.environ['TERM'] = 'xterm'
        os.execvp(os.environ['COC_VIM'], [os.environ['COC_VIM'], '-N', '-u', 'NONE', '-i', 'NONE', '-S', 'tests/coc_vim_check.vim'])
    deadline = time.monotonic() + 300
    reaped = False
    try:
        with (root / 'stdout.txt').open('wb') as log:
            while time.monotonic() < deadline:
                ready, _, _ = select.select([master], [], [], 1)
                if ready:
                    try:
                        data = os.read(master, 65536)
                    except OSError:
                        break
                    if not data:
                        break
                    log.write(data)
                    log.flush()
                child, _ = os.waitpid(pid, os.WNOHANG)
                if child:
                    reaped = True
                    break
    finally:
        if not reaped:
            child, _ = os.waitpid(pid, os.WNOHANG)
            if not child:
                os.kill(pid, signal.SIGTERM)
                os.waitpid(pid, 0)
        os.close(master)
    if not result.exists():
        print('Vim/coc did not finish; inspect', root / 'stdout.txt')
        return 1
    report = result.read_text()
    print(report, end='')
    return int('[FAIL]' in report)


if __name__ == '__main__':
    raise SystemExit(main())
