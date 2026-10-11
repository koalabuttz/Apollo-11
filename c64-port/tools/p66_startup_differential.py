#!/usr/bin/env python3
"""Compare three native P66 source entries against unchanged Luminary099."""
import hashlib,itertools,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[1]; fixture=pathlib.Path(sys.argv[1]).resolve(); engine=root/'deps/virtualagc/yaAGC'
for f in ['LUNAR_LANDING_GUIDANCE_EQUATIONS.agc','EXECUTIVE.agc','WAITLIST.agc','PHASE_TABLE_MAINTENANCE.agc']:
 assert (root.parent/'Luminary099'/f).read_bytes()==(fixture/f).read_bytes()
subprocess.run(['cc','-std=c99','-O2','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests','src/agc_word.c','src/executive.c','src/waitlist.c','src/phase.c','src/p66_startup.c','tests/p66_startup_cases.c','tools/p66_startup_snapshot.c','-o','build/p66-startup-snapshot'],cwd=root,check=True)
subprocess.run(['gcc','-std=c99','-O2','-I'+str(engine),'-Iinclude','-Itests','-DP66_STARTUP_INPUT_ONLY','tools/p66_startup_reference.c','tests/scenarios.c','tests/p66_startup_cases.c',str(engine/'agc_engine.o'),str(engine/'agc_engine_init.o'),str(engine/'rfopen.o'),'-o','build/p66-startup-reference'],cwd=root,check=True)
for name,cmd in [('native',['./build/p66-startup-snapshot']),('reference',['./build/p66-startup-reference',str(fixture/'MAIN.agc.bin')])]:
 with (root/f'build/p66-startup-{name}.snapshots').open('w') as out,(root/f'build/p66-startup-{name}.trace').open('w') as err:
  run=subprocess.run(cmd,cwd=root,stdout=out,stderr=err)
 if run.returncode: sys.exit((root/f'build/p66-startup-{name}.trace').read_text()[-3000:])
errors=[]; n=0
with (root/'build/p66-startup-native.snapshots').open() as native,(root/'build/p66-startup-reference.snapshots').open() as original:
 for n,(a,b) in enumerate(itertools.zip_longest(native,original),1):
  assert a is not None and b is not None, 'Native/reference row count differs'
  av=list(map(int,a.split())); bv=list(map(int,b.split())); assert len(av)==len(bv)
  if av!=bv:
   for field,(x,y) in enumerate(zip(av,bv)):
    if x!=y and len(errors)<80: errors.append(f'case {av[0]} field {field}: {x} != {y}')
 assert n==73728
if errors: sys.exit('\n'.join(errors))
print('PASS: 73,728 original source-entry checkpoints: 65,536 DESCBITS, 4,096 RODTASK, 4,096 P66VERTA; every modeled word compared')
print('Boundaries RESUME/TASKOVER/RODCOMP or BAILOUT1; no STARTP66/vector bias/display completion claim')
print('Snapshot SHA256:',hashlib.sha256((root/'build/p66-startup-reference.snapshots').read_bytes()).hexdigest())
