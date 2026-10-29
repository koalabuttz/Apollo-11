#!/usr/bin/env python3
"""Run mixed native contexts with real IRQ admission and masked restoration."""
import argparse,json,os,pathlib,re,subprocess,sys
ROOT=pathlib.Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('--vice',default='x64sc')
p.add_argument('--rom-dir',required=True,type=pathlib.Path)
p.add_argument('--data-dir',type=pathlib.Path)
a=p.parse_args(); build=ROOT/'build'; rom=a.rom_dir.resolve()
prg=build/'interpreter-native.prg'; mapping=(build/'interpreter-native.map').read_text()
entry=re.search(rb'\x9e\s*(\d+)\x00',prg.read_bytes()[:32])
if entry is None: sys.exit('missing BASIC SYS entry')
for section in ('zp','zp.data','zp.bss','zp.noinit'):
 m=re.search(r'^\s*[0-9a-f]+\s+[0-9a-f]+\s+([0-9a-f]+)\s+1\s+\.'+re.escape(section)+r'$',mapping,re.M)
 if m and int(m[1],16)!=0: sys.exit('unpreserved extra ZP')
s=re.search(r'^\s*([0-9a-f]+)\s+[0-9a-f]+\s+100\s+1\s+c64_irq_stack$',mapping,re.M)
if not s: sys.exit('missing isolated IRQ stack')
isolated=int(s[1],16)
for name in ('text','rodata','bss','noinit'):
 m=re.search(r'^\s*([0-9a-f]+)\s+[0-9a-f]+\s+([0-9a-f]+)\s+1\s+\.'+name+r'$',mapping,re.M)
 if m and int(m[1],16)+int(m[2],16)>0xc000: sys.exit('image overlaps reserved C64 diagnostic RAM')
mail=build/'interpreter-native-mailbox.bin'; mail.unlink(missing_ok=True)
mon=build/'interpreter-native.mon'; log=build/'interpreter-native-monitor.log'
mon.write_text(f'fill 0100 01f1 cd\nfill c100 cfff cd\nload "{prg}" 0\n'
 'watch store c046\nwatch store c040\nignore 2 1\nstopwatch reset\n'
 f'g {int(entry[1]):04x}\nm c046 c046\nstopwatch\ng\nm c046 c046\nstopwatch\ndelete 1\ng\nm c040 c049\n'
 f'bsave "{mail}" 0 c040 c049\n'
 f'bsave "{build}/interpreter-native-hardware-stack.bin" 0 0100 01f1\n'
 f'bsave "{build}/interpreter-native-software-stack.bin" 0 c100 cfff\n'
 f'bsave "{build}/interpreter-native-isolated-stack.bin" 0 {isolated:04x} {isolated+255:04x}\nscreen\nquit\n')
command=[a.vice,'-default','-console','-pal','-warp','+sound',
 '-kernal',str(rom/'kernal-901227-03.bin'),'-basic',str(rom/'basic-901226-01.bin'),
 '-chargen',str(rom/'chargen-901225-01.bin'),
 '-directory',(str(a.data_dir.resolve())+':' if a.data_dir else '')+str(rom.parent),
 '-initbreak','ready','-moncommands',str(mon),'-monlogname',str(log),'-monlog','-limitcycles','400000000']
env=os.environ.copy()
for key,directory in [('XDG_CACHE_HOME','vice-cache'),('XDG_CONFIG_HOME','vice-config')]:
 env.setdefault(key,str(build/directory)); pathlib.Path(env[key]).mkdir(parents=True,exist_ok=True)
with (build/'interpreter-native-vice.log').open('w') as f:
 subprocess.run(command,stdout=f,stderr=subprocess.STDOUT,check=True,timeout=60,env=env)
if not mail.exists(): sys.exit('missing mixed completion mailbox')
if mail.read_bytes()[:7]!=bytes([0xa5,0,0,140,16,1,0x91]): sys.exit('failure mailbox '+mail.read_bytes().hex())
stacks={}
for kind in ('hardware','software','isolated'):
 data=(build/f'interpreter-native-{kind}-stack.bin').read_bytes()
 used=[i for i,v in enumerate(data) if v!=0xcd]
 if used and min(used)==0: sys.exit('stack touched bottom sentinel: '+kind)
 stacks[kind]=len(data)-min(used) if used else 0
result={'mailbox':mail.read_bytes().hex(),'fingerprints':140,'masked_repeat':16,
 'real_irq_posts':1,'stack_sentinel_extents':stacks,
 'scope':'Native continuations; frame payload restoration; producer publication followed by guarded foreground admission; no guidance deadline claim'}
(build/'interpreter-native-target.json').write_text(json.dumps(result,indent=2)+'\n')
print('PASS: PAL C64 140 original-derived mixed contexts + 16 masked checkpoints + real IRQ admission')
print('Mailbox:',result['mailbox'],'stack sentinel extents:',stacks)
