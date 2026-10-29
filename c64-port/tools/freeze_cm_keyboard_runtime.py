#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,subprocess,hashlib
R=pathlib.Path(__file__).resolve().parents[1]
inputs=['tests/p66_bias_cases.c','tests/dsky_nv_key_cases.c','tests/dsky_charin_nv_cases.c','tests/dsky_charin_number_cases.c','tests/cm_charin_nv_cases.c','tests/cm_charin_number_cases.c','tests/cm_keyboard_runtime_cases.c']
subprocess.run(['cc','-O2','-std=c99','-ffunction-sections','-Iinclude','-Itests','src/agc_word.c']+inputs+['tools/cm_keyboard_runtime_inputs.c','-Wl,--gc-sections','-o','build/cm-keyboard-runtime-inputs'],cwd=R,check=True)
before=[list(map(int,x.split()))[1:]for x in subprocess.check_output(['./build/cm-keyboard-runtime-inputs'],cwd=R,text=True).splitlines()]
a=(R/'build/cm-keyboard-runtime-reference.snapshots').read_bytes();assert a==(R/'build/cm-keyboard-runtime-native.snapshots').read_bytes()
rows=[list(map(int,x.split()))for x in a.decode().splitlines()];assert len(rows)==4096
numbers=[n*131 for n in range(8)];patch=[];offsets=[0]
for n in numbers:
 previous=before[n]
 for stage in range(4):
  after=rows[n*4+stage][3:];assert len(after)==len(previous)==2056
  patch.extend((i,v)for i,(v,b)in enumerate(zip(after,previous))if v!=b);offsets.append(len(patch));previous=after
text=['/* SPDX-License-Identifier: MIT',' * Complete original CM keyboard lifecycle source checkpoints.',' * Full stream SHA256 '+hashlib.sha256(a).hexdigest()+' */','#ifndef CM_KEYBOARD_RUNTIME_VECTORS_H','#define CM_KEYBOARD_RUNTIME_VECTORS_H','#include "cm_keyboard_runtime_cases.h"','static const uint16_t cm_keyboard_runtime_numbers[]={'+','.join(map(str,numbers))+'};','static const uint16_t cm_keyboard_runtime_offsets[]={'+','.join(map(str,offsets))+'};','static const uint16_t cm_keyboard_runtime_patch[][2]={']
text+=[' {'+str(i)+','+str(v)+'},'for i,v in patch];text+=['};','#endif',''];(R/'tests/cm_keyboard_runtime_vectors.h').write_text('\n'.join(text));print('Frozen8 full CM keyboard lifecycles32 original2056-field states;patches',len(patch))
