#!/usr/bin/env python3
"""Characterize builtin DR; the prior CLI rejects it, so no timing baseline exists."""
import benchmark_raw_tap_ir_scan as ir

FIXTURE = ir.ROOT / 'tests/cases/ours/pattern_generation/raw_tap_builtin_dr'


def scenarios():
    for mode, registers, repeats in [('idcode',1,1),('bypass',1,1),
                                     ('idcode',1024,1),('bypass',1024,1),
                                     ('idcode',1,128),('bypass',1,128)]:
        model=(FIXTURE/'model.icl').read_text()
        extra=''.join(f'  ScanRegister unused{i} {{ScanInSource si;}}\n' for i in range(registers-1))
        model=model.replace('  ScanMux mux',extra+'  ScanMux mux')
        bsdl=(FIXTURE/'model.bsdl').read_text()
        prefix='iPDLLevel 1\niProcsForModule BuiltinDrTop\niReset\n'
        if mode=='bypass':
            prefix+='iWrite tap.INSTRUCTION 15; iRead tap.INSTRUCTION 1; iApply\n'
            raw='iScan jtag 1 -si 1 -so 0 -stable; iApply\n'
        else:
            raw='iScan jtag 32 -si 0x89abcdef -so 0x10000003 -stable; iApply\n'
        yield f'{mode}-nodes{registers}-repeat{repeats}',model,bsdl,prefix+raw*repeats,None


if __name__ == '__main__':
    ir.main(scenarios,lambda data:data,'BuiltinDrTop','BuiltinDrProbe',candidate_only=True)
