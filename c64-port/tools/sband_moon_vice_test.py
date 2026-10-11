#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Native whole LM/CM antenna sphere/orbital caller controls on stock-ROM PAL C64."""
import argparse,os,re,subprocess,json
from pathlib import Path
R=Path(__file__).resolve().parents[1]
CORE=['tests/sband_moon_target.c','tests/sband_moon_cases.c','tests/frozen_words.c','src/sband_moon.c','src/sband_vector_linear.c','src/sband_shift.c','src/guidance_shift.c','src/agc_multiword.c','src/agc_word.c','platform/c64/sband_moon_diagnostic.c']
def main():
 p=argparse.ArgumentParser();p.add_argument('--cc',default='mos-c64-clang');p.add_argument('--vice',default='x64sc');p.add_argument('--rom-dir',type=Path,required=True);p.add_argument('--data-dir',type=Path);a=p.parse_args()
 name='sband-moon';prg=R/f'build/{name}.prg';mapfile=R/f'build/{name}.map'
 subprocess.run([a.cc,'-Os','-std=c99','-Wall','-Wextra','-Werror','-Iinclude','-Itests',*CORE,f'-Wl,-Map={mapfile}','-o',str(prg)],cwd=R,check=True)
 image=prg.read_bytes();assert int.from_bytes(image[:2],'little')==0x801;end=0x801+len(image)-2;bss=int(re.search(r'^\s*([0-9a-f]+)\s+[0-9a-f]+\s+0\s+1\s+__bss_end =',mapfile.read_text(),re.M)[1],16);assert max(end,bss)<0xc000
 start=re.search(rb'\x9e\s*(\d+)\x00',image[:32]);assert start;box=R/f'build/{name}-mailbox.bin';box.unlink(missing_ok=True);mon=R/f'build/{name}-vice.mon';mon.write_text(f'load "{prg}" 0\nwatch store c000\nignore 1 1\ng {int(start[1]):04x}\nbsave "{box}" 0 c000 c003\nquit\n')
 rom=a.rom_dir.resolve();env=os.environ.copy();env.setdefault('XDG_CACHE_HOME',str(R/'build/vice-cache'));env.setdefault('XDG_CONFIG_HOME',str(R/'build/vice-config'))
 command=[a.vice,'-default','-console','-pal','-warp','+sound','-kernal',str(rom/'kernal-901227-03.bin'),'-basic',str(rom/'basic-901226-01.bin'),'-chargen',str(rom/'chargen-901225-01.bin'),'-directory',(str(a.data_dir.resolve())+':' if a.data_dir else '')+str(rom.parent),'-initbreak','ready','-moncommands',str(mon),'-monlogname',str(R/f'build/{name}-monitor.log'),'-monlog','-limitcycles','1500000000']
 with (R/f'build/{name}-vice.log').open('w') as log:subprocess.run(command,stdout=log,stderr=subprocess.STDOUT,timeout=240,env=env,check=True)
 assert box.read_bytes()==bytes((0xa5,0x40,0,0)),box.read_bytes().hex()
 result=dict(states=64,prg_bytes=len(image),load_end=end,bss_end=bss,mailbox=box.read_bytes().hex(),words_each=2048,external_fields=['superbank','interrupt_enabled'])
 (R/'build/sband-moon-vice-results.json').write_text(json.dumps(result,indent=2)+'\n')
 print('PASS64 complete original/native states on stock-ROM C64; every2048word plusSUPER/IRQ exact;',result,flush=True)
if __name__=='__main__':main()
