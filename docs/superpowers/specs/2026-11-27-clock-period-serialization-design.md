# Clock period operand serialization

This bounded increment of original items 5 and 12 fixes generated period operands.
Parent master: bc6940265a6718c0e8ca24eb65ee096db7576620. The standing user
completion, native execution and integration authorization applies. Task193 is
complete; its records remain historical.

## Evidence and approaches

Sixteen isolated, sequential Tessent Shell 2026.2 probes have input/output hashes
under /tmp/icl-differential-override-local-20261127.tbcoqh0t/results.json and
remote-final/. Remote working directory: lianghuaxiao@192.168.6.20:
/tmp/icl-differential-override-20261127.sd4c4N. Six succeed, ten explicitly reject.
Compact20ns, decimal1.25 us and scientific1.0e-9 s reject as illegal characters.
Separate20 ns, bare20,1250 ns,1 ns and7 as succeed. Fractional0.1 as rejects.
Direct negative-leg overrides reject DifferentialInvOf targets; preserve that
guard. These are version-specific observations, not IEEE prohibitions. The local
v2023.4 iClockOverride manual describes the value and optional unit but does not
state this integer restriction. All probe command Context/Mode fields were read.

Normalize operands in the shared emitter. Translating only in the vendor harness
would leave generated files dependent on that harness; rejecting existing native
literals would remove accepted metadata. Use separate numeric/unit words and
exact integer equivalents where available. Retain native-only fractional values
losslessly when no supported finer unit can express an integer.

## Contract and implementation

- Keep every currently accepted native input spelling and all invalid-input guards.
- Emit two distinct value/unit operands; a string containing a space is one Tcl
  word and does not satisfy this contract. Unitless input still defaults to ns.
- Preserve the original unit when an integer fits it:1000ns becomes1000 ns.
  Refine only as needed:1.25us becomes1250 ns;1.0e-9s becomes1 ns;1.25fs becomes1250 as.
- Use exact decimal digits and powers of ten, with as/fs/ps/ns/us/ms/s scales
  0/3/6/9/12/15/18. No floating point, new dependency or elapsed-time engine.
- Reuse pdl::NormalizeClockPeriod validation. Current exponent spelling requires
  a decimal mantissa and a negative exponent. The existing DecimalBits helper
  converts bounded integers to bits with quadratic work; it is unsuitable here.
- Add a private PeriodWords(compact_validated_period) returning pair<string,string>
  in pattern_clock.cc. Invoke it only for requested normalized output. Do not
  change public validation, routing, checked versions, clocks, ownership or state.
- Cancel coefficient trailing zeros, fractional digits and the negative exponent.
  Select the nearest supported finer unit that yields an integer. Work and space
  are linear in literal size. Never allocate a number of zeros determined by an
  unbounded exponent; an exponent too large to cancel within the input yields
  lossless native operands. Fractional-attosecond values likewise remain native
  annotations, with no vendor-compatibility claim.
- Preserve ordinary output byte for byte, period-program ordering, logical target
  names, partial mixed-override rejection, reset/procedure behavior and all
  physical/raw/opaque/differential/stateful-time boundaries. No limit increase.

## Verification and limits

Test literal word boundaries and units, independent Decimal quantity equality,
leading/trailing zeros, exact exponents, native-only fallbacks, procedure and
mixed-source ordering, and a100000-zero coefficient/exponent cancellation (`"1" + "0"*100000 + ".0e-100000s"` →1 s).
Maintain existing native snapshot/copy/rollback assertions and all ten period
CLI methods with new exact expected operand spellings. Five-form native replay
must remain byte-identical. New test scripts live under tests/src/pattern/.

Measure the nine2/8/32-oscillator by8/128/1024-update five-form programs, the
single100000-zero case and matched ordinary parent/current controls. Each
workload must meet10seconds/512MiB; these targets do not assert their Cartesian
product or isolated speedups. Record wall time, GNU time RSS, environment and
input/product hashes. Full make check, all native/affected CLI sanitizers,
changed C++ format/tidy/C++17 checks and packaging precede one fresh review.

Run fresh sequential vendor pairs against the original ICL: independently
specified integer/unit references and native output from compact, decimal,
scientific and default-unit input. Assert actual exports/command traces agree.
Exports omit period metadata; this proves configured-run acceptance and
normalization, not standalone export replay, device behavior or timing signoff.
Vendor pairs consume canonical main PDL. Reset-only output preserves source
and immutable procedures, so native-only input spellings there need another
native generation pass before vendor loading.
Native-only sub-attosecond/extreme-exponent literals remain outside that vendor
claim. Broader original items,35 inherited deferred Minors and excluded item1
remain open. Integrate verified work into master, push and retire only this branch.
