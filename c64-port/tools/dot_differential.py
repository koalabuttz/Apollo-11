#!/usr/bin/env python3
"""Compare every modeled DOT word against unchanged original assembly."""
import hashlib,pathlib,re,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[1]
reference=pathlib.Path(sys.argv[1] if len(sys.argv)>1 else root/'deps/fixture').resolve()
engine=root/'deps/virtualagc/yaAGC'
assert (root.parent/'Luminary099/INTERPRETER.agc').read_bytes()==(reference/'INTERPRETER.agc').read_bytes()
assert (root.parent/'Luminary099/FIXED_FIXED_CONSTANT_POOL.agc').read_bytes()==(reference/'FIXED-FIXED_CONSTANT_POOL.agc').read_bytes()
listing=(reference/'assembly.lst').read_text()
for name,address in [('DOT','7331'),('PREDOT','7154'),('DOTSUB','7156'),('ENDDOT','7205'),('DANZIG','6061')]:
    assert re.search(r'\b'+name+r'\s+'+address+r'\b',listing),(name,address)
definitions=['-DELAPSED_INPUT_ONLY','-DPUSH_INPUT_ONLY','-DSCALE_INPUT_ONLY','-DPREPARE_INPUT_ONLY']
cases=['tests/scenarios.c','tests/prepare_cases.c','tests/scale_cases.c','tests/push_cases.c','tests/elapsed_cases.c','tests/dot_cases.c']
common=['gcc','-std=c99','-O2','-Wall','-Wextra','-Werror']+definitions+['-Iinclude','-Itests']
subprocess.run(common+['-Wconversion','-pedantic','src/agc_word.c','src/agc_multiword.c','src/guidance_product.c','src/guidance_dot.c']+cases+['tools/dot_snapshot.c','-o','build/dot-snapshot'],cwd=root,check=True)
subprocess.run(common+['-DDOT_INPUT_ONLY','-I'+str(engine),'tools/dot_reference.c']+cases+[str(engine/'agc_engine.o'),str(engine/'agc_engine_init.o'),str(engine/'rfopen.o'),'-o','build/dot-reference'],cwd=root,check=True)
outputs=[]
for name,command in [('native',['./build/dot-snapshot']),('reference',['./build/dot-reference',str(reference/'MAIN.agc.bin')])]:
    run=subprocess.run(command,cwd=root,capture_output=True,text=True)
    (root/f'build/dot-{name}.snapshots').write_text(run.stdout)
    if name=='reference':(root/'build/dot-reference.trace').write_text(run.stderr)
    if run.returncode:sys.exit(run.stderr[-3000:] or f'{name} exited {run.returncode}')
    rows=[list(map(int,line.split()))for line in run.stdout.splitlines()]
    assert len(rows)==10272 and all(len(row)==101 for row in rows)
    outputs.append(rows)
errors=[]
for a,b in zip(*outputs):
    if a!=b:errors.append(f'case {a[0]}: '+str([(i-1,x,y)for i,(x,y)in enumerate(zip(a,b)) if x!=y]))
if errors:sys.exit('\n'.join(errors[:12])+f'\n{len(errors)} differing rows')
print('PASS: 10,272 direct DOTs; all 100 modeled words match unchanged assembly')
print('32 explicit signed-zero/mixed-sign/half/max/cancellation cases; all 4096 six-word raw corners against same/opposite vector; 2048 deterministic independent random vectors')
print('Unrounded TP accumulation, both intermediate/final overflow corrections, MODE/MPAC exchange debris, scratch aliases, retained VAC/operands and excluded IRQ state verified')
print('DOT snapshot SHA256:',hashlib.sha256((root/'build/dot-reference.snapshots').read_bytes()).hexdigest())
