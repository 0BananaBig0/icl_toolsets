-- Verifies icl-lsp against neovim's *built-in* LSP client.
--
-- This is not a simulation: nvim spawns the server, does the real handshake,
-- converts our diagnostics into its own diagnostic API, and applies our
-- TextEdits to a real buffer.
--
--   LSPBIN=build/icl-lsp nvim --headless -u NONE -c "luafile tests/nvim_lsp_check.lua"

local binary = os.getenv('LSPBIN') or 'build/icl-lsp'
local dir = '/tmp/icl-nvim-lsp'
vim.fn.mkdir(dir, 'p')

local failures = 0

local function check(name, ok, detail)
  if ok then
    print('  [PASS] ' .. name)
  else
    failures = failures + 1
    print('  [FAIL] ' .. name .. '  ' .. tostring(detail))
  end
end

local function make_buf(name, lines)
  local bufnr = vim.api.nvim_create_buf(false, true)
  vim.api.nvim_buf_set_name(bufnr, dir .. '/' .. name)
  vim.api.nvim_buf_set_lines(bufnr, 0, -1, false, lines)
  vim.api.nvim_buf_set_option(bufnr, 'filetype', 'icl')
  return bufnr
end

local client_id, err = vim.lsp.start_client({
  name = 'icl-lsp',
  cmd = {binary},
  root_dir = dir,
})

if not client_id then
  print('could not start client: ' .. tostring(err))
  os.exit(1)
end

print('=== A. diagnostics on an invalid file ===')

local bad = make_buf('bad.icl', {'Module m {', '  ScanInPort SI', '}'})
vim.lsp.buf_attach_client(bad, client_id)
vim.wait(8000, function() return #vim.diagnostic.get(bad) > 0 end, 100)

local diags = vim.diagnostic.get(bad)
check('nvim received a diagnostic', #diags >= 1, vim.inspect(diags))
if #diags >= 1 then
  -- nvim reports 0-based lines; the offending `}` is source line 3.
  check('diagnostic is on the expected line',
        diags[1].lnum == 2, 'lnum=' .. tostring(diags[1].lnum))
  check('diagnostic carries a message',
        type(diags[1].message) == 'string' and diags[1].message ~= '',
        tostring(diags[1].message))
  check('diagnostic severity is Error',
        diags[1].severity == 1, tostring(diags[1].severity))
end

print('=== B. no diagnostics on a valid file ===')

local good = make_buf('good.icl', {'Module m {', '  ScanInPort SI;', '}'})
vim.lsp.buf_attach_client(good, client_id)
-- Give the server a moment; then assert what it reported is empty.
vim.wait(1500, function() return false end, 100)
check('valid file -> no diagnostics',
      #vim.diagnostic.get(good) == 0, vim.inspect(vim.diagnostic.get(good)))

print('=== C. formatting applied to a real buffer ===')

-- The server takes its indent width from the client's `options.tabSize`, which
-- nvim derives from the buffer's shiftwidth. Set it explicitly: nvim's default
-- is 8, and the point of the next check is that the value actually travels.
local function format_with_shiftwidth(name, width)
  local bufnr = make_buf(
      name, {'Module m{ScanInPort SI;ScanRegister r[7:0]{ScanInSource SI;}}'})
  vim.bo[bufnr].shiftwidth = width
  vim.bo[bufnr].tabstop = width
  vim.bo[bufnr].expandtab = true
  vim.lsp.buf_attach_client(bufnr, client_id)
  vim.wait(1500, function() return false end, 100)
  vim.api.nvim_set_current_buf(bufnr)
  vim.lsp.buf.formatting_sync(nil, 8000)
  return vim.api.nvim_buf_get_lines(bufnr, 0, -1, false)
end

local two = format_with_shiftwidth('messy2.icl', 2)
local two_text = table.concat(two, '\n')
check('buffer was reformatted (not left as one line)', #two > 1,
      vim.inspect(two))
check('shiftwidth=2 -> 2-space indent',
      two_text:find('\n  ScanInPort SI;', 1, true) ~= nil, vim.inspect(two))
check('shiftwidth=2 -> nested block at 4 spaces',
      two_text:find('\n    ScanInSource SI;', 1, true) ~= nil,
      vim.inspect(two))

local eight = format_with_shiftwidth('messy8.icl', 8)
local eight_text = table.concat(eight, '\n')
check('shiftwidth=8 -> 8-space indent (tabSize really is honoured)',
      eight_text:find('\n        ScanInPort SI;', 1, true) ~= nil,
      vim.inspect(eight))

print('=== D. a syntactically invalid buffer is left alone ===')

local broken = make_buf('broken.icl', {'Module m {', '  @ bad', '}'})
local before = vim.api.nvim_buf_get_lines(broken, 0, -1, false)
vim.lsp.buf_attach_client(broken, client_id)
vim.wait(1500, function() return false end, 100)
vim.api.nvim_set_current_buf(broken)
vim.lsp.buf.formatting_sync(nil, 8000)
local after = vim.api.nvim_buf_get_lines(broken, 0, -1, false)
check('invalid buffer unchanged by formatting',
      table.concat(before, '\n') == table.concat(after, '\n'),
      vim.inspect(after))

print('')
if failures == 0 then
  print('nvim LSP check: all passed')
else
  print('nvim LSP check: ' .. failures .. ' failure(s)')
end

vim.cmd('qa!')
