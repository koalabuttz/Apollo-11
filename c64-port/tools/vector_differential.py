#!/usr/bin/env python3
"""Compare complete VAD/VSU states against unchanged original assembly."""
import hashlib,pathlib,re,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[1]
reference=pathlib.Path(sys.argv[1]).resolve(); engine=root/'deps/virtualagc/yaAGC'
assert (root.parent/'Luminary099/INTERPRETER.agc').read_bytes()==(reference/'INTERPRETER.agc').read_bytes()
listing=(reference/'assembly.lst').read_text()
for name,address in [('VAD','6751'),('VSU','6747'),('OVERFLWY','7014'),('OVERFLWZ','7011'),('OVERFLOW','7017'),('DANZIG','6061')]:
    assert re.search(r'(?<!\w)'+re.escape(name)+r'\s+'+address+r'\b',listing),name
common=['gcc','-std=c99','-O2','-DPREPARE_INPUT_ONLY','-Iinclude','-Itests']
subprocess.run(common+['src/agc_word.c','src/agc_multiword.c','src/guidance_vector.c','tests/prepare_cases.c','tests/vector_cases.c','platform/host/vector_snapshot.c','-o','build/vector-snapshot'],cwd=root,check=True)
subprocess.run(common+['-DVECTOR_INPUT_ONLY','-I'+str(engine),'tools/vector_reference.c','tests/scenarios.c','tests/prepare_cases.c','tests/vector_cases.c',str(engine/'agc_engine.o'),str(engine/'agc_engine_init.o'),str(engine/'rfopen.o'),'-o','build/vector-reference'],cwd=root,check=True)
outputs=[]
for name,cmd in [('native',['./build/vector-snapshot']),('reference',['./build/vector-reference',str(reference/'MAIN.agc.bin')])]:
    run=subprocess.run(cmd,cwd=root,capture_output=True,text=True)
    (root/f'build/vector-{name}.snapshots').write_text(run.stdout)
    if name=='reference': (root/'build/vector-reference.trace').write_text(run.stderr)
    if run.returncode: sys.exit(run.stderr[-3000:] or f'{name} exited {run.returncode}')
    rows=[list(map(int,r.split())) for r in run.stdout.splitlines()]
    assert len(rows)==13600 and all(len(r)==53 for r in rows)
    outputs.append(rows)
errors=[]
for a,b in zip(*outputs):
    if a!=b: errors.append(f'case/sub/path {a[:3]} vs {b[:3]} differences '+str([(i-3,x,y) for i,(x,y) in enumerate(zip(a,b)) if x!=y]))
if errors: sys.exit('\n'.join(errors[:20])+f'\n{len(errors)} differing rows')
print('PASS: 6800 inputs x VAD/VSU = 13600 checkpoints; all 50 modeled words and overflow observations match unchanged assembly')
print('Complete input preservation, both signed zeros, mixed signs, ordinary DP corners, retained scratch, and all OVERFLOW paths across X/Y/Z')
print('Boundary: VAD 6751 / VSU 6747 -> DANZIG 6061; historical Y,Z,X order and unchanged MODE verified')
print('Snapshot SHA256:',hashlib.sha256((root/'build/vector-reference.snapshots').read_bytes()).hexdigest())
