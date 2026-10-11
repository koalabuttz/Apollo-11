#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,hashlib,subprocess
R=pathlib.Path(__file__).resolve().parents[1]
subprocess.run(['cc','-O2','-std=c99','-Iinclude','-Itests','src/agc_word.c','tests/p66_bias_cases.c','tests/dsky_enter_cases.c','tools/dsky_enter_inputs.c','-o','build/dsky-enter-inputs'],cwd=R,check=True)
numbers=[base+56*j+(j%8) for base in [0,2048,4096,6144] for j in range(16)]
before={};rows={};h=hashlib.sha256()
process=subprocess.Popen(['./build/dsky-enter-inputs'],cwd=R,stdout=subprocess.PIPE)
for n,line in enumerate(process.stdout):
 if n in numbers:before[n]=list(map(int,line.split()))[2:]
assert process.wait()==0 and n==8191
with (R/'build/dsky-enter-reference.snapshots').open('rb') as a,(R/'build/dsky-enter-native.snapshots').open('rb') as b:
 for n,line in enumerate(a):
  assert line==b.readline();h.update(line)
  if n in numbers:rows[n]=list(map(int,line.split()))[2:]
 assert n==8191 and not b.readline()
patch=[];offsets=[0]
for n in numbers:
 patch.extend((i,v)for i,(v,b)in enumerate(zip(rows[n],before[n]))if v!=b);offsets.append(len(patch))
text=['/* SPDX-License-Identifier: MIT',' * Complete original both-program ENTER pass selection images.',' * Full snapshot SHA256 '+h.hexdigest()+' */','#ifndef DSKY_ENTER_VECTORS_H','#define DSKY_ENTER_VECTORS_H','#include "dsky_enter_cases.h"','static const uint16_t dsky_enter_numbers[]={'+','.join(map(str,numbers))+'};','static const uint16_t dsky_enter_offsets[]={'+','.join(map(str,offsets))+'};','static const uint16_t dsky_enter_patch[][2]={'];text+=[' {'+str(i)+','+str(v)+'},'for i,v in patch];text+=['};','#endif',''];(R/'tests/dsky_enter_vectors.h').write_text('\n'.join(text));print('Frozen64 complete original2053-field LM+CM ENTER states, patches',len(patch))
