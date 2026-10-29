#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Freeze every final STOVL24D,R word, with two lossless C64 fixture halves."""
import hashlib
from pathlib import Path
from frozen_words import c_header,encode_rows
root=Path(__file__).resolve().parents[1];data=(root/'build/velocity-reference.snapshots').read_bytes()
assert data==(root/'build/velocity-native.snapshots').read_bytes()
rows=[list(map(int,row.split())) for row in data.decode().splitlines()]
assert len(rows)==4648 and all(len(row)==120 for row in rows)
selected=[row for row in rows if row[0]<147 and row[2]==1];assert len(selected)==156
for suffix,subset in [('all',selected),('a',selected[:80]),('b',selected[80:])]:
    packed=encode_rows([row[3:] for row in subset])
    guard='VELOCITY_VECTORS_'+suffix.upper()+'_H'
    text=('/* SPDX-License-Identifier: MIT\n * GENERATED from original assembly using pinned hardware DV by tools/freeze_velocity.py.\n'
          ' * Full snapshot SHA256: '+hashlib.sha256(data).hexdigest()+'\n'
          ' * Lossless storage: every decoded uint16_t is compared exactly.\n */\n#ifndef '+guard+'\n#define '+guard+'\n')
    text+=c_header(packed,'velocity')+'#endif\n';(root/f'tests/velocity_vectors_{suffix}.h').write_text(text)
    print(f'Frozen {suffix}: {len(subset)} complete 117-word states, {len(subset)*117*2} -> {packed.storage_bytes} bytes; exact lossless roundtrip')
text='''/* SPDX-License-Identifier: MIT */
#ifndef VELOCITY_VECTORS_H
#define VELOCITY_VECTORS_H
#if defined(VELOCITY_DIAGNOSTIC_HALF) && VELOCITY_DIAGNOSTIC_HALF==0
#define VL_FIRST_CASE 0u
#define VL_LAST_CASE 80u
#include "velocity_vectors_a.h"
#elif defined(VELOCITY_DIAGNOSTIC_HALF) && VELOCITY_DIAGNOSTIC_HALF==1
#define VL_FIRST_CASE 80u
#define VL_LAST_CASE 147u
#include "velocity_vectors_b.h"
#else
#define VL_FIRST_CASE 0u
#define VL_LAST_CASE 147u
#include "velocity_vectors_all.h"
#endif
#endif
'''
(root/'tests/velocity_vectors.h').write_text(text)
