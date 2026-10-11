#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,hashlib,subprocess,json
R=pathlib.Path(__file__).resolve().parents[1]
subprocess.run(['cc','-O2','-std=c99','-Iinclude','-Itests','src/agc_word.c','tests/p66_bias_cases.c','tests/dsky_operator_release_cases.c','tools/dsky_operator_release_inputs.c','-o','build/dsky-operator-release-inputs'],cwd=R,check=True)
rows={};before={};tags={};h=hashlib.sha256()
process=subprocess.Popen(['./build/dsky-operator-release-inputs'],cwd=R,stdout=subprocess.PIPE)
for n,line in enumerate(process.stdout):before[n]=list(map(int,line.split()))[2:]
assert process.wait()==0 and n==8191
with (R/'build/dsky-operator-release-reference.snapshots').open('rb')as a,(R/'build/dsky-operator-release-native.snapshots').open('rb')as b:
 for n,line in enumerate(a):
  assert line==b.readline();h.update(line);rows[n]=list(map(int,line.split()))[2:];v=before[n];o=rows[n]
  tags[n]={('pc',v[5]),('EB',v[3]),('SUPER',v[2048]),('IRQ',v[2049]),('A',v[0]),('L',v[1]),('Q',v[2]),('route',o[2055]),('restart',v[2054]),('CADRSTOR',v[0o1042]),('DSPLIST',v[0o1043]),('saved-lock',v[0o115]),('monitor',bool(v[0o1021]&0o20000)),('core0',v[0o167])}
 assert n==8191 and not b.readline()
numbers=[];audit={}
for mission in range(2):
 candidates=list(range(mission*4096,(mission+1)*4096));remaining=set().union(*(tags[n]for n in candidates));selected=[]
 while remaining:
  n=max(candidates,key=lambda n:(len(tags[n]&remaining),-n));assert tags[n]&remaining;selected.append(n);remaining-=tags[n];candidates.remove(n)
 assert len(selected)<=32
 selected+=candidates[:32-len(selected)];numbers+=selected;audit['cm'if mission else'lm']={'numbers':selected,'features':sorted(map(str,set().union(*(tags[n]for n in selected))))}
patch=[];offsets=[0]
for n in numbers:patch.extend((i,v)for i,(v,b)in enumerate(zip(rows[n],before[n]))if v!=b);offsets.append(len(patch))
text=['/* SPDX-License-Identifier: MIT',' * Complete original both-program operator release images.',' * Full snapshot SHA256 '+h.hexdigest()+' */','#ifndef DSKY_OPERATOR_RELEASE_VECTORS_H','#define DSKY_OPERATOR_RELEASE_VECTORS_H','#include "dsky_operator_release_cases.h"','static const uint16_t dsky_operator_release_numbers[]={'+','.join(map(str,numbers))+'};','static const uint16_t dsky_operator_release_offsets[]={'+','.join(map(str,offsets))+'};','static const uint16_t dsky_operator_release_patch[][2]={'];text+=[' {'+str(i)+','+str(v)+'},'for i,v in patch];text+=['};','#endif',''];(R/'tests/dsky_operator_release_vectors.h').write_text('\n'.join(text));(R/'build/dsky-operator-release-frozen-coverage.json').write_text(json.dumps(audit,indent=2)+'\n');print('Frozen64 complete original2056-field LM+CM lamp states, patches',len(patch))
