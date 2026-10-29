#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,hashlib,subprocess
R=pathlib.Path(__file__).resolve().parents[1]
subprocess.run(['cc','-O2','-std=c99','-Iinclude','-Itests','src/agc_word.c','tests/p66_bias_cases.c','tests/dsky_address_request_cases.c','tools/dsky_address_request_inputs.c','-o','build/dsky-address-request-inputs'],cwd=R,check=True)
numbers=[0, 400, 584, 800, 186, 222, 356, 496, 1, 3, 4, 5, 6, 7, 10, 11, 18, 19, 20, 33, 35, 38, 2, 8, 9, 12, 13, 14, 15, 16, 17, 21, 4096, 4497, 4704, 4904, 4345, 4552, 4276, 4432, 4099, 4100, 4102, 4109, 4128, 4129, 4133, 4134, 4136, 4140, 4141, 4142, 4143, 4147, 4097, 4098, 4101, 4103, 4104, 4105, 4106, 4107, 4108, 4110]
before={};rows={};h=hashlib.sha256()
process=subprocess.Popen(['./build/dsky-address-request-inputs'],cwd=R,stdout=subprocess.PIPE)
for n,line in enumerate(process.stdout):
 if n in numbers:before[n]=list(map(int,line.split()))[2:]
assert process.wait()==0 and n==8191
with (R/'build/dsky-address-request-reference.snapshots').open('rb') as a,(R/'build/dsky-address-request-native.snapshots').open('rb') as b:
 for n,line in enumerate(a):
  assert line==b.readline();h.update(line)
  if n in numbers:rows[n]=list(map(int,line.split()))[2:]
 assert n==8191 and not b.readline()
patch=[];offsets=[0]
for n in numbers:
 patch.extend((i,v)for i,(v,b)in enumerate(zip(rows[n],before[n]))if v!=b);offsets.append(len(patch))
text=['/* SPDX-License-Identifier: MIT',' * Complete original both-program address request full exchange images.',' * Full snapshot SHA256 '+h.hexdigest()+' */','#ifndef DSKY_ADDRESS_REQUEST_VECTORS_H','#define DSKY_ADDRESS_REQUEST_VECTORS_H','#include "dsky_address_request_cases.h"','static const uint16_t dsky_address_request_numbers[]={'+','.join(map(str,numbers))+'};','static const uint16_t dsky_address_request_offsets[]={'+','.join(map(str,offsets))+'};','static const uint16_t dsky_address_request_patch[][2]={'];text+=[' {'+str(i)+','+str(v)+'},'for i,v in patch];text+=['};','#endif',''];(R/'tests/dsky_address_request_vectors.h').write_text('\n'.join(text));print('Frozen64 complete original2053-field LM+CM address request states, patches',len(patch))
