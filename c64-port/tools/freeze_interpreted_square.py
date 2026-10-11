#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Extract lossless canonical COMP/SIGN source-only target states."""
import pathlib,struct,hashlib,subprocess,gzip,json
from frozen_words import encode_rows,c_header
r=pathlib.Path(__file__).resolve().parents[1]
local=list(range(16))+[31,32,33,63,64,65,127,128,129,255,256,257,511,512,513,1023,1024,1025,2047,2048,2049,4094,4095,4096,4097,8190,8191,8192,8193,16382,16383,16384,16385,24575,24576,32765,32766,32767,32768,32769,49150,49151,49152,49153,65532,65533,65534,65535]
assert len(local)==64
indices=local+[65536+n for n in local]
subprocess.run(['gcc','-O2','-std=c99','-Iinclude','-Itests','tools/interpreted_square_inputs.c','tests/interpreted_square_cases.c','src/agc_word.c','-o','build/interpreted-square-inputs'],cwd=r,check=True)
s='/* SPDX-License-Identifier: MIT */\n#ifndef INTERPRETED_SQUARE_VECTORS_H\n#define INTERPRETED_SQUARE_VECTORS_H\n';stats=[]
for mission,name in enumerate(['lm','cm']):
 data=subprocess.check_output(['build/interpreted-square-inputs',str(mission)],input=b''.join(struct.pack('<I',n)for n in indices),cwd=r)
 initial={n:struct.unpack_from('<2049H',data,k*4098)for k,n in enumerate(indices)};source={};h=hashlib.sha256();count=0
 with gzip.open(r/f'build/interpreted-square-{name}-reference.snapshots.gz','rb')as f:
  while raw:=f.read(4102):
   assert len(raw)==4102;h.update(raw);w=struct.unpack('<2051H',raw);n=w[0]+(w[1]<<16);count+=1
   if n in initial:source[n]=(w[3:],w[2])
 stats.append(dict(mission=name,decoded_sha256=h.hexdigest(),complete_states=count))
 for kind in range(2):
  rows=[]
  for n in indices[kind*64:(kind+1)*64]:
   after,irq=source[n];changes=[(i,v)for i,v in enumerate(after)if v!=initial[n][i+1]]
   rows.append([n&65535,n>>16,irq,len(changes)]+[v for pair in changes for v in pair])
  width=max(map(len,rows));packed=encode_rows([row+[0]*(width-len(row))for row in rows]);selector=mission*2+kind
  s+=f'#if !defined(INTERPRETED_SQUARE_SELECTED) || INTERPRETED_SQUARE_SELECTED=={selector}\n'+c_header(packed,f'interpreted_square{selector}')+'#endif\n';print(name,kind,packed.storage_bytes)
s+='#endif\n';(r/'tests/interpreted_square_vectors.h').write_text(s);(r/'build/interpreted-square-corpus-stats.json').write_text(json.dumps(stats,indent=2)+'\n')
