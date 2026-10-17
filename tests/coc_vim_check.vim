" Verifies the real chain:  vim -> coc.nvim (node server) -> icl-lsp.
"
" Nothing here is simulated. coc.nvim is loaded as a real plugin, spawns its
" own node server, that server reads coc-settings.json and launches icl-lsp as
" a child process, and diagnostics plus formatting are read back through coc's
" own API — so a break anywhere in that chain shows up as a failing check.
"
" Run:
"   COC_ROOT=/tmp/cocvim COC_PLUGIN=/tmp/cocvim/coc.nvim \
"     script -qec "vim -N -u NONE -S tests/coc_vim_check.vim" /dev/null
"
" Two things about the driver shape, both forced by vim's startup order:
"
" * The checks run from a VimEnter autocmd. A `-S` script executes *before*
"   VimEnter, and coc only initialises its service on VimEnter — so anything
"   done directly in the script body sees a half-built coc.
" * A pty is required. With stdin closed vim exits as soon as the script ends,
"   before any of the async work can complete.
"
" Results go to $COC_ROOT/result.txt; stdout belongs to vim.

let s:root = $COC_ROOT
let s:plugin = $COC_PLUGIN
let s:out = []

function! s:log(msg)
  call add(s:out, a:msg)
endfunction

function! s:check(name, ok, detail)
  if a:ok
    call s:log('  [PASS] ' . a:name)
  else
    call s:log('  [FAIL] ' . a:name . '  ' . string(a:detail))
  endif
endfunction

function! s:Finish()
  call writefile(s:out, s:root . '/result.txt')
  qa!
endfunction

" ---- environment -----------------------------------------------------------

set nocompatible
set encoding=utf-8
set hidden
set noswapfile
filetype plugin on

execute 'set runtimepath^=' . s:plugin

" Every g:coc_* variable has to be set *before* coc's plugin is sourced: that
" is when it resolves the config and data home. Setting them afterwards looks
" harmless and silently leaves coc reading the user's real ~/.vim instead —
" the language server never appears and `CocAction('services')` comes back [].
let g:coc_node_path = '/usr/local/bin/node'
let g:coc_config_home = s:root . '/config'
let g:coc_data_home = s:root . '/data'
let g:coc_disable_startup_warning = 1

" `-u NONE` skips plugin loading, so pull coc's own plugin/ files in by hand.
" Without this the commands and autocmds coc needs are never defined.
runtime! plugin/**/*.vim

" coc launches a language server for the configured filetypes only, so *.icl
" must be recognised *as icl*.
"
" Note `set filetype=` rather than `setfiletype`: vim's own filetype.vim maps
" *.icl to `clean` (the Clean programming language), and `setfiletype` is a
" no-op once a filetype is set — the gentle form silently loses, and then no
" language server is ever started.
augroup icl_ft
  autocmd!
  autocmd BufNewFile,BufRead *.icl set filetype=icl
augroup END

call s:log('=== environment ===')
call s:log('vim ' . string(v:version) . '  job=' . string(has('job')))
call s:check('vim has +job (coc needs it to spawn node)', has('job') == 1, '')
call s:check('coc.nvim plugin is on runtimepath',
      \ isdirectory(s:plugin . '/autoload') == 1, s:plugin)

" ---- async test driver ----------------------------------------------------
"
" Diagnostics and formatting are asynchronous, and vim has no blocking wait,
" so each step polls from a timer until its condition holds or it times out.

let s:stage = 0
let s:waited = 0
let s:limit = 45          " 45 polls x 1s = 45s budget per stage

" coc starts the language server lazily and asynchronously, so the checks have
" to wait for it rather than assume it is up.
function! s:IclServerRunning()
  try
    for l:service in CocAction('services')
      if l:service['id'] ==# 'languageserver.icl' && l:service['state'] ==# 'running'
        return 1
      endif
    endfor
  catch
  endtry
  return 0
endfunction

function! s:Poll(timer_id)
  let s:waited += 1

  if s:stage == 0
    " Wait for coc's own service handshake to finish.
    if get(g:, 'coc_service_initialized', 0) == 1
      call s:log('')
      call s:log('=== A. vim starts coc.nvim ===')
      call s:check('coc service initialised',
            \ get(g:, 'coc_service_initialized', 0) == 1, '')
      call s:BeginDiagnostics()
    endif

  elseif s:stage == 1
    " Wait for the language server to reach `running` before trusting an empty
    " diagnostic list: before that, "no diagnostics" just means "not asked yet".
    if !s:IclServerRunning()
      " keep polling
    elseif s:waited < s:limit
      let l:diags = s:DiagnosticsHere()
      if len(l:diags) > 0
        call s:log('  diagnostic: ' . string(l:diags[0]))
        let s:d = l:diags[0]
        call s:check('coc reports a diagnostic for an invalid file', 1, '')
        " coc's diagnosticList is 1-based; the offending `}` is source line 3.
        call s:check('diagnostic is on the expected line (1-based 3)',
              \ get(s:d, 'lnum', -1) == 3, get(s:d, 'lnum', 'missing'))
        call s:check('diagnostic has a message',
              \ strlen(get(s:d, 'message', '')) > 0, get(s:d, 'message', ''))
        call s:BeginClean()
      endif
    endif

  elseif s:stage == 2
    " The clean file should stay clean; only assert once we have given the
    " server a fair window to say otherwise.
    if s:waited >= 6
      call s:log('')
      call s:log('=== C. clean file stays clean ===')
      call s:check('valid file -> no diagnostics',
            \ len(s:DiagnosticsHere()) == 0, s:DiagnosticsHere())
      call s:BeginFormat()
    endif

  elseif s:stage == 3
    if s:waited >= 6
      call s:log('')
      call s:log('=== D. vim formats through coc through icl-lsp ===')
      let s:before = getline(1, '$')
      call s:log('  before: ' . string(s:before))
      let s:threw = ''
      try
        call CocAction('format')
      catch
        let s:threw = string(v:exception)
      endtry
      call s:check('CocAction("format") ran without throwing',
            \ empty(s:threw), s:threw)
      call s:BeginAfter()
    endif

  elseif s:stage == 4
    if s:waited >= 6
      let s:after = getline(1, '$')
      call s:log('  after:  ' . string(s:after))
      call s:check('buffer was reformatted into multiple lines',
            \ len(s:after) > 1, s:after)
      call s:check('output is indented with 2 spaces',
            \ match(s:after, '^  ScanInPort SI;') >= 0, s:after)
      call s:check('nested block is indented to 4 spaces',
            \ match(s:after, '^    ScanInSource SI;') >= 0, s:after)
      call s:BeginBroken()
    endif

  elseif s:stage == 5
    if s:waited >= 6
      call s:log('')
      call s:log('=== E. syntactically invalid buffer is not mangled ===')
      let s:broken_before = getline(1, '$')
      try
        call CocAction('format')
      catch
      endtry
      call s:BeginBrokenAfter(s:broken_before)
    endif

  elseif s:stage == 6
    if s:waited >= 6
      let s:broken_after = getline(1, '$')
      call s:check('invalid buffer unchanged by formatting',
            \ s:broken_before == s:broken_after,
            \ {'before': s:broken_before, 'after': s:broken_after})
      call s:Finish()
      return
    endif
  endif

  if s:waited >= s:limit
    call s:log('  [FAIL] stage ' . s:stage . ' timed out after ' . s:waited . ' polls')
    if s:stage == 1
      call s:log('  DEBUG services=' . string(s:SafeServices()))
      call s:log('  DEBUG diags=' . string(s:DiagnosticsHere()))
      call s:check('coc reports a diagnostic for an invalid file', 0,
            \ s:SafeDiagnosticList())
    endif
    call s:Finish()
    return
  endif

  " Poll slowly on purpose: vim only processes channel messages while
  " it is idle, and a tight timer starves coc's own callbacks.
  call timer_start(1000, 's:Poll')
endfunction

function! s:Go(stage)
  let s:stage = a:stage
  let s:waited = 0
endfunction

function! s:SafeServices()
  try
    return CocAction('services')
  catch
    return 'THREW: ' . string(v:exception)
  endtry
endfunction

function! s:SafeDiagnosticList()
  try
    return CocAction('diagnosticList')
  catch
    return []
  endtry
endfunction

" `diagnosticList` is global — every buffer's diagnostics, including stale ones
" from files already left. Filter down to the buffer under test.
function! s:DiagnosticsHere()
  let l:buf = bufnr('%')
  return filter(copy(s:SafeDiagnosticList()),
        \ {_, d -> get(d, 'bufnr', -1) == l:buf})
endfunction

function! s:BeginDiagnostics()
  execute 'edit ' . s:root . '/test/bad.icl'
  call s:log('')
  call s:log('=== B. diagnostics through coc ===')
  call s:check('buffer filetype is icl', &filetype ==# 'icl', &filetype)
  call s:Go(1)
endfunction

function! s:BeginClean()
  execute 'edit ' . s:root . '/test/good.icl'
  call s:Go(2)
endfunction

function! s:BeginFormat()
  execute 'edit ' . s:root . '/test/messy.icl'
  call s:check('messy buffer filetype is icl', &filetype ==# 'icl', &filetype)
  " vim defaults to shiftwidth=8, and coc forwards that as the LSP `tabSize`,
  " so the editor's setting is what decides the indent width. Say 2 explicitly
  " — this is the same knob the CLI exposes as -n, and the point of the check.
  setlocal shiftwidth=2 tabstop=2 expandtab
  call s:Go(3)
endfunction

function! s:BeginAfter()
  call s:Go(4)
endfunction

function! s:BeginBroken()
  execute 'edit ' . s:root . '/test/broken.icl'
  call s:Go(5)
endfunction

function! s:BeginBrokenAfter(before)
  let s:broken_before = a:before
  call s:Go(6)
endfunction

" ---- entry point -----------------------------------------------------------

augroup icl_coc_check
  autocmd!
  autocmd VimEnter * call timer_start(1000, 's:Poll')
augroup END
