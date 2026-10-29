#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Full original-only angle/conversion source-state delta fixtures."""
import pathlib,subprocess,struct,gzip,hashlib,json
from frozen_words import encode_rows,c_header
r=pathlib.Path(__file__).resolve().parents[1];indices=[0,1,2,3,4,5,6,7,15,16,17,31,32,33,255,256,257,511,512,513,1023,1024,2047,2048,4095,4096,8191,16383,32767,32768,65534,65535]
subprocess.run(['gcc','-O2','-std=c99','-Iinclude','-Itests','tools/rtb_vector_inputs.c','tests/rtb_vector_cases.c','src/agc_word.c','-o','build/rtb-vector-inputs'],cwd=r,check=True)
header='/* SPDX-License-Identifier: MIT\n * Original-only complete canonical source states. */\n#ifndef RTB_VECTOR_VECTORS_H\n#define RTB_VECTOR_VECTORS_H\n';stats={}
for mission in range(2):
 for entry in range(5):
  name=('lm','cm')[mission]+str(entry)
  raw=subprocess.check_output(['build/rtb-vector-inputs',str(mission),str(entry)],input=b''.join(struct.pack('<I',n)for n in indices),cwd=r)
  initial={n:struct.unpack_from('<2050H',raw,k*4100)for k,n in enumerate(indices)};source={};count=0;h=hashlib.sha256()
  with gzip.open(r/f'build/rtb-vector-{("lm","cm")[mission]}-{entry}-reference.snapshots.gz','rb')as f:
   while row:=f.read(4104):
    assert len(row)==4104;h.update(row);w=struct.unpack('<2052H',row);n=w[0]+(w[1]<<16);count+=1
    if n in initial:source[n]=w[2:]
  assert count==65536
  rows=[]
  for n in indices:
   irq,sup=source[n][:2];after=source[n][2:];changes=[(i,v)for i,v in enumerate(after)if v!=initial[n][i+2]]
   rows.append([n&65535,n>>16,irq,sup,len(changes)]+[v for pair in changes for v in pair])
  width=max(map(len,rows));packed=encode_rows([row+[0]*(width-len(row))for row in rows])
  header+=f'#if VECTOR_MISSION == {mission} && VECTOR_ENTRY == {entry}\n'+c_header(packed,'rtb_vector')+'#endif\n'
  stats[name]=dict(mission=mission,entry=entry,complete_states=count,decoded_sha256=h.hexdigest(),frozen_states=len(indices),frozen_storage_bytes=packed.storage_bytes)
header+='#endif\n';(r/'tests/rtb_vector_vectors.h').write_text(header);(r/'build/rtb-vector-corpus-stats.json').write_text(json.dumps(stats,indent=2)+'\n');print(sum(x['complete_states']for x in stats.values()),'complete states;',sum(x['frozen_states']for x in stats.values()),'target states')
