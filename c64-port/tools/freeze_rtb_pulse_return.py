#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,subprocess,struct,lzma,hashlib,json
from frozen_words import encode_rows,c_header
r=pathlib.Path(__file__).resolve().parents[1]
indices=[0,1,2,3,15,16,31,32,47,48,63,64,95,96,127,128,255,256,511,512,1023,1024,2047,2048,4095,8191,16383,32767,32768,49151,65534,65535]
subprocess.run(['gcc','-O2','-std=c99','-Iinclude','-Itests','tools/rtb_pulse_return_inputs.c','tests/rtb_pulse_return_cases.c','tests/rtb_angles_cases.c','src/agc_word.c','-o','build/rtb-pulse-return-inputs'],cwd=r,check=True)
header='/* SPDX-License-Identifier: MIT\n * Frozen solely from original assembled software. */\n#ifndef RTB_PULSE_RETURN_VECTORS_H\n#define RTB_PULSE_RETURN_VECTORS_H\n';stats={}
for mission in range(2):
 name=('lm','cm')[mission];raw=subprocess.check_output(['build/rtb-pulse-return-inputs',str(mission)],input=b''.join(struct.pack('<I',n)for n in indices),cwd=r)
 initial={n:struct.unpack_from('<2050H',raw,k*4100)for k,n in enumerate(indices)};source={};h=hashlib.sha256();count=0
 with lzma.open(r/f'build/rtb-pulse-return-{name}.states.xz','rb')as f:
  while row:=f.read(4106):
   assert len(row)==4106;h.update(row);w=struct.unpack('<2053H',row);n=w[0]+(w[1]<<16);count+=1
   if n in initial:source[n]=w[2:]
 assert count==65536
 rows=[]
 for n in indices:
  irq,sup,status=source[n][:3];after=source[n][3:];changes=[(i,v)for i,v in enumerate(after)if v!=initial[n][i+2]]
  rows.append([n&65535,n>>16,irq,sup,status,len(changes)]+[v for pair in changes for v in pair])
 width=max(map(len,rows));packed=encode_rows([row+[0]*(width-len(row))for row in rows]);header+=f'#if RTBP_MISSION == {mission}\n'+c_header(packed,'rtb_pulse_return')+'#endif\n'
 stats[name]=dict(mission=mission,complete_states=count,decoded_sha256=h.hexdigest(),frozen_states=len(indices),frozen_storage_bytes=packed.storage_bytes)
header+='#endif\n';(r/'tests/rtb_pulse_return_vectors.h').write_text(header);(r/'build/rtb-pulse-return-corpus-stats.json').write_text(json.dumps(stats,indent=2)+'\n');print(sum(x['complete_states']for x in stats.values()),'complete states;',sum(x['frozen_states']for x in stats.values()),'target states')
