#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Actual LM/CM antenna full LM yaw source calculation on stock-ROM PAL C64."""
import argparse,os,re,subprocess,json
from pathlib import Path
R=Path(__file__).resolve().parents[1]
CORE=['tests/sband_lm_yaw_target.c','tests/sband_lm_yaw_cases.c','tests/frozen_words.c','src/sband_lm_yaw.c','src/sband_trig.c','src/guidance_trig.c','src/sband_vector_linear.c','src/sband_vector.c','src/sband_shift.c','src/sband_length.c','src/guidance_length_phased.c','src/interpreted_sign.c','src/sband_dot.c','src/sband_scalar.c','src/inverse_trig.c','src/guidance_shift.c','src/guidance_product.c','src/guidance_norm.c','src/agc_word.c','src/agc_multiword.c','platform/c64/sband_lm_yaw_diagnostic.c']
def main():
 p=argparse.ArgumentParser();p.add_argument('--cc',default='mos-c64-clang');p.add_argument('--vice',default='x64sc');p.add_argument('--rom-dir',type=Path,required=True);p.add_argument('--data-dir',type=Path);a=p.parse_args()
 results=[run(a,g)for g in range(4)]
 (R/'build/sband-lm-yaw-vice-results.json').write_text(json.dumps(dict(states=64,groups=results),indent=2)+'\n')
 print('PASS64 complete exact original antenna LM yaw states across four stock-ROM C64 diagnostics',flush=True)
def run(a,group):
 name=f'sband-lm-yaw-{group}';prg=R/f'build/{name}.prg';mapfile=R/f'build/{name}.map'
 subprocess.run([a.cc,'-Oz','-fno-inline-functions',f'-DSBAND_LM_YAW_GROUP={group}','-std=c99','-Wall','-Wextra','-Werror','-Iinclude','-Itests',*CORE,f'-Wl,-Map={mapfile}','-o',str(prg)],cwd=R,check=True)
 image=prg.read_bytes();assert int.from_bytes(image[:2],'little')==0x801;end=0x801+len(image)-2;bss=int(re.search(r'^\s*([0-9a-f]+)\s+[0-9a-f]+\s+0\s+1\s+__bss_end =',mapfile.read_text(),re.M)[1],16);heap_end=int(re.search(r'^\s*([0-9a-f]+)\s+[0-9a-f]+\s+0\s+1\s+__heap_start =',mapfile.read_text(),re.M)[1],16);assert max(end,bss,heap_end)<0xc000
 start=re.search(rb'\x9e\s*(\d+)\x00',image[:32]);assert start;box=R/f'build/{name}-mailbox.bin';box.unlink(missing_ok=True);mon=R/f'build/{name}-vice.mon';mon.write_text(f'load "{prg}" 0\nwatch store c000\nignore 1 1\ng {int(start[1]):04x}\nbsave "{box}" 0 c000 c003\nquit\n')
 rom=a.rom_dir.resolve();env=os.environ.copy();env.setdefault('XDG_CACHE_HOME',str(R/'build/vice-cache'));env.setdefault('XDG_CONFIG_HOME',str(R/'build/vice-config'))
 command=[a.vice,'-default','-console','-pal','-warp','+sound','-kernal',str(rom/'kernal-901227-03.bin'),'-basic',str(rom/'basic-901226-01.bin'),'-chargen',str(rom/'chargen-901225-01.bin'),'-directory',(str(a.data_dir.resolve())+':' if a.data_dir else '')+str(rom.parent),'-initbreak','ready','-moncommands',str(mon),'-monlogname',str(R/f'build/{name}-monitor.log'),'-monlog','-limitcycles','1500000000']
 with (R/f'build/{name}-vice.log').open('w') as log:subprocess.run(command,stdout=log,stderr=subprocess.STDOUT,timeout=240,env=env,check=True)
 assert box.read_bytes()==bytes((0xa5,16,0,0)),box.read_bytes().hex()
 result=dict(group=group,states=16,prg_bytes=len(image),load_end=end,bss_end=bss,static_storage_end=heap_end,mailbox=box.read_bytes().hex(),words_each=2048,external_fields=['superbank','interrupt_enabled'],canonical_image_address=0xd000,expected_image_address=0xe000,diagnostic_cpu_port_mask=0xfc,software_stack_top=0xd000)
 (R/f'build/{name}-vice-results.json').write_text(json.dumps(result,indent=2)+'\n')
 print('PASS group',result,flush=True)
 return result
if __name__=='__main__':main()
