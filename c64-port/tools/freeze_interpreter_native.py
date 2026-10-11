#!/usr/bin/env python3
"""Freeze original mixed-context assembly fingerprints."""
import pathlib
ROOT=pathlib.Path(__file__).resolve().parents[1]
rows=[list(map(int,line.split())) for line in
      (ROOT/'build/interpreter-native-reference.snapshots').read_text().splitlines()]
assert len(rows)==140 and all(len(row)==332 for row in rows)
values=[]
for row in rows:
    h=2166136261
    for word in row[2:]:
        for b in (word&255,(word>>8)&255): h=((h^b)*16777619)&0xffffffff
    values.append(h)
text='''/* SPDX-License-Identifier: MIT; frozen unchanged original assembly fingerprints. */
#ifndef INTERPRETER_NATIVE_VECTORS_H
#define INTERPRETER_NATIVE_VECTORS_H
#include <stdint.h>
static const uint32_t interpreter_native_vectors[140]={
'''
for i in range(0,len(values),5): text+='    '+','.join('UINT32_C(0x%08x)'%v for v in values[i:i+5])+',\n'
text+='};\n#endif\n'
(ROOT/'tests/interpreter_native_vectors.h').write_text(text)
