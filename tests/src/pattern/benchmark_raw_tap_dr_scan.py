#!/usr/bin/env python3
"""Measure single-register raw TAP DR against prior ordinary scheduling."""
import re
import benchmark_raw_tap_ir_scan as ir

FIXTURE = ir.ROOT / 'tests/cases/ours/pattern_generation/raw_tap_dr'


def scenarios():
    for width, registers, repeats in [(8,1,1),(32,1,1),(256,1,1),
                                     (4096,1,1),(8,1024,1),(8,1,128)]:
        model=(FIXTURE/'model.icl').read_text().replace('[7:0]',f'[{width-1}:0]')
        model=model.replace("ResetValue 8'b0",f"ResetValue {width}'b0")
        extra=''.join(f'  ScanRegister unused{i} {{ScanInSource si;}}\n' for i in range(registers-1))
        model=model.replace('  ScanMux mux',extra+'  ScanMux mux')
        bsdl=(FIXTURE/'model.bsdl').read_text()
        prefix='iPDLLevel 1\niProcsForModule RawDrTop\niReset\niWrite tap.INSTRUCTION 3; iApply\n'
        raw=f'iScan jtag {width} -si 0x96 -so 0; iApply\n'
        ordinary='iWrite core.r 0x96; iRead core.r 0; iApply\n'
        yield f'w{width}-nodes{registers}-repeat{repeats}',model,bsdl,prefix+raw*repeats,prefix+ordinary*repeats


def canonical(data):
    return re.sub(rb'iScan jtag [0-9]+ -si (0b[01]+) -so (0b[01]+)\n',
                  rb'iWrite core.r \1\niRead core.r \2\n',data)


if __name__ == '__main__':
    ir.main(scenarios,canonical,'RawDrTop','RawDrProbe')
