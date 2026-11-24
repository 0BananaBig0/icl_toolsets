#!/usr/bin/env python3
"""Compare invoked read-callback observations with direct native endpoint controls."""
import benchmark_merged_readback as base


def fixture(kind, calls, width, merge, observed=True, owner=None):
    model, text, notes = base.fixture(kind, calls, width, merge, observed)
    if kind == 'pio':
        declaration = (f' DataRegister observed[{width-1}:0] {{'
            'ReadCallBack Chains load <D>; ReadDataSource p;}\n')
        end = model.rfind('}')
        model = model[:end]+declaration+model[end:]
    if owner == 'parent' or kind == 'raw':
        # Retain the callback's empty outer Apply in the native controls too.
        annotation = ('; iNote -comment "modeled callback read history unavailable '+('1'*width)+' observed"') if kind == 'pio' else ''
        return model, text.replace('; iApply;', '; iApply'+annotation+'; iApply;', 1), notes
    endpoint = 'iRead q $data'
    text = text.replace('iRead q 0', 'iRead observed 0')
    text = text.replace('set ::seen {}', 'iProc load {data} {'+endpoint+'}\nset ::seen {}', 1)
    return model, text, notes


if __name__ == '__main__':
    raise SystemExit(base.main(fixture, wide_calls=2, applies_per_call=2))
