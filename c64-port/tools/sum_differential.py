#!/usr/bin/env python3
"""Compare generic scalar sums/reverse vector subtraction with assembly."""
import hashlib,pathlib,re,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[1]
reference=pathlib.Path(sys.argv[1] if len(sys.argv)>1 else root/'deps/fixture').resolve()
engine=root/'deps/virtualagc/yaAGC'
assert (root.parent/'Luminary099/INTERPRETER.agc').read_bytes()==(reference/'INTERPRETER.agc').read_bytes()
assert (root.parent/'Luminary099/FIXED_FIXED_CONSTANT_POOL.agc').read_bytes()==(reference/'FIXED-FIXED_CONSTANT_POOL.agc').read_bytes()
listing=(reference/'assembly.lst').read_text()
for name,address in [('DAD','6775'),('DSU','7005'),('BDSU','7062'),('BVSU','7036'),('OVERFLWY','7014'),('OVERFLWZ','7011'),('OVERFLOW','7017'),('DANZIG','6061')]:
    assert re.search(r'\b'+name+r'\s+'+address+r'\b',listing),(name,address)
common=['gcc','-std=c99','-O2','-Wall','-Wextra','-Werror','-DVECTOR_INPUT_ONLY','-DPREPARE_INPUT_ONLY','-Iinclude','-Itests']
cases=['tests/prepare_cases.c','tests/vector_cases.c','tests/sum_cases.c']
subprocess.run(common+['-Wconversion','-pedantic','src/agc_word.c','src/agc_multiword.c','src/guidance_sum.c']+cases+['tools/sum_snapshot.c','-o','build/sum-snapshot'],cwd=root,check=True)
subprocess.run(common+['-DSUM_INPUT_ONLY','-I'+str(engine),'tools/sum_reference.c','tests/scenarios.c']+cases+[str(engine/'agc_engine.o'),str(engine/'agc_engine_init.o'),str(engine/'rfopen.o'),'-o','build/sum-reference'],cwd=root,check=True)
outputs=[]
for name,command in [('native',['./build/sum-snapshot']),('reference',['./build/sum-reference',str(reference/'MAIN.agc.bin')])]:
    run=subprocess.run(command,cwd=root,capture_output=True,text=True)
    (root/f'build/sum-{name}.snapshots').write_text(run.stdout)
    if name=='reference':(root/'build/sum-reference.trace').write_text(run.stderr)
    if run.returncode:sys.exit(run.stderr[-3000:]or f'{name} exited {run.returncode}')
    rows=[list(map(int,line.split()))for line in run.stdout.splitlines()]
    assert len(rows)==27200 and all(len(row)==53 for row in rows)
    outputs.append(rows)
errors=[]
for a,b in zip(*outputs):
    if a!=b:errors.append(f'case/op/path {a[:3]} vs {b[:3]}: '+str([(i-3,x,y)for i,(x,y)in enumerate(zip(a,b))if x!=y]))
if errors:sys.exit('\n'.join(errors[:12])+f'\n{len(errors)} differing rows')
print('PASS: 6800 inputs x DAD/DSU/BDSU/BVSU = 27,200 complete 50-word states and original overflow observations')
print('256 four-word raw corners, 16 explicit signed overflow/skip families, 128 further arithmetic edges, 4096 cross-component corners and 2304 deterministic random cases per operation')
print('Both signed zeros/mixed signs, all paths0/1/5/7 for every active component, stable operands/PIPTIME, MODE/MPAC2/scratch and A/L preserved exactly')
print('Sum snapshot SHA256:',hashlib.sha256((root/'build/sum-reference.snapshots').read_bytes()).hexdigest())
