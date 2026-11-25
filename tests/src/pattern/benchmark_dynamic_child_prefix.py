#!/usr/bin/env python3
"""Dynamic child closure fixtures and measured actual-history replay."""
from benchmark_modeled_child_prefix import model as static_model
from benchmark_modeled_child_prefix import measure, payload, replay_observations


def model(children=1, width=8):
    source = static_model(children,width)
    leaf,root = source.split('Module Chains {',1)
    branch = '''Module DynamicBranch {
 ScanInPort si; SelectPort sel; CaptureEnPort ce; ShiftEnPort se;
 UpdateEnPort ue; TCKPort tck; ResetPort reset;
 Instance leaf Of Leaf {InputPort si = si; InputPort sel = enable;
  InputPort ce = ce; InputPort se = se; InputPort ue = ue;
  InputPort tck = tck; InputPort reset = reset;}
 ScanMux route SelectedBy enable {1'b0: si; 1'b1: leaf.so;}
 ScanRegister enable {ScanInSource route; CaptureSource enable; ResetValue 0;}
 ScanOutPort so {Source enable;}
 ScanInterface local {Port si; Port so; Port sel; Port ce; Port se; Port ue; Port tck; Port reset; DefaultLoadValue 1'b0;}
}
'''
    root = root.replace(' Of Leaf {',' Of DynamicBranch {')
    root = root.replace("DefaultLoadValue %d'b0;"%(children*width+8),
                        "DefaultLoadValue %d'b0;"%(children+8))
    return leaf+branch+'Module Chains {'+root


def commands(children, width, parent=False):
    text = 'iPDLLevel 1\niProcsForModule Chains\niReset\niWrite head 5; iWrite tail 19;\n'
    text += ''.join('iWrite child%d.enable 1;\n'%i for i in range(children))+'iApply\n'
    old = [payload(i,0,width) for i in range(children)]
    text += ''.join('iWrite child%d.leaf.value 0b%s;\n'%(i,bits) for i,bits in enumerate(old))+'iApply\n'
    expected,words,compares = [],[],[]
    long_width,short_width = children*(width+1)+8,children+8
    closed = '0b101'+'0'*children+'10011'
    opened = '0b101'+'1'*children+'10011'
    for group in range(4):
        new = [payload(i,group+1,width) for i in range(children)]
        for i,bits in enumerate(new):
            text += 'iScan child%d.local %d -si 0b%s0 -so 0\n'%(i,width+1,bits)
        text += 'iApply\n'
        for i,bits in enumerate(old):
            text += 'iNote [iGetReadData child%d.local -bin]\niNote [iGetMiscompares child%d.local -bin]\n'%(i,i)
            expected += ['0b'+bits+'1']*2
        text += 'iScan client %d -si %s -so %s; iApply\n'%(short_width,opened,closed)
        text += 'iNote [iGetReadData client -bin]\niNote [iGetMiscompares client -bin]\n'
        text += 'iNote [iGetStatus -clear]\niNote [iGetStatus]\n'
        expected += [closed,'0b'+'0'*short_width,str(sum(bits.count('1') for bits in old)+children),'0']
        words += ['0b101'+''.join(bits+'0' for bits in new)+'10011',opened]
        compares += ['0bxxx'+'0'*(long_width-8)+'xxxxx',closed]
        old = new
    return text,expected,words,compares


def main():
    return measure(model,commands,note_prefix='modeled dynamic child raw history ',
                   history_expected=lambda width:'0'*(width+1),setup_csus=2,stable=False)


if __name__=='__main__':
    raise SystemExit(main())
