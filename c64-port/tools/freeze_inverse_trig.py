#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Reconstruct complete source-only inverse trig target fixtures."""
import pathlib,struct,hashlib,gzip,lzma,subprocess,json
from frozen_words import encode_rows,c_header
r=pathlib.Path(__file__).resolve().parents[1]
subprocess.run(['gcc','-O2','-std=c99','-Iinclude','-Itests','tools/inverse_trig_inputs.c','tests/inverse_trig_cases.c','src/agc_word.c','-o','build/inverse-trig-inputs'],cwd=r,check=True)
s='/* SPDX-License-Identifier: MIT */\n#ifndef INVERSE_TRIG_VECTORS_H\n#define INVERSE_TRIG_VECTORS_H\n';stats=[]
preferred=[0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,20,21,31,32,33,106,107,127,128,129,255,256,257,1023,1024,2047,2048,4095,4096,4097,8191,8192,8193,16383,16384,16385,32767,32768,32769,49151,49152,49153,65533,65534,65535]
for mission,name in enumerate(['lm','cm']):
 source={};h=hashlib.sha256();phase_counts=[0]*4;status_counts={};path=r/f'build/inverse-trig-{name}-reference.snapshots.gz'
 if not path.exists():path=r/f'evidence/inverse-trig-{name}-complete.snapshots.xz'
 with (lzma.open if path.suffix=='.xz' else gzip.open)(path,'rb')as f:
  while raw:=f.read(4108):
   assert len(raw)==4108;h.update(raw);w=struct.unpack('<2054H',raw);n,high,phase,status,irq,_=w[:6];assert high==0;phase_counts[phase]+=1;status_counts[str(status)]=status_counts.get(str(status),0)+1
   if n in preferred or n<1024:source[phase,n]=(w[6:],irq,status)
 stats.append({'mission':name,'decoded_sha256':h.hexdigest(),'complete_states':sum(phase_counts),'phase_counts':phase_counts,'status_counts':status_counts})
 for phase in range(4):
  indices=[n for n in preferred if (phase,n)in source]
  if phase==0:indices=indices[:16]+indices[-16:]
  else:indices=(indices+[n for n in range(1024)if n not in indices and (phase,n)in source])[:32]
  assert len(indices)==32,(name,phase,len(indices))
  data=subprocess.check_output(['build/inverse-trig-inputs',str(mission)],input=b''.join(struct.pack('<H',n)for n in indices),cwd=r);rows=[]
  for k,n in enumerate(indices):
   initial=struct.unpack_from('<2049H',data,k*4098);after,irq,status=source[phase,n];changes=[(i,v)for i,v in enumerate(after)if v!=initial[i+1]]
   rows.append([n,irq,status,len(changes)]+[v for pair in changes for v in pair])
  width=max(map(len,rows));packed=encode_rows([row+[0]*(width-len(row))for row in rows]);selector=mission*4+phase
  s+=f'#if !defined(INVERSE_TRIG_SELECTED) || INVERSE_TRIG_SELECTED=={selector}\n'+c_header(packed,f'inverse_trig{selector}')+'#endif\n';print(name,phase,packed.storage_bytes)
s+='#endif\n';(r/'tests/inverse_trig_vectors.h').write_text(s);(r/'build/inverse-trig-corpus-stats.json').write_text(json.dumps(stats,indent=2)+'\n')
