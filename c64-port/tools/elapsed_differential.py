#!/usr/bin/env python3
"""Compare all modeled states and skip paths against original DSU PIPTIME."""
import hashlib,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[1]
reference=pathlib.Path(sys.argv[1]).resolve(); engine=root/'deps/virtualagc/yaAGC'
for source in ['LUNAR_LANDING_GUIDANCE_EQUATIONS.agc','INTERPRETER.agc','CONTROLLED_CONSTANTS.agc']:
    assert (root.parent/'Luminary099'/source).read_bytes()==(reference/source).read_bytes()
assert (root.parent/'Luminary099/FIXED_FIXED_CONSTANT_POOL.agc').read_bytes()==(reference/'FIXED-FIXED_CONSTANT_POOL.agc').read_bytes()
subprocess.run(['gcc','-std=c99','-O2','-DELAPSED_INPUT_ONLY','-DPUSH_INPUT_ONLY','-DSCALE_INPUT_ONLY','-DPREPARE_INPUT_ONLY',
    '-I'+str(engine),'-Iinclude','-Itests','tools/elapsed_reference.c',
    'tests/scenarios.c','tests/prepare_cases.c','tests/scale_cases.c','tests/push_cases.c','tests/elapsed_cases.c',
    str(engine/'agc_engine.o'),str(engine/'agc_engine_init.o'),str(engine/'rfopen.o'),
    '-o','build/elapsed-reference'],cwd=root,check=True)
outputs=[]
for name,cmd in [('native',['./build/elapsed-snapshot']),
                 ('reference',['./build/elapsed-reference',str(reference/'MAIN.agc.bin')])]:
    run=subprocess.run(cmd,cwd=root,capture_output=True,text=True)
    (root/f'build/elapsed-{name}.snapshots').write_text(run.stdout)
    if name=='reference': (root/'build/elapsed-reference.trace').write_text(run.stderr)
    if run.returncode: sys.exit(run.stderr[-3000:] or f'{name} exited {run.returncode}')
    rows=[list(map(int,r.split())) for r in run.stdout.splitlines()]
    assert len(rows)==4664 and all(len(r)==93 for r in rows)
    outputs.append(rows)
errors=[f'case/step {a[:2]}: native {a}, reference {b}' for a,b in zip(*outputs) if a!=b]
if errors: sys.exit('\n'.join(errors[:12])+f'\n{len(errors)} differing rows')
print('PASS: 3491 cases / 4664 checkpoints; all 89 modeled words and DSU path observations match original assembly')
print('16 signed-zero combinations, 16 arithmetic edges, 256 four-word corners, 128 integrated skip inputs, three four-step histories, 2048 direct / 1024 integrated random cases')
print('Stops after PDDL / after DSU before STORE 30D; operand bank, MODE, pointer, guards and both overflow signs/path families verified')
print('Elapsed snapshot SHA256:',hashlib.sha256((root/'build/elapsed-reference.snapshots').read_bytes()).hexdigest())
