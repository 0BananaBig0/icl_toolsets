#!/usr/bin/env python3
"""Count ordinary replay input assignments in a separate compiled producer."""
import argparse
import hashlib
import json
from pathlib import Path
import platform
import re
import shlex
import subprocess
import time


def fixture(count, width):
    tail = 9 if width == 4 else (9 << (width // 2)) | 9
    source = ('Module Chains {ScanInPort si; '
              f'ScanRegister a[{width-1}:0] {{ScanInSource si; '
              'CaptureSource a; ResetValue 0;} ScanOutPort oa {Source a[0];}')
    body = 'iProcsForModule Chains\niReset; iWrite a 13; '
    expected = [(b'a', f'0b{13:0{width}b}'.encode())]
    reads = 'iRead a 0bx; '
    for i in range(count-1):
        w = width if i % 2 else width // 2
        value = tail if w == width else tail >> (width - w)
        source += (f'ScanRegister r{i}[{w-1}:0] {{ScanInSource a[0]; '
                   f'CaptureSource r{i}; ResetValue 0;}} '
                   f'ScanOutPort o{i} {{Source r{i}[0];}}')
        body += f'iWrite r{i} {value}; '
        expected.append((f'r{i}'.encode(), f'0b{value:0{w}b}'.encode()))
        reads += f'iRead r{i} 0bx; '
    return source + '}\n', body + 'iApply; ' + reads + 'iApply;\n', expected


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def instrument(source):
    begin = source.index('    std::map<Key, size_t> longest_lanes;')
    end = source.index('    for (size_t id : writable)', begin)
    region = source[begin:end]
    region, assignments = re.subn(
        r'(?m)^([ \t]*)entry->second\s*\.at\(\s*Offset\(',
        lambda match: match[1] + '++icl_benchmark_fanout_assignments;\n' + match[0], region)
    region, loops = re.subn(
        r'(?m)^([ \t]*)for \(size_t tick = 0; tick < clocks; \+\+tick\) \{',
        lambda match: match[1] + 'icl_benchmark_fanout_expected += '
        'static_cast<unsigned long long>(clocks) * longest_lanes.size();\n' + match[0], region)
    if assignments != 1 or loops != 1:
        raise RuntimeError('ordinary replay assignment/clock hooks must each match once')
    prefix = r'''#include <cstdio>
namespace {
volatile unsigned long long icl_benchmark_fanout_assignments = 0;
volatile unsigned long long icl_benchmark_fanout_expected = 0;
struct FanoutWorkReport {
  ~FanoutWorkReport() {
    std::fprintf(stderr, "fanout_pin_assignments=%llu expected=%llu\n",
        static_cast<unsigned long long>(icl_benchmark_fanout_assignments),
        static_cast<unsigned long long>(icl_benchmark_fanout_expected));
  }
};
FanoutWorkReport icl_benchmark_fanout_report;
}
'''
    return prefix + source[:begin] + region + source[end:]


def main():
    project = Path(__file__).resolve().parents[3]
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--binary', type=Path, default=project/'build/icl-pattern-generation')
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    args.binary = args.binary.resolve()
    args.output = args.output.resolve()
    args.output.mkdir(parents=True, exist_ok=False)
    producer = args.output/'producer'
    producer.mkdir()
    source = project/'src/pattern/pattern_scheduler.cc'
    original = producer/'original.cc'
    original.write_bytes(source.read_bytes())
    instrumented = producer/'instrumented.cc'
    instrumented.write_text(instrument(original.read_text()))
    commands = subprocess.check_output(['make', '-n', '-B', 'build/icl-pattern-generation'],
                                       cwd=project, text=True).splitlines()
    (producer/'make-commands.log').write_text('\n'.join(commands) + '\n')
    compile_commands = [shlex.split(line) for line in commands
                        if '-c src/pattern/pattern_scheduler.cc -o ' in line]
    link_commands = [shlex.split(line) for line in commands
                     if line.endswith('-o build/icl-pattern-generation')]
    if len(compile_commands) != 1 or len(link_commands) != 1:
        raise RuntimeError('expected one scheduler compile and generator link command')
    compile_command, link_command = compile_commands[0], link_commands[0]
    obj, binary = producer/'pattern_scheduler.o', producer/'generator'
    compile_command[compile_command.index('-c')+1] = str(instrumented)
    compile_command[compile_command.index('-o')+1] = str(obj)
    old_object = 'build/pattern/pattern_scheduler.o'
    if link_command.count(old_object) != 1:
        raise RuntimeError('generator must link exactly one scheduler object')
    link_command = [str(obj) if item == old_object else item for item in link_command]
    link_command[link_command.index('-o')+1] = str(binary)
    stages = []
    for name, command in [('compile', compile_command), ('link', link_command)]:
        start = time.monotonic()
        run = subprocess.run(command, cwd=project, capture_output=True, text=True)
        (producer/(name+'.log')).write_text(run.stdout + run.stderr)
        stages.append(dict(name=name, command=command, exit=run.returncode,
                           seconds=time.monotonic()-start))
        if run.returncode:
            raise RuntimeError(name+' failed: '+run.stdout[-3000:]+run.stderr[-3000:])
    report = dict(environment=platform.platform(), stages=stages,
                  source_sha256=digest(original), instrumented_source_sha256=digest(instrumented),
                  instrumented_binary_sha256=digest(binary), reference_binary=str(args.binary),
                  reference_binary_sha256=digest(args.binary), rows=[], work_failures=[])
    for count in (2, 8, 32):
        for width in (4, 64, 256):
            root = args.output/('%d-%d' % (count, width))
            root.mkdir()
            model, body, expected_writes = fixture(count, width)
            icl, pdl = root/'source.icl', root/'source.pdl'
            icl.write_text(model)
            pdl.write_text(body)
            results = {}
            for owner, executable in [('reference', args.binary), ('instrumented', binary)]:
                folder = root/owner
                folder.mkdir()
                output, metric = folder/'result.pdl', folder/'time.txt'
                command = [str(executable), '--icl', str(icl), '--pdl', str(pdl),
                           '--top_module', 'Chains', '--output', str(output), '--debug']
                start = time.monotonic()
                run = subprocess.run(['/usr/bin/time', '-f', '%e %M', '-o', str(metric), *command],
                                     cwd=root, capture_output=True, text=True, timeout=30)
                elapsed = time.monotonic()-start
                (folder/'process.log').write_text(run.stdout + run.stderr)
                if run.returncode:
                    raise RuntimeError(owner+' producer failed: '+run.stdout+run.stderr)
                seconds, rss = metric.read_text().split()
                results[owner] = dict(command=command, exit=run.returncode,
                    wall_seconds=elapsed, time_seconds=float(seconds), rss_kib=int(rss),
                    output_sha256=digest(output))
                if owner == 'reference':
                    if elapsed > 10 or int(rss) > 512*1024:
                        raise RuntimeError('reference source process exceeds 10s/512MiB')
                else:
                    records = re.findall(r'fanout_pin_assignments=(\d+) expected=(\d+)', run.stderr)
                    if len(records) != 1:
                        raise RuntimeError('missing unique executed assignment counter')
                    actual, expected = map(int, records[0])
                    if not expected or not actual:
                        raise RuntimeError('ordinary replay counter must execute positive work')
            if results['reference']['output_sha256'] != results['instrumented']['output_sha256']:
                raise RuntimeError('instrumentation changes complete reference PDL')
            if actual != expected:
                report['work_failures'].append([count, width])
            row = dict(fanout=count, width=width, actual_assignments=actual,
                       expected_assignments=expected, ratio=actual/expected,
                       icl_sha256=digest(icl), pdl_sha256=digest(pdl),
                       register_bits=sum(len(bits)-2 for _, bits in expected_writes),
                       results=results)
            report['rows'].append(row)
            (args.output/'results.json').write_text(json.dumps(report, indent=2)+'\n')
            print(json.dumps({key: row[key] for key in
                              ('fanout', 'width', 'actual_assignments', 'expected_assignments')}), flush=True)
    print(json.dumps(dict(cases=len(report['rows']), work_failures=report['work_failures'])), flush=True)
    return int(bool(report['work_failures']))


if __name__ == '__main__':
    raise SystemExit(main())
