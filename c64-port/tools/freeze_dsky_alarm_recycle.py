#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,hashlib,subprocess
R=pathlib.Path(__file__).resolve().parents[1]
subprocess.run(['cc','-O2','-std=c99','-Iinclude','-Itests','src/agc_word.c','tests/p66_bias_cases.c','tests/dsky_alarm_recycle_cases.c','tools/dsky_alarm_recycle_inputs.c','-o','build/dsky-alarm-recycle-inputs'],cwd=R,check=True)
numbers=[0, 416, 604, 1918, 201, 304, 3735, 992, 707, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 4096, 4543, 4847, 4949, 6440, 4241, 5744, 5056, 4141, 4644, 4097, 4098, 4099, 4100, 4101, 4102, 4103, 4104, 4105, 4106, 4107, 4108, 4109, 4110, 4111, 4112, 4113, 4114, 4115, 4116, 4117, 4118]
before={};rows={};h=hashlib.sha256()
process=subprocess.Popen(['./build/dsky-alarm-recycle-inputs'],cwd=R,stdout=subprocess.PIPE)
for n,line in enumerate(process.stdout):
 if n in numbers:before[n]=list(map(int,line.split()))[2:]
assert process.wait()==0 and n==8191
with (R/'build/dsky-alarm-recycle-reference.snapshots').open('rb') as a,(R/'build/dsky-alarm-recycle-native.snapshots').open('rb') as b:
 for n,line in enumerate(a):
  assert line==b.readline();h.update(line)
  if n in numbers:rows[n]=list(map(int,line.split()))[2:]
 assert n==8191 and not b.readline()
patch=[];offsets=[0]
for n in numbers:
 patch.extend((i,v)for i,(v,b)in enumerate(zip(rows[n],before[n]))if v!=b);offsets.append(len(patch))
text=['/* SPDX-License-Identifier: MIT',' * Complete original both-program continuous alarm recycle full exchange images.',' * Full snapshot SHA256 '+h.hexdigest()+' */','#ifndef DSKY_ALARM_RECYCLE_VECTORS_H','#define DSKY_ALARM_RECYCLE_VECTORS_H','#include "dsky_alarm_recycle_cases.h"','static const uint16_t dsky_alarm_recycle_numbers[]={'+','.join(map(str,numbers))+'};','static const uint16_t dsky_alarm_recycle_offsets[]={'+','.join(map(str,offsets))+'};','static const uint16_t dsky_alarm_recycle_patch[][2]={'];text+=[' {'+str(i)+','+str(v)+'},'for i,v in patch];text+=['};','#endif',''];(R/'tests/dsky_alarm_recycle_vectors.h').write_text('\n'.join(text));print('Frozen64 complete original2053-field LM+CM continuous alarm recycle states, patches',len(patch))
