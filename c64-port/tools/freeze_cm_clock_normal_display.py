#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,subprocess,lzma
R=pathlib.Path(__file__).resolve().parents[1]
core=['src/agc_word.c','src/cm_executive.c','src/cm_waitlist.c','src/cm_native_image.c','tests/cm_service_loop_cases.c','tests/cm_mission_loop_cases.c','tests/cm_clock_mission_cases.c','tests/cm_clock_display_mission_cases.c']
subprocess.run(['cc','-O2','-ffunction-sections','-Wl,--gc-sections','-Iinclude','-Itests']+core+['tools/cm_clock_normal_display_inputs.c','-o','build/cm-clock-normal-display-inputs'],cwd=R,check=True)
inputs={int(row.split()[0]):list(map(int,row.split()))[1:]for row in subprocess.check_output(['build/cm-clock-normal-display-inputs'],cwd=R,text=True).splitlines()}
bo=[0];ao=[0];bp=[];ap=[];numbers=[];prior=None
with lzma.open(R/'build/cm-clock-normal-display-reference.snapshots.xz','rt')as f:
 for line in f:
  row=list(map(int,line.split()));after=row[4:]
  if row[2]==329:
   n=row[0];before=prior;assert before[5]==0o5217
   bp.extend((i,v)for i,(v,b)in enumerate(zip(before,inputs[n]))if v!=b)
   ap.extend((i,v)for i,(v,b)in enumerate(zip(after,before))if v!=b)
   bo.append(len(bp));ao.append(len(ap));numbers.append(n)
  prior=after
assert len(numbers)==9
lines=['/* SPDX-License-Identifier: MIT */','#ifndef CM_CLOCK_NORMAL_DISPLAY_VECTORS_H','#define CM_CLOCK_NORMAL_DISPLAY_VECTORS_H','static const uint16_t cm_clock_normal_numbers[]={'+','.join(map(str,numbers))+'};']
for name,values in [('before_offsets',bo),('after_offsets',ao)]:lines+=['static const uint16_t cm_clock_normal_'+name+'[]={'+','.join(map(str,values))+'};']
for name,values in [('before_patch',bp),('after_patch',ap)]:lines+=['static const uint16_t cm_clock_normal_'+name+'[][2]={']+[' {'+str(i)+','+str(v)+'},'for i,v in values]+['};']
lines+=['#endif',''];(R/'tests/cm_clock_normal_display_vectors.h').write_text('\n'.join(lines));print('Frozen9 genuine mission END source transitions;',len(bp),'input patches;',len(ap),'output patches')
