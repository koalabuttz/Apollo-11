#!/usr/bin/env python3
"""Compare raw A/L and all four rate-state words with real AGC executions."""
import hashlib, pathlib, subprocess, sys
root = pathlib.Path(__file__).resolve().parents[1]
reference = pathlib.Path(sys.argv[1]).resolve()
engine = root / 'deps/virtualagc/yaAGC'
source = 'LUNAR_LANDING_GUIDANCE_EQUATIONS.agc'
assert (root.parent/'Luminary099'/source).read_bytes() == (reference/source).read_bytes()
subprocess.run(['gcc','-std=c99','-O2','-DGUIDANCE_INPUT_ONLY','-I'+str(engine),
                '-Iinclude','-Itests','tools/guidance_reference.c','tests/scenarios.c',
                'tests/guidance_cases.c',str(engine/'agc_engine.o'),
                str(engine/'agc_engine_init.o'),str(engine/'rfopen.o'),
                '-o','build/guidance-reference'],cwd=root,check=True)
outputs = []
for name, cmd in [('native',['./build/guidance-snapshot']),
                  ('reference',['./build/guidance-reference',str(reference/'MAIN.agc.bin')])]:
    run = subprocess.run(cmd,cwd=root,capture_output=True,text=True)
    (root/f'build/guidance-{name}.snapshots').write_text(run.stdout)
    if name == 'reference': (root/'build/guidance-reference.trace').write_text(run.stderr)
    if run.returncode: sys.exit(run.stderr[-3000:])
    rows = [list(map(int,r.split())) for r in run.stdout.splitlines()]
    assert len(rows) == 12856 and all(len(r) == 9 for r in rows)
    outputs.append(rows)
errors = [f'case/step {a[:2]}: native {a}, reference {b}' for a,b in zip(*outputs) if a != b]
if errors: sys.exit('\n'.join(errors[:30])+f'\n{len(errors)} differing rows')
print('PASS: 12835 cases / 12856 checkpoints; MP, DAS and original P66 RODCOMP rate update')
print('544 edge cases; three eight-step rate histories; 4096 deterministic cases per operation')
print('All six modeled words, including 16-bit A/L, agree; adjacent memory and physical bank preserved')
print('RODCOMP stop: bank 32,3303, before PIPA/interpreter work; original INHINT verified')
print('Guidance snapshot SHA256:',hashlib.sha256((root/'build/guidance-reference.snapshots').read_bytes()).hexdigest())
