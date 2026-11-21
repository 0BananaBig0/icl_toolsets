# DataRegister callback model

Increment Task145 of the authorized completion program; base 8ce41f4.
Use the existing elaborator and flat serializer to represent IEEE1687-2014
6.4.9 callbacks before introducing invocation. The current lexer/parser already
accept callback properties. Resolve each name and argument in its declaration
scope once, preserve data/register placeholders distinctly from literal strings,
and retain numeric width and unknown bits. Flat ICL must re-elaborate to the
same callback description and original scope through existing origin metadata.

Reject selectable/addressable/read-callback/write-callback type mixtures at
elaboration, including ReadDataSource mixed with WriteCallBack. Procedure
existence and invocation belong to the subsequent runtime increment: no Tcl
execution, pretend access or observed value is introduced here. Read-only model
serialization is valid even when its callback procedure has not been loaded.

Reuse Expression/Constant, existing parameter evaluation and literal quoting;
no dependency or metadata version change. Expected workload follows existing
object/value limits. Work must be linear in callback tokens/argument bytes,
with no per-declaration scan of the full design or subprocess invocation.

Acceptance: native typed-descriptor assertions; source/flat/source canonical
identity with nested instances, special strings, placeholders, parameterized
names/values and numeric unknowns; atomic invalid-type rejection; existing
native, both generator variants, sanitizer, style/C++17 and source-package checks.
One independent final branch review and one tested blocking-fix pass precede
merge/push/deletion. Item4 remains pending callback execution.
