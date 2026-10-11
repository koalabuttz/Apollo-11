#!/usr/bin/env python3
"""Lossless original full-image snapshots expressed as deltas from input data."""
import pathlib,hashlib,subprocess
root=pathlib.Path(__file__).resolve().parents[1]
subprocess.run(['cc','-O2','-std=c99','-Iinclude','-Itests','tests/dsky_key_irq_cases.c','tools/dsky_key_irq_inputs.c','-o','build/dsky-key-irq-inputs'],cwd=root,check=True)
inputs=subprocess.check_output(['./build/dsky-key-irq-inputs'],cwd=root,text=True)
a=(root/'build/dsky-key-irq-reference.snapshots').read_bytes();assert a==(root/'build/dsky-key-irq-native.snapshots').read_bytes()
rows=[list(map(int,l.split())) for l in a.decode().splitlines()];before=[list(map(int,l.split())) for l in inputs.splitlines()]
assert len(rows)==len(before)==4096 and all(len(r)==2053 for r in rows+before)
patch=[];offsets=[0];numbers=list(range(32))+[128*n+(n*29)%128 for n in range(32)]
for n in numbers:
 for f,(v,b) in enumerate(zip(rows[n][2:],before[n][2:])):
  if v!=b:patch.append((f,v))
 offsets.append(len(patch))
lines=['/* SPDX-License-Identifier: MIT',' * Lossless complete original key IRQ erasable images relative to input data.',' * Snapshot SHA256 '+hashlib.sha256(a).hexdigest()+' */','#ifndef DSKY_KEY_IRQ_VECTORS_H','#define DSKY_KEY_IRQ_VECTORS_H','#include "dsky_key_irq_cases.h"','static const uint16_t dsky_key_irq_numbers[]={'+','.join(map(str,numbers))+'};','static const uint16_t dsky_key_irq_offsets[]={'+','.join(map(str,offsets))+'};','static const uint16_t dsky_key_irq_patch[][2]={']
lines+=[' {'+str(f)+','+str(v)+'},' for f,v in patch];lines+=['};','static const uint16_t dsky_key_irq_results[]={'+','.join(str(rows[n][1]) for n in numbers)+'};','#endif','']
(root/'tests/dsky_key_irq_vectors.h').write_text('\n'.join(lines))
print('Frozen64 complete2050wordimages, patches:',len(patch),'packed fixturebytes:',len(patch)*4+65*2+64*2)
