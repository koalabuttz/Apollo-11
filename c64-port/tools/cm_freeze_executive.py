#!/usr/bin/env python3
"""Freeze full CM assembly state samples and all-case fingerprints."""
import pathlib
ROOT=pathlib.Path(__file__).resolve().parents[1]
rows=[list(map(int,x.split())) for x in (ROOT/'build/cm-executive-reference.snapshots').read_text().splitlines()]
assert len(rows)==1536 and all(len(x)==314 for x in rows)
indices=list(range(26))+[26+(1509*i)//13 for i in range(14)]
assert len(indices)==len(set(indices))==40
text='''/* SPDX-License-Identifier: MIT
 * Generated from original CM Executive with five documented pseudo-label
 * indentation repairs. Full-state samples supplement all-case hashes. */
#ifndef CM_EXECUTIVE_VECTORS_H
#define CM_EXECUTIVE_VECTORS_H
#include <stdint.h>
static const uint16_t cm_full_indices[40]={'''+','.join(map(str,indices))+'''};
static const int16_t cm_full_results[40]={'''+','.join(str(rows[i][1]) for i in indices)+'''};
static const uint16_t cm_full_words[40][312]={
'''
for i in indices: text+=' {'+','.join(map(str,rows[i][2:]))+'},\n'
text+='};\nstatic const uint32_t cm_case_hashes[1536]={\n'
for row in rows:
 h=2166136261
 for word in row[1:]:
  for b in (word&255,(word>>8)&255): h=((h^b)*16777619)&0xffffffff
 text+=' UINT32_C(0x%08x),\n'%h
text+='};\n#endif\n'
(ROOT/'tests/cm_executive_vectors.h').write_text(text)
