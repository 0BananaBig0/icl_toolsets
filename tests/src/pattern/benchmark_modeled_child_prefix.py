#!/usr/bin/env python3
"""Static modeled child request identities and measured live-history replay."""
def model(children=1, width=8, inverted=False):
    leaf = """Module Leaf {
 ScanInPort si; SelectPort sel; CaptureEnPort ce; ShiftEnPort se;
 UpdateEnPort ue; TCKPort tck; ResetPort reset;
 ScanRegister value[%d:0] {ScanInSource si; CaptureSource value; ResetValue 0;}
 ScanOutPort so {Source %svalue[0];}
 ScanInterface local {Port si; Port so; Port sel; Port ce; Port se; Port ue; Port tck; Port reset;}
}
Module Chains {
 ScanInPort si; SelectPort sel; CaptureEnPort ce; ShiftEnPort se;
 UpdateEnPort ue; TCKPort tck; ResetPort reset;
 ScanRegister head[2:0] {ScanInSource si; CaptureSource head; ResetValue 0;}
""" % (width-1, '~' if inverted else '')
    for i in range(children):
        source = 'head[0]' if not i else 'child%d.so' % (i-1)
        leaf += ' Instance child%d Of Leaf {InputPort si = %s; ' % (i,source)
        leaf += ' '.join('InputPort %s = %s;' % (p,p) for p in ('sel','ce','se','ue','tck','reset'))+'}\n'
    return leaf+''' ScanRegister tail[4:0] {ScanInSource child%d.so; CaptureSource tail; ResetValue 0;}
 ScanOutPort so {Source tail[0];}
 ScanInterface client {Port si; Port so; Port sel; Port ce; Port se; Port ue; Port tck; Port reset; DefaultLoadValue %d'b0;}
}
''' % (children-1,children*width+8)


def payload(index, group, width):
    unit = '10100101' if (index+group)%2==0 else '00111100'
    return (unit*((width+7)//8))[:width]


def commands(children, width, parent=False):
    text = 'iPDLLevel 1\niProcsForModule Chains\niReset\niWrite head 5; iWrite tail 19;\n'
    old = [payload(i,0,width) for i in range(children)]
    text += ''.join('iWrite child%d.value 0b%s;\n' % (i,bits) for i,bits in enumerate(old))
    text += 'iApply\n'
    expected,words,compares = [],[],[]
    size = children*width+8
    for group in range(4):
        new = [payload(i,group+1,width) for i in range(children)]
        for i,bits in enumerate(new):
            text += 'iScan child%d.local %d -si 0b%s -so 0 -stable\n' % (i,width,bits)
        text += 'iApply\n'
        if parent:
            text += ''.join('iNote -comment "modeled child raw history client child%d.local 0b%s"\n' %
                            (i,'0'*width) for i in sorted(range(children),key=lambda i:'child%d.local'%i))
        for i,bits in enumerate(old):
            text += 'iNote [iGetReadData child%d.local -bin]\niNote [iGetMiscompares child%d.local -bin]\n' % (i,i)
            expected += ['0b'+bits]*2
        word = '0b101'+''.join(new)+'10011'
        text += 'iScan client %d -si %s -so %s -stable; iApply\n' % (size,word,word)
        text += 'iNote [iGetReadData client -bin]\niNote [iGetMiscompares client -bin]\n'
        text += 'iNote [iGetStatus -clear]\niNote [iGetStatus]\n'
        expected += [word,'0b'+'0'*size,str(sum(bits.count('1') for bits in old)),'0']
        words += [word]*2
        compares += ['0bxxx'+'0'*(children*width)+'xxxxx',word]
        old = new
    return text,expected,words,compares


def replay_observations(data, children, expected):
    lines,position = [],0
    for line in data.splitlines():
        if line.startswith('iNote ') and 'modeled ' not in line:
            if position>=len(expected) or line[6:]!=expected[position]:
                raise ValueError('generated observation differs from the independent fixture')
            field = position%(2*children+4)
            if field<2*children:
                query = ('iGetReadData' if field%2==0 else 'iGetMiscompares')+' child%d.local -bin' % (field//2)
            else:
                query = ('iGetReadData client -bin','iGetMiscompares client -bin',
                         'iGetStatus -clear','iGetStatus')[field-2*children]
            if query=='iGetStatus -clear' and lines and lines[-1].startswith(
                    'iNote -comment "modeled readback status clear '):
                if lines.pop()!='iNote -comment "modeled readback status clear '+expected[position]+'"':
                    raise ValueError('recorded clear differs from the expected count')
            line = 'iNote ['+query+']'
            position += 1
        lines.append(line)
    if position!=len(expected):
        raise ValueError('generated observation list is incomplete')
    return ('\n'.join(lines)+'\n').replace('iPDLLevel 0','iPDLLevel 1',1)


def main():
    import argparse
    import hashlib
    import json
    import os
    from pathlib import Path
    import platform
    import subprocess
    import tempfile
    import time

    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('binary','parent','sanitized','output'):
        parser.add_argument('--'+name,type=Path,required=True)
    args = parser.parse_args()
    output = args.output.resolve()
    output.mkdir(parents=True,exist_ok=True)
    root = Path(__file__).resolve().parents[3]
    binaries = dict(current=args.binary.resolve(),parent=args.parent.resolve(),sanitized=args.sanitized.resolve())
    digest = lambda p: hashlib.sha256(p.read_bytes()).hexdigest()
    report = dict(environment=platform.platform(),
                  binaries={k:dict(path=str(p),sha256=digest(p)) for k,p in binaries.items()},runs=[])
    grid = [(n,w) for n in (1,2,8,32) for w in (8,64,256)]+[(n,1024) for n in (1,2,8)]
    cases = [(owner,n,w) for owner in ('parent','current') for n,w in grid]
    cases += [('sanitized',n,w) for n,w in ((2,8),(8,8),(2,256),(2,1024))]
    expected_rows = sum(3*(1 if owner=='parent' else 3) for owner,_,_ in cases)
    failed,parents = False,{}
    for owner,count,width in cases:
        for repeat in range(3):
            source = None
            for form in (('source',) if owner=='parent' else ('source','flat','generated')):
                folder = Path(tempfile.mkdtemp(prefix='%s-%d-%d-%s-'%(owner,count,width,form),dir=output))
                icl,pdl,result,metric = (folder/n for n in ('source.icl','source.pdl','result.pdl','time.txt'))
                text,notes,words,compares = commands(count,width,parent=owner=='parent')
                icl.write_text(model(count,width));pdl.write_text(text)
                if form=='flat' and source:
                    icl = Path(source['directory'])/'debug/Chains.icl'
                replay_error = ''
                if form=='generated' and source:
                    previous = Path(source['directory'])/'result.pdl'
                    pdl = folder/'generated-observations.pdl'
                    if previous.exists():
                        try:
                            pdl.write_text(replay_observations(previous.read_text(),count,notes))
                        except ValueError as error:
                            replay_error = str(error);pdl.write_text(previous.read_text())
                    else:
                        replay_error = 'source output missing'
                        pdl.write_text('')
                command = [str(binaries[owner]),'--icl',str(icl),'--pdl',str(pdl),
                           '--top_module','Chains','--output',str(result),'--debug','--model_readback']
                started = time.monotonic()
                with (folder/'stdout.log').open('w') as stdout,(folder/'stderr.log').open('w') as stderr:
                    process = subprocess.run(['/usr/bin/time','-f','%e %M','-o',str(metric),
                        'timeout','--kill-after=2s','60s',*command],cwd=root,
                        env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1',UBSAN_OPTIONS='halt_on_error=1'),
                        stdout=stdout,stderr=stderr)
                seconds = time.monotonic()-started
                measured = metric.read_text().splitlines()[-1].split() if metric.exists() else []
                rss = int(measured[1]) if len(measured)==2 and measured[1].isdigit() else None
                data = result.read_text() if result.exists() else ''
                lines = data.splitlines()
                actual = [line[6:] for line in lines if line.startswith('iNote ') and 'modeled ' not in line]
                scans = [line.split() for line in lines if line.startswith('iScan ')]
                size = count*width+8
                shape = (len(scans)==8 and lines.count('iApply')==9 and
                         all(len(s)==8 and s[1:4]==['client',str(size),'-si'] and
                             s[5]=='-so' and s[-1]=='-stable' for s in scans) and
                         [s[4] for s in scans]==words and [s[6] for s in scans]==compares)
                markers = [line for line in lines if 'modeled child raw history ' in line]
                expected_markers = ['iNote -comment "modeled child raw history client child%d.local 0b%s"'%
                                    (i,'0'*width) for i in sorted(range(count),key=lambda i:'child%d.local'%i)]*4
                verified = (process.returncode==0 and not replay_error and actual==notes and shape and
                            markers==expected_markers and (folder/'debug/Chains.icl').exists())
                if form!='source' and source:
                    previous = Path(source['directory'])/'result.pdl'
                    verified &= previous.exists() and data==previous.read_text()
                if (count,width,repeat) in parents:
                    verified &= data==parents[count,width,repeat]
                unchanged = digest(binaries[owner])==report['binaries'][owner]['sha256']
                bound = (256 if count==1 and width>=512 else 512)*1024
                within = seconds<=10 and rss is not None and rss<=bound
                row = dict(owner=owner,children=count,width=width,repeat=repeat,form=form,
                    root_width=size,modeled_storage_bits=size,raw_scan_count=len(scans),
                    csu_count=lines.count('iApply'),raw_shifted_bits=len(scans)*size,
                    history_annotation_count=len(markers),
                    root_scope_annotation_count=data.count('modeled raw history omitted '),
                    status_clear_annotation_count=data.count('modeled readback status clear '),
                    generated_live_histories=form=='generated',
                    native_child_queries=pdl.read_text().count('iGetReadData child') if pdl.exists() else 0,
                    native_child_miscompare_queries=pdl.read_text().count('iGetMiscompares child') if pdl.exists() else 0,
                    replay_error=replay_error,command=command,directory=str(folder),exit=process.returncode,
                    seconds=seconds,peak_rss_kib=rss,bound_kib=bound,
                    actual_histories_verified=bool(verified and unchanged),within_bounds=within,
                    icl_sha256=digest(icl) if icl.exists() else None,
                    pdl_sha256=digest(pdl) if pdl.exists() else None,
                    output_sha256=digest(result) if result.exists() else None)
                report['runs'].append(row)
                failed |= not (verified and unchanged and within)
                (output/'results.json').write_text(json.dumps(report,indent=2)+'\n')
                if form=='source':
                    source = row
                    if owner=='parent' and verified:
                        parents[count,width,repeat] = data
    complete = len(report['runs'])==expected_rows
    print(json.dumps(dict(runs=len(report['runs']),expected_runs=expected_rows,
                         matrix_complete=complete,all_verified=not failed)))
    return int(failed or not complete)


if __name__=='__main__':
    raise SystemExit(main())
