#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Source-only frozen complete canonical IMUPULSE stage states."""
import pathlib,subprocess,struct,lzma,hashlib,json
from frozen_words import encode_rows,c_header
r=pathlib.Path(__file__).resolve().parents[1]
indices=[0,1,2,3,15,16,31,32,47,48,63,64,95,96,127,128,255,256,511,512,1023,1024,2047,2048,4095,8191,16383,32767,32768,49151,65534,65535]
subprocess.run(['gcc','-O2','-std=c99','-Iinclude','-Itests','tools/imu_coarse_inputs.c','tests/imu_coarse_cases.c','tests/imu_pulse_cases.c','src/agc_word.c','-o','build/imu-coarse-inputs'],cwd=r,check=True)
header='/* SPDX-License-Identifier: MIT\n * Frozen solely from original assembled software. */\n#ifndef IMU_COARSE_VECTORS_H\n#define IMU_COARSE_VECTORS_H\n';stats={}
for mission in range(2):
 for entry in range(4):
  name=('lm','cm')[mission]+str(entry);raw=subprocess.check_output(['build/imu-coarse-inputs',str(mission),str(entry)],input=b''.join(struct.pack('<I',n)for n in indices),cwd=r)
  initial={n:struct.unpack_from('<2053H',raw,k*4106)for k,n in enumerate(indices)};source={};h=hashlib.sha256();count=0;branches={}
  with lzma.open(r/f'build/imu-coarse-{mission}-{entry}.snapshots.xz','rb')as f:
   while row:=f.read(4112):
    assert len(row)==4112;h.update(row);w=struct.unpack('<2056H',row);n=w[0]+(w[1]<<16);count+=1;branches[str(w[7])]=branches.get(str(w[7]),0)+1
    if n in initial:source[n]=w[2:]
  assert count==65536
  rows=[]
  for n in indices:
   irq,sup,ch11,ch12,ch14,status=source[n][:6];after=source[n][6:];changes=[(i,v)for i,v in enumerate(after)if v!=initial[n][i+5]]
   rows.append([n&65535,n>>16,irq,sup,ch11,ch12,ch14,status,len(changes)]+[v for pair in changes for v in pair])
  width=max(map(len,rows));packed=encode_rows([row+[0]*(width-len(row))for row in rows]);header+=f'#if IMUC_MISSION == {mission} && IMUC_ENTRY == {entry}\n'+c_header(packed,'imu_coarse')+'#endif\n'
  stats[name]=dict(mission=mission,entry=entry,complete_states=count,decoded_sha256=h.hexdigest(),frozen_states=len(indices),frozen_storage_bytes=packed.storage_bytes,source_branches=branches)
header+='#endif\n';(r/'tests/imu_coarse_vectors.h').write_text(header);(r/'build/imu-coarse-corpus-stats.json').write_text(json.dumps(stats,indent=2)+'\n');print(sum(x['complete_states']for x in stats.values()),'complete states;',sum(x['frozen_states']for x in stats.values()),'target states')
