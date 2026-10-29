#!/usr/bin/env python3
"""Freeze CM IRQ corpus after original assembly/full native agreement."""
import hashlib,pathlib
R=pathlib.Path(__file__).resolve().parents[1]; s='/* SPDX-License-Identifier: MIT\n * GENERATED from original CM assembly by tools/cm_freeze_irq.py. */\n#ifndef CM_IRQ_VECTORS_H\n#define CM_IRQ_VECTORS_H\n#include <stdint.h>\n'
def hashwords(words):
 h=2166136261
 for w in words:
  for b in (w&255,(w>>8)&255): h=((h^b)*16777619)&0xffffffff
 return h
for kind,count,cols in [('','512',314),('-sequence','64',317)]:
 p=R/f'build/cm-irq{kind}-reference.snapshots';assert p.read_bytes()==(R/f'build/cm-irq{kind}-native.snapshots').read_bytes();rows=[list(map(int,r.split())) for r in p.read_text().splitlines()];assert len(rows)==int(count) and all(len(r)==cols for r in rows)
 offset=2 if kind else 1; tag='sequence' if kind else 'admission'
 s+='/* Snapshot SHA256 '+hashlib.sha256(p.read_bytes()).hexdigest()+' */\n'
 s+=f'static const uint32_t cm_irq_{tag}_hashes[{count}]={{\n'+''.join(' UINT32_C(0x%08x),\n'%hashwords(r[offset:]) for r in rows)+'};\n'
 selected=rows[:8] if kind else [rows[i] for i in range(0,512,32)]
 if not kind: s+='static const uint16_t cm_irq_admission_indices[16]={'+','.join(str(r[0]) for r in selected)+'};\n'
 s+=f'static const uint16_t cm_irq_{tag}_full[{len(selected)}][{cols-offset}]={{\n'+''.join(' {'+','.join(str(w) for w in r[offset:])+'},\n' for r in selected)+'};\n'
(R/'tests/cm_irq_vectors.h').write_text(s+'#endif\n')
print('Frozen512admission/64sequence hashes,16fulladmissions and8fullsequence states')
