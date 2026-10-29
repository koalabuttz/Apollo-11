#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import ast,hashlib,subprocess
from pathlib import Path
root=Path(__file__).resolve().parents[1];scope={'root':root};names={'inputs','defines'}
for node in ast.parse((root/'tools/p66_complete_live_differential.py').read_text()).body:
 if isinstance(node,ast.Assign) and all(isinstance(t,ast.Name) and t.id in names for t in node.targets):exec(compile(ast.Module(body=[node],type_ignores=[]),'source-input-inventory','exec'),scope)
subprocess.run(['gcc','-std=c99','-O2','-ffunction-sections','-fdata-sections','-Wl,--gc-sections','-Iinclude','-Itests','-DP66_NATIVE_DISPATCH_ONLY']+scope['defines']+['src/agc_word.c','src/agc_multiword.c','src/executive.c','src/p66_native.c','tools/p66_application_live_inputs.c','tests/p66_application_live_cases.c']+scope['inputs']+['-o','build/p66-application-live-inputs'],cwd=root,check=True)
initial=subprocess.check_output(['build/p66-application-live-inputs'],cwd=root,text=True);(root/'build/p66-application-live-inputs.snapshots').write_text(initial)
raw=[list(map(int,l.split()))for l in initial.splitlines()];source=root/'build/p66-application-live-reference.snapshots';rows=[list(map(int,l.split()))for l in source.read_text().splitlines()]
header=['/* SPDX-License-Identifier: MIT */','#ifndef P66_APPLICATION_LIVE_INPUTS_H','#define P66_APPLICATION_LIVE_INPUTS_H','static const uint16_t pal_initial[2][2052]={']+['{'+','.join(map(str,r[1:]))+'},'for r in raw]+['};','#endif',''];(root/'tests/p66_application_live_inputs.h').write_text('\n'.join(header))
def fingerprint(words):
 h=2166136261
 for w in words:
  for byte in(w&255,w>>8):h=((h^byte)*16777619)&0xffffffff
 return h
seq=[[r for r in rows if r[0]==n]for n in(0,1)];counts=list(map(len,seq));maximum=max(counts)
header=['/* SPDX-License-Identifier: MIT',f' * Source SHA256 {hashlib.sha256(source.read_bytes()).hexdigest()} */','#ifndef P66_APPLICATION_LIVE_VECTORS_H','#define P66_APPLICATION_LIVE_VECTORS_H',f'static const uint8_t pal_counts[2]={{{counts[0]},{counts[1]}}};',f'static const uint16_t pal_locations[2][{maximum}]={{']+['{'+','.join(str(r[2])for r in s)+'},'for s in seq]+['};',f'static const uint32_t pal_fingerprints[2][{maximum}]={{']+['{'+','.join('UINT32_C(%d)'%fingerprint(r[3:])for r in s)+'},'for s in seq]+['};','#endif',''];(root/'tests/p66_application_live_vectors.h').write_text('\n'.join(header));print('Frozen2 startup inputs and',sum(counts),'raw2052-word original checkpoints')
