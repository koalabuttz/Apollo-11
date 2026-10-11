#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Freeze actual source entry states and prove every other word unchanged."""
from pathlib import Path
import hashlib,struct,json
ROOT=Path(__file__).resolve().parents[1]
CHANGED=[0,1,2,0o20,0o21,0o76,0o116,0o117,0o130,0o131,0o135,0o154,0o155,0o156,0o163,0o164]
def main():
 source=(ROOT/'build/gimbal-lm.snapshots').read_bytes()+(ROOT/'build/gimbal-cm.snapshots').read_bytes()
 native=(ROOT/'build/gimbal-native.snapshots').read_bytes();inputs=(ROOT/'build/gimbal-input.snapshots').read_bytes()
 assert source==native and len(source)==len(inputs)==2560*4104
 rows=[]
 for mission in range(2):
  for variant in range(8):
   for vac in range(5):
    for rate in range(4):
     bank=(variant+vac+rate)%8;n=mission*1280+variant*160+vac*32+bank*4+rate
     words=struct.unpack_from('<2052H',source,n*4104);before=struct.unpack_from('<2052H',inputs,n*4104)
     positions=CHANGED+[before[2+0o120]+0o46,before[2+0o120]+0o52]
     assert all(words[i+2]==before[i+2] for i in range(2048) if i not in positions),(n,'unowned source word')
     assert words[-2]==before[-2]
     rows.append((n,[words[2+i] for i in positions]+[words[-1]]))
 lines=['/* SPDX-License-Identifier: MIT',' * Frozen independently assembled original-source states.',
 ' * Reference SHA256: '+hashlib.sha256(source).hexdigest()+' */',
 '#ifndef GIMBAL_VECTORS_H','#define GIMBAL_VECTORS_H','#include <stdint.h>',
 '#define GIMBAL_TARGET_ROWS 320u','#define GIMBAL_FIXED_CHANGED 16u',
 'static const uint16_t gimbal_changed[16]={'+','.join(str(n) for n in CHANGED)+'};',
 'static const uint16_t gimbal_target_ids[320]={'+','.join(str(n) for n,_ in rows)+'};',
 'static const uint16_t gimbal_expected[320][19]={']
 for _,row in rows:lines.append('{'+','.join('0%06o'%w for w in row)+'},')
 lines+=['};','static const uint16_t gimbal_constant_expected[2][30]={']
 for mission in json.loads((ROOT/'build/gimbal-source/maps.json').read_text()):
  words=[word for row in mission['rows'] if row['op']=='2DEC' for word in row['words']]
  lines.append('{'+','.join('0%06o'%w for w in words)+'},')
 lines+=['};','#endif','']
 (ROOT/'tests/gimbal_vectors.h').write_text('\n'.join(lines))
 print('PASS: 320 source target states, 18 modified words and2030 unmodified physical words qualified exactly')
if __name__=='__main__':main()
