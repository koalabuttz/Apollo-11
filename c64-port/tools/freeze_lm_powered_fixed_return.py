#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Lossless complete-source-image fixtures; no native output is used."""
import pathlib,struct,hashlib,subprocess,gzip,lzma
from frozen_words import encode_rows,c_header
r=pathlib.Path(__file__).resolve().parents[1]
indices=[(bank*1024+bank*31)&32767 for bank in range(32)]
subprocess.run(['gcc','-O2','-std=c99','-Iinclude','-Itests','tools/lm_powered_fixed_return_inputs.c','tests/lm_powered_fixed_return_cases.c','tests/lm_powered_interface_cases.c','tests/lm_powered_axis_cases.c','tests/lm_powered_trig_cases.c','src/agc_word.c','-o','build/lm-powered-fixed-return-inputs'],cwd=r,check=True)
heads=b''.join(struct.pack('<H',n)for n in indices)
initial={}
for origin in range(4):
 data=subprocess.check_output(['build/lm-powered-fixed-return-inputs',str(origin)],input=heads,cwd=r);k=0
 for n in indices:initial[origin,n]=struct.unpack_from('<2049H',data,k);k+=4098
source={};digest=hashlib.sha256();path=r/'build/lm-powered-fixed-return-reference.snapshots.gz'
if not path.exists():path=r/'evidence/lm-powered-fixed-return-complete.snapshots.xz'
with (lzma.open if path.suffix=='.xz' else gzip.open)(path,'rb')as f:
 while raw:=f.read(4102):
  assert len(raw)==4102;digest.update(raw);x=struct.unpack('<2051H',raw);n,phase,irq=x[:3]
  if n in indices:source[phase,n]=(x[3:],irq)
s='/* SPDX-License-Identifier: MIT\n * Original complete-source corpus SHA256:'+digest.hexdigest()+'\n * Constructor SHA256:'+hashlib.sha256((r/'tests/lm_powered_fixed_return_cases.c').read_bytes()).hexdigest()+' */\n#ifndef LM_POWERED_FIXED_RETURN_VECTORS_H\n#define LM_POWERED_FIXED_RETURN_VECTORS_H\n'
for phase in range(4):
 rows=[]
 for n in indices:
  after,irq=source[phase,n];changes=[(i,v)for i,v in enumerate(after)if v!=initial[phase,n][i+1]]
  rows.append([n,irq,len(changes)]+[v for pair in changes for v in pair])
 width=max(map(len,rows));packed=encode_rows([row+[0]*(width-len(row))for row in rows])
 s+=f'#if !defined(LM_POWERED_FIXED_RETURN_SELECTED) || LM_POWERED_FIXED_RETURN_SELECTED=={phase}\n'+c_header(packed,f'lm_powered_fixed_return{phase}')+'#endif\n';print(phase,packed.storage_bytes)
s+='#endif\n';(r/'tests/lm_powered_fixed_return_vectors.h').write_text(s)
