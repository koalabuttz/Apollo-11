#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Freeze complete source snapshots as lossless input-relative patches."""
import pathlib,subprocess,hashlib
R=pathlib.Path(__file__).resolve().parents[1]
core=['src/agc_word.c','src/agc_multiword.c','src/dsky_pinball.c','src/dsky_two_digit.c','src/dsky_decimal.c','src/cm_formatter.c','tests/cm_pinball_cases.c','tests/cm_formatter_cases.c']
subprocess.run(['cc','-O2','-std=c99','-Iinclude','-Itests',*core,'tools/cm_formatter_inputs.c','-o','build/cm-formatter-inputs'],cwd=R,check=True)
inputs=subprocess.check_output(['./build/cm-formatter-inputs'],cwd=R,text=True)
a=(R/'build/cm-formatter-reference.snapshots').read_bytes();assert a==(R/'build/cm-formatter-native.snapshots').read_bytes()
rows=[list(map(int,l.split()))for l in a.decode().splitlines()];before=[list(map(int,l.split()))for l in inputs.splitlines()];assert len(rows)==len(before)==12288 and all(len(r)==2052 for r in rows+before)
numbers=[99*k for k in range(16)]+[4096+k for k in range(24)]+[8192+k for k in range(24)]
patch=[];offsets=[0]
for n in numbers:
 for f,(v,b) in enumerate(zip(rows[n][2:],before[n][2:])):
  if v!=b:patch.append((f,v))
 offsets.append(len(patch))
lines=['/* SPDX-License-Identifier: MIT',' * Full original CM source state, lossless deltas from independent inputs.',' * Snapshot SHA256 '+hashlib.sha256(a).hexdigest()+' */','#ifndef CM_FORMATTER_VECTORS_H','#define CM_FORMATTER_VECTORS_H','#include "cm_formatter_cases.h"','static const uint16_t cm_formatter_numbers[]={'+','.join(map(str,numbers))+'};','static const uint16_t cm_formatter_offsets[]={'+','.join(map(str,offsets))+'};','static const uint16_t cm_formatter_patch[][2]={']
lines+=[' {'+str(f)+','+str(v)+'},'for f,v in patch];lines+=['};','static const uint16_t cm_formatter_results[]={'+','.join(str(rows[n][1])for n in numbers)+'};','#endif',''];(R/'tests/cm_formatter_vectors.h').write_text('\n'.join(lines));print('Frozen64 complete2050-field original states; patches',len(patch))
