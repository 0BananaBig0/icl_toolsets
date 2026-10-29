#!/usr/bin/env bash
# Remote-only vendor probe; the copied script's directory owns every artifact.
set -euo pipefail
probe_root=$(cd -- "$(dirname -- "$0")" && pwd)
case "$probe_root" in
  /data/workspaces/lianghuaxiao/test/tessent_ijtag/test_icl_toolsets/test_mixed_io) ;;
  *) echo 'Run the staged probe inside test_icl_toolsets/test_mixed_io.' >&2; exit 2 ;;
esac
for probe_case in primary_data scan_data; do
  case_dir="$probe_root/run_2026_10_29_with_scan_interface/test_$probe_case"
  mkdir -p -- "$case_dir"
  (
    cd -- "$case_dir"
    ICL_TOOLSETS_CASE="$probe_case" \
      /data/eda/mentor/tessent_2026/tessent_2026.2/bin/tessent \
      -shell -replace -dofile "$probe_root/probe_mixed_io.tcl" -logfile tessent.log \
      > console.log 2>&1
  )
done
