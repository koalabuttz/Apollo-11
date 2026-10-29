#!/usr/bin/env python3
"""Compare complete modeled states against unchanged VXSC KPIP1 assembly."""
import hashlib,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[1]
reference=pathlib.Path(sys.argv[1]).resolve(); engine=root/'deps/virtualagc/yaAGC'
for source in ['LUNAR_LANDING_GUIDANCE_EQUATIONS.agc','INTERPRETER.agc','CONTROLLED_CONSTANTS.agc']:
    assert (root.parent/'Luminary099'/source).read_bytes()==(reference/source).read_bytes()
subprocess.run(['gcc','-std=c99','-O2','-DSCALE_INPUT_ONLY','-DPREPARE_INPUT_ONLY',
    '-I'+str(engine),'-Iinclude','-Itests','tools/scale_reference.c',
    'tests/scenarios.c','tests/prepare_cases.c','tests/scale_cases.c',
    str(engine/'agc_engine.o'),str(engine/'agc_engine_init.o'),str(engine/'rfopen.o'),
    '-o','build/scale-reference'],cwd=root,check=True)
outputs=[]
for name,cmd in [('native',['./build/scale-snapshot']),
                 ('reference',['./build/scale-reference',str(reference/'MAIN.agc.bin')])]:
    run=subprocess.run(cmd,cwd=root,capture_output=True,text=True)
    (root/f'build/scale-{name}.snapshots').write_text(run.stdout)
    if name=='reference': (root/'build/scale-reference.trace').write_text(run.stderr)
    if run.returncode: sys.exit(run.stderr[-3000:] or f'{name} exited {run.returncode}')
    rows=[list(map(int,r.split())) for r in run.stdout.splitlines()]
    assert len(rows)==3364 and all(len(r)==43 for r in rows)
    outputs.append(rows)
errors=[f'case/step {a[:2]}: native {a}, reference {b}' for a,b in zip(*outputs) if a!=b]
if errors: sys.exit('\n'.join(errors[:12])+f'\n{len(errors)} differing rows')
print('PASS: 3355 cases / 3364 checkpoints; all 41 modeled words match unchanged VXSC KPIP1')
print('144 DP edge pairs, 8 rounding-boundary cases, 128 integrated skip-mask inputs, three four-step histories, 2048 direct and 1024 integrated random cases')
print('Boundary: VXSC 7401 -> DANZIG 6061 before PDDL; integrated calls start at RODCOMP 32,3275; RELINT and operand-bank setup verified')
print('Scaling snapshot SHA256:',hashlib.sha256((root/'build/scale-reference.snapshots').read_bytes()).hexdigest())
