#!/usr/bin/env python3
"""Freeze three complete original pipeline checkpoint images per input."""
import pathlib,subprocess,hashlib
root=pathlib.Path(__file__).resolve().parents[1]
subprocess.run(['cc','-O2','-std=c99','-Iinclude','-Itests','tests/p66_bias_cases.c','tests/dsky_nv_key_cases.c','tests/dsky_keyboard_flow_cases.c','tools/dsky_keyboard_flow_inputs.c','-o','build/dsky-keyboard-flow-inputs'],cwd=root,check=True)
before=[list(map(int,l.split())) for l in subprocess.check_output(['./build/dsky-keyboard-flow-inputs'],cwd=root,text=True).splitlines()]
a=(root/'build/dsky-keyboard-flow-reference.snapshots').read_bytes();assert a==(root/'build/dsky-keyboard-flow-native.snapshots').read_bytes()
rows=[list(map(int,l.split())) for l in a.decode().splitlines()];assert len(rows)==3072 and len(before)==1024 and all(len(r)==2054 for r in rows+before)
numbers=list(range(16))+[128+n*53 for n in range(16)]
patch=[];offsets=[0]
for n in numbers:
 for phase in range(3):
  for f,(v,b) in enumerate(zip(rows[n*3+phase][3:],before[n][3:])):
   if v!=b:patch.append((f,v))
  offsets.append(len(patch))
s=['/* SPDX-License-Identifier: MIT',' * Complete unchanged original KEYCOM, NUDIRECT dispatch and CHARIN images.',' * Snapshot SHA256 '+hashlib.sha256(a).hexdigest()+' */','#ifndef DSKY_KEYBOARD_FLOW_VECTORS_H','#define DSKY_KEYBOARD_FLOW_VECTORS_H','#include "dsky_keyboard_flow_cases.h"','#define DSKY_KEYBOARD_FLOW_TARGET_COUNT 32u','static const uint16_t dsky_keyboard_flow_numbers[]={'+','.join(map(str,numbers))+'};','static const uint16_t dsky_keyboard_flow_offsets[]={'+','.join(map(str,offsets))+'};','static const uint16_t dsky_keyboard_flow_patch[][2]={']
s+=[' {'+str(f)+','+str(v)+'},' for f,v in patch];s+=['};','#endif',''];(root/'tests/dsky_keyboard_flow_vectors.h').write_text('\n'.join(s));print('Frozen32 pipelines /96 complete2051word images:',len(patch),'patches')
