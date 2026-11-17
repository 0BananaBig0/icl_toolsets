#!/usr/bin/env python3
"""Measure dense repeated IR; the saved planner exceeds its benchmark timeout."""
import benchmark_raw_tap_multiple_dr as multi


def scenarios():
    name, model, bsdl, source, _ = next(multi.scenarios([(64, 4, 0, 128, 0)]))
    prefix, body = source.split('iScan jtag', 1)[0].split('iReset\n', 1)
    program = prefix + 'iReset\n' + body * 128
    yield 'native-only-ir4-' + name, model, bsdl, program, None


if __name__ == '__main__':
    report = multi.ir.main(scenarios, lambda data: data,
                           'MultipleDrTop', 'MultipleDrProbe', candidate_only=True)
    assert all(row['max_rss_kib'] < 128 * 1024 for row in report['rows'])
