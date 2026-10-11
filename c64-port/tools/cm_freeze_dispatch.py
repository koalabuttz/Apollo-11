#!/usr/bin/env python3
"""Freeze full CM source-derived checkpoint samples and all-state fingerprints."""
import pathlib
ROOT=pathlib.Path(__file__).resolve().parents[1]; rows=[]
for stage,count in [('dispatch',109),('lifecycle',115),('interpreter',512),('mixed',140)]:
 data=[list(map(int,line.split())) for line in (ROOT/f'build/cm-{stage}-reference.snapshots').read_text().splitlines()]
 assert len(data)==count
 rows.extend(row[1:] if stage=='interpreter' else row[2:] for row in data)
assert len(rows)==876 and all(len(row) in (315,318) for row in rows)
indices=[0,6,35,42,45,49,108,109,185,215,223,224,255,735,736,875]
text="""/* SPDX-License-Identifier: MIT
 * Generated from CM assembly fixture with documented indentation repairs. */
#ifndef CM_DISPATCH_VECTORS_H
#define CM_DISPATCH_VECTORS_H
#include <stdint.h>
"""
for tag,start,end in [('BASIC',0,109),('LIFECYCLE',109,224),('INTERPRETER',224,736),('MIXED',736,876),('ALL',0,876)]:
 selected=[i for i in indices if start<=i<end]
 text+=('#if' if tag=='BASIC' else '#elif')+' defined(CM_CHECK_'+tag+')\n'
 text+=f'#define CM_HASH_FIRST {start}u\n#define CM_HASH_COUNT {end-start}u\n#define CM_FULL_COUNT {len(selected)}u\n'
 text+='static const uint16_t cm_dispatch_full_indices[CM_FULL_COUNT]={'+','.join(map(str,selected))+'};\n'
 text+='static const uint16_t cm_dispatch_full_lengths[CM_FULL_COUNT]={'+','.join(str(len(rows[i])) for i in selected)+'};\n'
 text+='static const uint16_t cm_dispatch_full_words[CM_FULL_COUNT][318]={\n'
 for i in selected: text+=' {'+','.join(map(str,rows[i]))+'},\n'
 text+='};\nstatic const uint32_t cm_dispatch_hashes[CM_HASH_COUNT]={\n'
 for row in rows[start:end]:
  h=2166136261
  for word in row:
   for b in (word&255,(word>>8)&255): h=((h^b)*16777619)&0xffffffff
  text+=' UINT32_C(0x%08x),\n'%h
 text+='};\n'
text+='#endif\n#endif\n'; (ROOT/'tests/cm_dispatch_vectors.h').write_text(text)
