#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,hashlib,subprocess
from frozen_words import encode_rows,c_header
root=pathlib.Path(__file__).resolve().parents[1]
flags=['-DACCELERATION_INPUT_ONLY','-DUNIT_INPUT_ONLY','-DDIVIDE_INPUT_ONLY','-DSTORE_INPUT_ONLY','-DELAPSED_INPUT_ONLY','-DPUSH_INPUT_ONLY','-DSCALE_INPUT_ONLY','-DPREPARE_INPUT_ONLY']
cases=['tests/scenarios.c','tests/prepare_cases.c','tests/scale_cases.c','tests/push_cases.c','tests/elapsed_cases.c','tests/store_cases.c','tests/divide_cases.c','tests/unit_cases.c','tests/acceleration_cases.c']
subprocess.run(['gcc','-std=c99','-O2','-Iinclude','-Itests']+flags+cases+['tools/acceleration_phased_inputs.c','-o','build/acceleration-phased-inputs'],cwd=root,check=True)
inputs=[list(map(int,r.split()))[1:]for r in subprocess.check_output(['build/acceleration-phased-inputs'],cwd=root,text=True).splitlines()]
data=(root/'build/acceleration-phased-reference.snapshots').read_bytes();assert data==(root/'build/acceleration-phased-native.snapshots').read_bytes();states=[list(map(int,r.split()))for r in data.decode().splitlines()];assert len(states)==65536
indices=list(range(16))+[4096+i*127 for i in range(8)]+[8192+i*1023 for i in range(8)]
s='/* SPDX-License-Identifier: MIT\n * Original source phase entry/exit states; SHA256:'+hashlib.sha256(data).hexdigest()+' */\n#ifndef ACCELERATION_PHASED_VECTORS_H\n#define ACCELERATION_PHASED_VECTORS_H\n'
for phase in range(4):
 rows=[]
 for n in indices:
  before=inputs[n]if phase==0 else states[n*4+phase-1][3:];after=states[n*4+phase];assert after[:3]==[n,phase,0];rows.append(before+after[2:])
 packed=encode_rows(rows);s+=f'#if !defined(ACCELERATION_PHASE_SELECTED) || ACCELERATION_PHASE_SELECTED=={phase}\n'+c_header(packed,f'acceleration_phase{phase}')+'#endif\n';print('Phase',phase,'32 original entry/exit states;',packed.storage_bytes,'packed bytes')
s+='#endif\n';(root/'tests/acceleration_phased_vectors.h').write_text(s)
