#!/usr/bin/env python3
"""Compare child preparation queues with explicit Task106 Apply boundaries."""
import benchmark_native_ordinary_scans as native


def scenarios():
    fixtures = native.ROOT / 'tests/cases/ours/pattern_generation'
    sib = (fixtures / 'gated_sib_raw/model.icl').read_text()
    prefix = 'iPDLLevel 1\niProcsForModule Top\niReset\n'
    width = 1048576
    model = sib.replace('[7:0]', f'[{width - 2}:0]')
    model = model.replace("ResetValue 8'b0", f"ResetValue {width - 1}'b0")
    prepare = 'iWrite open 1; iRead open 0;\n'
    raw = f'iScan child.local {width - 1} -si 0xa5 -so 0;\n'
    yield 'wide1048576', model, prefix + raw + prepare + 'iApply\n', \
        prefix + prepare + 'iApply\n' + raw + 'iApply\n', width, 1, True
    count = 1024
    prepare = ''.join(f'iWrite r{i}.enable 1;\n' for i in range(count))
    raw = ''.join(f'iScan r{i}.leaf.local 8 -si 0xa5;\n' for i in range(count))
    yield 'distinct1024', native.many_model(count), prefix + raw + prepare + 'iApply\n', \
        prefix + prepare + 'iApply\n' + raw + 'iApply\n', 9 * count, count, True
    prepare = 'iWrite open 1; iRead open 0;\n'
    raw = 'iScan child.local 8 -si 0xa5;\n'
    close = 'iWrite open 0; iRead open 1; iApply\n'
    mixed = raw + prepare + 'iApply\n' + close
    explicit = prepare + 'iApply\n' + raw + 'iApply\n' + close
    yield 'repeat128', sib, prefix + mixed * 128, prefix + explicit * 128, 9, 128, True
    static = next(row for row in native.scenarios() if row[0] == 'partial32')
    model = static[1]
    initial = 'iWrite head[31:0] 5; iApply\n'
    owner = 'iTake head[31:0]; iState head[31:0] 5;\n'
    prepare = 'iWrite child.value 1; iRead child.value 0;\n'
    raw = 'iScan child.local 8 -si 0xa5 -so 1;\n'
    yield 'partial32', model, prefix + initial + owner + raw + prepare + 'iApply\n', \
        prefix + initial + owner + prepare + 'iApply\n' + raw + 'iApply\n', width, 1, True


if __name__ == '__main__':
    native.main(scenarios)
