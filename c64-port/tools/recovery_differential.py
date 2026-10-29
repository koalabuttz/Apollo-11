#!/usr/bin/env python3
"""Verify all abort-flag values and exact selective phase-cleanup stores."""
import hashlib, pathlib, subprocess, sys
root=pathlib.Path(__file__).resolve().parents[1]
fixture=pathlib.Path(sys.argv[1]).resolve(); engine=root/'deps/virtualagc/yaAGC'
source='ALARM_AND_ABORT.agc'
for source in ['ALARM_AND_ABORT.agc','FRESH_START_AND_RESTART.agc']:
 assert (root.parent/'Luminary099'/source).read_bytes()==(fixture/source).read_bytes()
subprocess.run(['cc','-std=c99','-O2','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests','src/agc_word.c','src/recovery.c','tests/recovery_cases.c','platform/host/recovery_snapshot.c','-o','build/recovery-snapshot'],cwd=root,check=True)
subprocess.run(['gcc','-std=c99','-O2','-I'+str(engine),'-Iinclude','-Itests','tools/recovery_reference.c','tests/scenarios.c','tests/recovery_cases.c',str(engine/'agc_engine.o'),str(engine/'agc_engine_init.o'),str(engine/'rfopen.o'),'-o','build/recovery-reference'],cwd=root,check=True)
outputs=[]
for name,cmd in [('native',['./build/recovery-snapshot']),('reference',['./build/recovery-reference',str(fixture/'MAIN.agc.bin')])]:
 run=subprocess.run(cmd,cwd=root,capture_output=True,text=True)
 (root/f'build/recovery-{name}.snapshots').write_text(run.stdout)
 if name=='reference': (root/'build/recovery-reference.trace').write_text(run.stderr)
 if run.returncode: sys.exit(run.stderr[-3000:])
 outputs.append([list(map(int,line.split())) for line in run.stdout.splitlines()])
assert all(len(rows)==49152 for rows in outputs)
errors=[]
for native,original in zip(*outputs):
 assert len(native)==49 and len(original)==49
 for field,(a,b) in enumerate(zip(native,original)):
  if a!=b: errors.append(f'case {native[0]} field {field}: {a} != {b}')
if errors: sys.exit('\n'.join(errors[:80])+f'\n{len(errors)} differences')
print('PASS: 49,152 complete-state abort cleanup checkpoints; all 32,768 raw flag patterns plus four phase-cleanup routes')
print('All 47 modeled words and 14 surrounding guards agree at original TC Q / GOPOODOO entry')
print('FLAGS, MR.KLEAN, P00KLEAN, V37KLEAN, POODOO phase-4 write; no live SERVICER/restart execution')
for source in ['ALARM_AND_ABORT.agc','FRESH_START_AND_RESTART.agc']:
 print(source+' SHA256:',hashlib.sha256((fixture/source).read_bytes()).hexdigest())
print('Snapshot SHA256:',hashlib.sha256((root/'build/recovery-reference.snapshots').read_bytes()).hexdigest())
