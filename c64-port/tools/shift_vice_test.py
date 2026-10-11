#!/usr/bin/env python3
"""Run the native diagnostic in VICE and verify a RAM mailbox; requires C64 ROMs."""
import argparse
import json
import pathlib
import os
import re
import subprocess
import sys
ROOT=pathlib.Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser()
p.add_argument('--vice',default='x64sc')
p.add_argument('--rom-dir',required=True,type=pathlib.Path)
p.add_argument('--data-dir',type=pathlib.Path)
a=p.parse_args()
rom=a.rom_dir.resolve(); build=ROOT/'build'; build.mkdir(exist_ok=True)
prg=build/'shift.prg'; mailbox=build/'shift-vice-mailbox.bin'
# Initialized zero-page tables can move the entry. Read the actual BASIC SYS.
mapping=(build/'shift.map').read_text()
for section in ('.text','.rodata','.data','.bss','.noinit'):
    entries=re.findall(r'^\s*([0-9a-f]+)\s+([0-9a-f]+)\s+([0-9a-f]+)\s+\d+\s+'+re.escape(section)+r'$',mapping,re.M)
    if len(entries)!=1 or int(entries[0][0],16)+int(entries[0][2],16)>0xc000:
        sys.exit('FAIL: image/state overlaps reserved diagnostic RAM: '+section)
for section in ('.zp.data','.zp.bss'):
    matches=re.findall(r'^\s*([0-9a-f]+)\s+([0-9a-f]+)\s+([0-9a-f]+)\s+\d+\s+'+re.escape(section)+r'$',mapping,re.M)
    if len(matches)!=1 or int(matches[0][2],16)!=0: sys.exit('FAIL: extra compiler ZP allocation')
header=prg.read_bytes()[:32]
sys_match=re.search(rb'\x9e\s*(\d+)\x00',header)
if sys_match is None: sys.exit('FAIL: PRG has no BASIC SYS header')
entry=int(sys_match[1])
mailbox.unlink(missing_ok=True)
(build/'shift-vice-monitor.log').unlink(missing_ok=True)
commands=build/'shift-vice.mon'
commands.write_text(f'fill 0100 01f1 cd\nfill c100 cfff cd\nload "{prg}" 0\nwatch store c088\nwatch store c080\n'
                    f'ignore 2 1\ng {entry:04x}\n'+
                    ('stopwatch reset\ng\nstopwatch\ng\n' * 4)+
                    'stopwatch reset\ng\nstopwatch\ndelete 1\ng\nm c080 c088\n'+
                    f'bsave "{mailbox}" 0 c080 c088\n'
                    f'bsave "{build}/shift-vice-hardware-stack.bin" 0 0100 01f1\n'
                    f'bsave "{build}/shift-vice-software-stack.bin" 0 c100 cfff\nscreen\nquit\n')
cmd=[a.vice,'-default','-console','-pal','-warp','+sound','-kernal',str(rom/'kernal-901227-03.bin'),
     '-basic',str(rom/'basic-901226-01.bin'),'-chargen',str(rom/'chargen-901225-01.bin')]
if a.data_dir: cmd+=['-directory',str(a.data_dir.resolve())+':'+str(rom.parent)]
else: cmd+=['-directory',str(rom.parent)]
cmd+=['-initbreak','ready','-moncommands',str(commands),'-monlogname',
      str(build/'shift-vice-monitor.log'),'-monlog','-limitcycles','1000000000']
env=os.environ.copy()
env.setdefault('XDG_CACHE_HOME',str(build/'vice-cache'))
env.setdefault('XDG_CONFIG_HOME',str(build/'vice-config'))
for key in ('XDG_CACHE_HOME','XDG_CONFIG_HOME'): pathlib.Path(env[key]).mkdir(parents=True,exist_ok=True)
with (build/'shift-vice.log').open('w') as log:
    subprocess.run(cmd,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=60,env=env)
if not mailbox.exists(): sys.exit('FAIL: VICE did not produce a mailbox')
data=mailbox.read_bytes()
if data!=bytes.fromhex('a50000a80003b55924'):
    sys.exit('FAIL: VICE shift mailbox '+data.hex())
log=(build/'shift-vice-monitor.log').read_text()
print('PASS: VICE PAL, 168 complete original-assembly scalar/vector shift states; every count/form, signed-zero/rounding/overflow cases; restoring I')
print('Mailbox:',data.hex())
print('Target state bytes / modeled words:',data[6],data[7])
print('VICE command:',' '.join(cmd))
timings=[int(value) for value in re.findall(r'Stopwatch:\s+(\d+)',log)]
if len(timings)!=5: sys.exit('FAIL: expected five timing intervals')
for name,cycles in zip(['scalar SL1','scalar SR14','vector VSL2','vector VSR14','short vector VSR8'],timings):
    print(name+': '+str(cycles)+' PAL cycles (includes marker overhead and any IRQ/VIC bus effects)')
(build/'shift-timing.json').write_text(json.dumps({
    'pal_clock_hz':985248,'samples':dict(zip(['scalar_SL1','scalar_SR14','vector_VSL2','vector_VSR14','short_vector_VSR8'],timings)),
    'scope':'Finite excluded-call samples include marker overhead and VIC/IRQ effects; no WCET or live guidance deadline proof.'
},indent=2)+'\n')
for filename,top in [('shift-vice-hardware-stack.bin',0x1f2),('shift-vice-software-stack.bin',0xd000)]:
    values=(build/filename).read_bytes()
    base=0x100 if 'hardware' in filename else 0xc100
    modified=[i for i,b in enumerate(values) if b!=0xcd]
    extent=top-(base+min(modified)) if modified else 0
    if extent>=len(values): sys.exit('FAIL: exhausted instrumented stack region')
    print(filename+': measured sentinel extent '+str(extent)+' bytes')
