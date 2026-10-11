#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,hashlib,subprocess
R=pathlib.Path(__file__).resolve().parents[1]
subprocess.run(['cc','-O2','-std=c99','-Iinclude','-Itests','src/agc_word.c','tests/p66_bias_cases.c','tests/dsky_charin_alarm_cases.c','tools/dsky_charin_alarm_inputs.c','-o','build/dsky-charin-alarm-inputs'],cwd=R,check=True)
before=[list(map(int,x.split()))[2:]for x in subprocess.check_output(['./build/dsky-charin-alarm-inputs'],cwd=R,text=True).splitlines()]
a=(R/'build/dsky-charin-alarm-reference.snapshots').read_bytes();assert a==(R/'build/dsky-charin-alarm-native.snapshots').read_bytes();rows=[list(map(int,x.split()))[2:]for x in a.decode().splitlines()];assert len(rows)==len(before)==8192
numbers=[base+65*j for base in [0,4096] for j in range(32)]
numbers[16]=96
numbers[48]=4352
patch=[];offsets=[0]
for n in numbers:
 patch.extend((i,v)for i,(v,b)in enumerate(zip(rows[n],before[n]))if v!=b);offsets.append(len(patch))
text=['/* SPDX-License-Identifier: MIT',' * Complete original both-program CHARIN invalid-key source images.',' * Full snapshot SHA256 '+hashlib.sha256(a).hexdigest()+' */','#ifndef DSKY_CHARIN_ALARM_VECTORS_H','#define DSKY_CHARIN_ALARM_VECTORS_H','#include "dsky_charin_alarm_cases.h"','static const uint16_t dsky_charin_alarm_numbers[]={'+','.join(map(str,numbers))+'};','static const uint16_t dsky_charin_alarm_offsets[]={'+','.join(map(str,offsets))+'};','static const uint16_t dsky_charin_alarm_patch[][2]={'];text+=[' {'+str(i)+','+str(v)+'},'for i,v in patch];text+=['};','#endif',''];(R/'tests/dsky_charin_alarm_vectors.h').write_text('\n'.join(text));print('Frozen64 complete original2052-field LM+CM sign states, patches',len(patch))
