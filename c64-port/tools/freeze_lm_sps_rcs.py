#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Lossless full-state fixtures selected solely from the original SPS controller."""
import pathlib,struct,hashlib,subprocess,gzip,json
from frozen_words import encode_rows,c_header
r=pathlib.Path(__file__).resolve().parents[1]
local=[0,1,2,3,4,5,6,7,15,16,17,31,32,33,255,256,257,511,512,513,1023,1024,2047,2048,4095,4096,8191,16383,32767,32768,65534,65535]
indices=[base+n for base in (0,65536,131072)for n in local]
subprocess.run(['gcc','-O2','-std=c99','-Iinclude','-Itests','tools/lm_sps_rcs_inputs.c','tests/lm_sps_rcs_cases.c','src/agc_word.c','-o','build/lm-sps-rcs-inputs'],cwd=r,check=True)
data=subprocess.check_output(['build/lm-sps-rcs-inputs'],input=b''.join(struct.pack('<I',n)for n in indices),cwd=r)
initial={n:struct.unpack_from('<2049H',data,k*4098)for k,n in enumerate(indices)}
source={};h=hashlib.sha256();count=0;oldcounts={};jetcounts={}
with gzip.open(r/'build/lm-sps-rcs-reference.snapshots.gz','rb')as f:
 while raw:=f.read(4102):
  assert len(raw)==4102;h.update(raw);w=struct.unpack('<2051H',raw);n=w[0]+(w[1]<<16);irq=w[2];after=w[3:];count+=1
  oldcounts[str(after[0o3345])]=oldcounts.get(str(after[0o3345]),0)+1
  axis=n//65536-1;jet=after[0o3125+axis];kind='plus_zero'if jet==0 else('minus_zero'if jet==0o77777 else('negative'if jet&0o40000 else'positive'));jetcounts[kind]=jetcounts.get(kind,0)+1
  if n in initial:source[n]=(after,irq)
rows=[]
for n in indices:
 after,irq=source[n];changes=[(i,v)for i,v in enumerate(after)if v!=initial[n][i+1]]
 rows.append([n&65535,n>>16,irq,len(changes)]+[v for pair in changes for v in pair])
width=max(map(len,rows));packed=encode_rows([row+[0]*(width-len(row))for row in rows])
s='/* SPDX-License-Identifier: MIT\n * Complete original corpus SHA256:'+h.hexdigest()+' */\n#ifndef LM_SPS_RCS_VECTORS_H\n#define LM_SPS_RCS_VECTORS_H\n'+c_header(packed,'lm_sps_rcs')+'#endif\n'
(r/'tests/lm_sps_rcs_vectors.h').write_text(s)
x={'complete_states':count,'decoded_sha256':h.hexdigest(),'old_sense_counts':oldcounts,'final_jet_counts':jetcounts,'frozen_states':len(indices),'frozen_storage_bytes':packed.storage_bytes}
(r/'build/lm-sps-rcs-corpus-stats.json').write_text(json.dumps(x,indent=2)+'\n');print(x)
