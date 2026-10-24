set nocompatible
set nomore
set noswapfile
set shiftwidth=3 tabstop=6 noexpandtab
if !empty($ICL_PDL_FALLBACK_FIRST)
  autocmd FileType pdl if empty(globpath(&runtimepath, 'syntax/pdl.vim')) | setlocal syntax=tcl | endif
endif
if empty($ICL_PDL_PLUG)
  execute 'set runtimepath^=' . fnameescape($ICL_PDL_PLUGIN)
  filetype plugin on
else
  execute 'source ' . fnameescape($ICL_PDL_PLUG)
  call plug#begin($ICL_PDL_TEMP . '/plugged')
  execute "Plug " . string($ICL_PDL_PLUGIN) . ", {'for': ['icl', 'pdl']}"
  call plug#end()
  if !empty($ICL_PDL_INSTALL)
    PlugInstall --sync
    call plug#end()
  endif
endif
syntax enable
" Match the toolset installer's fallback without masking this plugin.
if empty($ICL_PDL_FALLBACK_FIRST)
  autocmd FileType pdl if empty(globpath(&runtimepath, 'syntax/pdl.vim')) | setlocal syntax=tcl | endif
endif
highlight link iclTypeName Special

function! CheckGroup(line, word, group) abort
  let column = stridx(getline(a:line), a:word) + 1
  call assert_true(column > 0, a:word . ' exists')
  call assert_equal(a:group, synIDattr(synID(a:line, column, 1), 'name'), a:word)
endfunction

" PDL must load first too, without an earlier ICL buffer initializing the plugin.
execute 'edit ' . fnameescape($ICL_PDL_TEMP . '/example.pdl')
call assert_equal('pdl', &filetype)
call assert_equal('pdl', &syntax)
call assert_equal('pdl', get(b:, 'current_syntax', ''))
call assert_equal('# %s', &commentstring)
" The iRead option here is a highlighting probe, not a syntax-validity claim.
call setline(1, ['iWrite r 0xAA', 'set s "[iCall ns::proc]"',
      \ 'set s "[iRead r 0xAA -hex]"',
      \ 'set data "[iPDLLevel 1 -version STD_1687_2014]"',
      \ 'iNote "STD_1687_2014"',
      \ 'set data "[iNote "STD_1687_2014"]"'])
call CheckGroup(1, 'iWrite', 'pdlCommand')
call CheckGroup(2, 'ns::proc', 'pdlProcedure')
call CheckGroup(3, '-hex', 'pdlOption')
call CheckGroup(4, '-version', 'pdlOption')
call CheckGroup(4, 'STD_1687_2014', 'pdlConstant')
call CheckGroup(5, 'STD_1687_2014', 'tclString')
call CheckGroup(6, 'STD_1687_2014', 'tclString')
execute 'edit! ' . fnameescape($ICL_PDL_TEMP . '/example.icl')
call assert_equal('icl', &filetype, 'override builtin Clean association')
call assert_equal('icl', get(b:, 'current_syntax', ''))
call setline(1, ['Module DataOutPort {',
      \ '  DataOutPort result[7:0] { Source reg; }',
      \ "  ScanRegister reg[7:0] { ResetValue 8'hA5; }",
      \ '  Parameter width = $WIDTH;',
      \ '  Attribute note = "iWrite // Module";',
      \ '  // Module iWrite TODO',
      \ '  /* DataOutPort',
      \ '     iProc */',
      \ '  Instance child Of ns::Leaf;',
      \ '  Enum mode { Ready = 1; }',
      \ '  iWrite x 0xAA;',
      \ '}'])
call CheckGroup(1, 'Module', 'iclTypeKeyword')
call CheckGroup(1, 'DataOutPort', 'iclTypeName')
call assert_equal('Special', synIDattr(synIDtrans(synID(1, 8, 1)), 'name'))
call CheckGroup(2, 'DataOutPort', 'iclPort')
call CheckGroup(2, 'result', 'iclName')
call CheckGroup(3, "8'hA5", 'iclNumber')
call CheckGroup(4, '$WIDTH', 'iclParameter')
call CheckGroup(5, 'iWrite', 'iclString')
call CheckGroup(6, 'Module', 'iclLineComment')
call CheckGroup(6, 'TODO', 'iclTodo')
call CheckGroup(8, 'iProc', 'iclBlockComment')
call CheckGroup(9, 'ns::Leaf', 'iclTypeName')
call CheckGroup(10, 'mode', 'iclTypeName')
call setline(13, 'ActivePolarity High; LaunchEdge Rising; AccessLink a of Generic {}')
call CheckGroup(13, 'High', 'iclConstant')
call CheckGroup(13, 'Rising', 'iclConstant')
call CheckGroup(13, 'Generic', 'iclConstant')
call CheckGroup(11, 'iWrite', '')
call CheckGroup(11, '0xAA', '')
call assert_equal('// %s', &commentstring)
call assert_equal([3, 6, 0], [&shiftwidth, &tabstop, &expandtab])
let icl_comments = &comments
execute b:undo_ftplugin
call assert_equal(&g:comments, &comments)

" Numbers may span lexer NUMGAP whitespace/comments, which retain their groups.
call setline(1, ["Module M { Parameter P = 'h", 'A5;',
      \ "Parameter Q = 'h /* comment */ A5;",
      \ "Parameter R = $WIDTH /* width */ 'B // digits", '10_x;',
      \ "Parameter S = 8", "/* width */ 'D /* digits", '*/ 12_3;',
      \ "Parameter T = 'H /* stars **/ FF; Parameter after = 1;",
      \ 'ToClockPort c { Period 10ns; }',
      \ 'Attribute text = "8''h /* text */ A5 10ns";',
      \ "Parameter bad = 'h;", 'A5; Parameter following = 2;',
      \ "Parameter bad = 'b /* no digits */ ;", 'FACE;',
      \ "Parameter bad = 'h /* closed */ ; /* separate */ FACE;",
      \ 'Module Next {}', '}'])
call CheckGroup(1, "'h", 'iclNumber')
call CheckGroup(2, 'A5', 'iclNumber')
call CheckGroup(3, "'h", 'iclNumber')
call CheckGroup(3, 'comment', 'iclBlockComment')
call CheckGroup(3, 'A5', 'iclNumber')
call CheckGroup(4, '$WIDTH', 'iclNumber')
call CheckGroup(4, 'width', 'iclBlockComment')
call CheckGroup(4, "'B", 'iclNumber')
call CheckGroup(4, 'digits', 'iclLineComment')
call CheckGroup(5, '10_x', 'iclNumber')
call CheckGroup(6, '8', 'iclNumber')
call CheckGroup(7, 'width', 'iclBlockComment')
call CheckGroup(7, "'D", 'iclNumber')
call CheckGroup(7, 'digits', 'iclBlockComment')
call CheckGroup(8, '*/', 'iclBlockComment')
call CheckGroup(8, '12_3', 'iclNumber')
call CheckGroup(9, 'stars', 'iclBlockComment')
call CheckGroup(9, 'FF', 'iclNumber')
call CheckGroup(9, 'after', 'iclName')
call CheckGroup(10, '10ns', 'iclNumber')
call CheckGroup(10, 'ns', 'iclNumber')
call CheckGroup(11, 'A5', 'iclString')
call CheckGroup(11, '10ns', 'iclString')
call CheckGroup(13, 'A5', '')
call CheckGroup(13, 'following', 'iclName')
call CheckGroup(14, 'no digits', 'iclBlockComment')
call CheckGroup(15, 'FACE', '')
call CheckGroup(16, 'FACE', '')
call CheckGroup(17, 'Module', 'iclTypeKeyword')
for unit in ['as', 'fs', 'ps', 'ns', 'us', 'ms', 's']
  call setline(19, 'ToClockPort c { Period 1_000' . unit . '; }')
  call CheckGroup(19, '1_000' . unit, 'iclNumber')
  call CheckGroup(19, unit . ';', 'iclNumber')
endfor
call setline(19, ["Parameter P = 'h /* stars ***", '*/ A5;'])
call CheckGroup(19, "'h", 'iclNumber')
call CheckGroup(19, 'stars', 'iclBlockComment')
call CheckGroup(20, 'A5', 'iclNumber')
call setline(19, ['Parameter bad = 10nseconds;',
      \ "Parameter bad = 'h /* unterminated", 'A5;'])
call CheckGroup(19, '10nseconds', '')
call CheckGroup(21, 'A5', 'iclBlockComment')

execute 'edit! ' . fnameescape($ICL_PDL_TEMP . '/example.pdl')
call assert_equal('pdl', &filetype)
call assert_equal('pdl', get(b:, 'current_syntax', ''))
call setline(1, ['iPDLLevel 1 -version STD_1687_2014',
      \ 'iProcsForModule Demo',
      \ 'iProc run {value} {',
      \ '  iWrite reg[7:0] 0xAX',
      \ '  iRead reg 0b1x',
      \ '  if {$value} { iCall child.ns::helper }',
      \ '  set status "value=[iGetStatus]"',
      \ '  iNote "iWrite Module 0xAA"',
      \ '  # iWrite Module TODO',
      \ '  set text value#not_comment',
      \ '  Module Foo',
      \ '}'])
call CheckGroup(1, 'iPDLLevel', 'pdlCommand')
call CheckGroup(1, '-version', 'pdlOption')
call CheckGroup(1, 'STD_1687_2014', 'pdlConstant')
call CheckGroup(3, 'run', 'pdlProcedure')
call CheckGroup(4, 'iWrite', 'pdlCommand')
call CheckGroup(4, '0xAX', 'pdlNumber')
call CheckGroup(5, '0b1x', 'pdlNumber')
call CheckGroup(6, 'if', 'tclConditional')
call CheckGroup(6, '$value', 'tclVarRef')
call CheckGroup(6, 'child.ns::helper', 'pdlProcedure')
call CheckGroup(7, 'iGetStatus', 'pdlCommand')
call CheckGroup(8, 'iWrite', 'tclString')
call CheckGroup(9, 'iWrite', 'tclComment')
call assert_notequal('tclComment', synIDattr(synID(10, stridx(getline(10), '#') + 1, 1), 'name'))
call CheckGroup(11, 'Module', '')
call setline(13, 'iState reg 0xAA -LastReadValue; iSetFail "bad" -quit')
call CheckGroup(13, '-LastReadValue', 'pdlOption')
call CheckGroup(13, '-quit', 'pdlOption')
call setline(14, ['iGetReadData reg -hex',
      \ 'set data "[iGetReadData reg -hex]"',
      \ 'iNote "iGetReadData reg -hex"'])
call CheckGroup(14, '-hex', 'pdlOption')
call CheckGroup(15, '-hex', 'pdlOption')
call CheckGroup(16, '-hex', 'tclString')
" IEEE 1687-2014 7.9.21: tags are options, but quoted note text is data.
call setline(17, ['iNote -comment "test conditions should be stable now"',
      \ 'iNote -status "INFO: PLLs started"',
      \ 'set note "[iNote -status "ready"]"',
      \ 'iNote -comment "-status is literal text"'])
call CheckGroup(17, '-comment', 'pdlOption')
call CheckGroup(18, '-status', 'pdlOption')
call CheckGroup(19, '-status', 'pdlOption')
call CheckGroup(20, '-status', 'tclString')
call assert_equal('# %s', &commentstring)
call assert_equal([3, 6, 0], [&shiftwidth, &tabstop, &expandtab])

execute 'edit! ' . fnameescape($ICL_PDL_TEMP . '/plain.tcl')
call assert_equal('tcl', &filetype)
call setline(1, 'iWrite register 0b1')
call assert_notequal('pdlCommand', synIDattr(synID(1, 1, 1), 'name'))
execute 'edit! ' . fnameescape($ICL_PDL_TEMP . '/plain.cpp')
call assert_equal('cpp', &filetype)

execute 'edit! ' . fnameescape($ICL_PDL_TEMP . '/new.icl')
call assert_equal('icl', &filetype, 'new ICL buffer')
execute 'edit! ' . fnameescape($ICL_PDL_TEMP . '/new.pdl')
call assert_equal('pdl', &filetype, 'new PDL buffer')

execute 'edit! ' . fnameescape($ICL_PDL_TEMP . '/keywords.icl')
for line in range(1, line('$'))
  call assert_match('^icl', synIDattr(synID(line, 1, 1), 'name'), getline(line))
endfor
execute 'edit! ' . fnameescape($ICL_PDL_TEMP . '/keywords.pdl')
for line in range(1, line('$'))
  call assert_equal('pdlCommand', synIDattr(synID(line, 1, 1), 'name'), getline(line))
endfor

" Force syntax queries across representative ~1 MB buffers, not just file loading.
let timings = []
for language in ['icl', 'pdl']
  execute 'edit! ' . fnameescape($ICL_PDL_TEMP . '/large.' . language)
  let sample = language ==# 'icl'
        \ ? "ScanRegister reg[31:0] { ScanInSource scan_in; CaptureSource data_in; ResetValue 32'h00000000; } // note"
        \ : 'iWrite instrument.register[31:0] 0x12345678; iRead instrument.register[31:0] 0x12345678; iApply; # note'
  call setline(1, repeat([sample], 10000))
  let started = reltime()
  let expected = language ==# 'icl' ? 'iclDeclaration' : 'pdlCommand'
  for line in range(1, line('$'))
    call assert_equal(expected, synIDattr(synID(line, 1, 1), 'name'))
    call synID(line, strlen(sample), 1)
  endfor
  call add(timings, printf('%s bytes=%d seconds=%s', language,
        \ (strlen(sample) + 1) * line('$'), reltimestr(reltime(started))))
endfor
call writefile(timings, $ICL_PDL_TEMP . '/timings')
let helpfile = globpath(&runtimepath, 'doc/vim-icl-pdl-modern.txt', 0, 1)[0]
execute 'helptags ' . fnameescape(fnamemodify(helpfile, ':h'))
help vim-icl-pdl-modern
call assert_equal('help', &filetype)
call assert_equal('vim-icl-pdl-modern.txt', expand('%:t'))

call writefile(v:errors, $ICL_PDL_RESULT)
if !empty(v:errors)
  cquit
endif
qa!
