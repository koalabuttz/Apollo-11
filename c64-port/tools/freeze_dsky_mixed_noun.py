#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,hashlib,subprocess
R=pathlib.Path(__file__).resolve().parents[1]
subprocess.run(['cc','-O2','-std=c99','-Iinclude','-Itests','src/agc_word.c','tests/p66_bias_cases.c','tests/dsky_mixed_noun_cases.c','tools/dsky_mixed_noun_inputs.c','-o','build/dsky-mixed-noun-inputs'],cwd=R,check=True)
numbers=[953,1010,196,2749,1206,982,168,2721,1907,364,2917,869,1150,336,112,841,1851,308,84,813,365,870,0,32,46,64,78,96,128,299,594,595,4825,4601,4516,5611,4797,4573,6621,5583,5498,4545,4460,5555,8108,4517,4432,5751,4713,5442,4404,5499,4685,4461,5695,5471,4096,4128,4160,4192,4224,4395,4690,4691]
before={};rows={};h=hashlib.sha256()
process=subprocess.Popen(['./build/dsky-mixed-noun-inputs'],cwd=R,stdout=subprocess.PIPE)
for n,line in enumerate(process.stdout):
 if n in numbers:before[n]=list(map(int,line.split()))[2:]
assert process.wait()==0 and n==8191
with (R/'build/dsky-mixed-noun-reference.snapshots').open('rb') as a,(R/'build/dsky-mixed-noun-native.snapshots').open('rb') as b:
 for n,line in enumerate(a):
  assert line==b.readline();h.update(line)
  if n in numbers:rows[n]=list(map(int,line.split()))[2:]
 assert n==8191 and not b.readline()
patch=[];offsets=[0]
for n in numbers:
 patch.extend((i,v)for i,(v,b)in enumerate(zip(rows[n],before[n]))if v!=b);offsets.append(len(patch))
text=['/* SPDX-License-Identifier: MIT',' * Complete original both-program MIXNOUN full exchange images.',' * Full snapshot SHA256 '+h.hexdigest()+' */','#ifndef DSKY_MIXED_NOUN_VECTORS_H','#define DSKY_MIXED_NOUN_VECTORS_H','#include "dsky_mixed_noun_cases.h"','static const uint16_t dsky_mixed_noun_numbers[]={'+','.join(map(str,numbers))+'};','static const uint16_t dsky_mixed_noun_offsets[]={'+','.join(map(str,offsets))+'};','static const uint16_t dsky_mixed_noun_patch[][2]={'];text+=[' {'+str(i)+','+str(v)+'},'for i,v in patch];text+=['};','#endif',''];(R/'tests/dsky_mixed_noun_vectors.h').write_text('\n'.join(text));print('Frozen64 complete original2051-field LM+CM MIXNOUN states, patches',len(patch))
