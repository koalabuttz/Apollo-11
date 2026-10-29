#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Freeze complete source snapshots as lossless input-relative patches."""
import pathlib,subprocess,hashlib
R=pathlib.Path(__file__).resolve().parents[1]
core=['src/agc_word.c','src/cm_executive.c','src/cm_native_image.c','src/cm_admission_image.c','src/cm_keyboard_irq.c','tests/cm_keyboard_irq_cases.c']
subprocess.run(['cc','-O2','-std=c99','-Iinclude','-Itests',*core,'tools/cm_keyboard_irq_inputs.c','-o','build/cm-keyboard-irq-inputs'],cwd=R,check=True)
inputs=subprocess.check_output(['./build/cm-keyboard-irq-inputs'],cwd=R,text=True)
a=(R/'build/cm-keyboard-irq-reference.snapshots').read_bytes();assert a==(R/'build/cm-keyboard-irq-native.snapshots').read_bytes()
rows=[list(map(int,l.split()))for l in a.decode().splitlines()];before=[list(map(int,l.split()))for l in inputs.splitlines()];assert len(rows)==len(before)==12288 and all(len(r)==2053 for r in rows+before)
numbers=[k*193 for k in range(21)]+[4096+k*193 for k in range(21)]+[8192+k*181 for k in range(22)]
patch=[];offsets=[0]
for n in numbers:
 for f,(v,b) in enumerate(zip(rows[n][2:],before[n][2:])):
  if v!=b:patch.append((f,v))
 offsets.append(len(patch))
lines=['/* SPDX-License-Identifier: MIT',' * Full original CM source state, lossless deltas from independent inputs.',' * Snapshot SHA256 '+hashlib.sha256(a).hexdigest()+' */','#ifndef CM_KEYBOARD_IRQ_VECTORS_H','#define CM_KEYBOARD_IRQ_VECTORS_H','#include "cm_keyboard_irq_cases.h"','static const uint16_t cm_keyboard_irq_numbers[]={'+','.join(map(str,numbers))+'};','static const uint16_t cm_keyboard_irq_offsets[]={'+','.join(map(str,offsets))+'};','static const uint16_t cm_keyboard_irq_patch[][2]={']
lines+=[' {'+str(f)+','+str(v)+'},'for f,v in patch];lines+=['};','static const uint16_t cm_keyboard_irq_results[]={'+','.join(str(rows[n][1])for n in numbers)+'};','#endif',''];(R/'tests/cm_keyboard_irq_vectors.h').write_text('\n'.join(lines));print('Frozen64 complete2051-field original states; patches',len(patch))
