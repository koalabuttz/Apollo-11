#!/usr/bin/env python3
"""Exhaust valid groups in all phase words, single and double source entries."""
import hashlib, pathlib, subprocess, sys
root=pathlib.Path(__file__).resolve().parents[1]
fixture=pathlib.Path(sys.argv[1]).resolve(); engine=root/'deps/virtualagc/yaAGC'
source='PHASE_TABLE_MAINTENANCE.agc'
assert (root.parent/'Luminary099'/source).read_bytes()==(fixture/source).read_bytes()
subprocess.run(['cc','-std=c99','-O2','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests','src/phase.c','tests/phase_cases.c','platform/host/phase_snapshot.c','-o','build/phase-snapshot'],cwd=root,check=True)
subprocess.run(['gcc','-std=c99','-O2','-I'+str(engine),'-Iinclude','-Itests','tools/phase_reference.c','tests/scenarios.c','tests/phase_cases.c',str(engine/'agc_engine.o'),str(engine/'agc_engine_init.o'),str(engine/'rfopen.o'),'-o','build/phase-reference'],cwd=root,check=True)
outputs=[]
for name,cmd in [('native',['./build/phase-snapshot']),('reference',['./build/phase-reference',str(fixture/'MAIN.agc.bin')])]:
 run=subprocess.run(cmd,cwd=root,capture_output=True,text=True)
 (root/f'build/phase-{name}.snapshots').write_text(run.stdout)
 if name=='reference': (root/'build/phase-reference.trace').write_text(run.stderr)
 if run.returncode: sys.exit(run.stderr[-3000:])
 outputs.append([list(map(int,line.split())) for line in run.stdout.splitlines()])
assert all(len(rows)==49152 for rows in outputs)
errors=[]
for native,original in zip(*outputs):
 assert len(native)==58 and len(original)==58
 for field,(a,b) in enumerate(zip(native,original)):
  if a!=b: errors.append(f'case {native[0]} field {field}: {a} != {b}')
if errors: sys.exit('\n'.join(errors[:80])+f'\n{len(errors)} differences')
print('PASS: 49,152 checkpoints; every raw phase word with groups 1..6 at PHASCHNG and 2PHSCHNG')
print('All 56 modeled words, exact final A/L/Q/BBANK and interrupt enable agree at original RELINT/DTCB return')
print('Stable counters; no restart execution or display service; 2PHSCHNG first word Type A')
print('Original source SHA256:',hashlib.sha256((fixture/source).read_bytes()).hexdigest())
print('Snapshot SHA256:',hashlib.sha256((root/'build/phase-reference.snapshots').read_bytes()).hexdigest())
