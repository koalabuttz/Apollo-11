#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Freeze complete original states with bounded streaming memory and XZ input."""
import pathlib,subprocess,lzma,hashlib
R=pathlib.Path(__file__).resolve().parents[1]
cases=['src/agc_word.c','src/cm_makeplay_sleep.c','src/cm_basic_wake.c','src/cm_basic_sleep.c','src/cm_native_image.c','src/cm_dispatch.c','src/cm_executive.c','tests/cm_trig_cases.c','tests/cm_clockjob_cases.c','tests/cm_wake_cases.c','tests/cm_display_sleep_lifecycle_cases.c']
subprocess.run(['cc','-O2','-std=c99','-ffunction-sections','-Wl,--gc-sections','-Iinclude','-Itests']+cases+['tools/cm_display_sleep_lifecycle_inputs.c','-o','build/cm-display-sleep-lifecycle-inputs'],cwd=R,check=True)
proc=subprocess.Popen(['build/cm-display-sleep-lifecycle-inputs'],cwd=R,stdout=subprocess.PIPE)
numbers=[n*4+p for n in list(range(8))+[64,65,126,127,256,257,510,511] for p in range(4)]
patch=[];offsets=[0];results=[];h=hashlib.sha256()
with lzma.open(R/'build/cm-display-sleep-lifecycle-reference.snapshots.xz','rb')as f:
 for n,line in enumerate(f):
  h.update(line);before=proc.stdout.readline()
  if n not in numbers:continue
  row=list(map(int,line.split()));old=list(map(int,before.split()));assert len(row)==len(old)==2053
  for field,(v,b)in enumerate(zip(row[2:],old[2:])):
   if v!=b:patch.append((field,v))
  offsets.append(len(patch));results.append(row[1])
assert n==2047 and len(results)==64 and proc.wait()==0
lines=['/* SPDX-License-Identifier: MIT',' * Full original CM MAKEPLAY states, uncompressed SHA256 '+h.hexdigest()+' */','#ifndef CM_DISPLAY_SLEEP_LIFECYCLE_VECTORS_H','#define CM_DISPLAY_SLEEP_LIFECYCLE_VECTORS_H','#include "cm_display_sleep_lifecycle_cases.h"','static const uint16_t cm_display_sleep_lifecycle_numbers[]={'+','.join(map(str,numbers))+'};','static const uint16_t cm_display_sleep_lifecycle_offsets[]={'+','.join(map(str,offsets))+'};','static const uint16_t cm_display_sleep_lifecycle_patch[][2]={']
lines+=[' {'+str(f)+','+str(v)+'},'for f,v in patch];lines+=['};','static const uint16_t cm_display_sleep_lifecycle_results[]={'+','.join(map(str,results))+'};','#endif','']
(R/'tests/cm_display_sleep_lifecycle_vectors.h').write_text('\n'.join(lines));print('Frozen64 complete2051 states;',len(patch),'word patches')
