#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,hashlib,subprocess
R=pathlib.Path(__file__).resolve().parents[1]
subprocess.run(['cc','-O2','-std=c99','-Iinclude','-Itests','src/agc_word.c','tests/p66_bias_cases.c','tests/dsky_request_control_cases.c','tools/dsky_request_control_inputs.c','-o','build/dsky-request-control-inputs'],cwd=R,check=True)
numbers=[0, 393, 594, 799, 163, 236, 372, 61, 134, 449, 4, 42, 1, 2, 3, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 4096, 4497, 4682, 4895, 4259, 4332, 4468, 4157, 4166, 4244, 4553, 4097, 4098, 4099, 4100, 4101, 4102, 4103, 4104, 4105, 4106, 4107, 4108, 4109, 4110, 4111, 4112, 4113, 4114, 4115, 4116, 4117]
before={};rows={};h=hashlib.sha256()
process=subprocess.Popen(['./build/dsky-request-control-inputs'],cwd=R,stdout=subprocess.PIPE)
for n,line in enumerate(process.stdout):
 if n in numbers:before[n]=list(map(int,line.split()))[2:]
assert process.wait()==0 and n==8191
with (R/'build/dsky-request-control-reference.snapshots').open('rb') as a,(R/'build/dsky-request-control-native.snapshots').open('rb') as b:
 for n,line in enumerate(a):
  assert line==b.readline();h.update(line)
  if n in numbers:rows[n]=list(map(int,line.split()))[2:]
 assert n==8191 and not b.readline()
patch=[];offsets=[0]
for n in numbers:
 patch.extend((i,v)for i,(v,b)in enumerate(zip(rows[n],before[n]))if v!=b);offsets.append(len(patch))
text=['/* SPDX-License-Identifier: MIT',' * Complete original both-program operator request control full exchange images.',' * Full snapshot SHA256 '+h.hexdigest()+' */','#ifndef DSKY_REQUEST_CONTROL_VECTORS_H','#define DSKY_REQUEST_CONTROL_VECTORS_H','#include "dsky_request_control_cases.h"','static const uint16_t dsky_request_control_numbers[]={'+','.join(map(str,numbers))+'};','static const uint16_t dsky_request_control_offsets[]={'+','.join(map(str,offsets))+'};','static const uint16_t dsky_request_control_patch[][2]={'];text+=[' {'+str(i)+','+str(v)+'},'for i,v in patch];text+=['};','#endif',''];(R/'tests/dsky_request_control_vectors.h').write_text('\n'.join(text));print('Frozen64 complete original2053-field LM+CM operator request control states, patches',len(patch))
