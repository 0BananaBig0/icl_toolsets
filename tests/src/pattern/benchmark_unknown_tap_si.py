#!/usr/bin/env python3
"""Measure literal-X TAP replay; the prior binary rejects this input."""
import re

import benchmark_raw_tap_multiple_dr as multi


def scenarios():
    shapes = [(1, 8, 0, 1, 0), (1, 32, 0, 1, 0), (1, 256, 0, 1, 0),
              (64, 4, 0, 1, 0), (64, 4, 0, 1, 512),
              (3, 4, 1024, 1, 0), (64, 4, 0, 128, 0)]
    for name, model, bsdl, source, _ in multi.scenarios(shapes):
        def literal(match):
            width, value = int(match[1]), int(match[2])
            bits = format(value & ((1 << width) - 1), f'0{width}b')
            bits = ''.join('x' if i % 7 == 0 else bit for i, bit in enumerate(bits))
            return f'iScan jtag {width} -si 0b{bits} -so 0b' + 'x' * width
        source = re.sub(r'iScan jtag (\d+) -si (\d+) -so \d+', literal, source)
        yield 'literal-' + name, model, bsdl, source, None
    _, model, bsdl, _, _ = next(multi.scenarios([(3, 4, 0, 1, 0)]))
    body = ('iScan -ir jtag 4 -si 0bx011 -so 0bxx01; iApply\n'
            'iScan -ir jtag 4 -si 3 -so 0bxx01; iApply\n')
    source = 'iPDLLevel 1\niProcsForModule MultipleDrTop\niReset\n' + body * 16
    yield 'literal-ir4-repeat16', model, bsdl, source, None


if __name__ == '__main__':
    multi.ir.main(scenarios, lambda data: data, 'MultipleDrTop',
                  'MultipleDrProbe', candidate_only=True)
