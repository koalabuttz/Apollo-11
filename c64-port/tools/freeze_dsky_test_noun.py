#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,subprocess,lzma
R=pathlib.Path(__file__).resolve().parents[1]
subprocess.run(['cc','-O2','-Iinclude','-Itests','tests/dsky_test_noun_cases.c','tools/dsky_test_noun_inputs.c','-o','build/dsky-test-noun-inputs'],cwd=R,check=True)
numbers=[m*8192+j%101+101*(j//101)*64 for m in range(2)for j in range(128)];inputs={int(row.split()[0]):list(map(int,row.split()))[1:]+[99]for row in subprocess.check_output(['build/dsky-test-noun-inputs']+list(map(str,numbers)),cwd=R,text=True).splitlines()};outputs={}
with lzma.open(R/'build/dsky-test-noun-reference.snapshots.xz','rt')as f:
 for line in f:
  n=int(line.split(' ',1)[0])
  if n in inputs:outputs[n]=list(map(int,line.split()))[1:]
assert len(outputs)==256
patch=[];offset=[0]
for n in numbers:patch.extend((i,v)for i,(v,base)in enumerate(zip(outputs[n],inputs[n]))if v!=base);offset.append(len(patch))
(R/'tests/dsky_test_noun_vectors.h').write_text('\n'.join(['/* SPDX-License-Identifier: MIT */','#define DSKY_TEST_NOUN_FROZEN_COUNT 256u','static const uint16_t dsky_test_noun_numbers[]={'+','.join(map(str,numbers))+'};','static const uint16_t dsky_test_noun_offsets[]={'+','.join(map(str,offset))+'};','static const uint16_t dsky_test_noun_patch[][2]={']+[' {'+str(i)+','+str(v)+'},'for i,v in patch]+['};','']))
print('256 full original states',len(patch),'patches')
