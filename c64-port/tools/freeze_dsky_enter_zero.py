#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,hashlib,subprocess
R=pathlib.Path(__file__).resolve().parents[1]
subprocess.run(['cc','-O2','-std=c99','-Iinclude','-Itests','src/agc_word.c','tests/p66_bias_cases.c','tests/dsky_enter_zero_cases.c','tools/dsky_enter_zero_inputs.c','-o','build/dsky-enter-zero-inputs'],cwd=R,check=True)
pairs=[(0,n) for n in [0,1,4,10,11,20,27,28,30,35,38,39,40,41,42,60,85,99,100]]+[(1,35),(2,40),(3,60),(4,10),(5,0),(5,100),(6,33),(7,77),(8,100),(8,40),(8,4),(8,1),(8,99)]
numbers=[]
for program in [0,4096]:
 for slot,noun in pairs:
  base=program+((slot-program//128)%9)*128;numbers.append(base+(noun-base)%101)
before={};rows={};h=hashlib.sha256()
process=subprocess.Popen(['./build/dsky-enter-zero-inputs'],cwd=R,stdout=subprocess.PIPE)
for n,line in enumerate(process.stdout):
 if n in numbers:before[n]=list(map(int,line.split()))[2:]
assert process.wait()==0 and n==8191
with (R/'build/dsky-enter-zero-reference.snapshots').open('rb') as a,(R/'build/dsky-enter-zero-native.snapshots').open('rb') as b:
 for n,line in enumerate(a):
  assert line==b.readline();h.update(line)
  if n in numbers:rows[n]=list(map(int,line.split()))[2:]
 assert n==8191 and not b.readline()
patch=[];offsets=[0]
for n in numbers:
 patch.extend((i,v)for i,(v,b)in enumerate(zip(rows[n],before[n]))if v!=b);offsets.append(len(patch))
text=['/* SPDX-License-Identifier: MIT',' * Complete original both-program ENTPAS0 noun admission images.',' * Full snapshot SHA256 '+h.hexdigest()+' */','#ifndef DSKY_ENTER_ZERO_VECTORS_H','#define DSKY_ENTER_ZERO_VECTORS_H','#include "dsky_enter_zero_cases.h"','static const uint16_t dsky_enter_zero_numbers[]={'+','.join(map(str,numbers))+'};','static const uint16_t dsky_enter_zero_offsets[]={'+','.join(map(str,offsets))+'};','static const uint16_t dsky_enter_zero_patch[][2]={'];text+=[' {'+str(i)+','+str(v)+'},'for i,v in patch];text+=['};','#endif',''];(R/'tests/dsky_enter_zero_vectors.h').write_text('\n'.join(text));print('Frozen64 complete original2051-field LM+CM ENTPAS0 states, patches',len(patch))
