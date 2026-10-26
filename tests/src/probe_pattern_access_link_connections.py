#!/usr/bin/env python3
"""Stage isolated Tessent AccessLink connection probes; never run EDA locally.

Inputs are self-constructed variants of the maintained access_link fixture.
Invoke once with a fresh staging directory, copy it to the authorized EDA
account, and run each run.tcl in its own process/directory. Tool output stays
outside the maintained fixture corpus. This is a manual vendor probe, not a
passing native-generator regression or a pattern-equivalence test.
"""
import argparse
from pathlib import Path
import shutil

ROOT = Path(__file__).resolve().parents[2]
FIXTURE = ROOT / 'tests/cases/ours/pattern_generation/access_link'


def replace(text, old, new):
    if text.count(old) != 1:
        raise ValueError(f'fixture anchor is not unique: {old}')
    return text.replace(old, new)


def stage(destination):
    base = (FIXTURE / 'simple.icl').read_text()
    instance = 'Instance core Of AccessInstrument {'
    top = 'Module AccessTop {'
    ports = ('\n  ScanInPort TDI; TMSPort TMS; TCKPort TCK; '
             'TRSTPort TRST; ScanOutPort TDO;\n')
    chain = replace(base, top, top + '\n  Instance peer Of AccessInstrument '
                    "{ InputPort mode = 1'b1; }\n")
    chain = replace(chain, 'ScanInterface { core.client; }\n      ActiveSignals',
                    'ScanInterface { core.client; peer.client; }\n      ActiveSignals')
    cases = {
        'baseline': base,
        'explicit_scan_input': replace(base, instance, instance + " InputPort si = 1'b0;"),
        'explicit_controls': replace(base, instance, instance +
                                    " InputPort select = 1'b0; InputPort shift = 1'b0;"
                                    " InputPort capture = 1'b0; InputPort update = 1'b0;"),
        'existing_tap_ports': replace(base, top, top + ports),
        'existing_tdo_source': replace(base, top, top + ports.replace(
            'ScanOutPort TDO;', 'ScanOutPort TDO { Source TDI; }')),
        'wrong_tdi_kind': replace(base, top, top + '\n  DataInPort TDI;\n'),
        'two_interfaces': chain,
        'explicit_chain_feedback': replace(chain, instance,
                                            instance + ' InputPort si = peer.so;'),
    }
    destination.mkdir(parents=True, exist_ok=False)
    for name, text in cases.items():
        directory = destination / name
        directory.mkdir()
        (directory / 'simple.icl').write_text(text)
        shutil.copyfile(FIXTURE / 'simple.bsdl', directory / 'simple.bsdl')
        (directory / 'run.tcl').write_text('''# Tessent patterns -ijtag / setup.
# Command legality checked in the Tessent Shell Reference Manual v2023.4.
if {[catch {
  set_context patterns -ijtag
  read_icl simple.icl -bsdl_file simple.bsdl
  set_current_design AccessTop
  write_icl expanded.icl
  puts "ICL_TOOLSETS_CONNECTION_PROBE_COMPLETE"
} message]} {
  puts stderr "ICL_TOOLSETS_CONNECTION_PROBE_ERROR: $message"
  exit 1
}
exit 0
''')
    print(f'Staged {len(cases)} isolated probes in {destination}')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('destination', type=Path)
    stage(parser.parse_args().destination)
