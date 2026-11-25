#!/usr/bin/env python3
"""Fixed opaque leaves behind modeled scalar bypasses."""
import re
from functools import partial

from benchmark_modeled_child_prefix import model as static_model
from benchmark_modeled_child_prefix import measure, payload, replay_observations


def model(children=1, width=8, visible=False):
    leaf,root = static_model(children,width).split('Module Chains {',1)
    if visible:
        leaf = leaf.replace('CaptureSource value; ResetValue 0;',
                            "CaptureSource %d'bx; ResetValue %d'bx;"%(width,width))
    else:
        leaf = re.sub(r' ScanRegister value[^\n]*\n','',leaf)
        leaf = leaf.replace('ScanOutPort so {Source value[0];}','ScanOutPort so;')
    leaf = leaf.replace('Port reset;}','Port reset; DefaultLoadValue %d\'b0;}'%width)
    for i in range(1,children):
        root = root.replace('InputPort si = child%d.so;'%(i-1),
                            'InputPort si = enable%d;'%(i-1))
    root = root.replace('ScanInSource child%d.so;'%(children-1),
                        'ScanInSource enable%d;'%(children-1))

    def gated(match):
        index = int(match.group(1))
        source = 'head[0]' if not index else 'enable%d'%(index-1)
        instance = match.group(0).replace('InputPort sel = sel;',
                                           'InputPort sel = enable%d;'%index)
        return instance+'''\n ScanMux route%d SelectedBy enable%d {1'b0: %s; 1'b1: child%d.so;}
 ScanRegister enable%d {ScanInSource route%d; CaptureSource enable%d; ResetValue 0;}
'''%(index,index,source,index,index,index,index)

    root = re.sub(r'Instance child(\d+) Of Leaf \{[^}]*\}',gated,root)
    root = root.replace("DefaultLoadValue %d'b0;"%(children*width+8),
                        "DefaultLoadValue %d'b0;"%(children+8))
    return leaf+'Module Chains {'+root


def commands(children, width, parent=False):
    long_width,short_width = children*(width+1)+8,children+8
    opened = '0b101'+'1'*children+'10011'
    closed = '0b101'+'0'*children+'10011'
    text = 'iPDLLevel 1\niProcsForModule Chains\niReset\n'
    text += 'iScan client %d -si %s -so 0bx; iApply\n'%(short_width,opened)
    expected,words,compares = [],[opened],['0b'+'x'*short_width]
    for group in range(4):
        new = [payload(i,group,width) for i in range(children)]
        for i,bits in enumerate(new):
            text += 'iScan child%d.local %d -si 0b%s -so 0bx\n'%(i,width,bits)
        text += 'iApply\n'
        for i in range(children):
            text += 'iNote [iGetReadData child%d.local -bin]\niNote [iGetMiscompares child%d.local -bin]\n'%(i,i)
            expected += ['0b'+'x'*width,'0b'+'0'*width]
        closing = '0b101'+''.join(bits+'0' for bits in new)+'10011'
        text += 'iScan client %d -si %s -so 0bx; iApply\n'%(long_width,closing)
        text += 'iScan client %d -si %s -so %s; iApply\n'%(short_width,opened,closed)
        text += 'iNote [iGetReadData client -bin]\niNote [iGetMiscompares client -bin]\n'
        text += 'iNote [iGetStatus -clear]\niNote [iGetStatus]\n'
        expected += [closed,'0b'+'0'*short_width,'0','0']
        words += ['0b101'+''.join(bits+'1' for bits in new)+'10011',closing,opened]
        compares += ['0b'+'x'*long_width,'0b'+'x'*long_width,closed]
    return text,expected,words,compares


def main():
    return measure(model,commands,note_prefix='modeled opaque child raw history ',
                   history_expected=lambda width:'x'*width,setup_csus=0,stable=False,
                   parent_model_factory=partial(model,visible=True),
                   parent_note_prefix='modeled dynamic child raw history ')


if __name__=='__main__':
    raise SystemExit(main())
