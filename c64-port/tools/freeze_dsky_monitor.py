#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,subprocess,lzma
R=pathlib.Path(__file__).resolve().parents[1]
subprocess.run(['cc','-O2','-Iinclude','-Itests','src/agc_word.c','tests/dsky_monitor_cases.c','tools/dsky_monitor_inputs.c','-o','build/dsky-monitor-inputs'],cwd=R,check=True)
selected=[]
for stage in range(11):
 rows=[]
 for m in range(2):
  ids=[m*32768+stage+11*((32767-stage)//11)*j//15//11*11 for j in range(16)]
  if stage==0:ids[0]=m*32768+110;ids[1]=m*32768+132
  if stage==9:ids[0]=m*32768+9;ids[1]=m*32768+20
  rows.extend(ids)
 selected.append(rows)
ids={n for rows in selected for n in rows}
inputs={int(row.split()[0]):list(map(int,row.split()))[1:]+[99]for row in subprocess.check_output(['build/dsky-monitor-inputs']+list(map(str,sorted(ids))),cwd=R,text=True).splitlines()}
outputs={}
with lzma.open(R/'build/dsky-monitor-reference.snapshots.xz','rt')as f:
 for line in f:
  n=int(line.split(' ',1)[0])
  if n in ids:outputs[n]=list(map(int,line.split()))[1:]
assert len(outputs)==len(ids)
def emit(path,rows):
 offsets=[0];patch=[]
 for n in rows:
  patch.extend((i,v)for i,(v,b)in enumerate(zip(outputs[n],inputs[n]))if v!=b);offsets.append(len(patch))
 (R/path).write_text('\n'.join(['/* SPDX-License-Identifier: MIT */',f'#define DSKY_MONITOR_FROZEN_COUNT {len(rows)}u','static const uint16_t dsky_monitor_numbers[]={'+','.join(map(str,rows))+'};','static const uint16_t dsky_monitor_offsets[]={'+','.join(map(str,offsets))+'};','static const uint16_t dsky_monitor_patch[][2]={']+[' {'+str(i)+','+str(v)+'},'for i,v in patch]+['};','']))
 print(path,len(rows),'states',len(patch),'patches')
for stage,rows in enumerate(selected):emit('tests/dsky_monitor_vectors_'+str(stage)+'.h',rows)
emit('tests/dsky_monitor_vectors_all.h',[n for rows in selected for n in rows])
lines=['/* SPDX-License-Identifier: MIT */','#ifndef DSKY_MONITOR_VECTORS_H','#define DSKY_MONITOR_VECTORS_H','#ifdef DSKY_MONITOR_ONLY_STAGE']
for stage in range(11):lines+=[('#if'if stage==0 else'#elif')+' DSKY_MONITOR_ONLY_STAGE=='+str(stage),'#include "dsky_monitor_vectors_'+str(stage)+'.h"']
lines+=['#endif','#else','#include "dsky_monitor_vectors_all.h"','#endif','#endif',''];(R/'tests/dsky_monitor_vectors.h').write_text('\n'.join(lines))
