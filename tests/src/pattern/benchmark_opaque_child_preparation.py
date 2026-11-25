#!/usr/bin/env python3
"""Measure native parent-only preparation before opaque child requests."""
from functools import partial

from benchmark_dynamic_opaque_routes import model
from benchmark_modeled_child_prefix import measure, payload, replay_observations


def commands(children, width, parent=False):
    size = children*(width+1)+8
    text = ('iPDLLevel 1\niProcsForModule Chains\niReset\n'
            'iWrite head 5; iWrite tail 19; iApply\n')
    expected,words,compares = [],[],[]
    returned = '0b101'+('x'*width+'1')*children+'10011'
    for group in range(4):
        new = [payload(i,group,width) for i in range(children)]
        for i,bits in enumerate(new):
            text += 'iScan child%d.local %d -si 0b%s -so 0bx\n'%(i,width,bits)
        text += 'iApply\n'
        for i in range(children):
            text += ('iNote [iGetReadData child%d.local -bin]\n'
                     'iNote [iGetMiscompares child%d.local -bin]\n')%(i,i)
            expected += ['0b'+'x'*width,'0b'+'0'*width]
        opened = '0b101'+''.join(bits+'1' for bits in new)+'10011'
        closed = '0b101'+''.join(bits+'0' for bits in new)+'10011'
        text += 'iScan client %d -si %s -so 0bx; iApply\n'%(size,closed)
        text += 'iNote [iGetReadData client -bin]\niNote [iGetMiscompares client -bin]\n'
        text += 'iNote [iGetStatus -clear]\niNote [iGetStatus]\n'
        expected += [returned,'0b'+'0'*size,'0','0']
        words += [opened,closed]
        compares += ['0b'+'x'*size]*2
    return text,expected,words,compares


def main():
    return measure(model,commands,note_prefix='modeled opaque child raw history ',
                   history_expected=lambda width:'x'*width,setup_csus=5,stable=False,
                   parent_model_factory=partial(model,visible=True),
                   parent_note_prefix='modeled dynamic child raw history ')


if __name__=='__main__':
    raise SystemExit(main())
