#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Losslessly freeze full-state source results, with unowned image checks."""
from pathlib import Path
import hashlib,json,struct
from frozen_words import encode_rows,c_header
ROOT=Path(__file__).resolve().parents[1]
FIXED=[0,1,2,0o20,0o21,0o76,0o116,0o117,0o121,0o122,0o123,0o124,0o125,0o126,0o127,0o130,0o131,0o132,0o133,0o134,0o135,0o136,0o137,0o140,0o141,0o154,0o155,0o156,0o157,0o160,0o161,0o162,0o163,0o164,0o166]
def positions(before,n):
 base=before[2+0o120];matrix=0o2230 if n<5120 else 0o3425;br=0o3325 if n<5120 else 0o3310;tm=0o3333 if n<5120 else 0o3316
 return FIXED+list(range(base,base+10))+[base+0o46,base+0o52]+list(range(matrix,matrix+18))+list(range(br,br+6))+list(range(tm,tm+2))+list(range(0o3274,0o3302))
def main():
 source=(ROOT/'build/gimbal-complete-lm.snapshots').read_bytes()+(ROOT/'build/gimbal-complete-cm.snapshots').read_bytes()
 assert source==(ROOT/'build/gimbal-complete-native.snapshots').read_bytes()
 inputs=(ROOT/'build/gimbal-complete-input.snapshots').read_bytes();assert len(source)==len(inputs)==10240*4104
 for n in range(10240):
  row=struct.unpack_from('<2052H',source,n*4104);before=struct.unpack_from('<2052H',inputs,n*4104);owned=set(positions(before,n))
  assert len(owned)==79 and all(row[i+2]==before[i+2] for i in range(2048) if i not in owned),(n,'unowned original word')
  # The target saves COF/AM's eight exceptions and reconstructs the rest of
  # the input-only pattern rather than retaining another4096-byte image.
  local=n%5120;bank=(local//64)%8;base=0o401+(local//1024)*0o54
  cof=0o3270 if n<5120 else 0o3342;am=0o3335 if n<5120 else 0o3364;rateaddr=0o1325 if n<5120 else 0o1130
  exceptions={3:bank<<8,4:0o44000,5:0o6070 if n<5120 else 0o6037,6:0o44000|bank,0o165:0o44000|bank,
   7:0,0o23:0,0o67:0,0o115:0o40000,0o120:base,rateaddr:2*(local%4)}
  exceptions.update({i:before[i+2] for i in list(range(cof,cof+6))+list(range(am,am+2))})
  assert all(before[i+2]==exceptions.get(i,(251*i+(local%16)*947+99)&0o77777) for i in range(2048) if i not in owned),(n,'target input reconstruction')
 maps=json.loads((ROOT/'build/gimbal-complete-source/maps.json').read_text())
 for mission,short in enumerate(['lm','cm']):
  rows=[];ids=[]
  for vac in range(5):
   for angle in range(16):
    direction=(angle+3*vac)%16;flag=(angle+vac)%4;n=mission*5120+vac*1024+flag*256+direction*16+angle
    row=struct.unpack_from('<2052H',source,n*4104);before=struct.unpack_from('<2052H',inputs,n*4104)
    rows.append([row[i+2] for i in positions(before,n)]+list(row[-2:]));ids.append(n)
  packed=encode_rows(rows)
  text='/* SPDX-License-Identifier: MIT\n * Exact original complete gimbal states.\n * Source SHA256: '+hashlib.sha256(source).hexdigest()+' */\n#ifndef GIMBAL_COMPLETE_VECTORS_H\n#define GIMBAL_COMPLETE_VECTORS_H\n'
  text+=c_header(packed,'gimbal_complete')
  text+='static const uint16_t gimbal_complete_target_ids[80]={'+','.join(map(str,ids))+'};\n'
  text+='static const uint16_t gimbal_complete_changed[35]={'+','.join(map(str,FIXED))+'};\n'
  constants=[word for r in maps[mission]['source_line_and_word_map'] if r['op']=='2DEC' for word in r['words']]
  assert len(constants)==(10 if mission==0 else 30)
  text+='static const uint16_t gimbal_complete_constant_words['+str(len(constants))+']={'+','.join(map(str,constants))+'};\n#endif\n'
  (ROOT/f'tests/gimbal_complete_vectors_{short}.h').write_text(text)
  print(short,': 80 source target states;',packed.storage_bytes,'lossless bytes')
 print('PASS: all10240 full source states preserve1969 unowned physical words; two C64 fixtures compare every word exactly')
if __name__=='__main__':main()
