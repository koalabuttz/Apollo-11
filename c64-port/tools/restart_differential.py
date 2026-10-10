#!/usr/bin/env python3
"""Verify bounded native recovery stages against byte-identical original assembly."""
import pathlib, subprocess, sys, hashlib
root=pathlib.Path(__file__).resolve().parents[1]
reference=pathlib.Path(sys.argv[1]).resolve(); engine=root/'deps/virtualagc/yaAGC'
for source in ['ALARM_AND_ABORT','FRESH_START_AND_RESTART','RESTARTS_ROUTINE','RESTART_TABLES','PHASE_TABLE_MAINTENANCE']:
    assert (root.parent/'Luminary099'/f'{source}.agc').read_bytes()==(reference/f'{source}.agc').read_bytes(), source
subprocess.run(['gcc','-std=c99','-O2','-DRESTART_INPUT_ONLY','-I'+str(engine),'-Iinclude','-Itests',
                'tools/restart_reference.c','tests/scenarios.c','tests/restart_cases.c',
                str(engine/'agc_engine.o'),str(engine/'agc_engine_init.o'),str(engine/'rfopen.o'),
                '-o','build/restart-reference'],cwd=root,check=True)
outputs=[]
for name,cmd in [('native',['./build/restart-snapshot']),('reference',['./build/restart-reference',str(reference/'MAIN.agc.bin')])]:
    run=subprocess.run(cmd,cwd=root,capture_output=True,text=True)
    (root/f'build/restart-{name}.snapshots').write_text(run.stdout)
    if name=='reference': (root/'build/restart-reference.trace').write_text(run.stderr)
    if run.returncode: sys.exit(run.stderr[-3000:])
    outputs.append([list(map(int,r.split())) for r in run.stdout.splitlines()])
assert all(len(rows)==226 for rows in outputs)
errors=[]
for native,original in zip(*outputs):
    if len(native)!=451 or len(original)!=451: errors.append(f'Unexpected row length {len(native)}/{len(original)}')
    for field,(nv,rv) in enumerate(zip(native,original)):
        if nv!=rv: errors.append(f'case/step {native[:2]} field {field}: native={nv} reference={rv}')
if errors: sys.exit('\n'.join(errors[:80])+f'\n{len(errors)} mismatches')
print('PASS: 190 cases / 226 checkpoints; status and all 448 modeled state words agree')
print('Boundaries: ALARM1 return / WHIMPER / POODOO phase-write entry; STARTSB1 return; MMDSPLAY/PTBAD; ENDFIND/BAILOUT1')
print('Four sequential 1202 -> alarm record -> cleanup -> phase check -> six protected job admissions')
print('Restart snapshot SHA256:',hashlib.sha256((root/'build/restart-reference.snapshots').read_bytes()).hexdigest())
