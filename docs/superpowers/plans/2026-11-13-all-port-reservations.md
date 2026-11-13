# Reservation membership for every ICL port

Task33 continues the required resource-ownership work. IEEE1687-2014
7.9.19(a,b) and7.9.20(a,b) allow iTake/iRelease on a register, port or instance;
instance ownership includes all its registers and ports. The current resource
classifier omits serial, reset, select and clock ports.

Broaden membership to all declared port nodes plus registers, while preserving
the existing writable/readable target rules. Reservation capability is not
permission to issue iWrite on a clock or scan-control port. Reuse bit masks,
scope indexing, inheritance, persistent takes and release logic. Taken but
unwritten ports remain mutable by automatic scheduling under7.9.19(d).
Existing iScan and iClock generator limitations remain explicit: this step
does not claim support for assigning or changing the frequency of those ports.

Touch the shared resource classifier, PDL access validation caller, reservation
diagnostic, native resource mask/inheritance tests, CLI tests, Make registration,
vendor evidence and current model ledger. Index resources once per scope;
instance traversal remains O(owned objects + selected bits). No new state
representation or broad port polling.

- [x] Add failing native/CLI tests for serial/control/clock ports, vectors,
  instance ownership, nested/persistent release, invalid targets and continued
  rejection of unsupported reads/writes before iReset can discard a queue.
- [x] Broaden the resource classifier and separate access-target validation.
- [x] Run focused/full dynamic/static checks, leak-enabled sanitizers,
  style/naming and independent review; qualify vendor acceptance and record limits.

Existing top-level SelectPort iState assignments are valid under7.9.22 and now
become protectable after iTake; retain positive, blocked-switch and release tests.
Clock/ScanIn/ShiftEn iState assignments remain invalid under that command's
explicit target list. Initial exploratory tests incorrectly assumed broader
iState targets; replace them with negative target-boundary tests.

Validation batching: Tasks33–34 share one final full dynamic/static regression
after both focused/native/sanitizer checks. The source is fixed before that run;
intermediate or stopped runs are not completion evidence.

Combined Task33–34 regression: build/access-state-regression.log/.exit0;
final access-state sanitizers, naming/format and independent review pass.
See the generator guide for exact counts, vendor evidence and remaining limits.
