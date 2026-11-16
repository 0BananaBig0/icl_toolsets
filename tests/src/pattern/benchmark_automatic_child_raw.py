#!/usr/bin/env python3
"""Compare automatic parent preparation with explicit ordinary setup."""
import benchmark_native_ordinary_scans as native


def scenarios():
    fixture=native.ROOT/'tests/cases/ours/pattern_generation/gated_sib_raw/model.icl'
    sib=fixture.read_text()
    prefix='iPDLLevel 1\niProcsForModule Top\niReset\n'
    for width in [8,256,1048575]:
        model=sib.replace('[7:0]',f'[{width-1}:0]').replace("ResetValue 8'b0",f"ResetValue {width}'b0")
        raw=f'iScan child.local {width} -si 0x96 -so 0; iApply\n'
        yield f'leaf{width}',model,prefix+raw,prefix+'iWrite open 1; iApply\n'+raw,width+1,1,True
    for count in [64,1024]:
        raw=''.join(f'iScan r{i}.leaf.local 8 -si {(0x96+i)%256} -so 0;\n' for i in range(count))+'iApply\n'
        setup=''.join(f'iWrite r{i}.enable 1;\n' for i in range(count))+'iApply\n'
        yield f'parents{count}-aliases512',native.many_model(count),prefix+raw,prefix+setup+raw,9*count,count,True
    nested=(native.ROOT/'tests/cases/ours/pattern_generation/dynamic_child_raw/model.icl').read_text()
    leaf,rest=nested.split('Module DynamicRawBranch',1)
    branch,top=rest.split('Module Top',1)
    depth=96
    model=leaf
    for level in range(depth):
        target='DynamicRawLeaf' if level==depth-1 else f'AutoRawBranch{level+1}'
        model+=f'Module AutoRawBranch{level}'+branch.replace('Of DynamicRawLeaf',f'Of {target}')
    model+='Module Top'+top.replace('Of DynamicRawBranch','Of AutoRawBranch0')
    target='child.'+'leaf.'*depth+'local'
    setup='iWrite open 1; iApply\n'+''.join(
        'iWrite child.'+'leaf.'*level+'enable 1; iApply\n' for level in range(depth))
    raw=f'iScan {target} 8 -si 0x96 -so 0; iApply\n'
    yield 'nested97',model,prefix+raw,prefix+setup+raw,depth+9,depth+1,True
    implicit=explicit=prefix
    for turn in range(128):
        raw=f'iScan child.local 8 -si 0x96 -so {0 if turn==0 else 0x96}; iApply\n'
        close='iWrite open 0; iApply\n'
        implicit+=raw+close
        explicit+='iWrite open 1; iApply\n'+raw+close
    yield 'reopen128',sib,implicit,explicit,9,128,True


if __name__=='__main__':
    native.main(scenarios,equivalent_kind='explicit ordinary parent setup')
