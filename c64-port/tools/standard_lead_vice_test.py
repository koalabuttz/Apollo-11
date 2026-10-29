#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Execute complete original lead-in controls on stock-ROM PAL C64."""
import argparse,os,re,subprocess
from pathlib import Path
from standard_lead_host_test import CORE,CASES
R=Path(__file__).resolve().parents[1]
def main():
 p=argparse.ArgumentParser();p.add_argument('--cc',default='mos-c64-clang');p.add_argument('--vice',default='x64sc');p.add_argument('--rom-dir',type=Path,required=True);p.add_argument('--data-dir',type=Path);args=p.parse_args()
 for mission,short,count in [(0,'lm',164),(1,'cm',188)]:
  name=f'standard-lead-{short}';prg=R/f'build/{name}.prg';mapfile=R/f'build/{name}.map'
  subprocess.run([args.cc,'-Os','-std=c99','-Wall','-Wextra','-Werror','-Iinclude','-Itests',f'-DSTANDARD_LEAD_TARGET_CM={mission}',*CORE,*CASES,'tests/standard_lead_target.c','tests/standard_lead_target_input.c','tests/frozen_words.c','platform/c64/standard_lead_diagnostic.c',f'-Wl,-Map={mapfile}','-o',str(prg)],cwd=R,check=True)
  image=prg.read_bytes();assert int.from_bytes(image[:2],'little')==0x801;end=0x801+len(image)-2;bss=int(re.search(r'^\s*([0-9a-f]+)\s+[0-9a-f]+\s+0\s+1\s+__bss_end =',mapfile.read_text(),re.M)[1],16);assert max(end,bss)<0xc000
  start=re.search(rb'\x9e\s*(\d+)\x00',image[:32]);assert start;box=R/f'build/{name}-mailbox.bin';box.unlink(missing_ok=True);mon=R/f'build/{name}-vice.mon';mon.write_text(f'load "{prg}" 0\nwatch store c000\nignore 1 1\ng {int(start[1]):04x}\nbsave "{box}" 0 c000 c002\nquit\n')
  rom=args.rom_dir.resolve();env=os.environ.copy();env.setdefault('XDG_CACHE_HOME',str(R/'build/vice-cache'));env.setdefault('XDG_CONFIG_HOME',str(R/'build/vice-config'))
  command=[args.vice,'-default','-console','-pal','-warp','+sound','-kernal',str(rom/'kernal-901227-03.bin'),'-basic',str(rom/'basic-901226-01.bin'),'-chargen',str(rom/'chargen-901225-01.bin'),'-directory',(str(args.data_dir.resolve())+':' if args.data_dir else '')+str(rom.parent),'-initbreak','ready','-moncommands',str(mon),'-monlogname',str(R/f'build/{name}-monitor.log'),'-monlog','-limitcycles','1500000000']
  with (R/f'build/{name}-vice.log').open('w') as log:subprocess.run(command,stdout=log,stderr=subprocess.STDOUT,timeout=180,env=env,check=True)
  assert box.read_bytes()==bytes((0xa5,count&255,count>>8)),box.read_bytes().hex()
  print('PASS',short,count,'complete canonical states on stock-ROM C64; every2048words andSUPER/IRQ exact; PRG',len(image),'load_end',hex(end),'bss_end',hex(bss),flush=True)
if __name__=='__main__':main()
