#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Freeze raw constructors and original complete2052word checkpoints for C64."""
import pathlib,subprocess,hashlib
root=pathlib.Path(__file__).resolve().parents[1]
inputs=[f'tests/{n}_cases.c'for n in('prepare','scale','push','elapsed','store','divide','gravity','velocity','radial','unit','trig','projection','p66_guidance_live','p66_descent_live','p66_complete_live')]
defines=['-D'+n+'_INPUT_ONLY'for n in('PREPARE','SCALE','PUSH','ELAPSED','STORE','DIVIDE','GRAVITY','VELOCITY','RADIAL','UNIT','TRIG','PROJECTION')]
subprocess.run(['gcc','-std=c99','-O2','-ffunction-sections','-fdata-sections','-Wl,--gc-sections','-Iinclude','-Itests','-DP66_NATIVE_DISPATCH_ONLY',*defines,'src/agc_word.c','src/agc_multiword.c','src/executive.c','src/p66_native.c','tools/p66_complete_live_inputs.c',*inputs,'-o','build/p66-complete-live-inputs'],cwd=root,check=True)
initial=subprocess.check_output(['build/p66-complete-live-inputs'],cwd=root,text=True);(root/'build/p66-complete-live-inputs.snapshots').write_text(initial)
source=root/'build/p66-complete-live-reference-raw.snapshots';assert hashlib.sha256(source.read_bytes()).hexdigest()=='9480366d0d7203683dd1d35ae06736dca9ef93646fd795bf210d405ae6689c52'
rows=[list(map(int,l.split()))for l in source.read_text().splitlines()];raw=[list(map(int,l.split()))for l in initial.splitlines()]
header=['/* SPDX-License-Identifier: MIT',f' * Raw constructor SHA256 {hashlib.sha256(initial.encode()).hexdigest()} */','#ifndef P66_COMPLETE_LIVE_INPUTS_H','#define P66_COMPLETE_LIVE_INPUTS_H','static const uint16_t pcl_initial[2][2052]={']
header+=['{'+','.join(map(str,r[1:]))+'},'for r in raw];header+=['};','#endif',''];(root/'tests/p66_complete_live_inputs.h').write_text('\n'.join(header))
def fingerprint(words):
 h=2166136261
 for w in words:
  for byte in(w&255,w>>8):h=((h^byte)*16777619)&0xffffffff
 return h
seq=[[r for r in rows if r[0]==n]for n in(0,1)];counts=list(map(len,seq));maxlen=max(counts)
header=['/* SPDX-License-Identifier: MIT',f' * Original2052word checkpoint SHA256 {hashlib.sha256(source.read_bytes()).hexdigest()} */','#ifndef P66_COMPLETE_LIVE_VECTORS_H','#define P66_COMPLETE_LIVE_VECTORS_H',f'static const uint8_t pcl_counts[2]={{{counts[0]},{counts[1]}}};',f'static const uint16_t pcl_locations[2][{maxlen}]={{']
header+=['{'+','.join(str(r[2])for r in s)+'},'for s in seq];header+=['};',f'static const uint32_t pcl_fingerprints[2][{maxlen}]={{']
header+=['{'+','.join('UINT32_C(%d)'%fingerprint(r[3:])for r in s)+'},'for s in seq];header+=['};','#endif',''];(root/'tests/p66_complete_live_vectors.h').write_text('\n'.join(header))
print('Frozen2 raw sourceconstructor images and',sum(counts),'complete2052word source checkpoint fingerprints')
