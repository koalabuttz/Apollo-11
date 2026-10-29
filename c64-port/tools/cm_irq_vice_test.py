#!/usr/bin/env python3
"""Run CM IRQ admission or hardware publication/guard evidence on PAL C64."""
import argparse,json,os,pathlib,re,subprocess,sys
R=pathlib.Path(__file__).resolve().parents[1];p=argparse.ArgumentParser(description=__doc__)
p.add_argument('--stage',choices=['admission','irq'],required=True);p.add_argument('--vice',default='x64sc');p.add_argument('--rom-dir',required=True,type=pathlib.Path);p.add_argument('--data-dir',type=pathlib.Path)
a=p.parse_args();b=R/'build';rom=a.rom_dir.resolve();name='cm-irq-admission' if a.stage=='admission' else 'cm-irq';count=512 if a.stage=='admission' else 7
prg=b/(name+'.prg');mapping=(b/(name+'.map')).read_text();payload=prg.read_bytes();load=int.from_bytes(payload[:2],'little');assert load==0x0801 and load+len(payload)-2<0xc000
for section in ('data','bss','noinit','zp','zp.data','zp.bss','zp.noinit'):
 m=re.search(r'^\s*([0-9a-f]+)\s+[0-9a-f]+\s+([0-9a-f]+)\s+1\s+\.'+re.escape(section)+r'$',mapping,re.M)
 if m: assert int(m[2],16)==0 if section.startswith('zp') else int(m[1],16)+int(m[2],16)<=0xc000,section
entry=re.search(rb'\x9e\s*(\d+)\x00',payload[:32]);assert entry
mail=b/(name+'-mailbox.bin');mail.unlink(missing_ok=True);mon=b/(name+'.mon');logpath=b/(name+'-monitor.log')
script=f'fill 0100 01f1 cd\nfill c100 cfff cd\nload "{prg}" 0\n'
if a.stage=='irq':script+='watch store c030\nwatch store c040\nignore 2 1\nstopwatch reset\n'
else:script+='watch store c040\nignore 1 1\n'
script+=f'g {int(entry[1]):04x}\n'
if a.stage=='irq':script+='m c030 c030\nstopwatch\ng\n'*14
script+=f'm c040 c045\nbsave "{mail}" 0 c040 c045\nbsave "{b}/{name}-hardware-stack.bin" 0 0100 01f1\nbsave "{b}/{name}-software-stack.bin" 0 c100 cfff\n'
stack=None
if a.stage=='irq':
 m=re.search(r'^\s*([0-9a-f]+)\s+[0-9a-f]+\s+100\s+1\s+cm_c64_irq_stack$',mapping,re.M);assert m;stack=int(m[1],16)
 script+=f'bsave "{b}/{name}-isolated-stack.bin" 0 {stack:04x} {stack+255:04x}\n'
script+='screen\nquit\n';mon.write_text(script)
cmd=[a.vice,'-default','-console','-pal','-warp','+sound','-kernal',str(rom/'kernal-901227-03.bin'),'-basic',str(rom/'basic-901226-01.bin'),'-chargen',str(rom/'chargen-901225-01.bin'),'-directory',(str(a.data_dir.resolve())+':' if a.data_dir else '')+str(rom.parent),'-initbreak','ready','-moncommands',str(mon),'-monlogname',str(logpath),'-monlog','-limitcycles','6000000000']
env=os.environ.copy()
for k,suffix in [('XDG_CACHE_HOME','cache'),('XDG_CONFIG_HOME','config')]:env[k]=str(b/(name+'-'+suffix));pathlib.Path(env[k]).mkdir(exist_ok=True)
with (b/(name+'-vice.log')).open('w') as f:subprocess.run(cmd,stdout=f,stderr=subprocess.STDOUT,check=True,timeout=900,env=env)
data=mail.read_bytes();assert data[:5]==bytes([0xa5,0,0,count&255,count>>8]),data.hex()
result={'mailbox':data.hex(),'command':cmd,'stack_extents':{}}
if a.stage=='irq':
 assert data[5]==4,data.hex()
 records=[(int(m,16),int(c)) for m,c in re.findall(r'>C:c030\s+([0-9a-f]{2}).*?Stopwatch:\s+(\d+)',logpath.read_text(),re.S|re.I)]
 assert [m for m,c in records]==[0x81,0x82]*7,records
 result['irq_cycles']=[records[i+1][1]-records[i][1] for i in range(0,14,2)]
for part,base,top in [('hardware',0x100,0x1f2),('software',0xc100,0xd000)]+([('isolated',stack,stack+256)] if stack is not None else []):
 values=(b/(name+'-'+part+'-stack.bin')).read_bytes();touched=[i for i,v in enumerate(values) if v!=0xcd];assert not touched or min(touched)>0,part+' exhausted'
 result['stack_extents'][part]=top-base-min(touched) if touched else 0
(b/(name+'-result.json')).write_text(json.dumps(result,indent=2)+'\n')
print('PASS: PAL C64 CM '+a.stage+'; mailbox '+data.hex());print(json.dumps(result['stack_extents']))
if a.stage=='irq':print('Seven real ISR measured cycles:',result['irq_cycles'])
