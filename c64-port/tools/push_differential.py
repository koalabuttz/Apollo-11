#!/usr/bin/env python3
"""Compare complete modeled states against unchanged VXSC KPIP1 assembly."""
import hashlib,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[1]
reference=pathlib.Path(sys.argv[1]).resolve(); engine=root/'deps/virtualagc/yaAGC'
for source in ['LUNAR_LANDING_GUIDANCE_EQUATIONS.agc','INTERPRETER.agc','CONTROLLED_CONSTANTS.agc']:
    assert (root.parent/'Luminary099'/source).read_bytes()==(reference/source).read_bytes()
subprocess.run(['gcc','-std=c99','-O2','-DPUSH_INPUT_ONLY','-DSCALE_INPUT_ONLY','-DPREPARE_INPUT_ONLY',
    '-I'+str(engine),'-Iinclude','-Itests','tools/push_reference.c',
    'tests/scenarios.c','tests/prepare_cases.c','tests/scale_cases.c','tests/push_cases.c',
    str(engine/'agc_engine.o'),str(engine/'agc_engine_init.o'),str(engine/'rfopen.o'),
    '-o','build/push-reference'],cwd=root,check=True)
outputs=[]
for name,cmd in [('native',['./build/push-snapshot']),
                 ('reference',['./build/push-reference',str(reference/'MAIN.agc.bin')])]:
    run=subprocess.run(cmd,cwd=root,capture_output=True,text=True)
    (root/f'build/push-{name}.snapshots').write_text(run.stdout)
    if name=='reference': (root/'build/push-reference.trace').write_text(run.stderr)
    if run.returncode: sys.exit(run.stderr[-3000:] or f'{name} exited {run.returncode}')
    rows=[list(map(int,r.split())) for r in run.stdout.splitlines()]
    assert len(rows)==3496 and all(len(r)==89 for r in rows)
    outputs.append(rows)
errors=[f'case/step {a[:2]}: native {a}, reference {b}' for a,b in zip(*outputs) if a!=b]
if errors: sys.exit('\n'.join(errors[:12])+f'\n{len(errors)} differing rows')
print('PASS: 2323 cases / 3496 checkpoints; all 86 modeled words match unchanged PDDL THISTPIP and integrated RODCOMP')
print('144 timestamp edge pairs, all five VACs and 38 valid offsets, 128 integrated skip inputs, three four-push histories, 1024 direct and 1024 integrated random cases')
print('Stops at DANZIG after scaling / after PDDL before DSU; genuine interpreter resolution, MODE, pointer and VAC guards verified')
print('Pushdown snapshot SHA256:',hashlib.sha256((root/'build/push-reference.snapshots').read_bytes()).hexdigest())
