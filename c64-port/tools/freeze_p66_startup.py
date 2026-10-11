#!/usr/bin/env python3
"""Freeze 128 complete original states using lossless baselines and patches."""
import pathlib,hashlib
root=pathlib.Path(__file__).resolve().parents[1]
a=(root/'build/p66-startup-reference.snapshots').read_bytes(); assert a==(root/'build/p66-startup-native.snapshots').read_bytes()
rows=[list(map(int,l.split())) for l in a.decode().splitlines()]; assert len(rows)==73728
base=[rows[start+i][2:] for start in [0,65536,69632] for i in range(8)]
selected=[rows[n//2*1024+n%2*32768 if n<32 else 65536+(n-32)*83 if n<80 else 69632+(n-80)*83] for n in range(128)]
patch=[];offsets=[0]
for r in selected:
 mode=0 if r[0]<65536 else 1 if r[0]<69632 else 2
 for field,(v,b) in enumerate(zip(r[2:],base[mode*8+r[0]%8])):
  if v!=b:patch.append((field,v))
 offsets.append(len(patch))
flat=[w for row in base for w in row]
lines=['/* SPDX-License-Identifier: MIT',' * Whole unchanged P66 entry states. SHA256 '+hashlib.sha256(a).hexdigest()+' */','#ifndef P66_STARTUP_VECTORS_H','#define P66_STARTUP_VECTORS_H','#include "p66_startup_cases.h"','static const agc_word p66_startup_base[]={'+','.join(map(str,flat))+'};','static const uint16_t p66_startup_offsets[]={'+','.join(map(str,offsets))+'};','static const uint16_t p66_startup_patch[][2]={']
lines+=[' {'+str(f)+','+str(v)+'},' for f,v in patch]
lines+=['};','static const agc_word p66_startup_result[]={'+','.join(str(r[1]) for r in selected)+'};','#endif','']
(root/'tests/p66_startup_vectors.h').write_text('\n'.join(lines))
print('128 full original P66 states; fixture bytes:',len(flat)*2+len(patch)*4+129*2+256,'word replacements:',len(patch))
