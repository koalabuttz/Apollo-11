#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Native Comanche four-entry ephemerides time caller on stock-ROM PAL C64."""
import argparse,os,re,subprocess,json
from pathlib import Path
R=Path(__file__).resolve().parents[1]
CORE=['tests/cm_ephemeris_solar_phase_target.c', 'tests/frozen_words.c', 'src/guidance_product.c', 'src/sband_trig.c', 'src/guidance_trig.c', 'src/sband_vector.c', 'src/rtb_unit.c', 'src/rtb_vector.c', 'src/rtb_angles.c', 'src/guidance_unit_phased.c', 'src/guidance_length_phased.c', 'src/guidance_norm.c', 'src/cm_ephemeris_solar.c', 'src/cm_ephemeris_time.c', 'src/ephemeris_time_scale.c', 'src/ephemeris_shift.c', 'src/ephemeris_add.c', 'src/interpreted_sign.c', 'src/sband_scalar.c', 'src/sband_vector_linear.c', 'src/sband_shift.c', 'src/guidance_shift.c', 'src/agc_word.c', 'src/agc_multiword.c', 'tests/cm_ephemeris_time_cases.c', 'platform/c64/cm_ephemeris_solar_phase_diagnostic.c']
def main():
 p=argparse.ArgumentParser();p.add_argument('--stage',type=int,required=True);p.add_argument('--cohort',type=int,required=True);p.add_argument('--cc',default='mos-c64-clang');p.add_argument('--vice',default='x64sc');p.add_argument('--rom-dir',type=Path,required=True);p.add_argument('--data-dir',type=Path);a=p.parse_args()
 name=f'cm-ephemeris-solar-{a.stage}-{a.cohort}';prg=R/f'build/{name}.prg';mapfile=R/f'build/{name}.map'
 subprocess.run([a.cc,'-Os','-fno-static-stack','-mlto-zp=0','-ffunction-sections','-Wl,--gc-sections',f'-DCM_EPHEMERIS_SOLAR_STAGE={a.stage}',f'-DCM_EPHEMERIS_SOLAR_COHORT={a.cohort}','-std=c99','-Wall','-Wextra','-Werror','-Iinclude','-Itests',*CORE,f'-Wl,-Map={mapfile}','-o',str(prg)],cwd=R,check=True)
 image=prg.read_bytes();assert int.from_bytes(image[:2],'little')==0x801;end=0x801+len(image)-2;bss=int(re.search(r'^\s*([0-9a-f]+)\s+[0-9a-f]+\s+0\s+1\s+__bss_end =',mapfile.read_text(),re.M)[1],16);assert max(end,bss)<0xc000
 start=re.search(rb'\x9e\s*(\d+)\x00',image[:32]);assert start;box=R/f'build/{name}-mailbox.bin';box.unlink(missing_ok=True);mon=R/f'build/{name}-vice.mon';(R/f'build/{name}-monitor.log').unlink(missing_ok=True);mon.write_text('fill 0100 01f1 cd\nfill c100 cfff cd\n'+f'load "{prg}" 0\nwatch store c000\nignore 1 1\ng {int(start[1]):04x}\nbsave "{box}" 0 c000 c003\nbsave "{R}/build/{name}-hardware-stack.bin" 0 0100 01f1\nbsave "{R}/build/{name}-software-stack.bin" 0 c100 cfff\nquit\n')
 rom=a.rom_dir.resolve();env=os.environ.copy();env.setdefault('XDG_CACHE_HOME',str(R/'build/vice-cache'));env.setdefault('XDG_CONFIG_HOME',str(R/'build/vice-config'))
 command=[a.vice,'-default','-console','-pal','-warp','+sound','-kernal',str(rom/'kernal-901227-03.bin'),'-basic',str(rom/'basic-901226-01.bin'),'-chargen',str(rom/'chargen-901225-01.bin'),'-directory',(str(a.data_dir.resolve())+':' if a.data_dir else '')+str(rom.parent),'-initbreak','ready','-moncommands',str(mon),'-monlogname',str(R/f'build/{name}-monitor.log'),'-monlog','-limitcycles','1500000000']
 with (R/f'build/{name}-vice.log').open('w') as log:subprocess.run(command,stdout=log,stderr=subprocess.STDOUT,timeout=240,env=env,check=True)
 states=next(row['states']for row in json.loads((R/'build/cm-ephemeris-solar-phases-target-results.json').read_text())['stages']if row['stage']==a.stage and row['cohort']==a.cohort);assert box.read_bytes()==bytes((0xa5,states&255,states>>8,0)),box.read_bytes().hex()
 extents={}
 for tag,base,top in [('hardware',0x100,0x1f2),('software',0xc100,0xd000)]:
  data=(R/f'build/{name}-{tag}-stack.bin').read_bytes();assert len(data)==top-base;changed=[i for i,b in enumerate(data)if b!=0xcd];extents[tag]=top-(base+min(changed))if changed else 0;assert not changed or min(changed)>0
 result=dict(measured_stack_bytes=extents,additional_zero_page=0,invalid_image_guards=2,null_guard=2,stage=a.stage,cohort=a.cohort,states=states,prg_bytes=len(image),load_end=end,bss_end=bss,mailbox=box.read_bytes().hex(),words_each=2048,external_fields=['superbank','interrupt_enabled'])
 (R/f'build/{name}-vice-results.json').write_text(json.dumps(result,indent=2)+'\n')
 print('PASS complete original/native states on stock-ROM C64; every2048word plusSUPER/IRQ exact;',result,flush=True)
if __name__=='__main__':main()
