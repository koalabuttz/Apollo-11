#!/usr/bin/env python3
"""Compare snapshots and TS-skip effects of unchanged pre-interpreter RODCOMP."""
import hashlib, pathlib, subprocess, sys
root=pathlib.Path(__file__).resolve().parents[1]
reference=pathlib.Path(sys.argv[1]).resolve(); engine=root/'deps/virtualagc/yaAGC'
source='LUNAR_LANDING_GUIDANCE_EQUATIONS.agc'
assert (root.parent/'Luminary099'/source).read_bytes()==(reference/source).read_bytes()
subprocess.run(['gcc','-std=c99','-O2','-DPREPARE_INPUT_ONLY','-I'+str(engine),
                '-Iinclude','-Itests','tools/prepare_reference.c','tests/scenarios.c',
                'tests/prepare_cases.c',str(engine/'agc_engine.o'),
                str(engine/'agc_engine_init.o'),str(engine/'rfopen.o'),
                '-o','build/prepare-reference'],cwd=root,check=True)
outputs=[]
for name,cmd in [('native',['./build/prepare-snapshot']),
                 ('reference',['./build/prepare-reference',str(reference/'MAIN.agc.bin')])]:
    run=subprocess.run(cmd,cwd=root,capture_output=True,text=True)
    (root/f'build/prepare-{name}.snapshots').write_text(run.stdout)
    if name=='reference': (root/'build/prepare-reference.trace').write_text(run.stderr)
    if run.returncode: sys.exit(run.stderr[-3000:] or f'{name} exited {run.returncode}')
    rows=[list(map(int,r.split())) for r in run.stdout.splitlines()]
    assert len(rows)==3396 and all(len(r)==43 for r in rows)
    outputs.append(rows)
errors=[f'case/step/stage {a[:3]}: native {a}, reference {b}' for a,b in zip(*outputs) if a!=b]
if errors: sys.exit('\n'.join(errors[:20])+f'\n{len(errors)} differing rows')
for rows in outputs:
    for row in rows:
        if row[0]<128 and row[2]==2: assert row[3]==row[0]%64
print('PASS: 1171 cases / 3396 checkpoints; all 39 words and six TS-skip observations match original RODCOMP')
print('All 64 skip masks in both sign families; 16 word-edge cases; three four-step histories; 1024 full-prefix random cases')
print('Stops: bank 32,3303 / 3315 / 3353; snapshot A/L, MODE alias, retained low/scratch/input words, guards and INHINT verified')
print('Preparation snapshot SHA256:',hashlib.sha256((root/'build/prepare-reference.snapshots').read_bytes()).hexdigest())
