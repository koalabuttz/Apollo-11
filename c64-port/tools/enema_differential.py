#!/usr/bin/env python3
"""Verify the complete bounded ENEMA prefix against unchanged original assembly."""
import hashlib, pathlib, subprocess, sys
root=pathlib.Path(__file__).resolve().parents[1]
fixture=pathlib.Path(sys.argv[1]).resolve(); engine=root/'deps/virtualagc/yaAGC'
source='ALARM_AND_ABORT.agc'
for source in ['ALARM_AND_ABORT.agc','FRESH_START_AND_RESTART.agc']:
 assert (root.parent/'Luminary099'/source).read_bytes()==(fixture/source).read_bytes()
subprocess.run(['cc','-std=c99','-O2','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests','src/agc_word.c','src/executive.c','src/waitlist.c','src/restart.c','src/enema.c','tests/restart_cases.c','tests/enema_cases.c','platform/host/enema_snapshot.c','-o','build/enema-snapshot'],cwd=root,check=True)
subprocess.run(['gcc','-std=c99','-O2','-I'+str(engine),'-Iinclude','-Itests','-DRESTART_INPUT_ONLY','tools/enema_reference.c','tests/scenarios.c','tests/restart_cases.c','tests/enema_cases.c',str(engine/'agc_engine.o'),str(engine/'agc_engine_init.o'),str(engine/'rfopen.o'),'-o','build/enema-reference'],cwd=root,check=True)
outputs=[]
for name,cmd in [('native',['./build/enema-snapshot']),('reference',['./build/enema-reference',str(fixture/'MAIN.agc.bin')])]:
 run=subprocess.run(cmd,cwd=root,capture_output=True,text=True)
 (root/f'build/enema-{name}.snapshots').write_text(run.stdout)
 if name=='reference': (root/'build/enema-reference.trace').write_text(run.stderr)
 if run.returncode: sys.exit(run.stderr[-3000:])
 outputs.append([list(map(int,line.split())) for line in run.stdout.splitlines()])
assert all(len(rows)==4096 for rows in outputs)
errors=[]
for native,original in zip(*outputs):
 assert len(native)==459 and len(original)==459
 for field,(a,b) in enumerate(zip(native,original)):
  if a!=b: errors.append(f'case {native[0]} field {field}: {a} != {b}')
if errors: sys.exit('\n'.join(errors[:80])+f'\n{len(errors)} differences')
print('PASS: 4,096 original ENEMA checkpoints, all457 words including register/channel state')
print('Boundaries: MMDSPLAY, PTBAD or fresh-start STARTSUB entry; subsequent services excluded')
print('Real original ENEMA entry, STARTSB1/LIGHTSET/RSFLGBTS mask and phase agreement; native composition verified')
for source in ['ALARM_AND_ABORT.agc','FRESH_START_AND_RESTART.agc']:
 print(source+' SHA256:',hashlib.sha256((fixture/source).read_bytes()).hexdigest())
print('Snapshot SHA256:',hashlib.sha256((root/'build/enema-reference.snapshots').read_bytes()).hexdigest())
