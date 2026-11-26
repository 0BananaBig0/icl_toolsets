#!/usr/bin/env python3
"""Measure automatic primary observations against parent hardware controls."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import platform
import subprocess
import tempfile
import time

HEADER=b'iNote {modeled register/PIO readback; no live device}\n'
ASAN_OPTIONS='detect_leaks=1:quarantine_size_mb=128'

def model(count,width):
    text='Module Chains {\n'
    for i in range(count):
        text+='''DataInPort en{0}; DataInPort p{0}[{1}:0];
DataRegister value{0}[{1}:0] {{WriteCallBack Chains store{0} <D> <R>; ResetValue 0;}}
DataRegister dest{0}[{1}:0] {{WriteDataSource value{0}; WriteEnSource en{0}; ResetValue 0;}}
DataOutPort endpoint{0}[{1}:0] {{Source p{0};}}
DataOutPort q{0}[{1}:0] {{Source dest{0};}}
'''.format(i,width-1)
    return text+'}\n'

def commands(count,width,observe=True):
    text='iPDLLevel 1\niProcsForModule Chains\n'
    for i in range(count):
        text+='iProc store%d {data reg} {iNote cb%d; iWrite en%d 0; iWrite p%d $data}\n'%(i,i,i,i)
    text+='iReset\n';notes=[];queries=[];failures=0
    for i in range(count):
        word=format(((i+1)*37)%(1<<width),'0%db'%width)
        text+='iWrite dest%d 0b%s\n'%(i,word)
        failures+=word.count('1')
    text+='iApply\n'
    for i in range(count):text+='iRead endpoint%d 0; iRead q%d 0bx\n'%(i,i)
    text+='iApply\n'
    for i in range(count):
        word=format(((i+1)*37)%(1<<width),'0%db'%width)
        notes+=['0b'+word,'0b'+word,'0b'+'x'*width,'0b'+'0'*width]
        queries+=['iNote [iGetReadData endpoint%d -bin]'%i,
                  'iNote [iGetMiscompares endpoint%d -bin]'%i,
                  'iNote [iGetReadData q%d -bin]'%i,
                  'iNote [iGetMiscompares q%d -bin]'%i]
    notes+=[str(failures),str(failures),'0']
    queries+=['iNote [iGetStatus]','iNote [iGetStatus -clear]','iNote [iGetStatus]']
    return text+('\n'.join(queries)+'\n' if observe else ''),notes,queries

def hardware_output(data,count,width):
    _,notes,queries=commands(count,width)
    expected=[]
    for note,query in zip(notes,queries):
        if '-clear' in query:expected.append(('iNote -comment "modeled readback status clear '+note+'"\n').encode())
        expected.append(('iNote '+note+'\n').encode())
    suffix=b''.join(expected)
    if not data.endswith(suffix):raise ValueError('actual observations, masks, clear counts or timing differ')
    hardware=data[:-len(suffix)]
    if not hardware.endswith(b'iApply\n'):raise ValueError('observations are not after the actual final frame')
    if hardware.count(HEADER)!=1:raise ValueError('missing or repeated modeled header')
    if any(line.startswith(b'iNote ') and not line.startswith(b'iNote cb') and line!=HEADER
           for line in hardware.splitlines(keepends=True)):
        raise ValueError('unexpected or displaced observation')
    return hardware.replace(HEADER,b'',1)

def replay_observations(data,count,width):
    hardware_output(data,count,width)
    _,notes,queries=commands(count,width)
    suffix=''
    for note,query in zip(notes,queries):
        if '-clear' in query:suffix+='iNote -comment "modeled readback status clear '+note+'"\n'
        suffix+='iNote '+note+'\n'
    return data[:-len(suffix.encode())].decode().replace('iPDLLevel 0\n','iPDLLevel 1\n',1)+'\n'.join(queries)+'\n'

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for name in ('binary','parent','sanitized','output'):parser.add_argument('--'+name,type=Path,required=True)
    args=parser.parse_args();output=args.output.resolve();output.mkdir(parents=True,exist_ok=True)
    root=Path(__file__).resolve().parents[3]
    digest=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
    binaries=dict(current=args.binary.resolve(),parent=args.parent.resolve(),sanitized=args.sanitized.resolve())
    report=dict(environment=platform.platform(),sanitizer_options=ASAN_OPTIONS,parent_control='default automatic callback hardware, without unsupported parent modeled observations',
                binaries={key:dict(path=str(path),sha256=digest(path)) for key,path in binaries.items()},runs=[])
    grid=[(n,width) for n in (1,2,8) for width in (8,32)]+[(n,64) for n in (1,2)]+[(1,256)]
    cases=[(owner,n,width) for owner in ('parent','current') for n,width in grid]
    cases += [('sanitized',n,width) for n,width in ((2,8),(8,32),(1,256))]
    parents={};failed=False
    for owner,count,width in cases:
        for repeat in range(3):
            source=None
            for form in (('source',) if owner=='parent' else ('source','flat','generated')):
                folder=Path(tempfile.mkdtemp(prefix='%s-%d-%d-%s-'%(owner,count,width,form),dir=output))
                icl,pdl,result,metric=(folder/n for n in ('source.icl','source.pdl','result.pdl','time.txt'))
                icl.write_text(model(count,width));pdl.write_text(commands(count,width,owner!='parent')[0]);error=''
                if form=='flat':icl=Path(source['directory'])/'debug/Chains.icl'
                if form=='generated':
                    try:pdl.write_text(replay_observations((Path(source['directory'])/'result.pdl').read_bytes(),count,width))
                    except (OSError,ValueError) as issue:error=str(issue);pdl.write_text('')
                cmd=[str(binaries[owner]),'--icl',str(icl),'--pdl',str(pdl),'--top_module','Chains','--output',str(result),'--debug']
                if owner!='parent':cmd+=['--model_readback']
                started=time.monotonic()
                with (folder/'stdout.log').open('w') as stdout,(folder/'stderr.log').open('w') as stderr:
                    p=subprocess.run(['/usr/bin/time','-f','%e %M','-o',str(metric),'timeout','--kill-after=2s','60s',*cmd],cwd=root,
                        env=dict(os.environ,ASAN_OPTIONS=ASAN_OPTIONS,UBSAN_OPTIONS='halt_on_error=1'),stdout=stdout,stderr=stderr)
                elapsed=time.monotonic()-started
                values=metric.read_text().splitlines()[-1].split() if metric.exists() else []
                rss=int(values[1]) if len(values)==2 and values[1].isdigit() else None
                raw=result.read_bytes() if result.exists() else b''
                try:hardware=raw if owner=='parent' else hardware_output(raw,count,width)
                except ValueError as issue:hardware=None;error=error or str(issue)
                key=count,width,repeat
                verified=p.returncode==0 and not error and hardware is not None and bool(raw) and (folder/'debug/Chains.icl').exists()
                verified &= all(hardware.count(('iNote cb%d\n'%i).encode())==1 for i in range(count)) if hardware is not None else False
                if owner!='parent':verified &= key in parents and hardware==parents.get(key)
                if form!='source':verified &= raw==(Path(source['directory'])/'result.pdl').read_bytes() if (Path(source['directory'])/'result.pdl').exists() else False
                if owner=='parent' and verified:parents[key]=hardware
                bound=(256 if count==1 and width>=512 else 512)*1024
                within=elapsed<=10 and rss is not None and rss<=bound
                row=dict(owner=owner,lanes=count,width=width,repeat=repeat,form=form,directory=str(folder),command=cmd,exit=p.returncode,
                    replay_error=error,seconds=elapsed,peak_rss_kib=rss,bound_kib=bound,within_bounds=within,actual_histories_verified=bool(verified),
                    generated_live_histories=form=='generated',callback_count=count,primary_input_bits=count*(width+1),
                    fresh_queries=pdl.read_text().count('iGetReadData '),fresh_miscompare_queries=pdl.read_text().count('iGetMiscompares '),
                    icl_sha256=digest(icl) if icl.exists() else None,pdl_sha256=digest(pdl),output_sha256=digest(result) if result.exists() else None,
                    hardware_output_sha256=hashlib.sha256(hardware).hexdigest() if hardware is not None else None)
                report['runs'].append(row);failed |= not (verified and within)
                (output/'results.json').write_text(json.dumps(report,indent=2)+'\n')
                if form=='source':source=row
    unchanged=all(digest(path)==report['binaries'][key]['sha256'] for key,path in binaries.items())
    complete=len(report['runs'])==135
    print(json.dumps(dict(runs=len(report['runs']),matrix_complete=complete,all_verified=not failed,binaries_unchanged=unchanged)))
    return int(failed or not complete or not unchanged)

if __name__=='__main__':raise SystemExit(main())
