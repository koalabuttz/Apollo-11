#!/usr/bin/env python3
"""Compare all shift-owned numerical state against unchanged assembly."""
import hashlib,pathlib,re,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[1]
reference=pathlib.Path(sys.argv[1] if len(sys.argv)>1 else root/'deps/fixture').resolve()
engine=root/'deps/virtualagc/yaAGC'
assert (root.parent/'Luminary099/INTERPRETER.agc').read_bytes()==(reference/'INTERPRETER.agc').read_bytes()
assert (root.parent/'Luminary099/FIXED_FIXED_CONSTANT_POOL.agc').read_bytes()==(reference/'FIXED-FIXED_CONSTANT_POOL.agc').read_bytes()
listing=(reference/'assembly.lst').read_text()
for name,address in [('GENSHIFT','00,2214'),('OPJUMP','6077'),('DANZIG','6061')]:
    assert re.search(r'\b'+name+r'\s+'+address+r'\b',listing),(name,address)
definitions=['-DELAPSED_INPUT_ONLY','-DPUSH_INPUT_ONLY','-DSCALE_INPUT_ONLY','-DPREPARE_INPUT_ONLY']
cases=['tests/scenarios.c','tests/prepare_cases.c','tests/scale_cases.c','tests/push_cases.c','tests/elapsed_cases.c','tests/shift_cases.c']
common=['gcc','-std=c99','-O2','-Wall','-Wextra','-Werror']+definitions+['-Iinclude','-Itests']
subprocess.run(common+['-Wconversion','-pedantic','src/agc_word.c','src/agc_multiword.c','src/guidance_shift.c']+cases+['tools/shift_snapshot.c','-o','build/shift-snapshot'],cwd=root,check=True)
subprocess.run(common+['-DSHIFT_INPUT_ONLY','-I'+str(engine),'tools/shift_reference.c']+cases+[str(engine/'agc_engine.o'),str(engine/'agc_engine_init.o'),str(engine/'rfopen.o'),'-o','build/shift-reference'],cwd=root,check=True)
outputs=[]
for name,command in [('native',['./build/shift-snapshot']),('reference',['./build/shift-reference',str(reference/'MAIN.agc.bin')])]:
    run=subprocess.run(command,cwd=root,capture_output=True,text=True)
    (root/f'build/shift-{name}.snapshots').write_text(run.stdout)
    if name=='reference': (root/'build/shift-reference.trace').write_text(run.stderr)
    if run.returncode: sys.exit(run.stderr[-3000:] or f'{name} exited {run.returncode}')
    rows=[list(map(int,line.split())) for line in run.stdout.splitlines()]
    assert len(rows)==26880 and all(len(row)==90 for row in rows)
    outputs.append(rows)
errors=[]
for a,b in zip(*outputs):
    if a!=b:errors.append(f'case {a[0]}: '+str([(i-1,x,y)for i,(x,y)in enumerate(zip(a,b)) if x!=y]))
if errors: sys.exit('\n'.join(errors[:12])+f'\n{len(errors)} differing rows')
print('PASS: 26,880 direct scalar/vector shifts; all 89 modeled words match unchanged assembly')
print('General counts 0..14, all short scalar counts 1..4 / vector counts 1..8; 64 raw triple corners and 256 deterministic random inputs per form/count')
print('MODE, all VAC words, scratch, A/L, signed zeros, overflow correction, exact vector rounding, whole-word count14 and excluded IRQ state verified')
print('Shift snapshot SHA256:',hashlib.sha256((root/'build/shift-reference.snapshots').read_bytes()).hexdigest())
