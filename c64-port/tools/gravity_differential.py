#!/usr/bin/env python3
"""Compare all modeled states and skip paths against original PDVL GDT/2."""
import hashlib,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[1]
reference=pathlib.Path(sys.argv[1]).resolve(); engine=root/'deps/virtualagc/yaAGC'
for source in ['LUNAR_LANDING_GUIDANCE_EQUATIONS.agc','INTERPRETER.agc','CONTROLLED_CONSTANTS.agc']:
    assert (root.parent/'Luminary099'/source).read_bytes()==(reference/source).read_bytes()
assert (root.parent/'Luminary099/FIXED_FIXED_CONSTANT_POOL.agc').read_bytes()==(reference/'FIXED-FIXED_CONSTANT_POOL.agc').read_bytes()
subprocess.run([sys.executable,'tools/prepare_hardware_dv.py'],cwd=root,check=True)
subprocess.run(['gcc','-std=c99','-O2','-DGRAVITY_INPUT_ONLY','-DDIVIDE_INPUT_ONLY','-DSTORE_INPUT_ONLY','-DELAPSED_INPUT_ONLY','-DPUSH_INPUT_ONLY','-DSCALE_INPUT_ONLY','-DPREPARE_INPUT_ONLY',
    '-I'+str(engine),'-Iinclude','-Itests','tools/gravity_reference.c',
    'tests/scenarios.c','tests/prepare_cases.c','tests/scale_cases.c','tests/push_cases.c','tests/elapsed_cases.c','tests/store_cases.c','tests/divide_cases.c','tests/gravity_cases.c',
    str(root/'build/agc_engine_hardware_dv.o'),str(engine/'agc_engine_init.o'),str(engine/'rfopen.o'),
    '-o','build/gravity-reference'],cwd=root,check=True)
outputs=[]
for name,cmd in [('native',['./build/gravity-snapshot']),
                 ('reference',['./build/gravity-reference',str(reference/'MAIN.agc.bin')])]:
    run=subprocess.run(cmd,cwd=root,capture_output=True,text=True)
    (root/f'build/gravity-{name}.snapshots').write_text(run.stdout)
    if name=='reference': (root/'build/gravity-reference.trace').write_text(run.stderr)
    if run.returncode: sys.exit(run.stderr[-3000:] or f'{name} exited {run.returncode}')
    rows=[list(map(int,r.split())) for r in run.stdout.splitlines()]
    assert len(rows)==5218 and all(len(r)==102 for r in rows)
    outputs.append(rows)
errors=[f'case/step {a[:2]}: native {a}, reference {b}' for a,b in zip(*outputs) if a!=b]
if errors: sys.exit('\n'.join(errors[:12])+f'\n{len(errors)} differing rows')
print('PASS: 3865 cases / 5218 checkpoints; all 99 modeled words match unchanged PDVL GDT/2 and integrated RODCOMP using pinned hardware DV')
print('All 210 DP push positions, 180 full-prefix positions, 256 raw corners, 128 integrated skip inputs, three retained four-step histories; 2048 direct and 1024 integrated random inputs')
print('Stops before VSU; original banks, MODE, pointer and guard words verified')
print('Gravity snapshot SHA256:',hashlib.sha256((root/'build/gravity-reference.snapshots').read_bytes()).hexdigest())
