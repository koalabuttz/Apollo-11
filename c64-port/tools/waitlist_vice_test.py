#!/usr/bin/env python3
"""Run native C64 waitlist checks and a real IRQ-admitted waking job."""
import argparse
import json
import os
import pathlib
import re
import subprocess
import sys
root=pathlib.Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('--vice',default='x64sc')
p.add_argument('--rom-dir',required=True,type=pathlib.Path)
p.add_argument('--data-dir',type=pathlib.Path)
a=p.parse_args(); rom=a.rom_dir.resolve(); build=root/'build'
prg=build/'waitlist.prg'; mapping=(build/'waitlist.map').read_text()
entry=re.search(rb'\x9e\s*(\d+)\x00',prg.read_bytes()[:32])
if entry is None: sys.exit('FAIL: missing BASIC SYS header')
for section in ('zp','zp.data','zp.bss','zp.noinit'):
    match=re.search(r'^\s*[0-9a-f]+\s+[0-9a-f]+\s+([0-9a-f]+)\s+1\s+\.'+re.escape(section)+r'$',mapping,re.M)
    if match is not None and int(match[1],16)!=0: sys.exit('FAIL: unpreserved extra ZP allocation')
stack_match=re.search(r'^\s*([0-9a-f]+)\s+[0-9a-f]+\s+100\s+1\s+c64_timer_stack$',mapping,re.M)
if stack_match is None: sys.exit('FAIL: missing isolated ISR stack')
stack=int(stack_match[1],16)
mailbox=build/'waitlist-mailbox.bin'; log_path=build/'waitlist-monitor.log'
mailbox.unlink(missing_ok=True); log_path.unlink(missing_ok=True)
commands=build/'waitlist.mon'
events=331
script=(f'fill 0100 01f1 cd\nfill c100 cfff cd\nload "{prg}" 0\n'
        f'watch store c058\nwatch store c060\nwatch store c050\nignore 3 1\nstopwatch reset\ng {int(entry[1]):04x}\n')
script+='m c058 c05d\nm c060 c060\nstopwatch\n'
script+='g\nm c058 c05d\nm c060 c060\nstopwatch\n'*(events-1)
script+=(f'delete 1\ndelete 2\ng\nm c050 c05c\nbsave "{mailbox}" 0 c050 c05c\n'
         f'bsave "{build}/waitlist-hardware-stack.bin" 0 0100 01f1\n'
         f'bsave "{build}/waitlist-software-stack.bin" 0 c100 cfff\n'
         f'bsave "{build}/waitlist-isolated-stack.bin" 0 {stack:04x} {stack+255:04x}\n'
         'screen\nquit\n')
commands.write_text(script)
cmd=[a.vice,'-default','-console','-pal','-warp','+sound',
     '-kernal',str(rom/'kernal-901227-03.bin'),'-basic',str(rom/'basic-901226-01.bin'),
     '-chargen',str(rom/'chargen-901225-01.bin'),
     '-directory',(str(a.data_dir.resolve())+':' if a.data_dir else '')+str(rom.parent),
     '-initbreak','ready','-moncommands',str(commands),'-monlogname',str(log_path),
     '-monlog','-limitcycles','400000000']
env=os.environ.copy()
for key,directory in [('XDG_CACHE_HOME','vice-cache'),('XDG_CONFIG_HOME','vice-config')]:
    env.setdefault(key,str(build/directory)); pathlib.Path(env[key]).mkdir(parents=True,exist_ok=True)
with (build/'waitlist-vice.log').open('w') as log:
    subprocess.run(cmd,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=60,env=env)
if not mailbox.exists(): sys.exit('FAIL: missing waitlist mailbox')
data=mailbox.read_bytes()
if data!=bytes([0xa5,0,0,196,0x32,1,10,6,0x96,58,0,5,0]):
    sys.exit('FAIL: WAITLIST mailbox '+data.hex())
log=log_path.read_text()
records=[]
for watch,chunk in re.findall(r'#([12]) \(Stop on store c0(?:58|60)\)(.*?Stopwatch:\s+\d+)',log,re.S):
    foreground=re.search(r'>C:c058\s+((?:[0-9a-f]{2}\s+){5}[0-9a-f]{2})',chunk,re.I)
    irq=re.search(r'>C:c060\s+([0-9a-f]{2})',chunk,re.I)
    cycle=re.search(r'Stopwatch:\s+(\d+)',chunk)
    if foreground is None or irq is None or cycle is None: sys.exit('FAIL: incomplete marker record')
    values=foreground[1].split()
    records.append(dict(marker=int(values[0] if watch=='1' else irq[1],16),case=int(values[5],16),cycle=int(cycle[1])))
expected=[0x10,0x20]*156+[0x81,0x82,0x90]+[0x81,0x82]*4+[0x91,0x92,0x81,0x82,0x93,0x94,0x95,0x96]
if len(records)!=events or [r['marker'] for r in records]!=expected:
    sys.exit('FAIL: WAITLIST marker count/order: '+str([r['marker'] for r in records][-25:]))
samples=[]
for n in range(156):
    first,second=records[2*n:2*n+2]
    if first['case']!=n or second['case']!=n: sys.exit('FAIL: timing fixture metadata')
    group='insert' if n<120 else 'dispatch' if n<144 else 'taskover' if n<152 else 'bad delay'
    samples.append(dict(case=n,group=group,cycles=second['cycle']-first['cycle']))
irq_samples=[dict(cycles=b['cycle']-a['cycle'],case=a['case']) for a,b in zip(records,records[1:]) if a['marker']==0x81 and b['marker']==0x82]
starts=[r['cycle'] for r in records if r['marker']==0x81]
periods=[b-a for a,b in zip(starts[1:5],starts[2:5])]
if len(irq_samples)!=6 or any(abs(value-19705)>120 for value in periods):
    sys.exit('FAIL: missing/coalesced periodic CIA events: '+str(periods))
bridge={}
for first,second in [(0x92,0x93),(0x93,0x94),(0x94,0x95),(0x95,0x96)]:
    start=next(r['cycle'] for r in records if r['marker']==first)
    end=next(r['cycle'] for r in records if r['marker']==second)
    bridge[f'{first:02x}_to_{second:02x}']=end-start
stacks={}
for kind,top in [('hardware',0x1f2),('software',0xd000),('isolated',stack+256)]:
    values=(build/f'waitlist-{kind}-stack.bin').read_bytes()
    touched=[i for i,b in enumerate(values) if b!=0xcd]
    if touched and min(touched)==0: sys.exit('FAIL: stack exhausted instrumented range')
    stacks[kind]=dict(top=top,instrumented=len(values),sentinel_extent=len(values)-min(touched) if touched else 0)
result=dict(mailbox=data.hex(),command=cmd,samples=samples,stacks=stacks,
            irq_samples=irq_samples,periods=periods,bridge=bridge,markers=records)
(build/'waitlist-timing.json').write_text(json.dumps(result,indent=2)+'\n')
print('PASS: PAL C64 196 WAITLIST cases / 306 reference fingerprints, ten native task sequences, six real CIA Timer B IRQs and task-admitted/woken jobs')
print('Mailbox:',data.hex())
for name in ('insert','dispatch','taskover','bad delay'):
    group=[s for s in samples if s['group']==name]; maximum=max(group,key=lambda s:s['cycles'])
    print(f"{name}: {len(group)} samples, maximum observed {maximum['cycles']} PAL cycles (case {maximum['case']})")
print('CIA IRQ intervals:',[r['cycles'] for r in irq_samples])
print('Continuous timer periods:',periods)
print('Task bridge:',bridge)
print('Stack sentinel extents:',{k:v['sentinel_extent'] for k,v in stacks.items()})
