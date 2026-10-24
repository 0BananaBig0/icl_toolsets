" Query the actual collected files as text; never source/evaluate corpus input.
set nocompatible nomore noswapfile nomodeline
set synmaxcol=0
execute 'set runtimepath^=' . fnameescape($ICL_PDL_PLUGIN)
filetype plugin on
syntax enable
let manifest = json_decode(join(readfile($ICL_PDL_TEMP . '/manifest.json'), "\n"))
let report = {'files': 0, 'lines': 0, 'probes': 0, 'errors': []}
let started = reltime()
try
  for path in manifest.files
    let v:errmsg = ''
    execute 'edit ' . fnameescape(path)
    call assert_equal($ICL_PDL_LANGUAGE, &filetype, path)
    call assert_equal($ICL_PDL_LANGUAGE, &syntax, path)
    call assert_equal($ICL_PDL_LANGUAGE, get(b:, 'current_syntax', ''), path)
    " Sequential queries force syntax processing through every line, including
    " long lines and malformed fixtures. They are not a per-token oracle.
    for lnum in range(1, line('$'))
      let text = getline(lnum)
      call synID(lnum, max([1, match(text, '\S') + 1]), 1)
      call synID(lnum, max([1, strlen(text)]), 1)
    endfor
    for probe in get(manifest.probes, path, [])
      let matches = []
      for lnum in range(1, line('$'))
        if stridx(getline(lnum), probe[0]) >= 0
          call add(matches, lnum)
        endif
      endfor
      call assert_equal(1, len(matches), path . ': unique anchor ' . probe[0])
      if len(matches) == 1
        let lnum = matches[0]
        let column = stridx(getline(lnum), probe[1]) + 1
        call assert_true(column > 0, path . ': token ' . probe[1])
        call assert_equal(probe[2], synIDattr(synID(lnum, column, 1), 'name'),
              \ path . ':' . lnum . ': ' . probe[1])
      endif
      let report.probes += 1
    endfor
    call assert_equal('', v:errmsg, path)
    let report.files += 1
    let report.lines += line('$')
    bwipeout!
    if !empty(v:errors)
      break
    endif
  endfor
catch
  call add(v:errors, path . ': ' . v:exception . ' at ' . v:throwpoint)
endtry
let report.seconds = reltimefloat(reltime(started))
let report.errors = v:errors
call writefile([json_encode(report)], $ICL_PDL_TEMP . '/result.json')
if !empty(v:errors)
  cquit
endif
qa!
