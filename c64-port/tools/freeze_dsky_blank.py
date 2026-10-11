#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,hashlib,subprocess
R=pathlib.Path(__file__).resolve().parents[1]
subprocess.run(['cc','-O2','-std=c99','-Iinclude','-Itests','src/agc_word.c','tests/p66_bias_cases.c','tests/dsky_blank_cases.c','tools/dsky_blank_inputs.c','-o','build/dsky-blank-inputs'],cwd=R,check=True)
before=[list(map(int,x.split()))[2:]for x in subprocess.check_output(['./build/dsky-blank-inputs'],cwd=R,text=True).splitlines()]
a=(R/'build/dsky-blank-reference.snapshots').read_bytes();assert a==(R/'build/dsky-blank-native.snapshots').read_bytes();rows=[list(map(int,x.split()))[2:]for x in a.decode().splitlines()];assert len(rows)==len(before)==8192
numbers=[base+56*j+(j%8) for base in [0,2048,4096,6144] for j in range(16)]
patch=[];offsets=[0]
for n in numbers:
 patch.extend((i,v)for i,(v,b)in enumerate(zip(rows[n],before[n]))if v!=b);offsets.append(len(patch))
text=['/* SPDX-License-Identifier: MIT',' * Complete original both-program BLNKSUB1/bank return source images.',' * Full snapshot SHA256 '+hashlib.sha256(a).hexdigest()+' */','#ifndef DSKY_BLANK_VECTORS_H','#define DSKY_BLANK_VECTORS_H','#include "dsky_blank_cases.h"','static const uint16_t dsky_blank_numbers[]={'+','.join(map(str,numbers))+'};','static const uint16_t dsky_blank_offsets[]={'+','.join(map(str,offsets))+'};','static const uint16_t dsky_blank_patch[][2]={'];text+=[' {'+str(i)+','+str(v)+'},'for i,v in patch];text+=['};','#endif',''];(R/'tests/dsky_blank_vectors.h').write_text('\n'.join(text));print('Frozen64 complete original2050-field LM+CM blanking/return states, patches',len(patch))
