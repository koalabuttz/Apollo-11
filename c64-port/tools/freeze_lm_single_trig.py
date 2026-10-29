#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Lossless full source image fixtures from source-only independent outputs."""
import pathlib,struct,hashlib,subprocess,gzip
from frozen_words import encode_rows,c_header
r=pathlib.Path(__file__).resolve().parents[1]
indices=list(range(10))+[8191,8192,8193,16383,16384,16385,24575,24576,24577,32767,32768,32769,40959,40960,40961,49151,49152,49153,57343,57344,57345,65535]
subprocess.run(['gcc','-O2','-std=c99','-Iinclude','-Itests','tools/lm_single_trig_inputs.c','tests/lm_single_trig_cases.c','src/agc_word.c','-o','build/lm-single-trig-inputs'],cwd=r,check=True)
heads=b''.join(struct.pack('<2H',n,op%2)for op in range(4)for n in indices);data=subprocess.check_output(['build/lm-single-trig-inputs'],input=heads,cwd=r);initial={};k=0
for op in range(4):
 for n in indices:initial[op,n]=struct.unpack_from('<2049H',data,k);k+=4098
source={};digest=hashlib.sha256()
source_path=r/'build/lm-single-trig-reference.snapshots'
with (source_path.open('rb') if source_path.exists() else gzip.open(r/'evidence/lm-single-trig-complete.snapshots.gz','rb')) as f:
 while raw:=f.read(4104):
  digest.update(raw);x=struct.unpack('<2052H',raw);n,selector,phase,irq=x[:4];op=selector+2*phase
  if n in indices:source[op,n]=(x[4:],irq)
s='/* SPDX-License-Identifier: MIT\n * Complete2048-word source-image delta fixtures. Original corpusSHA256:'+digest.hexdigest()+'\n * ConstructorSHA256:'+hashlib.sha256((r/'tests/lm_single_trig_cases.c').read_bytes()).hexdigest()+' */\n#ifndef LM_SINGLE_TRIG_VECTORS_H\n#define LM_SINGLE_TRIG_VECTORS_H\n'
for op in range(4):
 rows=[]
 for n in indices:
  after,irq=source[op,n];changes=[(i,v)for i,v in enumerate(after)if v!=initial[op,n][i+1]];rows.append([n,irq,len(changes)]+[v for pair in changes for v in pair])
 width=max(map(len,rows));packed=encode_rows([row+[0]*(width-len(row))for row in rows]);s+=f'#if !defined(LM_SINGLE_TRIG_SELECTED) || LM_SINGLE_TRIG_SELECTED=={op}\n'+c_header(packed,f'lm_single_trig{op}')+'#endif\n';print(op,packed.storage_bytes)
s+='#endif\n';(r/'tests/lm_single_trig_vectors.h').write_text(s)
