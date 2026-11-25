#!/usr/bin/env python3
"""Measure opaque root merges against real all-explicit sequential controls."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import platform
import subprocess
import tempfile
import time


def model(kind, count, width):
    vector = '[%d:0]'%(count-1) if kind=='named' else ''
    source = 'Module Chains {ScanInPort si%s; ScanOutPort so%s; '%(vector,vector)
    source += 'SelectPort sel; CaptureEnPort ce; ShiftEnPort se; UpdateEnPort ue; TCKPort tck; ResetPort reset;\n'
    source += ' ScanInterface client {Port sel; Port ce; Port se; Port ue; Port tck; Port reset;\n'
    if kind=='scalar':
        source += " Port si; Port so; DefaultLoadValue %d'b0;\n"%width
    else:
        source += ''.join(" Chain c%d {Port si[%d]; Port so[%d]; DefaultLoadValue %d'b0;}\n"%
                          (i,i,i,width) for i in range(count))
    return source+' }}\n'


def observations(kind, count, width):
    notes,queries = [],[]
    for group in range(4):
        for i in range(count):
            chain = ' -chain c%d'%i if kind=='named' else ''
            notes += ['0b'+'x'*width,'0b'+'0'*width]
            queries += ['iNote [iGetReadData client%s -bin]'%chain,
                        'iNote [iGetMiscompares client%s -bin]'%chain]
        notes += ['0','0']
        queries += ['iNote [iGetStatus -clear]','iNote [iGetStatus]']
    return notes,queries


def commands(kind, count, width, parent=False):
    text = 'iPDLLevel 1\niProcsForModule Chains\niProc load {'
    text += ' '.join('v%d'%i for i in range(count))+'} {\n'
    for i in range(count):
        chain = ' -chain c%d'%i if kind=='named' else ''
        text += 'iScan client%s %d -si $v%d -so 0bx -stable\n'%(chain,width,i)
    text += 'iApply\n}\niReset\n'
    notes,queries = observations(kind,count,width)
    frames = []
    order = sorted(range(count),key=lambda i:'c%d'%i) if kind=='named' else [0]
    for group in range(4):
        if not parent:text += 'iMerge -begin\n'
        for phase in (2*group,2*group+1):
            words = ['0b'+format(((i+1)*37+phase*19)%(1<<width),'0%db'%width)
                     for i in range(count)]
            text += 'iCall load '+' '.join(words)+'\n'
            frames.append([('c%d'%i if kind=='named' else '',words[i],'0b'+'x'*width)
                           for i in order])
        if not parent:text += 'iMerge -end\n'
        text += '\n'.join(queries[group*(2*count+2):(group+1)*(2*count+2)])+'\n'
    return text,notes,frames


def replay_observations(generated, kind, count, width):
    expected,queries = observations(kind,count,width)
    result,index,clears,pending = [],0,0,False
    for line in generated.splitlines():
        if line=='iNote -comment "modeled readback status clear 0"':
            if pending:raise ValueError('duplicate status clear')
            pending = True;clears += 1
            continue
        if line.startswith('iNote ') and 'modeled ' not in line:
            if index>=len(expected) or line!='iNote '+expected[index]:
                raise ValueError('actual opaque observation differs')
            if pending != ('-clear' in queries[index]):
                raise ValueError('status clear does not precede its actual count')
            pending = False;result.append(queries[index]);index += 1
        else:
            result.append('iPDLLevel 1' if line=='iPDLLevel 0' else line)
    if index!=len(expected) or clears!=4 or pending:
        raise ValueError('missing actual observations or status clears')
    return '\n'.join(result)+'\n'


def checked_output(data, kind, count, width, notes, frames):
    lines = data.splitlines()
    actual = [line[6:] for line in lines if line.startswith('iNote ') and 'modeled ' not in line]
    chunks = data.split('\niApply\n')
    if actual!=notes or len(chunks)!=9:return False
    for chunk,frame in zip(chunks,frames):
        wanted = ['iScan client%s %d -si %s -so %s -stable'%
                  (' -chain '+chain if chain else '',width,word,expected)
                  for chain,word,expected in frame]
        if [line for line in chunk.splitlines() if line.startswith('iScan ')]!=wanted:
            return False
    return (not any(line.startswith('iScan ') for line in chunks[-1].splitlines()) and
            data.count('iNote -comment "modeled readback status clear 0"')==4 and
            'modeled raw history omitted ' not in data and '__opaque_scan_state' not in data and
            all(line.split()[0] in ('iPDLLevel','iProcsForModule','iReset','iScan','iApply','iNote')
                for line in lines if line.strip()))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('binary','parent','sanitized','output'):
        parser.add_argument('--'+name,type=Path,required=True)
    args = parser.parse_args();output = args.output.resolve();output.mkdir(parents=True,exist_ok=True)
    root = Path(__file__).resolve().parents[3]
    digest = lambda path:hashlib.sha256(path.read_bytes()).hexdigest()
    binaries = dict(current=args.binary.resolve(),parent=args.parent.resolve(),sanitized=args.sanitized.resolve())
    report = dict(environment=platform.platform(),parent_control='same opaque model; all lanes explicitly requested by sequential parent calls',
                  binaries={key:dict(path=str(path),sha256=digest(path)) for key,path in binaries.items()},runs=[])
    grid = [('scalar',1,width) for width in (8,64,256,1024)]
    grid += [('named',n,width) for n in (1,2,8,32) for width in (8,64,256)]
    grid += [('named',n,1024) for n in (1,2,8)]
    cases = [(owner,*point) for owner in ('parent','current') for point in grid]
    cases += [('sanitized',*point) for point in
              (('scalar',1,8),('scalar',1,1024),('named',2,8),('named',8,8),('named',2,256),('named',2,1024))]
    parents = {};failed = False
    for owner,kind,count,width in cases:
        for repeat in range(3):
            source = None
            for form in (('source',) if owner=='parent' else ('source','flat','generated')):
                folder = Path(tempfile.mkdtemp(prefix='%s-%s-%d-%d-%s-'%(owner,kind,count,width,form),dir=output))
                icl,pdl,result,metric = (folder/name for name in ('source.icl','source.pdl','result.pdl','time.txt'))
                text,notes,frames = commands(kind,count,width,parent=owner=='parent')
                icl.write_text(model(kind,count,width));pdl.write_text(text);error = ''
                if form=='flat':icl = Path(source['directory'])/'debug/Chains.icl'
                if form=='generated':
                    previous = Path(source['directory'])/'result.pdl'
                    try:pdl.write_text(replay_observations(previous.read_text(),kind,count,width))
                    except (OSError,ValueError) as issue:error = str(issue);pdl.write_text('')
                command = [str(binaries[owner]),'--icl',str(icl),'--pdl',str(pdl),'--top_module','Chains',
                           '--output',str(result),'--debug','--model_readback']
                started = time.monotonic()
                with (folder/'stdout.log').open('w') as stdout,(folder/'stderr.log').open('w') as stderr:
                    process = subprocess.run(['/usr/bin/time','-f','%e %M','-o',str(metric),
                        'timeout','--kill-after=2s','60s',*command],cwd=root,
                        env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1',UBSAN_OPTIONS='halt_on_error=1'),
                        stdout=stdout,stderr=stderr)
                elapsed = time.monotonic()-started
                values = metric.read_text().splitlines()[-1].split() if metric.exists() else []
                rss = int(values[1]) if len(values)==2 and values[1].isdigit() else None
                data = result.read_text() if result.exists() else ''
                scans = [line.split() for line in data.splitlines() if line.startswith('iScan ')]
                key = kind,count,width,repeat
                verified = (process.returncode==0 and not error and checked_output(data,kind,count,width,notes,frames)
                            and (folder/'debug/Chains.icl').exists())
                if owner!='parent':verified &= key in parents and data==parents.get(key)
                if form!='source':
                    previous = Path(source['directory'])/'result.pdl'
                    verified &= previous.exists() and data==previous.read_text()
                if owner=='parent' and verified:parents[key] = data
                bound = (256 if count==1 and width>=512 else 512)*1024
                within = elapsed<=10 and rss is not None and rss<=bound
                row = dict(owner=owner,kind=kind,chains=count,width=width,repeat=repeat,form=form,
                    directory=str(folder),command=command,exit=process.returncode,replay_error=error,
                    seconds=elapsed,peak_rss_kib=rss,bound_kib=bound,within_bounds=within,
                    actual_histories_verified=bool(verified),all_lanes_explicit=True,
                    generated_live_histories=form=='generated',csu_count=data.splitlines().count('iApply'),
                    raw_scan_count=len(scans),
                    raw_shifted_bits=sum(len(words[words.index('-si')+1])-2 for words in scans
                                         if '-si' in words and words.index('-si')+1<len(words)),
                    fresh_queries=pdl.read_text().count('iGetReadData client'),
                    fresh_miscompare_queries=pdl.read_text().count('iGetMiscompares client'),
                    status_clear_annotation_count=data.count('modeled readback status clear '),
                    icl_sha256=digest(icl) if icl.exists() else None,pdl_sha256=digest(pdl),
                    output_sha256=digest(result) if result.exists() else None)
                report['runs'].append(row);failed |= not (verified and within)
                (output/'results.json').write_text(json.dumps(report,indent=2)+'\n')
                if form=='source':source = row
    unchanged = all(digest(path)==report['binaries'][key]['sha256'] for key,path in binaries.items())
    complete = len(report['runs'])==282
    print(json.dumps(dict(runs=len(report['runs']),matrix_complete=complete,all_verified=not failed,binaries_unchanged=unchanged)))
    return int(failed or not complete or not unchanged)


if __name__=='__main__':
    raise SystemExit(main())
