#!/usr/bin/env python3
"""New evidence for exact WTLST5 CCS zeros, preserving retained old proofs."""
import pathlib,subprocess,hashlib,sys
root=pathlib.Path(__file__).resolve().parents[1]; fixture=pathlib.Path(sys.argv[1]).resolve(); engine=root/'deps/virtualagc/yaAGC'
assert (root.parent/'Luminary099/WAITLIST.agc').read_bytes()==(fixture/'WAITLIST.agc').read_bytes()
subprocess.run(['cc','-std=c99','-O2','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests','src/agc_word.c','src/waitlist.c','tests/waitlist_zero_cases.c','tools/waitlist_zero_snapshot.c','-o','build/waitlist-zero-snapshot'],cwd=root,check=True)
subprocess.run(['gcc','-std=c99','-O2','-I'+str(engine),'-Iinclude','-Itests','tools/waitlist_zero_reference.c','tests/scenarios.c','tests/waitlist_zero_cases.c',str(engine/'agc_engine.o'),str(engine/'agc_engine_init.o'),str(engine/'rfopen.o'),'-o','build/waitlist-zero-reference'],cwd=root,check=True)
for name,cmd in [('native',['./build/waitlist-zero-snapshot']),('reference',['./build/waitlist-zero-reference',str(fixture/'MAIN.agc.bin')])]:
 with (root/f'build/waitlist-zero-{name}.snapshots').open('w') as out,(root/f'build/waitlist-zero-{name}.trace').open('w') as err: subprocess.run(cmd,cwd=root,stdout=out,stderr=err,check=True)
a=(root/'build/waitlist-zero-native.snapshots').read_bytes();b=(root/'build/waitlist-zero-reference.snapshots').read_bytes()
if a!=b:
 for x,y in zip(a.decode().splitlines(),b.decode().splitlines()):
  if x!=y: print(x,y,sep='\n'); break
 sys.exit('FAIL: Waitlist zero differential')
rows=[list(map(int,l.split())) for l in b.decode().splitlines()]; assert len(rows)==256 and all(len(r)==31 for r in rows)
lines=['/* SPDX-License-Identifier: MIT',' * Unchanged original WAITLIST zero-route full states. SHA256 '+hashlib.sha256(b).hexdigest()+' */','#ifndef WAITLIST_ZERO_VECTORS_H','#define WAITLIST_ZERO_VECTORS_H','#include "waitlist_zero_cases.h"','static const agc_word wl_zero_expected[256][30]={']
lines+=[' {'+','.join(map(str,r[1:]))+'},' for r in rows]; lines+=['};','#endif','']
(root/'tests/waitlist_zero_vectors.h').write_text('\n'.join(lines))
print('PASS: 256 complete 29-word original WAITLIST checkpoints, all eight WTLST5 CCS positions x four sign/zero classes x two tail states x four BBCONs')
print('Snapshot SHA256:',hashlib.sha256(b).hexdigest())
