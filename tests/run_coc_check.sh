#!/usr/bin/env bash
# Runs tests/coc_vim_check.vim: the real vim -> coc.nvim -> icl-lsp chain.
#
# Two details the wrapper hides, both of which matter:
#
#   * vim is given a pty (via `script`) and a stdin that never delivers data,
#     so it actually blocks in its main loop. vim only processes channel
#     messages while idle, and with stdin closed it exits before coc can
#     finish its handshake.
#   * results are read from $COC_ROOT/result.txt, not stdout, because stdout
#     belongs to vim.
#
# Usage:  tests/run_coc_check.sh
# Env:    COC_ROOT, COC_PLUGIN, COC_VIM (all set by tests/setup_coc.sh)

set -uo pipefail

ROOT="${COC_ROOT:-/tmp/cocvim}"
PLUGIN="${COC_PLUGIN:-$ROOT/coc.nvim}"
VIM="${COC_VIM:-vim}"

if [ ! -d "$PLUGIN/autoload" ]; then
  echo "coc.nvim not found at $PLUGIN — run tests/setup_coc.sh first." >&2
  exit 1
fi

# coc spawns the server by name, so it must be on PATH. Prefer the one just
# built: `make coccheck` should not depend on `make install` having run.
if [ -x build/icl-lsp ] && ! command -v icl-lsp >/dev/null 2>&1; then
  PATH="$PWD/build:$PATH"
  export PATH
fi
if ! command -v icl-lsp >/dev/null 2>&1; then
  echo "icl-lsp not on PATH — run 'make' or 'make install' first." >&2
  exit 1
fi
if ! command -v "$VIM" >/dev/null 2>&1; then
  echo "vim not found: $VIM" >&2
  exit 1
fi
if [ "$( "$VIM" --version | head -1 | sed -E 's/.*Vi IMproved ([0-9]+).*/\1/')" -lt 9 ]; then
  echo "$VIM is older than vim 9; coc.nvim needs vim9script ':export' support." >&2
  echo "run tests/setup_coc.sh to build vim 9." >&2
  exit 1
fi

result="$ROOT/result.txt"
rm -f "$result"

# A fifo with an open writer and no data: vim blocks reading it, which is what
# puts it in the state where it processes coc's channel messages.
fifo="$ROOT/stdin.fifo"
rm -f "$fifo"
mkfifo "$fifo"
sleep 600 > "$fifo" &
holder=$!

timeout 300 env TERM=xterm \
  COC_ROOT="$ROOT" COC_PLUGIN="$PLUGIN" \
  script -qec "$VIM -N -u NONE -S tests/coc_vim_check.vim" /dev/null \
  < "$fifo" > "$ROOT/stdout.txt" 2>&1 &
job=$!

status=0
for _ in $(seq 1 60); do
  [ -f "$result" ] && break
  sleep 5
done
wait "$job" 2>/dev/null || true
kill "$holder" 2>/dev/null || true
rm -f "$fifo"

if [ ! -f "$result" ]; then
  echo "no result file; vim did not finish. stdout was:" >&2
  head -20 "$ROOT/stdout.txt" >&2
  exit 1
fi

cat "$result"
grep -q '\[FAIL\]' "$result" && status=1
exit "$status"
