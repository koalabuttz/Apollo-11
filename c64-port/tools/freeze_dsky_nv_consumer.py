#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,subprocess,lzma
R=pathlib.Path(__file__).resolve().parents[1]
subprocess.run(['cc','-O2','-Iinclude','-Itests','src/agc_word.c','tests/dsky_nv_consumer_cases.c','tools/dsky_nv_consumer_inputs.c','-o','build/dsky-nv-consumer-inputs'],cwd=R,check=True)
selected=[group*128+n for group in range(14)for n in [0,127]]
inputs={int(row.split()[0]):list(map(int,row.split()))[1:]for row in subprocess.check_output(['build/dsky-nv-consumer-inputs'],cwd=R,text=True).splitlines()if int(row.split()[0])in selected}
ao=[0];ap=[];so=[0];old=None;count=0
with lzma.open(R/'build/dsky-nv-consumer-reference.snapshots.xz','rt')as f:
 for line in f:
  row=list(map(int,line.split()));n=row[0]
  if n in selected:
   if old is not None and n!=old:so.append(count)
   old=n;ap.extend((i,v)for i,(v,b)in enumerate(zip(row[2:],inputs[n]))if v!=b);ao.append(len(ap));count+=1
so.append(count)
assert count==92 and len(so)==29
lines=['/* SPDX-License-Identifier: MIT */','#ifndef DSKY_NV_CONSUMER_VECTORS_H','#define DSKY_NV_CONSUMER_VECTORS_H','static const uint16_t dsky_nv_consumer_numbers[]={'+','.join(map(str,selected))+'};','static const uint8_t dsky_nv_consumer_sequence_offsets[]={'+','.join(map(str,so))+'};','static const uint16_t dsky_nv_consumer_after_offsets[]={'+','.join(map(str,ao))+'};','static const uint16_t dsky_nv_consumer_after_patch[][2]={']+[' {'+str(i)+','+str(v)+'},'for i,v in ap]+['};','#endif',''];(R/'tests/dsky_nv_consumer_vectors.h').write_text('\n'.join(lines));print('Frozen28 full original continuous NVSUB sequences/92 checkpoints;',len(ap),'source output patches')
