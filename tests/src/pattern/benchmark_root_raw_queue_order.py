#!/usr/bin/env python3
"""Compare raw-first source queues to canonical Task105 ordinary-first output."""
import re

import benchmark_native_ordinary_scans as native


def scenarios():
    for name, model, source, _, active, setups, _ in native.scenarios():
        if name == 'partial32':
            source = source.replace('iRead child.value 0; iApply\n',
                                    'iRead child.value 0;\n')
        # These controlled fixtures use literal root scans and iApply lines.
        # This is a fixture transform, not a general Tcl parser.
        prefix, body = source.split('iReset\n', 1)
        groups = []
        for group in body.split('iApply\n'):
            scan = re.search(r'iScan client [^;]+;', group)
            if scan:
                group = scan.group() + '\n' + group[:scan.start()] + group[scan.end():]
            groups.append(group)
        reordered = prefix + 'iReset\n' + 'iApply\n'.join(groups)
        yield name, model, reordered, source, active, setups, True


if __name__ == '__main__':
    native.main(scenarios)
