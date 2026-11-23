# icl_toolsets

Format, inspect, and navigate IEEE 1687-2014 ICL and PDL in the terminal,
Vim, or Neovim. Generate flattened PDL for supported serial, single-TAP and
primary-input DataRegister access networks.

| Tool | Purpose |
|---|---|
| `icl-format` | Format ICL files or standard input; check formatting in CI. |
| `icl-lsp` | ICL diagnostics, completion, navigation, symbols, and editor formatting. |
| `pdl-format` | Format PDL while respecting Tcl scripts, data, and substitutions. |
| `pdl-lsp` | PDL diagnostics, completion, navigation, symbols, and editor formatting. |
| `icl-pattern-generation` | Experimental PDL expansion and prerequisite scheduling for serial, single-TAP and primary-input DataRegister networks. |
| `vim-icl-pdl-modern` | Standalone ICL/PDL highlighting and filetype detection. |

ICL (**Instrument Connectivity Language**) and PDL (**Procedural Description
Language**) are separate languages. Each language's formatter and language server
share a frontend and formatting engine. The editor tools parse source without
executing PDL/Tcl or generating patterns.

Install the executables for formatting and language services. For highlighting
alone, [install the Vim plugin](#highlighting-only) without building anything.
The generator is available with a [serial and single-TAP backend](#generate-pdl-patterns); full retargeting remains under development.

## Build and install

On Linux, the build requires:

- A C++17 compiler and standard library with `std::filesystem` support.
- GNU Make 4.3+, Bison 3.8+, and Flex 2.6+.
- For PDL: `pkg-config` and Tcl development headers/libraries, version 8.6 or
  newer. A Tcl stub library alone is insufficient.
- For pattern generation: Z3 development headers/libraries. Install the system
  package or run the explicit `make z3-deps` bootstrap before `make dynamic`.

```sh
git clone https://gitee.com/banana33/icl_toolsets.git
cd icl_toolsets
make dynamic
make install-dynamic
export PATH="$HOME/.local/bin:$PATH"

icl-format --version
icl-lsp --version
pdl-format --version
pdl-lsp --version
icl-pattern-generation --version
```

`make dynamic` builds all five commands in `build/`. Installation puts them in
`~/.local/bin` and prints coc.nvim configuration instructions; it does not modify
editor settings or install the highlighting plugin. All five commands support
`--help`, `-h`, and `--version`, which print information and exit.

Keep the installation directory on your editor's `PATH`, or use absolute server
paths. To install elsewhere:

```sh
make install-dynamic prefix="$HOME/opt/icl_toolsets"
```

Dynamic binaries require compatible C/C++ runtime libraries and, for PDL,
the matching Tcl shared library on the target machine. Use
`BISON=/path/to/bison` when selecting a newer Bison installation.

### Build variants

| Command | Result |
|---|---|
| `make dynamic` | All five dynamic binaries. |
| `make static` | All five static binaries, named with a `-static` suffix. |
| `make icl-dynamic` / `make icl-static` | Only the ICL pair; no Tcl dependency. |
| `make pdl-dynamic` / `make pdl-static` | Only the PDL pair. |
| `make` | Both variants of all five tools. |

Static builds also need static C/C++ runtime libraries, a real Tcl archive
compatible with the selected Tcl headers, and a static Z3 archive for the
generator:

```sh
make static TCL_STATIC_LIB=/path/to/libtcl9.0.a
make install-static TCL_STATIC_LIB=/path/to/libtcl9.0.a
```

Both install variants use the normal five command names. Installing a different
variant into the same prefix replaces the previous installation. `make install`
selects the dynamic variant. See the [PDL build guide](docs/pdl-tools.md) for Tcl
build instructions and static Tcl/glibc portability limits; verify binaries on
the intended target host.

Use `make uninstall` with the same `prefix` to remove installed tools. Retain
`build/install_manifest.txt` so uninstall can identify installed files.

### Source archives

Archives created by `make dist` include generated parser/scanner sources and can
build without Bison or Flex. Automatic source downloads from GitHub/Gitee contain
tracked sources and require those generators. Generated build artifacts are not
committed to the repository.

The archive package name remains `icl-format`, despite the repository name:

```sh
tar xzf icl-format-1.0.0.tar.gz
cd icl-format-1.0.0
make dynamic
make install-dynamic
```

Compiler, Make, Tcl and Z3 requirements apply to both archive types.

## Format files

Both formatters accept the same options:

```sh
icl-format top.icl                   # print formatted source
icl-format -w top.icl                # rewrite in place
icl-format -w block_a.icl block_b.icl # format several files
icl-format --check *.icl             # check without rewriting
icl-format --indent 4 top.icl        # four-space indentation
icl-format -o formatted.icl top.icl  # write a separate file
icl-format < top.icl                 # read standard input

pdl-format procedures.pdl
pdl-format -w procedures.pdl
pdl-format --check *.pdl
pdl-format --indent 4 < procedures.pdl
```

CLI indentation defaults to two spaces, with a maximum of 16. The CLI does not
read vimrc or `.clang-format`. Use [LSP formatting](#vimneovim-with-cocnvim) to
reuse editor indentation settings without specifying `--indent`.

ICL formatting preserves comments and strings, normalizes spacing and
indentation, and retains at most one consecutive blank line. PDL formatting
indents recognized braced script bodies, including `iProc`, `proc`, and Tcl
control flow. It preserves the spelling of data words and substitutions; quoted
or dynamic script bodies are not reflowed. Most PDL blank lines are removed.

Invalid inputs receive diagnostics and are not rewritten. With several inputs,
valid files can still be processed if another fails. A second formatting pass
leaves the first pass's output unchanged; `-w` also preserves the modification
time of an unchanged file.

| Exit code | Meaning |
|---|---|
| `0` | Success; with `--check`, all files are already formatted. |
| `1` | `--check` found formatting changes. |
| `2` | Invalid source, a detected local semantic error, an I/O error, or invalid arguments. |

File writes use a sibling temporary file, so the destination directory must be
writable. Existing symlink targets, ownership, and permission bits are preserved;
dangling symlink outputs are rejected. Other hard links retain their original
contents. Replacement does not preserve extended attributes or ACLs.

## Vim/Neovim with coc.nvim

Install [coc.nvim](https://github.com/neoclide/coc.nvim) and its editor/Node.js
requirements. Make the language servers available on the editor's `PATH`.

Open `:CocConfig` and merge these entries into the existing `languageserver`
object, alongside any other servers such as `verilog`:

```json
{
  "languageserver": {
    "icl": {
      "command": "icl-lsp",
      "filetypes": ["icl"],
      "rootPatterns": [".git"]
    },
    "pdl": {
      "command": "pdl-lsp",
      "filetypes": ["pdl"],
      "rootPatterns": [".git"]
    }
  }
}
```

Each entry supplies both language services and formatting. A separate formatter
plugin is unnecessary. The same configuration is provided in
[docs/editor/coc-lsp.json](docs/editor/coc-lsp.json).

The highlighting plugin supplies filetype detection. Without it, add this to
vimrc or your Neovim Vimscript configuration:

```vim
augroup icl_toolsets
  autocmd!
  autocmd BufNewFile,BufRead *.icl setlocal filetype=icl
  autocmd BufNewFile,BufRead *.pdl setlocal filetype=pdl
  autocmd FileType pdl if empty(globpath(&runtimepath, 'syntax/pdl.vim')) | setlocal syntax=tcl | endif
augroup END
```

Setting `icl` explicitly overrides Vim's built-in `.icl` association with the
Clean language. Remove older unconditional `setlocal syntax=tcl` PDL autocmds
when using the highlighting plugin.

Reuse your coc.nvim mappings or add:

```vim
nmap <silent> gd <Plug>(coc-definition)
nmap <silent> gi <Plug>(coc-implementation)
nmap <silent> gr <Plug>(coc-references)
```

Run `:CocRestart`, reopen the file, and format the buffer with:

```vim
:call CocAction('format')
```

Formatting uses coc.nvim's `tabSize`, normally derived from `shiftwidth`, or
`tabstop` when `shiftwidth` is zero. No `--indent` server argument is needed.
Output uses spaces even with `noexpandtab`; indentation is capped at 16.
Invalid buffers receive diagnostics and no formatting edits.

For optional format-on-save:

```vim
augroup icl_pdl_format_on_save
  autocmd!
  autocmd BufWritePre *.icl,*.pdl call CocAction('format')
augroup END
```

An optional [coc-diagnostic CLI adapter](docs/editor/coc-cli.json) invokes
`icl-format` with explicit indentation. It has not been tested end to end.

### Language services

Both servers provide diagnostics, completion, definition/declaration,
implementation, references, hover, document/workspace symbols, and whole-document
formatting.

- **ICL:** keyword and scoped-name completion, instance members, module types,
  and local semantic checks. Implementation navigation finds ICL module definitions.
- **PDL:** Tcl/PDL syntax checks, recognized command options and literal values,
  level-0 restrictions, and static procedure/variable navigation. Tcl and PDL
  namespaces and procedure-local variables are distinguished.
- **PDL references to ICL:** module bindings and statically resolvable objects,
  including paths through `iPrefix`.

Workspace navigation includes unopened `.icl` files for ICL, and `.pdl`, `.tcl`,
and `.icl` files for PDL. Hidden directories, `build/`, and symlinked directories
are skipped. Open buffers take precedence over disk files. Ambiguous definitions
remain unresolved.

### Troubleshooting

- Check `:setlocal filetype?`: it should be `icl` or `pdl`.
- Check `:echo executable('icl-lsp')` or `:echo executable('pdl-lsp')`.
  Configure absolute command paths if the editor has a different `PATH`.
- Inspect `:CocInfo` and `:CocCommand workspace.showOutput`.
- A server started without arguments in a terminal waits for LSP messages.
  Use `--help` or `--version` for an immediate response.
- Diagnostics report the first error per document. Correct it to reveal later errors.

## Highlighting only

[vim-icl-pdl-modern](vim-icl-pdl-modern/README.md) provides colors and filetype
detection without coc.nvim, a compiler, or the executables. It follows your
colorscheme and leaves indentation preferences to you.

With [vim-plug](https://github.com/junegunn/vim-plug), add this between your
existing `plug#begin()` and `plug#end()` calls:

```vim
Plug '0BananaBig0/icl_toolsets', {'rtp': 'vim-icl-pdl-modern', 'for': ['icl', 'pdl']}
```

This assumes publication at https://github.com/0BananaBig0/icl_toolsets. Reload
your configuration, run `:PlugInstall`, and reopen the editor and a source file.

`rtp` loads only the `vim-icl-pdl-modern/` directory as a plugin. Vim-plug downloads
the repository but does not build or install the executables or their
dependencies. To avoid downloading the rest of the repository, install a local
copy of that directory using the [plugin guide](vim-icl-pdl-modern/README.md).

The plugin has separate ICL highlighting and extends Vim's Tcl syntax for PDL,
including all 24 IEEE PDL command names and recognized options. Ordinary `.tcl`
filetypes are unchanged. Highlighting does not validate syntax; use a language
server for diagnostics and navigation.

## Scope and limitations

The editor tools target IEEE 1687-2014 and retain documented compatibility cases
from Tessent and Synopsys files. They do not replace vendor design checks.

- ICL editor analysis does not fully elaborate connectivity, arithmetic-derived
  widths, or child-module parameter overrides. Generic AccessLink payloads are opaque.
- PDL editor analysis does not execute Tcl or resolve runtime-generated names
  and scripts. Escaped `switch` list bodies and scripts assembled from multiple
  `namespace eval` arguments have limited static analysis. NUL input and nesting
  above 128 levels are rejected. See [PDL syntax limits](docs/pdl-syntax-oracles.md).
- Symbol rename, range formatting, inferred HDL implementation targets, and
  assignment-column alignment are not provided.

Regression covers standard examples, vendor/public fixtures, malformed input,
CLI/LSP consistency, and Vim/Neovim integration. Formatter tests compare reviewed
expected output and check that formatting it again makes no changes. See
[formatter self-tests](docs/formatter-self-tests.md).

Tessent-qualified regression standards are also available:

```sh
make tessent-golden-check BISON=/usr/bin/bison
```

This check is included in `make check` and needs no Tessent license. It compares
local flat patterns and formatted ICL/PDL against explicitly certified standards,
including a second-pass stability check. Tessent runs only when deliberately
creating new standards. Formatter certification compares both exported PDL and
the complete Tessent Verilog testbench bundle before and after formatting; only
the leading generation-date comment is normalized. Generator qualification checks
both round-trip directions separately: our flat PDL through Tessent, or Tessent's
source-PDL export through our generator. Either passing direction qualifies the
case; independent source schedules need not match. `iApply` order is preserved,
while independent writes within one group may reorder. Fifteen directional standards
cover scalar/vector operations, reserved SIBs, same-update SIB closure/reopening
and transitive local-reset release, alongside the seven legacy standards
(including six formatter cases with testbench evidence).
This comparison does not run RTL simulation.
See [qualification, limitations and refresh commands](docs/tessent-goldens.md).

Passing regression does not establish complete language conformance. The strict
handwritten-code coverage target remains unmet. ICL has been tested in a live
Vim/coc.nvim session; a full PDL coc.nvim session remains unverified. Detailed
verification results and development requirements belong in [docs/](docs/README.md).

## Generate PDL patterns

`icl-pattern-generation` expands procedures and Tcl computations, then emits
level-0 PDL with explicit prerequisite writes and applies. It currently handles
supported standalone/simultaneous scan chains and common-phase TAP networks
with disjoint lanes, ScanRegisters, muxes, logic-controlled paths and acyclic
local-reset dependencies. Hardware starts
unknown: use explicit `iState` declarations where appropriate, and `iReset`
before TAP transactions.

Contiguous vector-port functions can share a base name while preserving their
individual types, attributes and enums. Whole/sliced references and child bindings
replay through associated flat models. See [typed port functions and bounds](docs/pattern-generation.md#task142-typed-vector-port-functions).

Flat TAP output includes instruction-register writes, capture comparisons and
their `iApply` boundaries. Explicit `tap.INSTRUCTION` reads/writes can be replayed
using the same BSDL model. If reset permits several instruction encodings, a
read or partial write must first supply the unknown instruction bits explicitly.

For write-only batches in larger networks, a dependency-guided planner groups
reachable reset/selector controls before accessing their targets. It can handle
deep SIB chains beyond the bounded solver's 32-scan horizon and retain open
paths for reuse. Each scan still passes concrete replay; explicit user writes,
including partial writes, retain their requested values. Protected resource bits
remain fixed unless that apply explicitly requests a new value. Cases the heuristic
cannot plan use the existing bounded solver. It does not guarantee globally
shortest patterns. See [scheduling behavior and limits](docs/pattern-generation.md#dependency-guided-scheduling).

`iMerge` blocks are flattened into the same output stream. Compatible calls
containing reads/writes of disjoint ScanRegisters or DataRegisters and writes to unconnected
primary data/control inputs (including slices, aliases and X masks) can
share applies by apply depth, preserving each call's order. Other supported calls
retain serial order, including AccessTogether aliases, waits and resource commands; nested merges are supported. The tool validates each result
through the existing scheduler and concrete replay, and retries serial execution
if a combined schedule fails. `iTake`/`iRelease` protect individual ScanRegister,
DataRegister, DataInPort/DataOutPort and parallel address/read-enable/write-enable
port bits assigned by an applied write or
supported `iState` assumptions, including indexed and alias resources, instance storage ownership and nested-call lifetime handling.
A partial release leaves other taken bits protected. Tessent's default-instance
`iTake` and `-persistent` option are supported within the ordered input stream.
An explicit `iReset` or `iReset -sync` clears existing resource ownership after
the hardware reset and discards queued reads/writes without applying them;
use a new `iTake` to reserve resources again. Malformed queued targets and values
still fail validation. Calls containing
reservations remain serialized. Other serial-control/clock-port reservations and
cross-pattern-set lifecycles remain unsupported; see [resource limits](docs/pattern-generation.md#resource-reservations).

For an ordinary scan network:

```sh
icl-pattern-generation --icl design.icl overrides.icl \
  --pdl setup.pdl test.pdl --top_module Top \
  --output result/Top.pdl --debug --log result/run.log
```

For an AccessLink with a BSDL model:

```sh
icl-pattern-generation --icl design.icl --bsdl DEVICE device.bsdl \
  --pdl setup.pdl test.pdl --output result/Top.pdl --debug
```

Vendor-exported physical patterns can contain scan loads discarded by local reset.
Use `--physical_pdl` to replay those loads literally on a supported single bound
TAP, with one CSU per nonempty `iApply` and comparisons before update. Generated
output retains the shifted bits and must be replayed with the same option.
Ordinary mode continues treating `iWrite` as a final register-value objective.
Physical mode requires binary loads, direct common TAP controls and explicit
`iReset`; it rejects mixed data access, AccessTogether aliases, end-state
defaults, raw scans and `--local_reset`.

The PDL must contain `iReset` before the first TAP transaction. `iReset -sync`
uses five TMS-high cycles even when TRST exists. The scheduler selects IR/DR
transactions and checks each result through concrete controller/event replay.
`iApply -together` requires all queued writes to update on one TCK cycle;
prerequisite scans may precede that cycle. It does not require reads to occur
together. An ICL alias marked `AccessTogether` requires its supported members
to share one scan or parallel-port operation. This includes direct primary-input
writes and primary-output observations. Nested aliases and slices retain the
whole-group constraint; unrelated accesses can use separate operations. Alias `iApplyEndState` supplies
a default for supported ScanRegister, DataRegister or primary-input bits. Explicit writes
override it for one apply; the next apply or end of the input sequence restores
the default. A SIB can close in the update that writes its selected TDR. See the
[runnable alias example](docs/pattern-generation.md#alias-access-group-example)
for generation and replay commands.

`iRead` and `iWrite` accept numeric values or names from the target's `RefEnum`.
An alias uses its own `RefEnum`; generated PDL spells the resolved values as
binary numbers, including X masks.
See the [command support table](docs/pattern-generation.md#command-support)
before choosing a PDL sequence; parsing a command does not imply that the
experimental generator can execute it.

For purely combinational networks, `iWrite` can drive primary DataInPort pins
through data ports and aliases, including OneHotDataGroup Port contributors
with uniform explicit enables. Output X writes retain the target's controlling
value; fixed OneHot width padding follows the measured Tessent behavior.
Networks driven by primary pins also support selectable and addressed
DataRegister writes, cascades, and reads routed to DataOutPorts through forwarding,
DataMuxes and OneHotDataGroups. Shared address buses use separate applies as needed.
Read comparisons do not set register contents. Reads preserve the storage they
observe, including while resolving output enables. Generated PDL and flattened
ICL are checked by replay; see the [DataRegister example and limits](docs/pattern-generation.md#dataregister-access).
The serial and single-TAP backends also support scan-driven selectable/addressed
DataRegister writes and reads captured through ScanRegisters. Capture uses the
value before the same scan's Update; a read and write can share that scan.
Shared-address write banks use address groups to avoid searching write-order
permutations; already-satisfied values share the required controller update.

Explicit DataRegister ReadCallBack/WriteCallBack requests now execute their
PDL procedures at iApply, using real endpoint operations. Reset cancels queued
requests. Groups support separate ordinary requests alongside explicit
callbacks, with known caller write goals protected across procedure frames,
including indirect native effects. Aliases can combine ordinary bits with
explicit callbacks; inversion, sparse ranges, segmented ports and source-preserving
local-reset releases retain their bit and procedure identities.
--model_readback observes actual ordinary endpoints; abstract callback storage
stays unknown. Invoked read-callback histories, automatic downstream callbacks
and physical callback output remain open.
See [mixed alias bits](docs/pattern-generation.md#mixed-ordinary-and-callback-alias-bits).
See [mixed caller groups](docs/pattern-generation.md#mixed-ordinary-and-callback-caller-groups).
See
[write-callback observations](docs/pattern-generation.md#actual-endpoint-observations-in-write-callbacks).
See the [callback example and limits](docs/pattern-generation.md#task153-explicit-dataregister-callback-execution).
For callback-heavy models with unused storage, the measured1,024-target
primary-scheduling case improves from26.21s to2.61s with matching output;
see the [workload and limits](docs/pattern-generation.md#task154-primary-solver-state-in-callback-heavy-models).

Scalar `OneHotScanGroup` returns also work on standalone and single-TAP scan
networks, including nested groups and inverted returns. Scheduling requires one
known-enabled contributor and holds the other contributors disabled; inactive
nested groups may be zero-hot. These one-hot paths have local replay/regression
coverage; they are not new Tessent certificates.

A single physical scan path may be observed by several direct, inverted or
hierarchically forwarded `ScanOutPort` bits. Equivalent observations reuse one
scan transaction. Independently enabled observations of that same path retain
their enable conditions; the scheduler chooses a known-enabled observation.
A standalone client with named `Chain` pairs can also clock disjoint chains
simultaneously, including unequal lengths. Shared storage or scan inputs still
require additional event scheduling. Interface-free networks also infer disjoint
lanes, including vector one-hot returns, when every possible route from an
output reaches the same distinct input bit and no lane shares graph vertices.
This uses the existing common virtual clock; TAPs and declared interfaces still
require their supported event/chain models.

A single scan chain can use indexed bits of a vector `ScanInPort`, including
mux-selected or inverted inputs and one lane of a vector `OneHotScanGroup`.
Replay preserves the other input bits. Declared scan controls require supported
event handling. Explicit or inferred standalone clients now drive capture, shift,
update and clock phases through their bound controls, with concrete replay.
Bounded event planning also tracks hidden storage clocked by those controls.
Disjoint named chains may use separate vector input/output lanes.

`iState` can record numeric assumptions for ScanRegister/DataRegister storage and top-level
DataInPort/SelectPort pins without generating a hardware transaction. Internal
ports and controller-owned instruction state cannot be overwritten this way.
Use --model_readback for concrete PIO and serial-register observations,
including bound TAPs and existing scalar raw interface scans. Named opaque
parallel chains return unknown data with separate variable-width histories.
The mode returns bitwise miscompares and failure counts during generation.
It is opt-in and uses no device transport; timed histories and broader raw
topologies remain open.
See the [runnable readback examples](docs/pattern-generation.md#task152-raw-interface-and-chain-readback).

One declared-width opaque scalar child can be traversed between modeled parent
ScanRegisters, including pure scalar wrappers. Stable/default input supports
implicit preparation; actual opaque returns remain unknown. Dynamic/multiple/
parallel opaque children, raw merges, clock operations and physical PDL remain
unsupported. See the [opaque-child example](docs/pattern-generation.md#one-fixed-width-opaque-child-on-a-scalar-scan-route).

The generator's [verification record](docs/pattern-generation.md#current-verification)
lists regression and replay checks, along with the strict coverage gate that
remains unmet.

For qualified common-phase TAPs after explicit reset, `iRunLoop COUNT` (or `COUNT -tck`)
adds idle TCK cycles and retains the full count in generated PDL. Counts must
be known unsigned 64-bit integers, with no unapplied reads or writes.
Checked system-clock waits (`iClock` followed by `iRunLoop COUNT -sck PORT`)
support relative clock routes that do not drive modeled data or events.
Absolute-time waits (`iRunLoop -time VALUE`) preserve the exact requested
minimum delay on models without storage, opaque hardware, clock-driven data
or forwarded test clocks. General clock events
and stateful absolute-time execution remain unsupported; see the
[clock and time limits](docs/pattern-generation.md#absolute-time-waits-without-clock-driven-state-2026-11-18).

| Option | Behavior |
|---|---|
| `--icl FILE...` | Load ICL in order; later module definitions override earlier ones with warnings. |
| `--pdl FILE...` | Execute PDL in source order; procedure replacements affect subsequent calls. |
| `--bsdl ENTITY FILE` | Load an AccessLink BSDL model; repeat for additional models. |
| `--top_module NAME` | Select the top; otherwise infer a unique root and print a warning. |
| `--output PATH` | Write generated PDL; defaults to `out/<top>.pdl`. |
| `--physical_pdl` | Replay binary scan loads as one exact CSU per nonempty apply on a supported single bound TAP; use the same option for generated-output replay. |
| `--debug` | Also write `debug/<top>.icl` beside the output, including TAP/BSDL metadata for replay. |
| `--log PATH` | Copy stdout diagnostics to a file. |
| `--local_reset PATH` | Write source PDL with transitive local-reset releases before main-level writes, calls, Tcl compounds or whole merge blocks; preserve procedure definitions and combine ordered input files. |

Repeated caller sites can use recorded invocation order when identical arguments
need different releases; procedure text remains unchanged.

The reset-only output releases prerequisite TDRs in dependency order and groups
independent releases under one `iApply`. It chooses release paths that respect
resource reservations, including taken-but-unwritten bits of a control register.
Inserted partial writes avoid acquiring new reservations. It does not insert
SIB-opening commands.
Before publishing it, the tool re-executes the rewritten PDL and verifies the
operation sequence. Whole-loop prerequisites preserve repeated pending writes
and entry prefixes; literal-site fallback retains repairable internal-reset loops.
Repeated literal write/call sites can dispatch different releases from their
once-evaluated argument lists, retaining caller scope and procedure bytes.
Identical arguments requiring different releases, procedure-body repairs and
unsafe queue changes retain diagnostics. See
[local-reset behavior and limits](docs/pattern-generation.md#local-reset-output).

Mixed DataRegister access can alternate primary-input and scan frames, including
single-TAP networks and primary-output reads. The shared scheduler preserves
pending observations and reservations, verifies concrete state transitions, and
retains required scan-controller updates. Forwarded, inverted and computed data-port
writes use the same scheduler, including mux/logic/one-hot sources and supported
`AccessTogether` groups; see [data-port writes](docs/pattern-generation.md#computed-data-port-writes-2026-10-29). Two mixed-I/O cases have forward
Tessent fixed-point certificates; see [support and limits](docs/pattern-generation.md#mixed-parallel-and-serial-frames-2026-10-29).

Independent register accesses and direct primary-input writes, including slices,
aliases and X masks, inside an
`iMerge` can share apply groups, including multi-apply procedures. Each procedure's
apply order is preserved; conflicts and stateful commands retain serial handling.
See [merge behavior](docs/pattern-generation.md#merge-scheduling).

Three further TAP cases covering computed-port writes, child-enable ownership,
and merged observations are certified in both directions with Tessent 2026.2.
The offline suite contains 77 directional certificates across 52 cases
(39 forward, 38 reverse), plus seven legacy certificates; see
[qualification scope](docs/tessent-goldens.md#port-control-and-merge-qualification-2026-10-30).

This is an experimental retargeter. Common-phase multiple TAPs, qualified
relative system-clock waits and bounded feedback reset handling are supported.
Independent TAP phases/shared serial storage, general gated or serial-derived
events, overlapping scan-return lanes, broader resource assignment/lifecycle,
stateful absolute time, additional repairable local-reset rewriting and
comprehensive vendor round-trip qualification remain unfinished. The
[current completion tracker](docs/remaining-work.md) records the remaining
items2–15; strict coverage closure is excluded from that work. Single-TAP shadow registers with phase-qualified capture, shift and
update controls now use event scheduling and full concrete replay, including
bypassed storage. Four slice-reservation probes have fresh Tessent 2026.2
certificates in both directions. See [shadow-register support and bounds](docs/pattern-generation.md#hidden-serial-storage-events-2026-10-29).
Earlier Tessent 2026.2 runs qualify the three reservation probes and
the SIB-close/reopen probe in both directions, and the transitive-reset probe
in the forward direction. The latter feeds our flat output to Tessent; Tessent
does not automatically release the local resets in the original source PDL.
These certified cases run offline in ordinary regression; see
[evidence and remaining limits](docs/tessent-goldens.md#tap-and-reset-qualification-2026-10-28).
Unsupported cases report errors. Search limits are reported as limits, not proof
that a target is unreachable. Generated PDL describes register operations, not
ATE pin-vector files. Input files are protected from output-path aliasing.

Start with the [complete input example](docs/pattern-generation.md#complete-serial-example).
For BSDL-backed access and idle cycles, use the
[runnable TAP example](docs/pattern-generation.md#single-tap-idle-example).
See the [generator guide](docs/pattern-generation.md) for commands, limits,
round-trip checks and validation evidence. The four editor tools remain
independent of PDL execution and pattern generation.

Fixed disjoint modeled root parallel raw chains now retain actual per-chain
readback under one common capture/shift/update group. See the
[parallel raw example and limits](docs/pattern-generation.md#task155-modeled-parallel-raw-scans).

Use make model-corpus-check to audit per-file default models against an explicit
canonical/unsupported registry, with current source and tool evidence. See the
[model audit commands and limits](docs/pattern-generation.md#task156-default-model-and-source-round-trip-audit).

## Documentation and license

The [documentation index](docs/README.md) links user guides, architecture,
requirements, compatibility decisions, and verification records. The
[developer guide](docs/development.md) covers tests and the optional local
`.worktrees/` layout; no worktree directory is required to use the tools.

The project uses the [MIT License](LICENSE). Third-party components and fixtures
retain their notices; see [third-party notices](docs/third-party-notices.md) and
[test sources](tests/cases/SOURCES.md).

Transparent downstream WriteCallBack prerequisites are being qualified for
Source/Alias/selectable WriteDataSource paths. See the complete
[model and replay commands](docs/pattern-generation.md#transparent-downstream-write-callbacks)
and [remaining inventory](docs/remaining-work.md) for current boundaries.
