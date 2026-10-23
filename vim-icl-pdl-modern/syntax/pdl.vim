" Copyright 2026 The ICL Toolsets Authors. SPDX-License-Identifier: MIT
" IEEE 1687-2014 clauses 7-8: extend the editor's Tcl runtime, never ICL syntax.
if exists('b:current_syntax')
  finish
endif
runtime! syntax/tcl.vim
unlet! b:current_syntax
let s:save_cpo = &cpo
set cpo&vim
syntax case match

syntax keyword pdlCommand iPDLLevel iProcsForModule iUseProcNameSpace iPrefix
      \ iReset iRead iWrite iScan iOverrideScanInterface iApply iClock
      \ iClockOverride iRunLoop iMerge iTake iRelease iNote iState
      \ iGetReadData iGetMiscompares iGetStatus iSetFail
      \ containedin=tclEmbeddedStatement
syntax keyword pdlCommand iProc iCall nextgroup=pdlProcedure skipwhite skipnl
      \ containedin=tclEmbeddedStatement
syntax match pdlProcedure contained /\<[A-Za-z][A-Za-z0-9_.:]*\>/
syntax keyword pdlConstant STD_1687_2014
syntax match pdlNumber /\<0[xX][0-9a-fA-FxX]\+\>/ containedin=tclEmbeddedStatement
syntax match pdlNumber /\<0[bB][01xX]\+\>/ containedin=tclEmbeddedStatement
syntax match pdlOption /\%(^\|\s\)\zs-\%(version\|iProcNameSpace\|sync\|ir\|chain\|si\|so\|stable\|capture\|update\|broadcast\|together\|end_in_pause\|source\|freqMultiplier\|freqDivider\|tck\|sck\|time\|begin\|end\|persistent\|bin\|hex\|dec\|clear\|quit\|LastWrittenValue\|LastReadValue\|LastMiscompareValue\|ignore_when_value_already_present\)\>/ containedin=tclEmbeddedStatement

highlight default link pdlCommand Statement
highlight default link pdlProcedure Function
highlight default link pdlConstant Constant
highlight default link pdlNumber Number
highlight default link pdlOption PreProc
let b:current_syntax = 'pdl'
let &cpo = s:save_cpo
unlet s:save_cpo
