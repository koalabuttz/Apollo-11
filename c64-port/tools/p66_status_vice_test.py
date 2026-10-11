#!/usr/bin/env python3
"""Check full-width callback metadata transport on stock C64 ROMs."""
import argparse,os,pathlib,re,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[1];build=root/'build'
p=argparse.ArgumentParser();p.add_argument('--vice',default='x64sc');p.add_argument('--rom-dir',type=pathlib.Path,required=True);p.add_argument('--data-dir',type=pathlib.Path,required=True);a=p.parse_args()
rom=a.rom_dir.resolve();prg=build/'p66-status.prg';mailbox=build/'p66-status-vice-mailbox.bin'
mapping=(build/'p66-status.map').read_text()
for section in ('.text','.rodata','.data','.bss','.noinit'):
    values=re.findall(r'^\s*([0-9a-f]+)\s+[0-9a-f]+\s+([0-9a-f]+)\s+\d+\s+'+re.escape(section)+r'$',mapping,re.M)
    assert len(values)==1 and int(values[0][0],16)+int(values[0][1],16)<=0x7000,section
for section in ('.zp.data','.zp.bss'):
    values=re.findall(r'^\s*[0-9a-f]+\s+[0-9a-f]+\s+([0-9a-f]+)\s+\d+\s+'+re.escape(section)+r'$',mapping,re.M)
    assert len(values)==1 and int(values[0],16)==0,section
entry=int(re.search(rb'\x9e\s*(\d+)\x00',prg.read_bytes()[:32])[1])
mailbox.unlink(missing_ok=True);mon=build/'p66-status-vice.mon'
mon.write_text('fill 0100 01f1 cd\nfill c800 cfff cd\n'+f'load "{prg}" 0\nwatch store c080\nignore 1 1\ng {entry:04x}\nm c080 c084\n'+f'bsave "{mailbox}" 0 c080 c084\nbsave "{build}/p66-status-hardware-stack.bin" 0 0100 01f1\nbsave "{build}/p66-status-software-stack.bin" 0 c800 cfff\nquit\n')
cmd=[a.vice,'-default','-console','-pal','-warp','+sound','-kernal',str(rom/'kernal-901227-03.bin'),'-basic',str(rom/'basic-901226-01.bin'),'-chargen',str(rom/'chargen-901225-01.bin'),'-directory',str(a.data_dir.resolve())+':'+str(rom.parent),'-initbreak','ready','-moncommands',str(mon),'-monlogname',str(build/'p66-status-vice-monitor.log'),'-monlog','-limitcycles','1000000000']
env=os.environ.copy();env.setdefault('XDG_CACHE_HOME',str(build/'vice-cache'));env.setdefault('XDG_CONFIG_HOME',str(build/'vice-config'))
for key in ('XDG_CACHE_HOME','XDG_CONFIG_HOME'): pathlib.Path(env[key]).mkdir(exist_ok=True)
with (build/'p66-status-vice.log').open('w') as log:subprocess.run(cmd,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=60,env=env)
assert mailbox.read_bytes()==bytes.fromhex('a500000a02'),mailbox.read_bytes().hex()
print('PASS: VICE PAL C64, ten raw status transports with full alarm payload retention, I enabled/masked')
print('Mailbox:',mailbox.read_bytes().hex());print('PRG bytes:',prg.stat().st_size)
for filename,base,top in [('p66-status-hardware-stack.bin',0x100,0x1f2),('p66-status-software-stack.bin',0xc800,0xd000)]:
    values=(build/filename).read_bytes();changed=[i for i,v in enumerate(values) if v!=0xcd]
    extent=top-(base+min(changed)) if changed else 0
    assert extent<len(values)
    print(filename+': measured stack extent '+str(extent)+' bytes')
