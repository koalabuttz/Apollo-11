#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,hashlib,subprocess
R=pathlib.Path(__file__).resolve().parents[1]
subprocess.run(['cc','-O2','-std=c99','-Iinclude','-Itests','src/agc_word.c','tests/p66_bias_cases.c','tests/dsky_charin_release_cases.c','tools/dsky_charin_release_inputs.c','-o','build/dsky-charin-release-inputs'],cwd=R,check=True)
before=[list(map(int,x.split()))[2:]for x in subprocess.check_output(['./build/dsky-charin-release-inputs'],cwd=R,text=True).splitlines()]
a=(R/'build/dsky-charin-release-reference.snapshots').read_bytes();assert a==(R/'build/dsky-charin-release-native.snapshots').read_bytes();rows=[list(map(int,x.split()))[2:]for x in a.decode().splitlines()];assert len(rows)==len(before)==8192
numbers=[base+64*j+((j//4)%4)*16+parity for base in [0,4096] for j in range(16) for parity in range(2)]
# Explicit active/dormant, pending-priority, unsuspend and -0 fallthrough cases.
numbers[8:16]=[65,80,28,48,81,96,112,117]
numbers[40:48]=[4161,4176,4124,4144,4177,4192,4208,4213]
patch=[];offsets=[0]
for n in numbers:
 patch.extend((i,v)for i,(v,b)in enumerate(zip(rows[n],before[n]))if v!=b);offsets.append(len(patch))
text=['/* SPDX-License-Identifier: MIT',' * Complete original both-program CHARIN KEYREL source images.',' * Full snapshot SHA256 '+hashlib.sha256(a).hexdigest()+' */','#ifndef DSKY_CHARIN_RELEASE_VECTORS_H','#define DSKY_CHARIN_RELEASE_VECTORS_H','#include "dsky_charin_release_cases.h"','static const uint16_t dsky_charin_release_numbers[]={'+','.join(map(str,numbers))+'};','static const uint16_t dsky_charin_release_offsets[]={'+','.join(map(str,offsets))+'};','static const uint16_t dsky_charin_release_patch[][2]={'];text+=[' {'+str(i)+','+str(v)+'},'for i,v in patch];text+=['};','#endif',''];(R/'tests/dsky_charin_release_vectors.h').write_text('\n'.join(text));print('Frozen64 complete original2053-field LM+CM sign states, patches',len(patch))
