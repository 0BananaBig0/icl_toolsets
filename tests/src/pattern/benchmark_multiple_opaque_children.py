#!/usr/bin/env python3
"""Multiple private opaque intervals: fixtures and measured replay."""
def model(widths=(8, 8), defaults=None):
    defaults = [0] * len(widths) if defaults is None else defaults
    if not widths or len(defaults) != len(widths):
        raise ValueError('one default per child is required')
    source = """Module ChildLeaf {
 Parameter W = 8; Parameter D = $W'b0;
 ScanInPort si; ScanOutPort so; SelectPort sel; CaptureEnPort ce;
 ShiftEnPort se; UpdateEnPort ue; TCKPort tck; ResetPort reset;
 ScanInterface local { Port si; Port so; Port sel; Port ce; Port se; Port ue;
 Port tck; Port reset; DefaultLoadValue $D; }
}
Module Top {
 ScanInPort si; SelectPort sel; CaptureEnPort ce; ShiftEnPort se;
 UpdateEnPort ue; TCKPort tck; ResetPort reset;
 ScanRegister head[2:0] { ScanInSource si; CaptureSource head; ResetValue 3'b0; }
"""
    previous = 'head[0]'
    for index, (width, value) in enumerate(zip(widths, defaults)):
        source += """ Instance child%d Of ChildLeaf {
 Parameter W = %d; Parameter D = %d'b%s;
 InputPort si = %s; InputPort sel = sel; InputPort ce = ce;
 InputPort se = se; InputPort ue = ue; InputPort tck = tck; InputPort reset = reset;
 }
""" % (index, width, width, format(value, '0%db' % width), previous)
        previous = 'child%d.so' % index
    source += """ ScanRegister tail[4:0] { ScanInSource %s; CaptureSource tail; ResetValue 5'b0; }
 ScanOutPort so { Source tail[0]; }
 ScanInterface client { Port si; Port so; Port sel; Port ce; Port se; Port ue;
 Port tck; Port reset; DefaultLoadValue %d'b0; }
}
""" % (previous, sum(widths) + 8)
    return source


def commands(widths, parent=False):
    body = 'iPDLLevel 1\niProcsForModule Top\niReset\niWrite head 5; iWrite tail 19; iApply\n'
    expected = []
    raw_words, compares = [], []
    for group in range(4):
        payloads = []
        for index, width in enumerate(widths):
            unit = '10100101' if (group + index) % 2 == 0 else '00111100'
            bits = (unit * ((width + 7)//8))[:width]
            payloads.append(bits)
            body += 'iScan child%d.local %d -si 0b%s -stable\n' % (index, width, bits)
        body += 'iApply\n'
        for index, width in enumerate(widths):
            body += 'iNote [iGetReadData child%d.local -bin]\n' % index
            expected.append('0b'+'x'*width)
        size = sum(widths)+8
        raw_words += ['0b101'+''.join(payloads)+'10011']*2
        compares += ['0b'+'x'*size, '0b101'+'x'*sum(widths)+'10011']
        body += 'iScan client %d -si 0b101%s10011 -so 0b101%s10011 -stable; iApply\n' % (
            size, ''.join(payloads), 'x'*sum(widths))
        body += 'iNote [iGetReadData client -bin]\niNote [iGetMiscompares client -bin]\n'
        body += 'iNote [iGetStatus -clear]\niNote [iGetStatus]\n'
        expected += ['0b101'+'x'*sum(widths)+'10011', '0b'+'0'*size, '0', '0']
    return body, expected, raw_words, compares

def replay_observations(data, children, expected):
    result, position = [], 0
    for line in data.splitlines():
        if line.startswith('iNote ') and 'modeled ' not in line:
            if position >= len(expected) or line[6:] != expected[position]:
                raise ValueError('generated observation does not match the hand-derived fixture')
            field = position % (children+4)
            if field < children:
                query = 'iGetReadData child%d.local -bin' % field
            else:
                query = ('iGetReadData client -bin', 'iGetMiscompares client -bin',
                         'iGetStatus -clear', 'iGetStatus')[field-children]
            if query == 'iGetStatus -clear' and result and result[-1].startswith(
                    'iNote -comment "modeled readback status clear '):
                marker = 'iNote -comment "modeled readback status clear '+expected[position]+'"'
                if result.pop() != marker:
                    raise ValueError('recorded clear count differs from the actual observation fixture')
            line = 'iNote ['+query+']'
            position += 1
        result.append(line)
    if position != len(expected):
        raise ValueError('generated observation list is incomplete')
    return ('\n'.join(result)+'\n').replace('iPDLLevel 0', 'iPDLLevel 1', 1)


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
    for name in ('binary', 'parent', 'sanitized', 'output'):
        parser.add_argument('--'+name, type=Path, required=True)
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[3]
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=True)
    binaries = dict(current=args.binary.resolve(), parent=args.parent.resolve(),
                    sanitized=args.sanitized.resolve())
    digest = lambda p: hashlib.sha256(p.read_bytes()).hexdigest()
    report = dict(environment=platform.platform(),
                  binaries={key:dict(path=str(path), sha256=digest(path))
                            for key,path in binaries.items()}, runs=[])
    grid = [(n,width) for n in (1,2,8,32) for width in (8,64,256)]
    grid += [(n,1024) for n in (1,2,8)]
    cases = [(owner,n,width) for owner in ('parent','current') for n,width in grid]
    cases += [('sanitized',n,width) for n,width in ((2,8),(8,8),(2,256),(2,1024))]
    parents = {}
    failed = False
    expected_rows = sum(3*(1 if owner=='parent' else 3) for owner,_,_ in cases)
    for owner, count, width in cases:
        for repeat in range(3):
            source = None
            for form in (('source',) if owner=='parent' else ('source','flat','generated')):
                folder = Path(tempfile.mkdtemp(prefix='%s-%d-%d-%s-' % (
                    owner,count,width,form), dir=output))
                text, notes, raw_words, compares = commands([width]*count, parent=owner=='parent')
                icl, pdl, result, metric = [folder/name for name in
                    ('source.icl','source.pdl','result.pdl','time.txt')]
                icl.write_text(model([width]*count))
                pdl.write_text(text)
                if form=='flat' and source:
                    icl = Path(source['directory'])/'debug/Top.icl'
                replay_error = ''
                if form=='generated' and source:
                    previous = Path(source['directory'])/'result.pdl'
                    if previous.exists():
                        pdl = folder/'generated-observations.pdl'
                        try:
                            rewritten = replay_observations(previous.read_text(), count, notes)
                        except ValueError as error:
                            replay_error = str(error)
                            rewritten = previous.read_text()
                        pdl.write_text(rewritten)
                    else:
                        pdl = previous
                command = [str(binaries[owner]), '--icl',str(icl),'--pdl',str(pdl),
                           '--top_module','Top','--output',str(result),'--debug','--model_readback']
                env = dict(os.environ, ASAN_OPTIONS='detect_leaks=1',
                           UBSAN_OPTIONS='halt_on_error=1')
                started = time.monotonic()
                with (folder/'stdout.log').open('w') as stdout, (folder/'stderr.log').open('w') as stderr:
                    process = subprocess.run(['/usr/bin/time','-f','%e %M','-o',str(metric),
                        'timeout','--kill-after=2s','60s',*command], cwd=root, env=env,
                        stdout=stdout, stderr=stderr)
                seconds = time.monotonic()-started
                measured = metric.read_text().splitlines()[-1].split() if metric.exists() else []
                rss = int(measured[1]) if len(measured)==2 and measured[1].isdigit() else None
                data = result.read_text() if result.exists() else ''
                actual = [line[6:] for line in data.splitlines()
                          if line.startswith('iNote ') and 'modeled ' not in line]
                root_width = count*width+8
                scans = [line.split() for line in data.splitlines() if line.startswith('iScan ')]
                shape = (len(scans)==8 and data.splitlines().count('iApply')==9 and
                         all(len(fields)==8 and fields[1:3]==['client',str(root_width)] and
                             fields[3]=='-si' and fields[5]=='-so' and
                             fields[-1]=='-stable' and len(fields[4])==root_width+2
                             for fields in scans) and
                         [fields[4] for fields in scans]==raw_words and
                         [fields[6] for fields in scans]==compares)
                history_notes = [line for line in data.splitlines()
                                 if 'modeled opaque child raw history ' in line]
                expected_history = ['iNote -comment "modeled opaque child raw history client child%d.local 0b%s"' %
                                    (n,'x'*width) for n in sorted(range(count), key=lambda n:'child%d.local'%n)]*4
                verified = (process.returncode==0 and not replay_error and actual==notes and shape and
                            history_notes==expected_history and
                            '__opaque_scan_state' not in data and
                            (folder/'debug/Top.icl').exists() and
                            '__opaque_scan_state' not in (folder/'debug/Top.icl').read_text())
                reference = parents.get((count,width,repeat))
                if form!='source' and source:
                    reference_path = Path(source['directory'])/'result.pdl'
                    verified &= reference_path.exists() and data==reference_path.read_text()
                if reference:
                    verified &= data==reference
                unchanged = digest(binaries[owner])==report['binaries'][owner]['sha256']
                bound_kib = (256 if count==1 and width>=512 else 512)*1024
                within = seconds<=10 and rss is not None and rss<=bound_kib
                row = dict(owner=owner,children=count,width=width,repeat=repeat,form=form,
                           root_width=root_width,opaque_bits=count*width,modeled_storage_bits=8,
                           csu_count=9,raw_scan_count=len(scans),raw_shifted_bits=len(scans)*root_width,
                           native_child_queries=sum(text.count('iGetReadData child%d.local -bin'%n)
                                                    for n in range(count)) if form!='generated' else
                                                    (pdl.read_text().count('iGetReadData child') if pdl.exists() else 0),
                           history_annotation_count=len(history_notes),
                           root_scope_annotation_count=data.count('modeled raw history omitted '),
                           status_clear_annotation_count=data.count('modeled readback status clear '),
                           generated_live_histories=form=='generated',replay_error=replay_error,
                           command=command,directory=str(folder),exit=process.returncode,
                           seconds=seconds,peak_rss_kib=rss,bound_kib=bound_kib,
                           actual_histories_verified=bool(verified and unchanged),
                           within_bounds=within,icl_sha256=digest(icl) if icl.exists() else None,
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


if __name__ == '__main__':
    raise SystemExit(main())
