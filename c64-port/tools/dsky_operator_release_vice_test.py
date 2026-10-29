#!/usr/bin/env python3
"""Run 64 complete operator release full-image assembly snapshots on native C64 code."""
import argparse, os, pathlib, re, subprocess, sys
root=pathlib.Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser(); p.add_argument('--vice',default='x64sc'); p.add_argument('--rom-dir',required=True,type=pathlib.Path); p.add_argument('--data-dir',type=pathlib.Path)
p.add_argument('--mission',required=True,choices=['lm','cm'])
a=p.parse_args(); stem='dsky_operator_release-'+a.mission; build=root/'build'; rom=a.rom_dir.resolve(); prg=build/(stem+'.prg')
mapping=(build/(stem+'.map')).read_text()
match=re.search(r'^\s*([0-9a-f]+)\s+[0-9a-f]+\s+5\s+1\s+dsky_operator_release_mailbox$',mapping,re.M)
if not match: sys.exit('FAIL: dsky_operator_release mailbox symbol missing')
address=int(match[1],16)
difference_match=re.search(r'^\s*([0-9a-f]+)\s+[0-9a-f]+\s+8\s+\d+\s+dsky_operator_release_difference$',mapping,re.M)
if not difference_match:sys.exit('FAIL: difference record missing')
difference_address=int(difference_match[1],16)
(build/(stem+'-vice-monitor.log')).unlink(missing_ok=True)
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
mailbox=build/(stem+'-vice-mailbox.bin'); mailbox.unlink(missing_ok=True)
commands=build/(stem+'-vice.mon'); commands.write_text(f'fill 0100 01f1 cd\nfill c100 cfff cd\nload "{prg}" 0\nwatch store {address:04x}\nignore 1 1\ng {int(sys_match[1]):04x}\nm {address:04x} {address+4:04x}\nbsave "{mailbox}" 0 {address:04x} {address+4:04x}\nbsave "{build}/{stem}-vice-difference.bin" 0 {difference_address:04x} {difference_address+7:04x}\nbsave "{build}/{stem}-vice-hardware-stack.bin" 0 0100 01f1\nbsave "{build}/{stem}-vice-software-stack.bin" 0 c100 cfff\nquit\n')
cmd=[a.vice,'-default','-console','-pal','-warp','+sound','-kernal',str(rom/'kernal-901227-03.bin'),'-basic',str(rom/'basic-901226-01.bin'),'-chargen',str(rom/'chargen-901225-01.bin'),'-directory',(str(a.data_dir.resolve())+':' if a.data_dir else '')+str(rom.parent),'-initbreak','ready','-moncommands',str(commands),'-monlogname',str(build/(stem+'-vice-monitor.log')),'-monlog','-limitcycles','800000000']
env=os.environ.copy(); env.setdefault('XDG_CACHE_HOME',str(build/'vice-cache')); env.setdefault('XDG_CONFIG_HOME',str(build/'vice-config'))
for key in ('XDG_CACHE_HOME','XDG_CONFIG_HOME'): pathlib.Path(env[key]).mkdir(parents=True,exist_ok=True)
with (build/(stem+'-vice.log')).open('w') as log: subprocess.run(cmd,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=60,env=env)
if not mailbox.exists(): sys.exit('FAIL: missing mailbox')
data=mailbox.read_bytes()
if data!=bytes.fromhex('a5000020'+('00'if a.mission=='lm'else '01')): sys.exit('FAIL: dsky_operator_release mailbox '+data.hex()+' first difference '+str([int.from_bytes((build/(stem+'-vice-difference.bin')).read_bytes()[i:i+2],'little')for i in range(0,8,2)]))
print('PASS: VICE PAL, 32 complete original assembly '+a.mission+' dsky_operator_release states, zero failures')
print('Mailbox:',data.hex()); print('PRG file/loaded bytes:',len(image),len(image)-2)
for name,top,base in [('hardware',0x1f2,0x100),('software',0xd000,0xc100)]:
 values=(build/f'{stem}-vice-{name}-stack.bin').read_bytes(); modified=[i for i,v in enumerate(values) if v!=0xcd]
 extent=top-(base+min(modified)) if modified else 0
 if extent>=len(values): sys.exit('FAIL: sentinel stack region exhausted')
 print(name+' stack sentinel extent:',extent)
