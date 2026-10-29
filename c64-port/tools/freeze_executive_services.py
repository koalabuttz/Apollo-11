#!/usr/bin/env python3
"""Freeze original-assembly Executive service state fingerprints."""
import pathlib
ROOT=pathlib.Path(__file__).resolve().parents[1]
rows=[list(map(int,line.split())) for line in
      (ROOT/'build/executive-service-reference.snapshots').read_text().splitlines()]
assert len(rows)==768 and all(len(row)==332 for row in rows)
values=[]
for row in rows:
    h=2166136261
    for word in row[1:]:
        for b in (word&255,(word>>8)&255): h=((h^b)*16777619)&0xffffffff
    values.append(h)
text='''/* SPDX-License-Identifier: MIT
 * Generated from unchanged original SPVAC/CHANG2 assembly. Hash regressions
 * supplement the full-word host comparison; hash collisions remain possible. */
#ifndef EXECUTIVE_SERVICE_VECTORS_H
#define EXECUTIVE_SERVICE_VECTORS_H
#include <stdint.h>
static const uint32_t executive_service_vectors[768]={
'''
for i in range(0,len(values),6): text+='    '+','.join('UINT32_C(0x%08x)'%v for v in values[i:i+6])+',\n'
text+='};\n#endif\n'
(ROOT/'tests/executive_service_vectors.h').write_text(text)
