#!/usr/bin/env python3
"""Compare all modeled states and skip paths against original DDV 4SEC(28)."""
import hashlib,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[1]
reference=pathlib.Path(sys.argv[1]).resolve(); engine=root/'deps/virtualagc/yaAGC'
for source in ['LUNAR_LANDING_GUIDANCE_EQUATIONS.agc','INTERPRETER.agc','CONTROLLED_CONSTANTS.agc']:
    assert (root.parent/'Luminary099'/source).read_bytes()==(reference/source).read_bytes()
assert (root.parent/'Luminary099/FIXED_FIXED_CONSTANT_POOL.agc').read_bytes()==(reference/'FIXED-FIXED_CONSTANT_POOL.agc').read_bytes()
subprocess.run([sys.executable,'tools/prepare_hardware_dv.py'],cwd=root,check=True)
subprocess.run(['gcc','-std=c99','-O2','-DDIVIDE_INPUT_ONLY','-DSTORE_INPUT_ONLY','-DELAPSED_INPUT_ONLY','-DPUSH_INPUT_ONLY','-DSCALE_INPUT_ONLY','-DPREPARE_INPUT_ONLY',
    '-I'+str(engine),'-Iinclude','-Itests','tools/divide_reference.c',
    'tests/scenarios.c','tests/prepare_cases.c','tests/scale_cases.c','tests/push_cases.c','tests/elapsed_cases.c','tests/store_cases.c','tests/divide_cases.c',
    str(root/'build/agc_engine_hardware_dv.o'),str(engine/'agc_engine_init.o'),str(engine/'rfopen.o'),
    '-o','build/divide-reference'],cwd=root,check=True)
outputs=[]
for name,cmd in [('native',['./build/divide-snapshot']),
                 ('reference',['./build/divide-reference',str(reference/'MAIN.agc.bin')])]:
    run=subprocess.run(cmd,cwd=root,capture_output=True,text=True)
    (root/f'build/divide-{name}.snapshots').write_text(run.stdout)
    if name=='reference': (root/'build/divide-reference.trace').write_text(run.stderr)
    if run.returncode: sys.exit(run.stderr[-3000:] or f'{name} exited {run.returncode}')
    rows=[list(map(int,r.split())) for r in run.stdout.splitlines()]
    assert len(rows)==8098 and all(len(r)==97 for r in rows)
    outputs.append(rows)
errors=[f'case/step {a[:2]}: native {a}, reference {b}' for a,b in zip(*outputs) if a!=b]
if errors: sys.exit('\n'.join(errors[:12])+f'\n{len(errors)} differing rows')
print('PASS: 6735 cases / 8098 checkpoints; all 93 modeled words and zero/quotient/overflow paths match original assembly using pinned hardware DV')
print('16 zero/carry/overflow boundaries, 256 raw corners, 190 VAC/push offsets, 128 integrated skip inputs, three four-step histories, 2048 direct / 1024 integrated random cases and -1024..1023 sweep plus 1022 mixed-sign boundary representations')
print('Stops after STORE / after DDV before PDVL; fixed divisor, banks, MODE, pointer, guards and all division exits verified')
print('Divide snapshot SHA256:',hashlib.sha256((root/'build/divide-reference.snapshots').read_bytes()).hexdigest())

subprocess.run([sys.executable,'tools/audit_divide_models.py'],cwd=root,check=True)
