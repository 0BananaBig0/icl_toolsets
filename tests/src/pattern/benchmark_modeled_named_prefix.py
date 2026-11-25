#!/usr/bin/env python3
"""Measure named implicit-history replay against real native parent sources."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import platform
import subprocess
import tempfile
import time

PREFIX = 'modeled named raw history omitted '


def model(count,width):
    text = 'Module Chains {ScanInPort si[%d:0];\n'%(count-1)
    for i in range(count):
        text += 'ScanRegister r%d[%d:0] {ScanInSource si[%d]; CaptureSource r%d; ResetValue 0;}\n'%(i,width-1,i,i)
    text += 'ScanOutPort so[%d:0] {Source '%(count-1)
    text += ', '.join('r%d[0]'%i for i in reversed(range(count)))+';}\n'
    text += 'SelectPort sel; CaptureEnPort ce; ShiftEnPort se; UpdateEnPort ue; TCKPort tck; ResetPort reset;\n'
    text += 'ScanInterface client {Port sel; Port ce; Port se; Port ue; Port tck; Port reset;\n'
    text += ''.join('Chain c%d {Port si[%d]; Port so[%d];}\n'%(i,i,i) for i in range(count))
    return text+'}}\n'


def commands(count,width):
    text = 'iPDLLevel 1\niProcsForModule Chains\niReset\n'
    notes,queries,frames = [],[],[]
    state = ['0'*width for _ in range(count)]
    histories = {}
    for phase,selected in enumerate((0,count-1,0,count-1)):
        word = format(((selected+1)*37+phase*19)%(1<<width),'0%db'%width)
        expected = '1'*width if phase==0 else 'x'*width
        frame = []
        for i in sorted(range(count),key=lambda i:'c%d'%i):
            explicit = i==selected
            frame.append(('c%d'%i,'0b'+(word if explicit else state[i]),
                          '0b'+(expected if explicit else 'x'*width),explicit))
        frames.append(frame)
        text += 'iScan client -chain c%d %d -si 0b%s -so 0b%s -stable\niApply\n'%(selected,width,word,expected)
        histories[selected] = (state[selected],'1'*width if phase==0 else '0'*width)
        state[selected] = word
        group_notes,group_queries = [],[]
        for i in range(count):
            if i in histories:
                group_notes += ['0b'+value for value in histories[i]]
                group_queries += ['iNote [iGetReadData client -chain c%d -bin]'%i,
                                  'iNote [iGetMiscompares client -chain c%d -bin]'%i]
            else:
                group_notes.append('absent_c%d'%i)
                group_queries.append('if {![catch {iGetReadData client -chain c%d}]} {error {implicit history invented}}; iNote absent_c%d'%(i,i))
        group_notes += [str(width if phase==0 else 0),'0']
        group_queries += ['iNote [iGetStatus -clear]','iNote [iGetStatus]']
        notes += group_notes;queries += group_queries
        text += '\n'.join(group_queries)+'\n'
    return text,notes,queries,frames


def replay_observations(generated,count,width):
    _,expected,queries,_ = commands(count,width)
    result,index,clears,pending = [],0,0,None
    for line in generated.splitlines():
        marker = 'iNote -comment "modeled readback status clear '
        if line.startswith(marker):
            if pending is not None or not line.endswith('"'):raise ValueError('invalid status clear')
            pending = line[len(marker):-1];clears += 1
            continue
        if line.startswith('iNote ') and 'modeled ' not in line:
            if index>=len(expected) or line!='iNote '+expected[index]:raise ValueError('actual named history differs')
            if '-clear' in queries[index]:
                if pending!=expected[index]:raise ValueError('status clear count differs')
            elif pending is not None:raise ValueError('misplaced status clear')
            pending = None;result.append(queries[index]);index += 1
        else:result.append('iPDLLevel 1' if line=='iPDLLevel 0' else line)
    if index!=len(expected) or clears!=4 or pending is not None:raise ValueError('missing actual observations')
    return '\n'.join(result)+'\n'


def normalized_output(data,count,width,scope):
    _,notes,queries,frames = commands(count,width)
    lines = data.splitlines()
    actual = [line[6:] for line in lines if line.startswith('iNote ') and 'modeled ' not in line]
    chunks = data.split('\niApply\n')
    if actual!=notes or len(chunks)!=5:raise ValueError('actual histories or CSU count differ')
    groups=[];group=[]
    for note,query in zip(notes,queries):
        if '-clear' in query:group.append('iNote -comment "modeled readback status clear '+note+'"')
        group.append('iNote '+note)
        if query=='iNote [iGetStatus]':groups.append(group);group=[]
    for chunk,wanted in zip(chunks,[[]]+groups):
        observed=[line for line in chunk.splitlines() if line.startswith('iNote ') and
                  ('modeled ' not in line or 'modeled readback status clear ' in line)]
        if observed!=wanted:raise ValueError('actual observations moved between frames')
    for chunk,frame in zip(chunks,frames):
        wanted = []
        for chain,word,expected,explicit in frame:
            scan = 'iScan client -chain %s %d -si %s -so %s -stable'%(chain,width,word,expected)
            if scope and not explicit:wanted.append('iNote -comment "'+PREFIX+scan+'"')
            wanted.append(scan)
        emitted = [line for line in chunk.splitlines() if line.startswith('iScan ') or PREFIX in line]
        if emitted!=wanted:raise ValueError('physical words or exact omission scope differ')
    if any(line.startswith('iScan ') or PREFIX in line for line in chunks[-1].splitlines()):
        raise ValueError('trailing physical frame')
    for value,expected in ((str(width),1),('0',3)):
        if lines.count('iNote -comment "modeled readback status clear '+value+'"')!=expected:
            raise ValueError('status clear annotations differ')
    if any(line.split()[0] not in ('iPDLLevel','iProcsForModule','iReset','iScan','iApply','iNote') for line in lines if line.strip()):
        raise ValueError('unexpected emitted operation')
    return '\n'.join(line for line in lines if PREFIX not in line)+'\n'


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for name in ('binary','parent','sanitized','output'):parser.add_argument('--'+name,type=Path,required=True)
    args=parser.parse_args();output=args.output.resolve();output.mkdir(parents=True,exist_ok=True)
    root=Path(__file__).resolve().parents[3]
    digest=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
    binaries=dict(current=args.binary.resolve(),parent=args.parent.resolve(),sanitized=args.sanitized.resolve())
    report=dict(environment=platform.platform(),parent_control='same register-backed model; one explicit lane per real native CSU, other lanes physically filled',
                binaries={key:dict(path=str(path),sha256=digest(path)) for key,path in binaries.items()},runs=[])
    grid=[(n,width) for n in (1,2,8,32) for width in (8,64,256)]+[(n,1024) for n in (1,2,8)]
    cases=[(owner,n,width) for owner in ('parent','current') for n,width in grid]
    cases += [('sanitized',n,width) for n,width in ((2,8),(8,8),(2,256),(2,1024))]
    parents={};failed=False
    for owner,count,width in cases:
        for repeat in range(3):
            source=None
            for form in (('source',) if owner=='parent' else ('source','flat','generated')):
                folder=Path(tempfile.mkdtemp(prefix='%s-%d-%d-%s-'%(owner,count,width,form),dir=output))
                icl,pdl,result,metric=(folder/n for n in ('source.icl','source.pdl','result.pdl','time.txt'))
                icl.write_text(model(count,width));pdl.write_text(commands(count,width)[0]);error=''
                if form=='flat':icl=Path(source['directory'])/'debug/Chains.icl'
                if form=='generated':
                    try:pdl.write_text(replay_observations((Path(source['directory'])/'result.pdl').read_text(),count,width))
                    except (OSError,ValueError) as issue:error=str(issue);pdl.write_text('')
                cmd=[str(binaries[owner]),'--icl',str(icl),'--pdl',str(pdl),'--top_module','Chains','--output',str(result),'--debug','--model_readback']
                started=time.monotonic()
                with (folder/'stdout.log').open('w') as stdout,(folder/'stderr.log').open('w') as stderr:
                    p=subprocess.run(['/usr/bin/time','-f','%e %M','-o',str(metric),'timeout','--kill-after=2s','60s',*cmd],cwd=root,
                        env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1',UBSAN_OPTIONS='halt_on_error=1'),stdout=stdout,stderr=stderr)
                elapsed=time.monotonic()-started
                values=metric.read_text().splitlines()[-1].split() if metric.exists() else []
                rss=int(values[1]) if len(values)==2 and values[1].isdigit() else None
                data=result.read_text() if result.exists() else ''
                try:normalized=normalized_output(data,count,width,scope=owner!='parent')
                except ValueError as issue:normalized=None;error=error or str(issue)
                key=count,width,repeat
                verified=p.returncode==0 and not error and normalized is not None and (folder/'debug/Chains.icl').exists()
                if owner!='parent':verified &= key in parents and normalized==parents.get(key)
                if form!='source':verified &= (Path(source['directory'])/'result.pdl').exists() and data==(Path(source['directory'])/'result.pdl').read_text()
                if owner=='parent' and verified:parents[key]=normalized
                scans=[line.split() for line in data.splitlines() if line.startswith('iScan ')]
                bound=(256 if count==1 and width>=512 else 512)*1024
                within=elapsed<=10 and rss is not None and rss<=bound
                row=dict(owner=owner,chains=count,width=width,repeat=repeat,form=form,directory=str(folder),command=cmd,exit=p.returncode,
                    replay_error=error,seconds=elapsed,peak_rss_kib=rss,bound_kib=bound,within_bounds=within,actual_histories_verified=bool(verified),
                    generated_live_histories=form=='generated',implicit_lane_fills=count>1,csu_count=data.splitlines().count('iApply'),raw_scan_count=len(scans),
                    raw_shifted_bits=sum(len(words[words.index('-si')+1])-2 for words in scans if '-si' in words and words.index('-si')+1<len(words)),
                    fresh_queries=pdl.read_text().count('iGetReadData client'),fresh_miscompare_queries=pdl.read_text().count('iGetMiscompares client'),
                    omission_count=data.count(PREFIX),status_clear_annotation_count=data.count('modeled readback status clear '),
                    icl_sha256=digest(icl) if icl.exists() else None,pdl_sha256=digest(pdl),output_sha256=digest(result) if result.exists() else None,
                    normalized_output_sha256=hashlib.sha256(normalized.encode()).hexdigest() if normalized is not None else None)
                report['runs'].append(row);failed |= not (verified and within)
                (output/'results.json').write_text(json.dumps(report,indent=2)+'\n')
                if form=='source':source=row
    unchanged=all(digest(path)==report['binaries'][key]['sha256'] for key,path in binaries.items())
    complete=len(report['runs'])==216
    print(json.dumps(dict(runs=len(report['runs']),matrix_complete=complete,all_verified=not failed,binaries_unchanged=unchanged)))
    return int(failed or not complete or not unchanged)


if __name__=='__main__':raise SystemExit(main())
