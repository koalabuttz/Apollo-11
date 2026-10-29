#!/usr/bin/env python3
"""Run native C64 lifecycle checks and a real IRQ-admitted waking job."""
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
prg=build/'lifecycle.prg'; mapping=(build/'lifecycle.map').read_text()
entry=re.search(rb'\x9e\s*(\d+)\x00',prg.read_bytes()[:32])
if entry is None: sys.exit('FAIL: missing BASIC SYS header')
for section in ('zp','zp.data','zp.bss','zp.noinit'):
    match=re.search(r'^\s*[0-9a-f]+\s+[0-9a-f]+\s+([0-9a-f]+)\s+1\s+\.'+re.escape(section)+r'$',mapping,re.M)
    if match is not None and int(match[1],16)!=0: sys.exit('FAIL: unpreserved extra ZP allocation')
stack_match=re.search(r'^\s*([0-9a-f]+)\s+[0-9a-f]+\s+100\s+1\s+c64_irq_stack$',mapping,re.M)
if stack_match is None: sys.exit('FAIL: missing isolated ISR stack')
stack=int(stack_match[1],16)
mailbox=build/'lifecycle-mailbox.bin'; log_path=build/'lifecycle-monitor.log'
mailbox.unlink(missing_ok=True); log_path.unlink(missing_ok=True)
commands=build/'lifecycle.mon'
events=157
script=(f'fill 0100 01f1 cd\nfill c100 cfff cd\nload "{prg}" 0\n'
        f'watch store c048\nwatch store c040\nignore 2 1\nstopwatch reset\ng {int(entry[1]):04x}\n')
script+='m c048 c048\nm c04d c04d\nstopwatch\n'
script+='g\nm c048 c048\nm c04d c04d\nstopwatch\n'*(events-1)
script+=(f'delete 1\ng\nm c040 c04c\nbsave "{mailbox}" 0 c040 c04c\n'
         f'bsave "{build}/lifecycle-hardware-stack.bin" 0 0100 01f1\n'
         f'bsave "{build}/lifecycle-software-stack.bin" 0 c100 cfff\n'
         f'bsave "{build}/lifecycle-isolated-stack.bin" 0 {stack:04x} {stack+255:04x}\n'
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
with (build/'lifecycle-vice.log').open('w') as log:
    subprocess.run(cmd,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=60,env=env)
if not mailbox.exists(): sys.exit('FAIL: missing lifecycle mailbox')
data=mailbox.read_bytes()
if data!=bytes([0xa5,0,0,81,115,5,1,76,0x94,0x88,2,0xd2,4]):
    sys.exit('FAIL: lifecycle mailbox '+data.hex())
log=log_path.read_text()
markers=[int(v,16) for v in re.findall(r'>C:c048\s+([0-9a-f]{2})',log,re.I)]
cases=[int(v,16) for v in re.findall(r'>C:c04d\s+([0-9a-f]{2})',log,re.I)]
times=[int(v) for v in re.findall(r'Stopwatch:\s+(\d+)',log)]
if len(times)!=events or len(cases)!=events or markers!=[0x10,0x20]*76+list(range(0x90,0x95)):
    sys.exit('FAIL: lifecycle marker count/order')
samples=[]
for n in range(76):
    if cases[n*2:n*2+2]!=[n,n]: sys.exit('FAIL: timing fixture metadata')
    samples.append(dict(case=n,group='sleep' if n<22 else 'wake' if n<51 else 'priority',
                        cycles=times[n*2+1]-times[n*2]))
stacks={}
for kind,top in [('hardware',0x1f2),('software',0xd000),('isolated',stack+256)]:
    values=(build/f'lifecycle-{kind}-stack.bin').read_bytes()
    touched=[i for i,b in enumerate(values) if b!=0xcd]
    if touched and min(touched)==0: sys.exit('FAIL: stack exhausted instrumented range')
    stacks[kind]=dict(top=top,instrumented=len(values),sentinel_extent=len(values)-min(touched) if touched else 0)
result=dict(mailbox=data.hex(),command=cmd,samples=samples,stacks=stacks,
            irq_bridge=dict(enable_marker_to_publication_observed=times[153]-times[152],
                            publication_observed_to_awakened_dispatch=times[154]-times[153],
                            awakened_completion=times[155]-times[154],
                            waker_completion=times[156]-times[155]),
            markers=[dict(marker=m,case=n,cycle=t) for m,n,t in zip(markers,cases,times)])
(build/'lifecycle-timing.json').write_text(json.dumps(result,indent=2)+'\n')
print('PASS: PAL C64 81 lifecycle cases / 115 reference fingerprints, five native sequences and a real IRQ-admitted waking job')
print('Mailbox:',data.hex())
for name in ('sleep','wake','priority'):
    group=[s for s in samples if s['group']==name]; maximum=max(group,key=lambda s:s['cycles'])
    print(f"{name}: {len(group)} samples, maximum observed {maximum['cycles']} PAL cycles (case {maximum['case']})")
print('IRQ bridge:',result['irq_bridge'])
print('Stack sentinel extents:',{k:v['sentinel_extent'] for k,v in stacks.items()})
