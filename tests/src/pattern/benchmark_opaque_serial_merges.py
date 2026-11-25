#!/usr/bin/env python3
"""Compare native opaque iMerge calls with actual sequential parent calls."""
from benchmark_dynamic_opaque_routes import model
from benchmark_modeled_child_prefix import measure, payload, replay_observations


def commands(children, width, parent=False):
    short,long = children+8,children*(width+1)+8
    opened = '0b101'+'1'*children+'10011'
    text = 'iPDLLevel 1\niProcsForModule Chains\n'
    text += 'iProc load {target word} {iScan $target %d -si $word -so 0bx -stable; iApply}\n'%width
    text += 'iReset\niScan client %d -si %s -so 0bx; iApply\n'%(short,opened)
    expected,words,compares = [],[opened],['0b'+'x'*short]
    loads = ['0'*width]*children
    for group in range(4):
        if not parent:
            text += 'iMerge -begin\n'
        for i in sorted(range(children),key=lambda i:'child%d.local'%i):
            loads[i] = payload(i,group,width)
            text += 'iCall load child%d.local 0b%s\n'%(i,loads[i])
            words.append('0b101'+''.join(bits+'1' for bits in loads)+'10011')
            compares.append('0b'+'x'*long)
        if not parent:
            text += 'iMerge -end\n'
        for i in range(children):
            text += ('iNote [iGetReadData child%d.local -bin]\n'
                     'iNote [iGetMiscompares child%d.local -bin]\n')%(i,i)
            expected += ['0b'+'x'*width,'0b'+'0'*width]
        text += 'iNote [iGetReadData client -bin]\niNote [iGetMiscompares client -bin]\n'
        text += 'iNote [iGetStatus -clear]\niNote [iGetStatus]\n'
        expected += ['0b'+'0'*short,'0b'+'0'*short,'0','0']
    return text,expected,words,compares


def main():
    return measure(model,commands,note_prefix='modeled opaque child raw history ',
                   history_expected=lambda width:'x'*width,setup_csus=0,
                   stable=lambda index:index!=0)


if __name__=='__main__':
    raise SystemExit(main())
