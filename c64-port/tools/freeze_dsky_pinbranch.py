#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,hashlib,subprocess
R=pathlib.Path(__file__).resolve().parents[1]
subprocess.run(['cc','-O2','-std=c99','-Iinclude','-Itests','src/agc_word.c','tests/p66_bias_cases.c','tests/dsky_pinbranch_cases.c','tools/dsky_pinbranch_inputs.c','-o','build/dsky-pinbranch-inputs'],cwd=R,check=True)
numbers=[base+56*j+(j%8) for base in [0,4096,8192,12288] for j in range(16)]
before={};rows={};h=hashlib.sha256()
process=subprocess.Popen(['./build/dsky-pinbranch-inputs'],cwd=R,stdout=subprocess.PIPE)
for n,line in enumerate(process.stdout):
 if n in numbers:before[n]=list(map(int,line.split()))[2:]
assert process.wait()==0 and n==16383
with (R/'build/dsky-pinbranch-reference.snapshots').open('rb') as a,(R/'build/dsky-pinbranch-native.snapshots').open('rb') as b:
 for n,line in enumerate(a):
  assert line==b.readline();h.update(line)
  if n in numbers:rows[n]=list(map(int,line.split()))[2:]
 assert n==16383 and not b.readline()
patch=[];offsets=[0]
for n in numbers:
 patch.extend((i,v)for i,(v,b)in enumerate(zip(rows[n],before[n]))if v!=b);offsets.append(len(patch))
text=['/* SPDX-License-Identifier: MIT',' * Complete original both-program PINBRNCH and display continuation images.',' * Full snapshot SHA256 '+h.hexdigest()+' */','#ifndef DSKY_PINBRANCH_VECTORS_H','#define DSKY_PINBRANCH_VECTORS_H','#include "dsky_pinbranch_cases.h"','static const uint16_t dsky_pinbranch_numbers[]={'+','.join(map(str,numbers))+'};','static const uint16_t dsky_pinbranch_offsets[]={'+','.join(map(str,offsets))+'};','static const uint16_t dsky_pinbranch_patch[][2]={'];text+=[' {'+str(i)+','+str(v)+'},'for i,v in patch];text+=['};','#endif',''];(R/'tests/dsky_pinbranch_vectors.h').write_text('\n'.join(text));print('Frozen64 complete original2051-field LM+CM PINBRNCH states, patches',len(patch))
