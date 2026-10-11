#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Freeze complete original states with bounded streaming memory and XZ input."""
import pathlib,subprocess,lzma,hashlib
R=pathlib.Path(__file__).resolve().parents[1]
cases=['src/agc_word.c','src/cm_makeplay.c','tests/cm_trig_cases.c','tests/cm_clockjob_cases.c','tests/cm_display_control_cases.c','tests/cm_makeplay_cases.c']
subprocess.run(['cc','-O2','-std=c99','-ffunction-sections','-Wl,--gc-sections','-Iinclude','-Itests']+cases+['tools/cm_makeplay_inputs.c','-o','build/cm-makeplay-inputs'],cwd=R,check=True)
proc=subprocess.Popen(['build/cm-makeplay-inputs'],cwd=R,stdout=subprocess.PIPE)
numbers=list(range(8))+[128+n*503 for n in range(16)]+[8192+n for n in range(8)]+[8320+n*503 for n in range(16)]+[16512+n*503 for n in range(16)]
patch=[];offsets=[0];results=[];h=hashlib.sha256()
with lzma.open(R/'build/cm-makeplay-reference.snapshots.xz','rb')as f:
 for n,line in enumerate(f):
  h.update(line);before=proc.stdout.readline()
  if n not in numbers:continue
  row=list(map(int,line.split()));old=list(map(int,before.split()));assert len(row)==len(old)==2053
  for field,(v,b)in enumerate(zip(row[2:],old[2:])):
   if v!=b:patch.append((field,v))
  offsets.append(len(patch));results.append(row[1])
assert n==24575 and len(results)==64 and proc.wait()==0
lines=['/* SPDX-License-Identifier: MIT',' * Full original CM MAKEPLAY states, uncompressed SHA256 '+h.hexdigest()+' */','#ifndef CM_MAKEPLAY_VECTORS_H','#define CM_MAKEPLAY_VECTORS_H','#include "cm_makeplay_cases.h"','static const uint16_t cm_makeplay_numbers[]={'+','.join(map(str,numbers))+'};','static const uint16_t cm_makeplay_offsets[]={'+','.join(map(str,offsets))+'};','static const uint16_t cm_makeplay_patch[][2]={']
lines+=[' {'+str(f)+','+str(v)+'},'for f,v in patch];lines+=['};','static const uint16_t cm_makeplay_results[]={'+','.join(map(str,results))+'};','#endif','']
(R/'tests/cm_makeplay_vectors.h').write_text('\n'.join(lines));print('Frozen64 complete2051 states;',len(patch),'word patches')
