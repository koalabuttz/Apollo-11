#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Lossless complete source-image deltas from the qualified input constructor."""
import pathlib,struct,hashlib,subprocess
from frozen_words import encode_rows,c_header
r=pathlib.Path(__file__).resolve().parents[1]
indices={0:list(range(10))+[8191,8192,8193,16383,16384,16385,24575,24576,24577,32767,32768,32769,40959,40960,40961,49151,49152,49153,57343,57344,57345,65535],1:list(range(10))+[8191,8192,8193,16383,16384,16385,24575,24576,24577,32767,32768,32769,40959,40960,40961,49151,49152,49153,57343,57344,57345,65535],2:list(range(16))+[256+2184*i for i in range(16)],3:list(range(16))+[256+17*i for i in range(16)]}
subprocess.run(['gcc','-O2','-std=c99','-Iinclude','-Itests','tools/cm_trig_inputs.c','tests/cm_trig_cases.c','src/agc_word.c','-o','build/cm-trig-inputs'],cwd=r,check=True)
heads=b''.join(struct.pack('<2H',n,op if op<3 else 2)for op in range(4)for n in indices[op]);inputs=subprocess.check_output(['build/cm-trig-inputs'],input=heads,cwd=r);initial={};k=0
for op in range(4):
 for n in indices[op]:initial[op,n]=struct.unpack_from('<2049H',inputs,k);k+=4098
source={};digest=hashlib.sha256()
with(r/'build/cm-trig-reference.snapshots').open('rb')as f:
 while raw:=f.read(4102):
  digest.update(raw);x=struct.unpack('<2051H',raw);n,op,irq=x[:3]
  if n in indices[op]:source[op,n]=(initial[op,n][1:],x[3:],irq)
clock=(r/'build/cm-clock-trig-reference.snapshots').read_bytes()
for o in range(0,len(clock),8204):
 before=struct.unpack_from('<2051H',clock,o);after=struct.unpack_from('<2051H',clock,o+4102);n=before[0]
 if n in indices[3]:source[3,n]=(before[3:],after[3:],after[2])
s='/* SPDX-License-Identifier: MIT\n * Complete2048word source images reconstructed losslessly from deterministic\n * input seed plus literal address/value patches. CorpusSHA256:'+digest.hexdigest()+'\n * ConstructorSHA256:'+hashlib.sha256((r/'tests/cm_trig_cases.c').read_bytes()).hexdigest()+' */\n#ifndef CM_TRIG_VECTORS_H\n#define CM_TRIG_VECTORS_H\n'
for op in range(4):
 rows=[]
 for n in indices[op]:
  before,after,irq=source[op,n];entry=[(i,v)for i,v in enumerate(before)if v!=initial[op,n][i+1]];exit=[(i,v)for i,v in enumerate(after)if v!=before[i]];row=[n,len(entry)]+[v for p in entry for v in p]+[irq,len(exit)]+[v for p in exit for v in p];rows.append(row)
 width=max(map(len,rows));rows=[row+[0]*(width-len(row))for row in rows];packed=encode_rows(rows);s+=f'#if !defined(CM_TRIG_SELECTED) || CM_TRIG_SELECTED=={op}\n'+c_header(packed,f'cm_trig{op}')+'#endif\n';print(op,'32 complete source entry/exit states',packed.storage_bytes,'packed bytes')
s+='#endif\n';(r/'tests/cm_trig_vectors.h').write_text(s)
