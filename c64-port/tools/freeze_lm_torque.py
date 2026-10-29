#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Freeze source words and prove every unowned source word remains unchanged."""
from pathlib import Path
import hashlib,struct
ROOT=Path(__file__).resolve().parents[1]
CHANGED=[0,1,2,5]+sorted([0o124+i for i in range(3)]+[0o337+i for i in range(3)]+[0o344]+[0o113+i for i in range(6)]+[0o346,0o347])
def positions(n):
 b=(n%8)*0o400
 return CHANGED[:4]+[b+i for i in CHANGED[4:]]
def main():
 reference=(ROOT/'build/lm-torque-reference.snapshots').read_bytes()
 native=(ROOT/'build/lm-torque-native.snapshots').read_bytes()
 inputs=(ROOT/'build/lm-torque-input.snapshots').read_bytes()
 assert reference==native and len(reference)==36864*4104==len(inputs)
 rows=[]
 for p in range(16):
  for j in range(16):
   n=j*128+p*8
   expected=None
   for bank in range(8):
    case=n+bank
    words=struct.unpack_from('<2052H',reference,case*4104)
    before=struct.unpack_from('<2052H',inputs,case*4104)
    changed=positions(case)
    assert all(words[i+2]==before[i+2] for i in range(2048) if i not in changed),(case,'unowned source mutation')
    assert words[-2:]==before[-2:]
    selected=[words[2+i] for i in changed]
    if expected is None:expected=selected
    else:assert selected==expected,(case,'bank-dependent owned output')
   rows.append((n,expected))
 lines=['/* SPDX-License-Identifier: MIT',' * Frozen independent original-source words, not native outputs.',
 ' * Full reference SHA256: '+hashlib.sha256(reference).hexdigest()+' */',
 '#ifndef LM_TORQUE_VECTORS_H','#define LM_TORQUE_VECTORS_H','#include <stdint.h>',
 '#define LM_TORQUE_TARGET_ROWS 256u','#define LM_TORQUE_CHANGED_WORDS 19u',
 'static const uint16_t lm_torque_changed[19]={'+','.join(str(i) for i in CHANGED)+'};',
 'static const uint16_t lm_torque_target_ids[256]={'+','.join(str(n) for n,_ in rows)+'};',
 'static const uint16_t lm_torque_expected[256][19]={']
 for _,row in rows:lines.append('{'+','.join('0%06o'%w for w in row)+'},')
 lines+=['};','#endif','']
 (ROOT/'tests/lm_torque_vectors.h').write_text('\n'.join(lines))
 print('PASS: 256 frozen source rows, all eight EB contexts equivalent; all other 2029 words verified unchanged')
if __name__=='__main__':main()
