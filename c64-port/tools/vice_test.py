#!/usr/bin/env python3
"""Run the native diagnostic in VICE and verify a RAM mailbox; requires C64 ROMs."""
import argparse
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
prg=build/'executive.prg'; mailbox=build/'vice-mailbox.bin'
# Initialized zero-page tables can move the entry. Read the actual BASIC SYS.
header=prg.read_bytes()[:32]
sys_match=re.search(rb'\x9e\s*(\d+)\x00',header)
if sys_match is None: sys.exit('FAIL: PRG has no BASIC SYS header')
entry=int(sys_match[1])
mailbox.unlink(missing_ok=True)
(build/'vice-monitor.log').unlink(missing_ok=True)
commands=build/'vice.mon'
commands.write_text(f'fill 0100 01f1 cd\nfill c100 cfff cd\nload "{prg}" 0\nwatch store c008\nwatch store c000\n'
                    f'ignore 2 1\ng {entry:04x}\nstopwatch reset\ng\nstopwatch\n'
                    f'g\nstopwatch reset\ng\nstopwatch\ng\nstopwatch reset\ng\nstopwatch\n'
                    f'g\nstopwatch reset\ng\nstopwatch\ndelete 1\ng\nm c000 c00c\nbsave "{mailbox}" 0 c000 c00c\n'
                    f'bsave "{build}/vice-hardware-stack.bin" 0 0100 01f1\n'
                    f'bsave "{build}/vice-software-stack.bin" 0 c100 cfff\nscreen\nquit\n')
cmd=[a.vice,'-default','-console','-pal','-warp','+sound','-kernal',str(rom/'kernal-901227-03.bin'),
     '-basic',str(rom/'basic-901226-01.bin'),'-chargen',str(rom/'chargen-901225-01.bin')]
if a.data_dir: cmd+=['-directory',str(a.data_dir.resolve())+':'+str(rom.parent)]
else: cmd+=['-directory',str(rom.parent)]
cmd+=['-initbreak','ready','-moncommands',str(commands),'-monlogname',
      str(build/'vice-monitor.log'),'-monlog','-limitcycles','400000000']
env=os.environ.copy()
env.setdefault('XDG_CACHE_HOME',str(build/'vice-cache'))
env.setdefault('XDG_CONFIG_HOME',str(build/'vice-config'))
for key in ('XDG_CACHE_HOME','XDG_CONFIG_HOME'): pathlib.Path(env[key]).mkdir(parents=True,exist_ok=True)
with (build/'vice.log').open('w') as log:
    subprocess.run(cmd,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=60,env=env)
if not mailbox.exists(): sys.exit('FAIL: VICE did not produce a mailbox')
data=mailbox.read_bytes()
if len(data)!=13 or data[:3]!=bytes([0xa5,0,26]) or data[9:13]!=bytes([53,109,8,0]) or data[7]!=3:
    sys.exit('FAIL: VICE mailbox '+data.hex())
log=(build/'vice-monitor.log').read_text()
print('PASS: VICE PAL native execution, 26 Executive scenarios, 53 dispatch cases / 109 fingerprints, 8 native two-job sequences')
print('Mailbox:',data.hex())
print('Target executive size:',int.from_bytes(data[3:5],'little'))
print('Target scenario size:',int.from_bytes(data[5:7],'little'))
print('VICE command:',' '.join(cmd))
timings=[int(value) for value in re.findall(r'Stopwatch:\s+(\d+)',log)]
if len(timings)!=4: sys.exit('FAIL: expected four timing intervals')
for name,cycles in zip(['idle NOVAC','full-core FINDVAC/1202','completion scan/slot-seven dispatch','slot-seven CHANG1 exchange'],timings):
    print(name+': '+str(cycles)+' PAL cycles (includes marker overhead and any IRQ/VIC bus effects)')
for filename,top in [('vice-hardware-stack.bin',0x1f2),('vice-software-stack.bin',0xd000)]:
    values=(build/filename).read_bytes()
    base=0x100 if filename=='vice-hardware-stack.bin' else 0xc100
    modified=[i for i,b in enumerate(values) if b!=0xcd]
    print(filename+': measured sentinel extent '+str(top-(base+min(modified)) if modified else 0)+' bytes')
