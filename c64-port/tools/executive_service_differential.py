#!/usr/bin/env python3
"""SPVAC/CHANG2 full-word comparisons with unchanged original assembly."""
import hashlib
import pathlib
import subprocess
import sys
ROOT = pathlib.Path(__file__).resolve().parents[1]
reference = pathlib.Path(sys.argv[1]).resolve()
engine = reference.parent / 'virtualagc' / 'yaAGC'
for name in ('EXECUTIVE.agc', 'INTERPRETER.agc'):
    if (ROOT.parent / 'Luminary099' / name).read_bytes() != (reference / name).read_bytes():
        sys.exit('fixture source mismatch: '+name)
listing = (reference / 'assembly.lst').read_text()
import re
for name, address in [('SPVAC','5116'), ('SPVACIN','5112'), ('CHANG2','5126'),
                      ('INTRSM','6050'), ('NEWOPS','6070'), ('INTBIT15','0115')]:
    if not re.search(r'(?<!\w)'+name+r'\s+'+address+r'\b', listing):
        sys.exit('fixture address mismatch: '+name)
common = ['tests/scenarios.c','tests/executive_service_cases.c',
          'platform/host/executive_service_snapshot.c']
subprocess.run(['cc','-std=c99','-O2','-Wall','-Wextra','-Werror','-Wconversion',
    '-pedantic','-Iinclude','-Itests','src/agc_word.c','src/executive.c',*common,
    '-o','build/executive-service-snapshot'],cwd=ROOT,check=True)
subprocess.run(['cc','-std=c99','-O2','-DEXEC_SERVICE_ROW_ONLY','-I'+str(engine),
    '-Iinclude','-Itests','tools/executive_service_reference.c',*common,
    str(engine/'agc_engine.o'),str(engine/'agc_engine_init.o'),str(engine/'rfopen.o'),
    '-o','build/executive-service-reference'],cwd=ROOT,check=True)
outputs=[]
for name, command in [('native',['./build/executive-service-snapshot']),
    ('reference',['./build/executive-service-reference',str(reference/'MAIN.agc.bin')])]:
    run=subprocess.run(command,cwd=ROOT,capture_output=True,text=True,check=True)
    (ROOT/('build/executive-service-'+name+'.snapshots')).write_text(run.stdout)
    if name=='reference': (ROOT/'build/executive-service-reference.trace').write_text(run.stderr)
    outputs.append([list(map(int,line.split())) for line in run.stdout.splitlines()])
if any(len(rows)!=768 for rows in outputs): sys.exit('expected 768 checkpoints')
for row,(native,oracle) in enumerate(zip(*outputs)):
    if len(native)!=332 or len(oracle)!=332: sys.exit('expected 332 fields per row')
    for field,(nv,rv) in enumerate(zip(native,oracle)):
        if nv!=rv: sys.exit(f'case {row}, field {field}: native={nv}, assembly={rv}')
print('PASS: 256 SPVAC and 512 CHANG2 original-assembly checkpoints; every persisted word matches')
print('SPVAC adjusted Q captured before admission; actual basic/interpreter BBANK/SUPERBNK independently checked')
print('Snapshot SHA256: '+hashlib.sha256((ROOT/'build/executive-service-reference.snapshots').read_bytes()).hexdigest())
