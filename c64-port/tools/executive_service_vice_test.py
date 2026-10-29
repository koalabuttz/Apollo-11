#!/usr/bin/env python3
"""Run 768 original-derived SPVAC/CHANG2 fingerprints on a stock PAL C64."""
import argparse
import os
import pathlib
import re
import subprocess
import sys
ROOT=pathlib.Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('--vice',default='x64sc')
p.add_argument('--rom-dir',required=True,type=pathlib.Path)
p.add_argument('--data-dir',type=pathlib.Path)
a=p.parse_args(); rom=a.rom_dir.resolve(); build=ROOT/'build'
prg=build/'executive-services.prg'; mapping=(build/'executive-services.map').read_text()
entry=re.search(rb'\x9e\s*(\d+)\x00',prg.read_bytes()[:32])
if entry is None: sys.exit('missing BASIC SYS entry')
for section in ('zp','zp.data','zp.bss','zp.noinit'):
    match=re.search(r'^\s*[0-9a-f]+\s+[0-9a-f]+\s+([0-9a-f]+)\s+1\s+\.'+re.escape(section)+r'$',mapping,re.M)
    if match is not None and int(match[1],16)!=0: sys.exit('unpreserved extra ZP allocation')
mail=build/'executive-services-mailbox.bin'; mail.unlink(missing_ok=True)
mon=build/'executive-services.mon'
mon.write_text(f'load "{prg}" 0\nwatch store c040\nignore 1 1\ng {int(entry[1]):04x}\nm c040 c044\nbsave "{mail}" 0 c040 c044\nscreen\nquit\n')
command=[a.vice,'-default','-console','-pal','-warp','+sound',
    '-kernal',str(rom/'kernal-901227-03.bin'),'-basic',str(rom/'basic-901226-01.bin'),
    '-chargen',str(rom/'chargen-901225-01.bin'),
    '-directory',(str(a.data_dir.resolve())+':' if a.data_dir else '')+str(rom.parent),
    '-initbreak','ready','-moncommands',str(mon),'-monlogname',str(build/'executive-services-monitor.log'),
    '-monlog','-limitcycles','2500000000']
env=os.environ.copy()
for key,directory in [('XDG_CACHE_HOME','vice-cache'),('XDG_CONFIG_HOME','vice-config')]:
    env.setdefault(key,str(build/directory)); pathlib.Path(env[key]).mkdir(parents=True,exist_ok=True)
with (build/'executive-services-vice.log').open('w') as log:
    subprocess.run(command,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=180,env=env)
if not mail.exists(): sys.exit('missing completion mailbox')
if mail.read_bytes()!=bytes([0xa5,0,0,0,3]): sys.exit('failure mailbox '+mail.read_bytes().hex())
print('PASS: PAL C64 768 original-derived SPVAC/CHANG2 state fingerprints; mailbox '+mail.read_bytes().hex())
