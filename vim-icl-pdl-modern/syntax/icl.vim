" Copyright 2026 The ICL Toolsets Authors. SPDX-License-Identifier: MIT
" IEEE 1687-2014 clause 6: ICL is a separate language from PDL/Tcl.
if exists('b:current_syntax')
  finish
endif
let s:save_cpo = &cpo
set cpo&vim
syntax case match

syntax keyword iclTypeKeyword Module Enum nextgroup=iclTypeName skipwhite skipnl
syntax keyword iclTypeReference Of RefEnum nextgroup=iclTypeName skipwhite skipnl
syntax match iclTypeName contained /\<[A-Za-z][A-Za-z0-9_]*\%(::[A-Za-z][A-Za-z0-9_]*\)*\>/
syntax keyword iclDeclaration Parameter LocalParameter Attribute Alias Instance
      \ ScanRegister DataRegister LogicSignal ScanMux DataMux ClockMux
      \ OneHotScanGroup OneHotDataGroup ScanInterface AccessLink PortGroup
      \ NameSpace UseNameSpace nextgroup=iclName skipwhite skipnl
syntax keyword iclPort ScanInPort ScanOutPort ShiftEnPort CaptureEnPort UpdateEnPort
      \ SelectPort ResetPort TCKPort ClockPort ToClockPort TMSPort TRSTPort
      \ ToTCKPort ToTMSPort ToTRSTPort ToShiftEnPort ToCaptureEnPort ToUpdateEnPort
      \ ToResetPort ToSelectPort ToIRSelectPort AddressPort WriteEnPort ReadEnPort
      \ DataInPort DataOutPort nextgroup=iclName skipwhite skipnl
syntax match iclName contained /\<[A-Za-z][A-Za-z0-9_]*\>/
syntax keyword iclProperty AllowBroadcastOnScanInterface AccessTogether
      \ ActivePolarity ActiveSignals AddressValue BSDLEntity CaptureSource Chain
      \ DefaultLoadValue DifferentialInvOf Enable FreqDivider FreqMultiplier Period
      \ InputPort Port ReadCallBack ReadDataSource ResetValue ScanInSource Source
      \ WriteCallBack WriteDataSource WriteEnSource iApplyEndState LaunchEdge SelectedBy
      \ of
syntax keyword iclConstant STD_1149_1_2001 STD_1149_1_2013 Generic High Low Rising Falling
syntax match iclInclude /#include\>/
syntax match iclParameter /\$[A-Za-z][A-Za-z0-9_]*/
syntax match iclNumber /\<[0-9][0-9_]*\>/
syntax match iclNumber /\%([0-9][0-9_]*\|\$[A-Za-z][A-Za-z0-9_]*\)\=\s*'[bB]\s*[01xX][01xX_]*\>/
syntax match iclNumber /\%([0-9][0-9_]*\|\$[A-Za-z][A-Za-z0-9_]*\)\=\s*'[hH]\s*[0-9a-fA-FxX][0-9a-fA-FxX_]*\>/
syntax match iclNumber /\%([0-9][0-9_]*\|\$[A-Za-z][A-Za-z0-9_]*\)\=\s*'[dD]\s*[0-9][0-9_]*\>/
syntax match iclOperator /[~!&|^=+*%\/<>-]/
syntax region iclString start=/"/ skip=/\\\\\|\\"/ end=/"/
syntax keyword iclTodo TODO FIXME XXX NOTE contained
syntax region iclBlockComment start=/\/\*/ end=/\*\// contains=iclTodo,@Spell
syntax match iclLineComment /\/\/.*/ contains=iclTodo,@Spell
syntax sync ccomment iclBlockComment

highlight default link iclTypeKeyword Structure
highlight default link iclTypeReference Keyword
highlight default link iclTypeName Type
highlight default link iclDeclaration Keyword
highlight default link iclPort Type
highlight default link iclName Identifier
highlight default link iclProperty Keyword
highlight default link iclConstant Constant
highlight default link iclInclude Include
highlight default link iclParameter Identifier
highlight default link iclNumber Number
highlight default link iclOperator Operator
highlight default link iclString String
highlight default link iclTodo Todo
highlight default link iclBlockComment Comment
highlight default link iclLineComment Comment
let b:current_syntax = 'icl'
let &cpo = s:save_cpo
unlet s:save_cpo
