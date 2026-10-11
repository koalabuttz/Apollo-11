#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,subprocess,struct,lzma,hashlib,json
from frozen_words import encode_rows,c_header
r=pathlib.Path(__file__).resolve().parents[1]
indices=[0,1,2,3,15,16,31,32,47,48,63,64,95,96,127,128,255,256,511,512,1023,1024,2047,2048,4095,8191,16383,32767,32768,49151,65534,65535]
subprocess.run(['gcc','-O2','-std=c99','-Iinclude','-Itests','tools/imu_gyro_device_inputs.c','tests/imu_gyro_device_cases.c','tests/imu_pulse_cases.c','src/agc_word.c','-o','build/imu-gyro-device-inputs'],cwd=r,check=True)
header='/* SPDX-License-Identifier: MIT\n * Frozen solely from original peripheral reference. */\n#ifndef IMU_GYRO_DEVICE_VECTORS_H\n#define IMU_GYRO_DEVICE_VECTORS_H\n';stats={}
for mission in range(2):
 for mode in range(2):
  name=('lm','cm')[mission]+str(mode);raw=subprocess.check_output(['build/imu-gyro-device-inputs',str(mission),str(mode)],input=b''.join(struct.pack('<I',n)for n in indices),cwd=r)
  initial={n:struct.unpack_from('<2057H',raw,k*4114)for k,n in enumerate(indices)};source={};h=hashlib.sha256();count=0;events={}
  with lzma.open(r/f'build/imu-gyro-device-{("lm","cm")[mission]}-{mode}.states.xz','rb')as f:
   while row:=f.read(4156):
    assert len(row)==4156;h.update(row);w=struct.unpack('<2078H',row);n=w[0]+(w[1]<<16);count+=1;events[str(w[7])]=events.get(str(w[7]),0)+1
    if n in initial:source[n]=w[2:]
  assert count==65536
  rows=[]
  for n in indices:
   extras=source[n][:28];after=source[n][28:];changes=[(i,v)for i,v in enumerate(after)if v!=initial[n][i+9]]
   rows.append([n&65535,n>>16]+list(extras)+[len(changes)]+[v for pair in changes for v in pair])
  width=max(map(len,rows));packed=encode_rows([row+[0]*(width-len(row))for row in rows]);header+=f'#if IMUD_MISSION == {mission} && IMUD_MODE == {mode}\n'+c_header(packed,'imu_gyro_device')+'#endif\n'
  stats[name]=dict(mission=mission,mode=mode,complete_states=count,decoded_sha256=h.hexdigest(),frozen_states=len(indices),frozen_storage_bytes=packed.storage_bytes,event_counts=events)
header+='#endif\n';(r/'tests/imu_gyro_device_vectors.h').write_text(header);(r/'build/imu-gyro-device-corpus-stats.json').write_text(json.dumps(stats,indent=2)+'\n');print(sum(x['complete_states']for x in stats.values()),'complete peripheral states;',sum(x['frozen_states']for x in stats.values()),'target states')
