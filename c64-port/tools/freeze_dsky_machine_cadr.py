#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,hashlib,subprocess
R=pathlib.Path(__file__).resolve().parents[1]
subprocess.run(['cc','-O2','-std=c99','-Iinclude','-Itests','src/agc_word.c','tests/p66_bias_cases.c','tests/dsky_machine_cadr_cases.c','tools/dsky_machine_cadr_inputs.c','-o','build/dsky-machine-cadr-inputs'],cwd=R,check=True)
numbers=[base+16*eb+offset for base in [0,4096] for eb in range(8) for offset in [0,129,258,391]]
before={};rows={};h=hashlib.sha256()
process=subprocess.Popen(['./build/dsky-machine-cadr-inputs'],cwd=R,stdout=subprocess.PIPE)
for n,line in enumerate(process.stdout):
 if n in numbers:before[n]=list(map(int,line.split()))[2:]
assert process.wait()==0 and n==8191
with (R/'build/dsky-machine-cadr-reference.snapshots').open('rb') as a,(R/'build/dsky-machine-cadr-native.snapshots').open('rb') as b:
 for n,line in enumerate(a):
  assert line==b.readline();h.update(line)
  if n in numbers:rows[n]=list(map(int,line.split()))[2:]
 assert n==8191 and not b.readline()
patch=[];offsets=[0]
for n in numbers:
 patch.extend((i,v)for i,(v,b)in enumerate(zip(rows[n],before[n]))if v!=b);offsets.append(len(patch))
text=['/* SPDX-License-Identifier: MIT',' * Complete original both-program machine-address display full exchange images.',' * Full snapshot SHA256 '+h.hexdigest()+' */','#ifndef DSKY_MACHINE_CADR_VECTORS_H','#define DSKY_MACHINE_CADR_VECTORS_H','#include "dsky_machine_cadr_cases.h"','static const uint16_t dsky_machine_cadr_numbers[]={'+','.join(map(str,numbers))+'};','static const uint16_t dsky_machine_cadr_offsets[]={'+','.join(map(str,offsets))+'};','static const uint16_t dsky_machine_cadr_patch[][2]={'];text+=[' {'+str(i)+','+str(v)+'},'for i,v in patch];text+=['};','#endif',''];(R/'tests/dsky_machine_cadr_vectors.h').write_text('\n'.join(text));print('Frozen64 complete original2051-field LM+CM machine-address display states, patches',len(patch))
