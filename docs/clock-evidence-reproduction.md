# Repeat the retained clock evidence checks

Use this guide in the original retained workspace,
`/home/banana/cpp_workspace/icl-format`. It repeats the Task209 measurements
and archived-job checks while preserving their canonical receipts.

You need Python 3, `/usr/bin/time`, the qualified generator and trace tools in
`.worktrees/bsdl-defaults/build/`, and the retained Task194, Task196 and Task209
directories under `build/`. The historical parent binary and all original
fixtures must still be present. A fresh source checkout alone is insufficient.
The manifest checks stop the run if qualified inputs or products differ.
Run Python with assertions enabled. The preflight rejects optimized execution;
unset `PYTHONOPTIMIZE` and rerun normally if it reports that error.

The archived driver uses fixed output paths. The command below verifies its
digest and derives a separate copy with two changes: a new receipt directory,
and removal of the final metadata-correction tail. It seeds the two required
qualified manifests. Original correction receipts are unnecessary because this
run omits their writes. Original and derived driver hashes remain distinct.
The active script is created under `tests/`; its retained copy, log and receipts
go into a new `/tmp/icl-clock-replay-*` directory.
The active script is removed if writing, copying or waiting raises; available
scratch files remain for diagnosis.

```sh
cd /home/banana/cpp_workspace/icl-format
python3 - <<'PY'
from pathlib import Path
import hashlib
import json
import os
import shutil
import subprocess
import sys
import tempfile

if not __debug__:
    raise SystemExit('Unset PYTHONOPTIMIZE and rerun with assertions enabled')

repo = Path.cwd()
canonical = repo / 'build/clock-evidence-receipts-20261128'
sha = lambda path: hashlib.sha256(path.read_bytes()).hexdigest()
source_path = canonical / 'measure/driver.py'
source_bytes = source_path.read_bytes()
original_sha = hashlib.sha256(source_bytes).hexdigest()
measurement = json.loads((canonical / 'measure/receipt.json').read_text())
comparison = json.loads((canonical / 'vendor/receipt.json').read_text())
assert original_sha == measurement['driver_sha256'] == comparison['driver_sha256']
assert source_bytes == (canonical / 'vendor/driver.py').read_bytes()
source = source_bytes.decode('utf-8')
assert f'r=Path({str(repo)!r});' in source, 'use the original retained workspace'

protected = {str(p): sha(p) for p in canonical.rglob('*') if p.is_file()}
for name in ('receipt.json', 'receipt-erratum.json'):
    p = repo / 'build/clock-period-serialization-20261127/qualification' / name
    protected[str(p)] = sha(p)
scratch = Path(tempfile.mkdtemp(prefix='icl-clock-replay-'))
print(f'Scratch: {scratch}', flush=True)
receipts = scratch / 'receipts'
receipts.mkdir()
for name in ('parent-qualified-inputs.json', 'parent-qualified-products.json'):
    shutil.copy2(canonical / name, receipts / name)
redirect = "e=r/'build/clock-evidence-receipts-20261128'"
tail = "path=period/'qualification/receipt.json';original=e/'original-period-native-receipt.json'"
assert source.count(redirect) == source.count(tail) == 1
derived = source.replace(redirect, f'e=Path({str(receipts)!r})', 1)
derived = derived.split(tail, 1)[0]
derived += '\nprint("ISOLATED_CLOCK_EVIDENCE_REPLAY_VERIFIED", flush=True)\n'
fd, filename = tempfile.mkstemp(prefix='.clock-evidence-replay-', suffix='.py',
                                dir=repo / 'tests')
driver = Path(filename)
try:
    with os.fdopen(fd, 'w', encoding='utf-8') as stream:
        stream.write(derived)
    shutil.copy2(driver, scratch / 'driver.py')
    args = [sys.executable, str(driver)]
    with (scratch / 'run.log').open('w') as log:
        completed = subprocess.run(args, cwd=repo, stdout=log, stderr=subprocess.STDOUT)
finally:
    driver.unlink()
assert completed.returncode == 0, (scratch / 'run.log').read_text()[-4000:]
assert all(sha(Path(name)) == digest for name, digest in protected.items())
measured = json.loads((receipts / 'measure/receipt.json').read_text())
attested = json.loads((receipts / 'vendor/receipt.json').read_text())
assert len(measured['rows']) == 45 and measured['cli_processes'] == 225
assert len(attested['native_inputs']) == 9 and attested['cli_processes'] == 45
assert len(attested['pairs']) == 8 and len(attested['jobs']) == 16
assert attested['fresh_vendor_jobs'] == 0
record = dict(exit=completed.returncode, command=args, scratch=str(scratch),
              original_driver_sha256=original_sha,
              derived_driver_sha256=sha(scratch / 'driver.py'),
              native_cli_processes=270, trace_cli_processes=16,
              canonical_files=protected,
              log_sha256=sha(scratch / 'run.log'))
(scratch / 'replay-receipt.json').write_text(json.dumps(record, indent=2) + '\n')
print(json.dumps(dict(status='passed', scratch=str(scratch),
                      receipt=str(scratch / 'replay-receipt.json'))))
PY
```

Success prints `status: passed` as JSON and the scratch location. Its
`replay-receipt.json` binds both driver hashes and the unchanged canonical
files; `run.log` contains the individual measurement and pair checks. The new
measurement and comparison receipts retain another copy of the derived driver.
Keep the scratch directory while auditing the results.

On failure, inspect the terminal traceback as well as the printed scratch
directory and `run.log`. Preflight errors can occur before a scratch directory
is printed; wrapper checks can fail after the child exits. Restore missing
prerequisites or the matching qualified artifacts, then run the command again;
it creates a new scratch directory. Do not clear canonical receipt directories
or edit their driver copies to force a rerun.

This checks 270 native replay processes and 16 parser invocations against the
original literal inputs and expected outputs, with the existing 10s/512MiB
measurement bounds. It attests eight pairs from 16 archived Tessent 2026.2 jobs;
one additional attached-unit case has native evidence only. It performs no
fresh vendor job, standalone export replay, device access or timing signoff.
The missing Task196 original driver bytes remain unrecovered. The sidecar's
`correction_date` and `correction_timezone` identify the Task209 correction;
`date_annotation_task` identifies the later explicit date annotation.
