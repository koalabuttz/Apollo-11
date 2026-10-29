#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,subprocess,struct,lzma,hashlib,json
from frozen_words import encode_rows,c_header
r=pathlib.Path(__file__).resolve().parents[1]
common=[0,1,2,3,15,16,31,32,47,48,63,64,95,96,127,128,255,256,511,512,1023,1024,2047,2048,4095,8191,16383,32767,32768,49151,65534,65535]
odd=sorted(set(n|1 for n in common))
for n in range(1,64,2):
 if len(odd)==32:break
 if n not in odd:odd.append(n)
odd.sort();assert len(odd)==32
indices=sorted(set(common+odd));subprocess.run(['gcc','-O2','-std=c99','-Iinclude','-Itests','tools/cm_quality_inputs.c','tests/cm_quality_cases.c','src/agc_word.c','-o','build/cm-quality-inputs'],cwd=r,check=True)
raw=subprocess.check_output(['build/cm-quality-inputs'],input=b''.join(struct.pack('<I',n)for n in indices),cwd=r)
initial={}
for k,n in enumerate(indices):
 words=struct.unpack_from('<2050H',raw,k*4100);initial[n]=(words[1],words[0])+words[2:]
states={};h=hashlib.sha256();count=0
with lzma.open(r/'build/cm-quality-original.states.xz','rb')as f:
 while row:=f.read(4108):
  assert len(row)==4108;h.update(row);w=struct.unpack('<2054H',row);n=w[0]+(w[1]<<16);count+=1
  if n in initial:states[n,w[4]]=w
record=json.loads((r/'build/cm-quality-source-stream.json').read_text());assert h.hexdigest()==record['decoded_sha256']and count==record['states']
stats={}
for phase in range(20):
 rows=[];chosen=common if phase==0 else odd
 for n in chosen:
  before=initial[n]if phase==0 else states[n,phase-1][2:4]+states[n,phase-1][6:]
  after=states[n,phase];words=after[6:]
  ip=[(i,v)for i,v in enumerate(before[2:])if v!=initial[n][i+2]]
  op=[(i,v)for i,v in enumerate(words)if v!=before[i+2]]
  rows.append([n&65535,n>>16,before[0],before[1],len(ip)]+[v for pair in ip for v in pair]+[after[2],after[3],after[5],len(op)]+[v for pair in op for v in pair])
 width=max(map(len,rows));packed=encode_rows([row+[0]*(width-len(row))for row in rows])
 header='/* SPDX-License-Identifier: MIT\n * Exact full-state input/output deltas solely from original assembly. */\n'+c_header(packed,'cm_quality')
 (r/f'tests/cm_quality_vectors_{phase}.h').write_text(header);stats[str(phase)]={'states':32,'storage_bytes':packed.storage_bytes,'indices':chosen}
(r/'build/cm-quality-corpus-stats.json').write_text(json.dumps(stats,indent=2)+'\n');print(count,'complete states;640 target source states')
