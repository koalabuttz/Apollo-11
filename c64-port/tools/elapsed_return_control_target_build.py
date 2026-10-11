#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import argparse,lzma,subprocess
from pathlib import Path
root=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--cc',default='mos-c64-clang');a=p.parse_args()
indices=[c*8+q for c in [0,255,50,103,18,39,178,231]for q in range(8)]
records={};initial=None
with lzma.open(root/'evidence/elapsed-return-control-reference.snapshots.xz','rt')as f:
 for line in f:
  data=list(map(int,line.split()));n=data[0]
  if n==0:initial=data[1:2049]
  if n in indices:records[n]=data[3]
assert len(records)==64 and len(initial)==2048
qs=[0,0o77777,3,0o1434,0o2701,0o2314,0o7777,0o1030];corners=[0,0o77777,0o37777,0o40000]
rows=[]
for n in indices:
 c=n>>3;rows.append([corners[c&3],corners[(c>>2)&3],corners[(c>>4)&3],corners[(c>>6)&3],qs[n&7],records[n]])
header='/* SPDX-License-Identifier: MIT; frozen original source inputs/output. */\nstatic const uint16_t eq_initial[2048]={'+','.join(map(str,initial))+'};\nstatic const uint16_t eq_target[64][6]={\n'+''.join('{'+','.join(map(str,r))+'},\n'for r in rows)+'};\n'
(root/'platform/c64/elapsed_return_control_fixture.h').write_text(header)
subprocess.run([a.cc,'-Iinclude','-Iplatform/c64','-std=c99','-Oz','-fno-static-stack','-mlto-zp=0','-Wall','-Wextra','-Werror','-T','platform/c64/overlay_resident.ld','-Wl,-Map,build/elapsed-return-control.map','src/agc_word.c','src/agc_multiword.c','src/p66_elapsed_return_control.c','platform/c64/elapsed_return_control_diagnostic.c','-o','build/elapsed-return-control.prg'],cwd=root,check=True)
