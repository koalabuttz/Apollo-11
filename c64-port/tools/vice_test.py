#!/usr/bin/env python3
"""Run the native diagnostic in VICE and verify a RAM mailbox; requires C64 ROMs."""
import argparse
import pathlib
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
mailbox.unlink(missing_ok=True)
(build/'vice-monitor.log').unlink(missing_ok=True)
commands=build/'vice.mon'
commands.write_text(f'fill 0100 01f1 cd\nfill c100 cfff cd\nload "{prg}" 0\nwatch store c008\nwatch store c000\n'
                    f'ignore 2 1\ng 080d\nstopwatch reset\ng\nstopwatch\n'
                    f'delete 1\ng\nm c000 c008\nbsave "{mailbox}" 0 c000 c008\n'
                    f'bsave "{build}/vice-hardware-stack.bin" 0 0100 01f1\n'
                    f'bsave "{build}/vice-software-stack.bin" 0 c100 cfff\nscreen\nquit\n')
cmd=[a.vice,'-default','-console','-pal','-warp','+sound','-kernal',str(rom/'kernal-901227-03.bin'),
     '-basic',str(rom/'basic-901226-01.bin'),'-chargen',str(rom/'chargen-901225-01.bin')]
if a.data_dir: cmd+=['-directory',str(a.data_dir.resolve())+':'+str(rom.parent)]
else: cmd+=['-directory',str(rom.parent)]
cmd+=['-initbreak','ready','-moncommands',str(commands),'-monlogname',
      str(build/'vice-monitor.log'),'-monlog','-limitcycles','200000000']
with (build/'vice.log').open('w') as log:
    subprocess.run(cmd,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=60)
if not mailbox.exists(): sys.exit('FAIL: VICE did not produce a mailbox')
data=mailbox.read_bytes()
if len(data)!=9 or data[:3]!=bytes([0xa5,0,26]):
    sys.exit('FAIL: VICE mailbox '+data.hex())
log=(build/'vice-monitor.log').read_text()
print('PASS: VICE PAL native execution, 26 scenarios and 26 assembly-derived state fingerprints')
print('Mailbox:',data.hex())
print('Target executive size:',int.from_bytes(data[3:5],'little'))
print('Target scenario size:',int.from_bytes(data[5:7],'little'))
print('VICE command:',' '.join(cmd))
for line in log.splitlines():
    if 'Stopwatch' in line or 'cycles' in line.lower(): print(line)
for filename,top in [('vice-hardware-stack.bin',0x1f2),('vice-software-stack.bin',0xd000)]:
    values=(build/filename).read_bytes()
    base=0x100 if filename=='vice-hardware-stack.bin' else 0xc100
    modified=[i for i,b in enumerate(values) if b!=0xcd]
    print(filename+': measured sentinel extent '+str(top-(base+min(modified)) if modified else 0)+' bytes')
