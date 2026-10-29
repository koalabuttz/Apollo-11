#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Freeze exact changed source words, establish all remaining words unchanged."""
from pathlib import Path
import hashlib,struct
from frozen_words import encode_rows,c_header
ROOT=Path(__file__).resolve().parents[1]
FIXED=[0,1,2,0o4,0o6,0o20,0o21,0o116,0o117,0o121,0o122,0o123,0o130,0o133,0o134,0o135,0o140,0o141,0o154,0o155,0o156,0o164,0o165,0o166]
def main():
 source=(ROOT/'build/delcomp-lm.snapshots').read_bytes()+(ROOT/'build/delcomp-cm.snapshots').read_bytes()
 assert source==(ROOT/'build/delcomp-native.snapshots').read_bytes()
 inputs=(ROOT/'build/delcomp-input.snapshots').read_bytes();assert len(inputs)==len(source)==10240*4104
 rows=[];ids=[]
 for mission in range(2):
  for variant in range(10):
   for angle in range(16):
    vac=variant%5;direction=(angle+3*variant)%16;flag=(angle+variant)%4
    n=mission*5120+vac*1024+flag*256+direction*16+angle
    words=struct.unpack_from('<2052H',source,n*4104);before=struct.unpack_from('<2052H',inputs,n*4104)
    base=before[2+0o120];matrix=0o2230 if mission==0 else 0o3425
    positions=FIXED+list(range(base,base+10))+list(range(matrix,matrix+18))
    assert all(words[i+2]==before[i+2] for i in range(2048) if i not in positions),(n,'unowned source word')
    rows.append([words[i+2] for i in positions]+list(words[-2:]));ids.append(n)
 packed=encode_rows(rows)
 text='/* SPDX-License-Identifier: MIT\n * Frozen exact original DELCOMP source states.\n * Source SHA256: '+hashlib.sha256(source).hexdigest()+' */\n#ifndef DELCOMP_VECTORS_H\n#define DELCOMP_VECTORS_H\n'
 text+=c_header(packed,'delcomp')
 text+='static const uint16_t delcomp_target_ids[320]={'+','.join(map(str,ids))+'};\n'
 text+='static const uint16_t delcomp_changed[24]={'+','.join(map(str,FIXED))+'};\n#endif\n'
 (ROOT/'tests/delcomp_vectors.h').write_text(text)
 print('PASS: 320 original-source target states; all52 potential modified words and1996 remaining words exact; lossless fixture',packed.storage_bytes,'bytes')
if __name__=='__main__':main()
