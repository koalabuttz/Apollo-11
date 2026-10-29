#!/usr/bin/env python3
"""Run 64 complete octal formatting full-image assembly snapshots on native C64 code."""
import argparse, os, pathlib, re, subprocess, sys
root=pathlib.Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser(); p.add_argument('--vice',default='x64sc'); p.add_argument('--rom-dir',required=True,type=pathlib.Path); p.add_argument('--data-dir',type=pathlib.Path)
a=p.parse_args(); build=root/'build'; rom=a.rom_dir.resolve(); prg=build/'dsky_octal_display.prg'
mapping=(build/'dsky_octal_display.map').read_text()
match=re.search(r'^\s*([0-9a-f]+)\s+[0-9a-f]+\s+5\s+1\s+dsky_octal_display_mailbox$',mapping,re.M)
if not match: sys.exit('FAIL: dsky_octal_display mailbox symbol missing')
address=int(match[1],16)
(build/'dsky_octal_display-vice-monitor.log').unlink(missing_ok=True)
for section in ('.zp.data','.zp.bss'):
 matches=re.findall(r'^\s*([0-9a-f]+)\s+([0-9a-f]+)\s+([0-9a-f]+)\s+\d+\s+'+re.escape(section)+r'$',mapping,re.M)
 if len(matches)!=1 or int(matches[0][2],16)!=0: sys.exit('FAIL: extra zero page storage')
for sec in ('.text','.rodata','.data','.bss','.noinit'):
 for match in re.finditer(r'^\s*([0-9a-f]+)\s+[0-9a-f]+\s+([0-9a-f]+)\s+\d+\s+'+re.escape(sec)+r'$',mapping,re.M):
  if int(match[1],16)+int(match[2],16)>0xc000:sys.exit('FAIL: '+sec+' exceeds source-image RAM budget')
image=prg.read_bytes(); base=int.from_bytes(image[:2],'little'); end=base+len(image)-2
if base!=0x801 or end>0xc000: sys.exit('FAIL: PRG exceeds native RAM image budget')
sys_match=re.search(rb'\x9e\s*(\d+)\x00',image[:32])
if not sys_match: sys.exit('FAIL: no BASIC SYS header')
mailbox=build/'dsky_octal_display-vice-mailbox.bin'; mailbox.unlink(missing_ok=True)
commands=build/'dsky_octal_display-vice.mon'; commands.write_text(f'fill 0100 01f1 cd\nfill c100 cfff cd\nload "{prg}" 0\nwatch store {address:04x}\nignore 1 1\ng {int(sys_match[1]):04x}\nm {address:04x} {address+4:04x}\nbsave "{mailbox}" 0 {address:04x} {address+4:04x}\nbsave "{build}/dsky_octal_display-vice-hardware-stack.bin" 0 0100 01f1\nbsave "{build}/dsky_octal_display-vice-software-stack.bin" 0 c100 cfff\nquit\n')
cmd=[a.vice,'-default','-console','-pal','-warp','+sound','-kernal',str(rom/'kernal-901227-03.bin'),'-basic',str(rom/'basic-901226-01.bin'),'-chargen',str(rom/'chargen-901225-01.bin'),'-directory',(str(a.data_dir.resolve())+':' if a.data_dir else '')+str(rom.parent),'-initbreak','ready','-moncommands',str(commands),'-monlogname',str(build/'dsky_octal_display-vice-monitor.log'),'-monlog','-limitcycles','800000000']
env=os.environ.copy(); env.setdefault('XDG_CACHE_HOME',str(build/'vice-cache')); env.setdefault('XDG_CONFIG_HOME',str(build/'vice-config'))
for key in ('XDG_CACHE_HOME','XDG_CONFIG_HOME'): pathlib.Path(env[key]).mkdir(parents=True,exist_ok=True)
with (build/'dsky_octal_display-vice.log').open('w') as log: subprocess.run(cmd,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=60,env=env)
if not mailbox.exists(): sys.exit('FAIL: missing mailbox')
data=mailbox.read_bytes()
if data!=bytes.fromhex('a500004002'): sys.exit('FAIL: dsky_octal_display mailbox '+data.hex())
print('PASS: VICE PAL, 64 complete original assembly dsky_octal_display states, zero failures')
print('Mailbox:',data.hex()); print('PRG file/loaded bytes:',len(image),len(image)-2)
for name,top,base in [('hardware',0x1f2,0x100),('software',0xd000,0xc100)]:
 values=(build/f'dsky_octal_display-vice-{name}-stack.bin').read_bytes(); modified=[i for i,v in enumerate(values) if v!=0xcd]
 extent=top-(base+min(modified)) if modified else 0
 if extent>=len(values): sys.exit('FAIL: sentinel stack region exhausted')
 print(name+' stack sentinel extent:',extent)
