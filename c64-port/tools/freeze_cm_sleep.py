#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Freeze complete original states with bounded streaming memory and XZ input."""
import pathlib,subprocess,lzma,hashlib
R=pathlib.Path(__file__).resolve().parents[1]
cases=['src/agc_word.c','tests/cm_basic_finish_cases.c','tests/cm_sleep_cases.c']
subprocess.run(['cc','-O2','-std=c99','-ffunction-sections','-Wl,--gc-sections','-Iinclude','-Itests']+cases+['tools/cm_sleep_inputs.c','-o','build/cm-sleep-inputs'],cwd=R,check=True)
proc=subprocess.Popen(['build/cm-sleep-inputs'],cwd=R,stdout=subprocess.PIPE)
numbers=list(range(16))+[256+n*331 for n in range(48)]
patch=[];offsets=[0];results=[];h=hashlib.sha256()
with lzma.open(R/'build/cm-sleep-reference.snapshots.xz','rb')as f:
 for n,line in enumerate(f):
  h.update(line);before=proc.stdout.readline()
  if n not in numbers:continue
  row=list(map(int,line.split()));old=list(map(int,before.split()));assert len(row)==len(old)==2053
  for field,(v,b)in enumerate(zip(row[2:],old[2:])):
   if v!=b:patch.append((field,v))
  offsets.append(len(patch));results.append(row[1])
assert n==16383 and len(results)==64 and proc.wait()==0
lines=['/* SPDX-License-Identifier: MIT',' * Full original CM MAKEPLAY states, uncompressed SHA256 '+h.hexdigest()+' */','#ifndef CM_SLEEP_VECTORS_H','#define CM_SLEEP_VECTORS_H','#include "cm_sleep_cases.h"','static const uint16_t cm_sleep_numbers[]={'+','.join(map(str,numbers))+'};','static const uint16_t cm_sleep_offsets[]={'+','.join(map(str,offsets))+'};','static const uint16_t cm_sleep_patch[][2]={']
lines+=[' {'+str(f)+','+str(v)+'},'for f,v in patch];lines+=['};','static const uint16_t cm_sleep_results[]={'+','.join(map(str,results))+'};','#endif','']
(R/'tests/cm_sleep_vectors.h').write_text('\n'.join(lines));print('Frozen64 complete2051 states;',len(patch),'word patches')
