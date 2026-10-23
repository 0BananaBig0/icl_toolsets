" Copyright 2026 The ICL Toolsets Authors. SPDX-License-Identifier: MIT
augroup vim_icl_pdl_modern
  autocmd!
  " Override Vim's builtin *.icl -> Clean association.
  autocmd BufRead,BufNewFile *.icl setlocal filetype=icl
  autocmd BufRead,BufNewFile *.pdl setlocal filetype=pdl
augroup END
