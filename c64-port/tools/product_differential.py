#!/usr/bin/env python3
"""Compare complete VXSC/DMP1 states against unchanged original assembly."""
import hashlib,pathlib,re,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[1]
reference=pathlib.Path(sys.argv[1]).resolve(); engine=root/'deps/virtualagc/yaAGC'
assert (root.parent/'Luminary099/INTERPRETER.agc').read_bytes()==(reference/'INTERPRETER.agc').read_bytes()
listing=(reference/'assembly.lst').read_text()
for name,address in [('VXSC','7401'),('DMP1','7572'),('DMPSUB','7107'),('VROUND','7140'),('DANZIG','6061')]:
    assert re.search(r'(?<!\w)'+re.escape(name)+r'\s+'+address+r'\b',listing),name
common=['gcc','-std=c99','-O2','-DPREPARE_INPUT_ONLY','-DVECTOR_INPUT_ONLY','-Iinclude','-Itests']
subprocess.run(common+['src/agc_word.c','src/agc_multiword.c','src/guidance_product.c','tests/prepare_cases.c','tests/vector_cases.c','tests/product_cases.c','platform/host/product_snapshot.c','-o','build/product-snapshot'],cwd=root,check=True)
subprocess.run(common+['-DPRODUCT_INPUT_ONLY','-I'+str(engine),'tools/product_reference.c','tests/scenarios.c','tests/prepare_cases.c','tests/vector_cases.c','tests/product_cases.c',str(engine/'agc_engine.o'),str(engine/'agc_engine_init.o'),str(engine/'rfopen.o'),'-o','build/product-reference'],cwd=root,check=True)
outputs=[]
for name,cmd in [('native',['./build/product-snapshot']),('reference',['./build/product-reference',str(reference/'MAIN.agc.bin')])]:
    run=subprocess.run(cmd,cwd=root,capture_output=True,text=True)
    (root/f'build/product-{name}.snapshots').write_text(run.stdout)
    if name=='reference': (root/'build/product-reference.trace').write_text(run.stderr)
    if run.returncode: sys.exit(run.stderr[-3000:] or f'{name} exited {run.returncode}')
    rows=[list(map(int,r.split())) for r in run.stdout.splitlines()]
    assert len(rows)==8192 and all(len(r)==52 for r in rows)
    outputs.append(rows)
errors=[]
for a,b in zip(*outputs):
    if a!=b: errors.append(f'case/vector {a[:2]} vs {b[:2]} differences '+str([(i-2,x,y) for i,(x,y) in enumerate(zip(a,b)) if x!=y]))
if errors: sys.exit('\n'.join(errors[:20])+f'\n{len(errors)} differing rows')
print('PASS: 4096 inputs x scalar DMP1 / vector VXSC = 8192 complete 50-word checkpoints match unchanged assembly')
print('Both signed zeros, mixed signs, corner partial products, rounding carry, exact MPTEMP/OVFIND/A/L and MODE preservation')
print('Boundary: VXSC 7401 / DMP1 7572 -> DANZIG 6061')
print('Snapshot SHA256:',hashlib.sha256((root/'build/product-reference.snapshots').read_bytes()).hexdigest())
