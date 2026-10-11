#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,subprocess,lzma
R=pathlib.Path(__file__).resolve().parents[1]
subprocess.run(['cc','-O2','-Iinclude','-Itests','tests/dsky_monitor_paste_cases.c','tools/dsky_monitor_paste_inputs.c','-o','build/dsky-monitor-paste-inputs'],cwd=R,check=True)
numbers=[m*8192+stage+3*j*79 for stage in range(3)for m in range(2)for j in range(16)];inputs={int(row.split()[0]):list(map(int,row.split()))[1:]for row in subprocess.check_output(['build/dsky-monitor-paste-inputs']+list(map(str,numbers)),cwd=R,text=True).splitlines()};outputs={}
with lzma.open(R/'build/dsky-monitor-paste-reference.snapshots.xz','rt')as f:
 for line in f:
  n=int(line.split(' ',1)[0])
  if n in inputs:outputs[n]=list(map(int,line.split()))[1:]
assert len(outputs)==96
patch=[];offset=[0]
for n in numbers:patch.extend((i,v)for i,(v,base)in enumerate(zip(outputs[n],inputs[n]))if v!=base);offset.append(len(patch))
(R/'tests/dsky_monitor_paste_vectors.h').write_text('\n'.join(['/* SPDX-License-Identifier: MIT */','#define DSKY_MONITOR_PASTE_FROZEN_COUNT 96u','static const uint16_t dsky_monitor_paste_numbers[]={'+','.join(map(str,numbers))+'};','static const uint16_t dsky_monitor_paste_offsets[]={'+','.join(map(str,offset))+'};','static const uint16_t dsky_monitor_paste_patch[][2]={']+[' {'+str(i)+','+str(v)+'},'for i,v in patch]+['};','']))
print('96 full original states',len(patch),'patches')
