#!/usr/bin/env python3
"""Compare full-path raw TAP DR with equivalent ordinary register assignments."""
import re
import benchmark_raw_tap_ir_scan as ir

FIXTURE=ir.ROOT/'tests/cases/ours/pattern_generation/raw_tap_multiple_dr'


def scenarios(shapes=None):
    for count,width,unused,repeats,aliases in shapes if shapes is not None else [(3,4,0,1,0),(16,4,0,1,0),
            (64,1,0,1,0),(16,16,0,1,0),(3,4,1024,1,0),(3,4,0,128,0),
            (64,1,0,1,512),(16,256,0,1,0)]:
        model=(FIXTURE/'model.icl').read_text()
        a=model.index('  ScanRegister a[');b=model.index('  ScanMux mux',a)
        declarations='Alias link0 = si;\n' if aliases else ''
        if aliases:
            declarations+=''.join(f'Alias link{i} = link{i-1};\n' for i in range(1,aliases))
        for i in range(count):
            source=('link'+str(aliases-1) if aliases else 'si') if i==0 else f'r{i-1}[0]'
            declarations+=f"ScanRegister r{i}[{width-1}:0] {{ScanInSource {source}; CaptureSource r{i}; ResetValue 0;}}\n"
        declarations+=''.join(f'ScanRegister unused{i} {{ScanInSource si;}}\n' for i in range(unused))
        model=(model[:a]+declarations+model[b:]).replace("1'b1: b[0]",f"1'b1: r{count-1}[0]")
        bits=count*width;packet=0xa53
        prefix='iPDLLevel 1\niProcsForModule MultipleDrTop\niReset\niWrite tap.INSTRUCTION 3; iRead tap.INSTRUCTION 1; iApply\n'
        source=ordinary=prefix
        for turn in range(repeats):
            expected=0 if turn==0 else packet
            source+=f'iScan jtag {bits} -si {packet} -so {expected}; iApply\n'
            for command,value in [('iWrite',packet),('iRead',expected)]:
                for i in range(count):
                    data=(value>>((count-i-1)*width))&((1<<width)-1)
                    ordinary+=f'{command} core.r{i} 0b'+format(data,f'0{width}b')+';\n'
            ordinary+='iApply\n'
        yield f'w{bits}-registers{count}-unused{unused}-repeat{repeats}-aliases{aliases}',model,(FIXTURE/'model.bsdl').read_text(),source,ordinary


def canonical(data):
    def packet(block):
        fields=re.findall(rb'core\.r([0-9]+) 0b([01]+)\n',block)
        return b''.join(bits for _,bits in sorted(fields,key=lambda item:int(item[0])))
    def combine(match):
        si,so=packet(match[1]),packet(match[2])
        assert len(si)==len(so)
        return b'iScan jtag '+str(len(si)).encode()+b' -si 0b'+si+b' -so 0b'+so+b'\n'
    return re.sub(rb'((?:iWrite core\.r[0-9]+ 0b[01]+\n)+)((?:iRead core\.r[0-9]+ 0b[01]+\n)+)',combine,data)


if __name__=='__main__':
    ir.main(scenarios,canonical,'MultipleDrTop','MultipleDrProbe')
