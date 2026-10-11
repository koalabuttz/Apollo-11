#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Freeze original-source full-state fingerprints and pure target inputs."""
import pathlib,hashlib
root=pathlib.Path(__file__).resolve().parents[1]
normal=[list(map(int,l.split()))for l in (root/'build/p66-guidance-live-reference.snapshots').read_text().splitlines()]
mixed=[list(map(int,l.split()))for l in (root/'build/p66-guidance-live-mixed-reference.snapshots').read_text().splitlines()]
inputs=[list(map(int,l.split()))for l in (root/'build/p66-guidance-live-inputs.snapshots').read_text().splitlines()]
assert len(normal)==2944 and len(mixed)==400 and len(inputs)==128
assert all(len(r)==495 for r in normal+mixed) and all(len(r)==167 for r in inputs)
def fnv(words):
 h=2166136261
 for w in words:
  for b in(w&255,w>>8):h=((h^b)*16777619)&0xffffffff
 return 'UINT32_C(%u)'%h
header='/* SPDX-License-Identifier: MIT\n * Pure source entry constructors; frozen without translated arithmetic. */\n#ifndef P66_GUIDANCE_LIVE_INPUTS_H\n#define P66_GUIDANCE_LIVE_INPUTS_H\nstatic const uint16_t pgl_inputs[2][166]={\n'+''.join('{'+','.join(map(str,row[1:]))+'},\n'for row in inputs[:2])+'};\n#endif\n'
(root/'tests/p66_guidance_live_inputs.h').write_text(header)
header='/* SPDX-License-Identifier: GPL-2.0-or-later\n * Independent unchanged-assembly493-word states; LE FNV32 fingerprints. */\n#ifndef P66_GUIDANCE_LIVE_VECTORS_H\n#define P66_GUIDANCE_LIVE_VECTORS_H\n#include <stdint.h>\n'
for name,rows,count in [('pgl_fingerprints',normal,23),('pgl_mixed_fingerprints',mixed,25)]:
 header+=f'static const uint32_t {name}[2][{count}]={{\n'+''.join('{'+','.join(fnv(r[2:])for r in rows[n*count:(n+1)*count])+'},\n'for n in range(2))+'};\n'
(root/'tests/p66_guidance_live_vectors.h').write_text(header+'#endif\n')
for name in('p66-guidance-live-reference.snapshots','p66-guidance-live-mixed-reference.snapshots','p66-guidance-live-inputs.snapshots'):print(name,hashlib.sha256((root/'build'/name).read_bytes()).hexdigest())
