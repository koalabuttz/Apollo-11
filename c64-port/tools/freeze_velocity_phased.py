#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,hashlib
from frozen_words import encode_rows,c_header
root=pathlib.Path(__file__).resolve().parents[1];data=(root/'build/velocity-phased-reference.snapshots').read_bytes();assert data==(root/'build/velocity-phased-native.snapshots').read_bytes();states=[list(map(int,r.split()))for r in data.decode().splitlines()];assert len(states)==13936 and all(len(r)==121 for r in states)
pairs={0:[],1:[],2:[]};prev={}
for r in states:
 n,step,stage=r[:3];key=(n,step)
 if stage:pairs[stage-1].append((n,step,prev[key],r[3:]))
 prev[key]=r[3:]
chosen=[(n,0)for n in range(16)]+[(16+i*16,0)for i in range(8)]+[(403+i*577,0)for i in range(4)]+[(2451+i*311,0)for i in range(4)]
s='/* SPDX-License-Identifier: MIT\n * Independent original source phase pairs; SHA256:'+hashlib.sha256(data).hexdigest()+'\n */\n#ifndef VELOCITY_PHASED_VECTORS_H\n#define VELOCITY_PHASED_VECTORS_H\n'
for phase in range(3):
 p={(n,step):(before,after)for n,step,before,after in pairs[phase]};packed=encode_rows([p[k][0]+[0]+p[k][1]for k in chosen]);s+=f'#if !defined(GUIDANCE_PHASE_SELECTED) || GUIDANCE_PHASE_SELECTED=={phase}\n'+c_header(packed,f'velocity_phase{phase}')+'#endif\n';print('phase',phase,'32 exact full118word source entry/exit pairs',packed.storage_bytes,'bytes')
s+='#endif\n';(root/'tests/velocity_phased_vectors.h').write_text(s)
