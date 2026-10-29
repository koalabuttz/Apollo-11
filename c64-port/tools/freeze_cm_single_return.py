#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,struct,hashlib,subprocess
from frozen_words import encode_rows,c_header
r=pathlib.Path(__file__).resolve().parents[1];raw=(r/'build/cm-single-return-reference.snapshots').read_bytes();a=[struct.unpack_from('<2052H',raw,o)for o in range(0,len(raw),4104)];indices=[list(range(12))+[219,220,662,663,256,511,512,1023,1024,1535,1536,2047,2048,2559,2560,2815,2816,3000,3069,3071],list(range(3072,3084))+[3098,3099,3541,3542,3328,3583,3584,4095,4096,4607,4608,5119,5120,5631,5632,5887,5888,6072,6141,6143]]
source={}
for before,after in zip(a[::2],a[1::2]):
 if before[0]in indices[before[1]]:source[before[0]]=(before[4:],after[4:],after[3])
subprocess.run(['gcc','-O2','-std=c99','-Iinclude','-Itests','tools/cm_single_return_inputs.c','tests/cm_single_return_cases.c','tests/cm_trig_cases.c','src/agc_word.c','-o','build/cm-single-return-inputs'],cwd=r,check=True)
heads=b''.join(struct.pack('<H',n)for group in indices for n in group);initial=subprocess.check_output(['build/cm-single-return-inputs'],input=heads,cwd=r);s='/* SPDX-License-Identifier: MIT\n * Exact complete source-image delta fixtures; SHA256:'+hashlib.sha256(raw).hexdigest()+' */\n#ifndef CM_SINGLE_RETURN_VECTORS_H\n#define CM_SINGLE_RETURN_VECTORS_H\n';k=0
for selector in range(2):
 rows=[]
 for n in indices[selector]:
  inp=struct.unpack_from('<2049H',initial,k);k+=4098;before,after,irq=source[n];entry=[(i,v)for i,v in enumerate(before)if v!=inp[i+1]];exit=[(i,v)for i,v in enumerate(after)if v!=before[i]];row=[n,len(entry)]+[v for p in entry for v in p]+[irq,len(exit)]+[v for p in exit for v in p];rows.append(row)
 width=max(map(len,rows));rows=[row+[0]*(width-len(row))for row in rows];p=encode_rows(rows);s+=f'#if !defined(CM_SINGLE_RETURN_SELECTED) || CM_SINGLE_RETURN_SELECTED=={selector}\n'+c_header(p,f'cm_single_return{selector}')+'#endif\n';print(selector,p.storage_bytes)
s+='#endif\n';(r/'tests/cm_single_return_vectors.h').write_text(s)
