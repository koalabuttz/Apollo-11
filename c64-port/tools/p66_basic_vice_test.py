#!/usr/bin/env python3
"""Run native DSKY DSPOUTSB full-image assembly snapshots on native C64 code."""
import argparse, os, pathlib, re, subprocess, sys
root=pathlib.Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser(); p.add_argument('--vice',default='x64sc'); p.add_argument('--rom-dir',required=True,type=pathlib.Path); p.add_argument('--data-dir',type=pathlib.Path)
a=p.parse_args(); build=root/'build'; rom=a.rom_dir.resolve(); prg=build/'p66_basic.prg'
mapping=(build/'p66_basic.map').read_text()
match=re.search(r'^\s*([0-9a-f]+)\s+[0-9a-f]+\s+5\s+1\s+p66_basic_mailbox$',mapping,re.M)
if not match: sys.exit('FAIL: p66_basic mailbox symbol missing')
address=int(match[1],16)
for section in ('.zp.data','.zp.bss'):
 matches=re.findall(r'^\s*([0-9a-f]+)\s+([0-9a-f]+)\s+([0-9a-f]+)\s+\d+\s+'+re.escape(section)+r'$',mapping,re.M)
 if len(matches)!=1 or int(matches[0][2],16)!=0: sys.exit('FAIL: extra zero page storage')
bss=re.search(r'^\s*([0-9a-f]+)\s+[0-9a-f]+\s+([0-9a-f]+)\s+1\s+\.bss$',mapping,re.M)
if not bss or int(bss[1],16)+int(bss[2],16)>0xc000: sys.exit('FAIL: BSS overlaps software stack reserve')
image=prg.read_bytes(); base=int.from_bytes(image[:2],'little'); end=base+len(image)-2
if base!=0x801 or end>0xc000: sys.exit('FAIL: PRG exceeds native RAM image budget')
sys_match=re.search(rb'\x9e\s*(\d+)\x00',image[:32])
if not sys_match: sys.exit('FAIL: no BASIC SYS header')
mailbox=build/'p66_basic-vice-mailbox.bin'; mailbox.unlink(missing_ok=True)
commands=build/'p66_basic-vice.mon'; commands.write_text(f'fill 0100 01f1 cd\nfill c800 cfff cd\nload "{prg}" 0\nwatch store {address:04x}\nignore 1 1\ng {int(sys_match[1]):04x}\nm {address:04x} {address+4:04x}\nbsave "{mailbox}" 0 {address:04x} {address+4:04x}\nbsave "{build}/p66_basic-vice-hardware-stack.bin" 0 0100 01f1\nbsave "{build}/p66_basic-vice-software-stack.bin" 0 c800 cfff\nbsave "{build}/p66_basic-vice-screen.bin" 0 0400 07e7\nbsave "{build}/p66_basic-vice-color.bin" 0 d800 dbe7\nquit\n')
cmd=[a.vice,'-default','-console','-pal','-warp','+sound','-kernal',str(rom/'kernal-901227-03.bin'),'-basic',str(rom/'basic-901226-01.bin'),'-chargen',str(rom/'chargen-901225-01.bin'),'-directory',(str(a.data_dir.resolve())+':' if a.data_dir else '')+str(rom.parent),'-initbreak','ready','-moncommands',str(commands),'-monlogname',str(build/'p66_basic-vice-monitor.log'),'-monlog','-limitcycles','400000000']
env=os.environ.copy(); env.setdefault('XDG_CACHE_HOME',str(build/'vice-cache')); env.setdefault('XDG_CONFIG_HOME',str(build/'vice-config'))
for key in ('XDG_CACHE_HOME','XDG_CONFIG_HOME'): pathlib.Path(env[key]).mkdir(parents=True,exist_ok=True)
with (build/'p66_basic-vice.log').open('w') as log: subprocess.run(cmd,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=60,env=env)
if not mailbox.exists(): sys.exit('FAIL: missing mailbox')
data=mailbox.read_bytes()
if data!=bytes.fromhex('a500000802'): sys.exit('FAIL: p66_basic mailbox '+data.hex())
print('PASS: VICE PAL, eight complete original2048word display checkpoints plus metadata, real ENDJOB/100cs RODTASK, zero failures')
screen=(build/'p66_basic-vice-screen.bin').read_bytes(); color=(build/'p66_basic-vice-color.bin').read_bytes()
expected={143:54,144:54}
for p,v in expected.items():
 if screen[p]!=v: sys.exit(f'FAIL: actual native screen0400+{p}={screen[p]}, expected{v}')
 if color[p]&15!=5: sys.exit(f'FAIL: actual colorRAM{p}={color[p]&15}, expected green5')
print('PASS: actual C64 screen0400/colorD800 receives mode66 through source DSPOUTSB')
print('Mailbox:',data.hex()); print('PRG file/loaded bytes:',len(image),len(image)-2)
for name,top,base in [('hardware',0x1f2,0x100),('software',0xd000,0xc800)]:
 values=(build/f'p66_basic-vice-{name}-stack.bin').read_bytes(); modified=[i for i,v in enumerate(values) if v!=0xcd]
 extent=top-(base+min(modified)) if modified else 0
 if extent>=len(values): sys.exit('FAIL: sentinel stack region exhausted')
 print(name+' stack sentinel extent:',extent)
