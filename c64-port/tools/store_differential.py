#!/usr/bin/env python3
"""Compare all modeled states and skip paths against original STORE 30D."""
import hashlib,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[1]
reference=pathlib.Path(sys.argv[1]).resolve(); engine=root/'deps/virtualagc/yaAGC'
for source in ['LUNAR_LANDING_GUIDANCE_EQUATIONS.agc','INTERPRETER.agc','CONTROLLED_CONSTANTS.agc']:
    assert (root.parent/'Luminary099'/source).read_bytes()==(reference/source).read_bytes()
assert (root.parent/'Luminary099/FIXED_FIXED_CONSTANT_POOL.agc').read_bytes()==(reference/'FIXED-FIXED_CONSTANT_POOL.agc').read_bytes()
subprocess.run(['gcc','-std=c99','-O2','-DSTORE_INPUT_ONLY','-DELAPSED_INPUT_ONLY','-DPUSH_INPUT_ONLY','-DSCALE_INPUT_ONLY','-DPREPARE_INPUT_ONLY',
    '-I'+str(engine),'-Iinclude','-Itests','tools/store_reference.c',
    'tests/scenarios.c','tests/prepare_cases.c','tests/scale_cases.c','tests/push_cases.c','tests/elapsed_cases.c','tests/store_cases.c',
    str(engine/'agc_engine.o'),str(engine/'agc_engine_init.o'),str(engine/'rfopen.o'),
    '-o','build/store-reference'],cwd=root,check=True)
outputs=[]
for name,cmd in [('native',['./build/store-snapshot']),
                 ('reference',['./build/store-reference',str(reference/'MAIN.agc.bin')])]:
    run=subprocess.run(cmd,cwd=root,capture_output=True,text=True)
    (root/f'build/store-{name}.snapshots').write_text(run.stdout)
    if name=='reference': (root/'build/store-reference.trace').write_text(run.stderr)
    if run.returncode: sys.exit(run.stderr[-3000:] or f'{name} exited {run.returncode}')
    rows=[list(map(int,r.split())) for r in run.stdout.splitlines()]
    assert len(rows)==5044 and all(len(r)==93 for r in rows)
    outputs.append(rows)
errors=[f'case/step {a[:2]}: native {a}, reference {b}' for a,b in zip(*outputs) if a!=b]
if errors: sys.exit('\n'.join(errors[:12])+f'\n{len(errors)} differing rows')
print('PASS: 3681 cases / 5044 checkpoints; all 89 modeled words and retained DSU path observations match original assembly')
print('16 signed-zero inputs, 16 arithmetic edge inputs, 256 source/destination corners, all 190 VAC/push offsets, 128 integrated skip inputs, three four-step histories, 2048 direct / 1024 integrated random cases')
print('Stops after DSU / after STORE before DDV; relative destination, MODE, pointer, guards and both prior overflow signs/path families verified')
print('Store snapshot SHA256:',hashlib.sha256((root/'build/store-reference.snapshots').read_bytes()).hexdigest())
