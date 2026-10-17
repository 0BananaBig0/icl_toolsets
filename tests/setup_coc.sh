#!/usr/bin/env bash
# Prepares everything needed to run tests/coc_vim_check.vim, i.e. the real
# vim -> coc.nvim -> icl-lsp chain.
#
# Two downloads are involved, and plain github.com is 403 in some networks, so
# each tries a mirror before giving up:
#   * coc.nvim  — codeload.github.com serves repo tarballs even when
#                 github.com and the usual git mirrors refuse.
#   * vim       — built from source, because coc.nvim requires vim 9+ and
#                 distro vim (8.2 on Ubuntu 22.04, whatever Rocky 8 ships) is
#                 too old. Only needed if the system vim is < 9.
#
# Usage:  tests/setup_coc.sh [coc-root]
# Sets up $(1)/coc.nvim and, if required, /usr/local/vim9/bin/vim.

set -euo pipefail

ROOT="${1:-/tmp/cocvim}"
COC_VERSION="${COC_VERSION:-release}"
mkdir -p "$ROOT/config" "$ROOT/data" "$ROOT/test"

# ---- icl-lsp has to be findable on PATH ------------------------------------
if ! command -v icl-lsp >/dev/null 2>&1; then
  echo "icl-lsp is not on PATH. Run 'make install' first." >&2
  exit 1
fi

# ---- coc.nvim --------------------------------------------------------------
if [ ! -d "$ROOT/coc.nvim/autoload" ]; then
  echo "fetching coc.nvim ($COC_VERSION)..."
  tmp="$ROOT/coc.tar.gz"
  # codeload serves tarballs; github.com itself and most mirrors are 403 here.
  for url in \
    "https://codeload.github.com/neoclide/coc.nvim/tar.gz/refs/heads/$COC_VERSION" \
    "https://ghproxy.com/https://codeload.github.com/neoclide/coc.nvim/tar.gz/refs/heads/$COC_VERSION"
  do
    if curl -fsSL -o "$tmp" "$url"; then break; fi
  done
  [ -s "$tmp" ] || { echo "could not download coc.nvim" >&2; exit 1; }
  tar xzf "$tmp" -C "$ROOT"
  rm -f "$tmp"
  mv "$ROOT"/coc.nvim-* "$ROOT/coc.nvim" 2>/dev/null || true
fi
echo "coc.nvim: $ROOT/coc.nvim ($(grep -m1 '"version"' "$ROOT/coc.nvim/package.json" || echo '?'))"

# ---- coc-settings.json ----------------------------------------------------
cat > "$ROOT/config/coc-settings.json" <<'JSON'
{
  "languageserver": {
    "icl": {
      "command": "icl-lsp",
      "filetypes": ["icl"],
      "settings": {}
    }
  }
}
JSON

# ---- test fixtures --------------------------------------------------------
printf 'Module m {\n  ScanInPort SI\n}\n'                            > "$ROOT/test/bad.icl"
printf 'Module m {\n  ScanInPort SI;\n}\n'                           > "$ROOT/test/good.icl"
printf 'Module m{ScanInPort SI;ScanRegister r[7:0]{ScanInSource SI;}}\n' > "$ROOT/test/messy.icl"
printf 'Module m {\n  @ bad\n}\n'                                    > "$ROOT/test/broken.icl"

# ---- vim 9+ ---------------------------------------------------------------
#
# coc.nvim's autoload/coc/api.vim is vim9script using `:export`, which vim 8.2
# rejects (E1043); coc then never finishes its handshake. Skip the build if the
# system vim is already new enough.
vim_version() {
  "$1" --version 2>/dev/null | head -1 | sed -E 's/.*Vi IMproved ([0-9]+).*/\1/'
}

VIM="${VIM:-}"
if [ -z "$VIM" ]; then
  for candidate in vim /usr/local/vim9/bin/vim; do
    if command -v "$candidate" >/dev/null 2>&1 && \
       [ "$(vim_version "$candidate")" -ge 9 ] 2>/dev/null; then
      VIM="$candidate"
      break
    fi
  done
fi

if [ -z "$VIM" ]; then
  echo "system vim is older than 9; building vim 9 from source..."
  command -v cc >/dev/null 2>&1 || \
    { echo "need a C compiler (and ncurses headers) to build vim" >&2; exit 1; }
  work="$(mktemp -d)"
  curl -fsSL -o "$work/vim.tar.gz" \
    "https://codeload.github.com/vim/vim/tar.gz/refs/tags/v9.1.1000"
  tar xzf "$work/vim.tar.gz" -C "$work"
  ( cd "$work"/vim-9.1.1000 &&
    ./configure --with-features=huge --disable-gui --without-x \
                --disable-perlinterp --disable-rubyinterp --disable-luainterp \
                --prefix=/usr/local/vim9 >/dev/null &&
    make -j"$(nproc)" >/dev/null &&
    make install >/dev/null )
  rm -rf "$work"
  VIM=/usr/local/vim9/bin/vim
fi
echo "vim: $VIM ($("$VIM" --version | head -1))"

echo
echo "ready. run the check with:"
echo "  COC_VIM=$VIM COC_ROOT=$ROOT COC_PLUGIN=$ROOT/coc.nvim make coccheck"
