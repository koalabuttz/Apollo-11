#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Lossless complete-source-image fixtures; no native output is used."""
import pathlib,struct,hashlib,subprocess,gzip,lzma
from frozen_words import encode_rows,c_header
r=pathlib.Path(__file__).resolve().parents[1]
indices=[0,1,2,3,4,5,6,7,15,16,17,31,32,33,127,128,129,255,256,257,1023,1024,1025,2047,2048,2049,4095,4096,4097,8189,8190,8191]
subprocess.run(['gcc','-O2','-std=c99','-Iinclude','-Itests','tools/lm_fleshpot_interface_inputs.c','tests/lm_fleshpot_interface_cases.c','tests/lm_powered_interface_cases.c','tests/lm_powered_axis_cases.c','tests/lm_powered_trig_cases.c','src/agc_word.c','-o','build/lm-fleshpot-interface-inputs'],cwd=r,check=True)
heads=b''.join(struct.pack('<H',n)for n in indices)
data=subprocess.check_output(['build/lm-fleshpot-interface-inputs'],input=heads,cwd=r);initial={};k=0
for n in indices:initial[n]=struct.unpack_from('<2049H',data,k);k+=4098
source={};digest=hashlib.sha256();path=r/'build/lm-fleshpot-interface-reference.snapshots.gz'
if not path.exists():path=r/'evidence/lm-fleshpot-interface-complete.snapshots.xz'
with (lzma.open if path.suffix=='.xz' else gzip.open)(path,'rb')as f:
 while raw:=f.read(4102):
  assert len(raw)==4102;digest.update(raw);x=struct.unpack('<2051H',raw);n,phase,irq=x[:3]
  if n in indices:source[phase,n]=(x[3:],irq)
s='/* SPDX-License-Identifier: MIT\n * Original complete-source corpus SHA256:'+digest.hexdigest()+'\n * Constructor SHA256:'+hashlib.sha256((r/'tests/lm_fleshpot_interface_cases.c').read_bytes()).hexdigest()+' */\n#ifndef LM_FLESHPOT_INTERFACE_VECTORS_H\n#define LM_FLESHPOT_INTERFACE_VECTORS_H\n'
for phase in range(8):
 rows=[]
 for n in indices:
  after,irq=source[phase,n];changes=[(i,v)for i,v in enumerate(after)if v!=initial[n][i+1]]
  rows.append([n,irq,len(changes)]+[v for pair in changes for v in pair])
 width=max(map(len,rows));packed=encode_rows([row+[0]*(width-len(row))for row in rows])
 s+=f'#if !defined(LM_FLESHPOT_INTERFACE_SELECTED) || LM_FLESHPOT_INTERFACE_SELECTED=={phase}\n'+c_header(packed,f'lm_fleshpot_interface{phase}')+'#endif\n';print(phase,packed.storage_bytes)
s+='#endif\n';(r/'tests/lm_fleshpot_interface_vectors.h').write_text(s)
