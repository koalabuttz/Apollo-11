#!/usr/bin/env python3
"""Verify basic P66 initialization dependencies against original assembly."""
import pathlib,subprocess,hashlib,sys,shutil
root=pathlib.Path(__file__).resolve().parents[1];fixture=pathlib.Path(sys.argv[1]).resolve();engine=root/'deps/virtualagc/yaAGC'
assert (root.parent/'Luminary099/LUNAR_LANDING_GUIDANCE_EQUATIONS.agc').read_bytes()==(fixture/'LUNAR_LANDING_GUIDANCE_EQUATIONS.agc').read_bytes()
# DAP fixture upstream comments differ from the historical repository. Build a
# private fixture containing the byte-identical historical DAP source and prove
# its complete rope is unchanged; do not alter the retained shared fixture.
private=root/'build/p66-initialization-fixture'; private.mkdir(exist_ok=True)
for f in fixture.glob('*.agc'):shutil.copyfile(f,private/f.name)
shutil.copyfile(root.parent/'Luminary099/DAP_INTERFACE_SUBROUTINES.agc',private/'DAP_INTERFACE_SUBROUTINES.agc')
run=subprocess.run([str(root/'deps/virtualagc/yaYUL/yaYUL'),'MAIN.agc'],cwd=private,capture_output=True,text=True)
(root/'build/p66-initialization-assembly.log').write_text(run.stdout+run.stderr)
assert run.returncode==0, run.stdout+run.stderr
assert (private/'MAIN.agc.bin').read_bytes()==(fixture/'MAIN.agc.bin').read_bytes()
assert (private/'DAP_INTERFACE_SUBROUTINES.agc').read_bytes()==(root.parent/'Luminary099/DAP_INTERFACE_SUBROUTINES.agc').read_bytes()
fixture=private
subprocess.run(['cc','-O2','-std=c99','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests','src/p66_initialization.c','tests/p66_initialization_cases.c','tools/p66_initialization_snapshot.c','-o','build/p66-initialization-snapshot'],cwd=root,check=True)
subprocess.run(['gcc','-O2','-std=c99','-Iinclude','-Itests','-I'+str(engine),'tools/p66_initialization_reference.c','tests/scenarios.c','tests/p66_initialization_cases.c',str(engine/'agc_engine.o'),str(engine/'agc_engine_init.o'),str(engine/'rfopen.o'),'-o','build/p66-initialization-reference'],cwd=root,check=True)
for name,cmd in [('native',['./build/p66-initialization-snapshot']),('reference',['./build/p66-initialization-reference',str(fixture/'MAIN.agc.bin')])]:
 with (root/f'build/p66-initialization-{name}.snapshots').open('w') as out,(root/f'build/p66-initialization-{name}.trace').open('w') as err:subprocess.run(cmd,cwd=root,stdout=out,stderr=err,check=True)
a=(root/'build/p66-initialization-reference.snapshots').read_bytes();b=(root/'build/p66-initialization-native.snapshots').read_bytes()
if a!=b:
 for x,y in zip(a.decode().splitlines(),b.decode().splitlines()):
  if x!=y:print(x,y,sep='\n');break
 sys.exit('FAIL: initialization differential')
rows=[list(map(int,l.split())) for l in a.decode().splitlines()];assert len(rows)==1024 and all(len(r)==19 for r in rows)
selected=[rows[n//2*14+n%2] for n in range(128)]
lines=['/* SPDX-License-Identifier: MIT',' * Original STOPRATE/FASTCHNG full states SHA256 '+hashlib.sha256(a).hexdigest()+' */','#ifndef P66_INITIALIZATION_VECTORS_H','#define P66_INITIALIZATION_VECTORS_H','#include "p66_initialization_cases.h"','static const agc_word p66_init_expected[128][17]={']
lines+=[' {'+','.join(map(str,r[2:]))+'},' for r in selected];lines+=['};','#endif','']
(root/'tests/p66_initialization_vectors.h').write_text('\n'.join(lines))
print('PASS: 1,024 complete original STOPRATE/FASTCHNG checkpoints, all17words/registers/channels')
print('STOPRATE clears nine actual DAP interface words/restores EBANK; FASTCHNG stores original Q in PHSNAME3 restart location only')
print('Snapshot SHA256:',hashlib.sha256(a).hexdigest())
