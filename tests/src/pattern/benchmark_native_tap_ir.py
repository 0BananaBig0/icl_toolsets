#!/usr/bin/env python3
"""Compare native IR with the saved ordinary instruction planner."""
import benchmark_raw_tap_multiple_dr as multi


def scenarios():
    for name, model, bsdl, source, _ in multi.scenarios(
            [(64, 4, 0, 1, 0), (64, 4, 0, 1, 512),
             (3, 4, 1024, 1, 0), (1, 8, 0, 128, 0)]):
        program = source.split('iScan jtag', 1)[0]
        if '-repeat128-' in name:
            prefix, body = program.split('iReset\n', 1)
            program = prefix + 'iReset\n' + body * 128
        yield 'ir4-' + name, model, bsdl, program, program
    for width in [4, 32, 256]:
        _, model, _, source, _ = next(multi.scenarios([(1, 8, 0, 1, 0)]))
        row = next(row for row in multi.ir.scenarios()
                   if row[0] == f'w{width}-nodes1-repeat1')
        bsdl = row[2].replace('RawIrProbe', 'MultipleDrProbe')
        program = source.split('iScan jtag', 1)[0]
        yield f'ir{width}-registers1', model, bsdl, program, program


if __name__ == '__main__':
    report = multi.ir.main(scenarios, lambda data: data,
                           'MultipleDrTop', 'MultipleDrProbe')
    excessive = [(row['scenario'], row['form'], row['max_rss_kib'])
                 for row in report['rows'] if not row['form'].startswith('reference')
                 and row['max_rss_kib'] >= 128 * 1024]
    assert not excessive, ('native IR exceeds128MiB peak RSS', excessive)
