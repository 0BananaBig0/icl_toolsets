#!/usr/bin/env python3
"""Compare automatic opaque-containing preparation with native manual control."""
from benchmark_dynamic_opaque_routes import model
from benchmark_modeled_child_prefix import measure, payload, replay_observations

GRID = [(n,width) for n in (2,8,32) for width in (8,64,256)]+[(n,1024) for n in (2,8)]


def commands(children, width, parent=False):
    short,active,long = children+8,children+width+8,children*(width+1)+8
    opened = '0b101'+'1'+'0'*(children-1)+'10011'
    text = 'iPDLLevel 1\niProcsForModule Chains\niReset\n'
    text += 'iScan client %d -si %s -so 0bx; iApply\n'%(short,opened)
    expected,words,compares = [],[opened],['0b'+'x'*short]
    returned = '0b101'+('x'*width+'1')*children+'10011'
    for group in range(4):
        new = [payload(i,group,width) for i in range(children)]
        text += 'iScan child0.local %d -si 0b%s -so 0bx -stable; iApply\n'%(width,new[0])
        if parent:
            text += ''.join('iWrite enable%d 1; '%i for i in range(1,children))+'iApply\n'
        for i in range(1,children):
            text += 'iScan child%d.local %d -si 0b%s -so 0bx -stable\n'%(i,width,new[i])
        text += 'iApply\n'
        for i in range(children):
            text += ('iNote [iGetReadData child%d.local -bin]\n'
                     'iNote [iGetMiscompares child%d.local -bin]\n')%(i,i)
            expected += ['0b'+'x'*width,'0b'+'0'*width]
        closed = '0b101'+new[0]+'1'+''.join(bits+'0' for bits in new[1:])+'10011'
        text += 'iScan client %d -si %s -so 0bx; iApply\n'%(long,closed)
        text += 'iNote [iGetReadData client -bin]\niNote [iGetMiscompares client -bin]\n'
        text += 'iNote [iGetStatus -clear]\niNote [iGetStatus]\n'
        expected += [returned,'0b'+'0'*long,'0','0']
        words += ['0b101'+new[0]+'1'+'0'*(children-1)+'10011',
                  '0b101'+''.join(bits+'1' for bits in new)+'10011',closed]
        compares += ['0b'+'x'*active,'0b'+'x'*long,'0b'+'x'*long]
    return text,expected,words,compares


def main():
    return measure(model,commands,note_prefix='modeled opaque child raw history ',
                   history_expected=lambda width:'x'*width,setup_csus=4,
                   stable=lambda index:index%3!=0,grid=GRID)


if __name__=='__main__':
    raise SystemExit(main())
