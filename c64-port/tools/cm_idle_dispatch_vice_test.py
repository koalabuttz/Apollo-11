#!/usr/bin/env python3
"""Verify one bounded CM context diagnostic on stock PAL C64."""
import argparse,os,pathlib,re,subprocess,sys
ROOT=pathlib.Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('--vice',default='x64sc'); p.add_argument('--rom-dir',required=True,type=pathlib.Path);p.add_argument('--data-dir',type=pathlib.Path)
a=p.parse_args(); build=ROOT/'build'; rom=a.rom_dir.resolve(); name='cm-idle-dispatch'
count=128
prg=build/(name+'.prg'); mapping=(build/(name+'.map')).read_text(); payload=prg.read_bytes(); load=int.from_bytes(payload[:2],'little')
if load!=0x0801 or load+len(payload)-2>0xc000: sys.exit('image overlaps reserved RAM')
for section in ('data','bss','noinit','zp','zp.data','zp.bss','zp.noinit'):
 m=re.search(r'^\s*([0-9a-f]+)\s+[0-9a-f]+\s+([0-9a-f]+)\s+1\s+\.'+re.escape(section)+r'$',mapping,re.M)
 if m and ((section.startswith('zp') and int(m[2],16)!=0) or (not section.startswith('zp') and int(m[1],16)+int(m[2],16)>0xc000)): sys.exit('invalid section: '+section)
entry=re.search(rb'\x9e\s*(\d+)\x00',payload[:32]); assert entry
mail=build/(name+'-mailbox.bin'); mail.unlink(missing_ok=True); mon=build/(name+'.mon')
mon.write_text(f'fill 0100 01f1 cd\nfill c100 cfff cd\nload "{prg}" 0\nwatch store c080\nignore 1 1\ng {int(entry[1]):04x}\nm c080 c084\nbsave "{mail}" 0 c080 c084\nbsave "{build}/{name}-hardware-stack.bin" 0 0100 01f1\nbsave "{build}/{name}-software-stack.bin" 0 c100 cfff\nscreen\nquit\n')
cmd=[a.vice,'-default','-console','-pal','-warp','+sound','-kernal',str(rom/'kernal-901227-03.bin'),'-basic',str(rom/'basic-901226-01.bin'),'-chargen',str(rom/'chargen-901225-01.bin'),'-directory',(str(a.data_dir.resolve())+':' if a.data_dir else '')+str(rom.parent),'-initbreak','ready','-moncommands',str(mon),'-monlogname',str(build/(name+'-monitor.log')),'-monlog','-limitcycles','30000000000']
env=os.environ.copy()
for k,tail in [('XDG_CACHE_HOME','cache'),('XDG_CONFIG_HOME','config')]:
 env[k]=str(build/(name+'-'+tail)); pathlib.Path(env[k]).mkdir(exist_ok=True)
with (build/(name+'-vice.log')).open('w') as log: subprocess.run(cmd,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=2400,env=env)
assert mail.read_bytes()==bytes([0xa5,0,0,count&255,count>>8]),mail.read_bytes().hex()
print(f'PASS: PAL C64 CM idle adoption: {count} source fingerprints plus 2 full 2055-value state samples; mailbox '+mail.read_bytes().hex())
for part,base,top in [('hardware',0x100,0x1f2),('software',0xc100,0xd000)]:
 values=(build/(name+'-'+part+'-stack.bin')).read_bytes(); modified=[i for i,v in enumerate(values) if v!=0xcd]
 print(part+' stack measured sentinel extent:',top-base-min(modified) if modified else 0,'bytes')
