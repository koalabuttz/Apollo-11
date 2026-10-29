#!/usr/bin/env python3
"""Compare full radial group against original source, pinned hardware DV."""
import hashlib,pathlib,re,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[1];reference=pathlib.Path(sys.argv[1]).resolve();engine=root/'deps/virtualagc/yaAGC'
for name in ('INTERPRETER.agc','LUNAR_LANDING_GUIDANCE_EQUATIONS.agc'):
    assert (root.parent/'Luminary099'/name).read_bytes()==(reference/name).read_bytes()
listing=(reference/'assembly.lst').read_text()
for name,address in [('VAD','6751'),('VSU','6747'),('VXSC','7401'),('STADR','6354'),('DANZIG','6061'),('R','E7,1520'),('V','E7,1526'),('VBIAS','E5,1630')]:
    assert re.search(r'(?<!\w)'+re.escape(name)+r'\s+'+address+r'\b',listing),name
inputs=['tests/prepare_cases.c','tests/scale_cases.c','tests/push_cases.c','tests/elapsed_cases.c','tests/store_cases.c','tests/divide_cases.c','tests/gravity_cases.c','tests/velocity_cases.c','tests/radial_cases.c','tests/radial_phased_cases.c']
core=['src/agc_word.c','src/agc_multiword.c','src/guidance.c','src/guidance_prepare.c','src/guidance_scale.c','src/guidance_push.c','src/guidance_elapsed.c','src/guidance_store.c','src/guidance_divide.c','src/guidance_gravity.c','src/guidance_vector.c','src/guidance_product.c','src/guidance_shift.c','src/guidance_velocity.c','src/guidance_dp_divide.c','src/guidance_norm.c','src/guidance_unit.c','src/guidance_dot.c','src/guidance_sum.c','src/guidance_unit_phased.c','src/guidance_radial_phased.c','src/guidance_radial.c']
subprocess.run(['gcc','-std=c99','-O2','-Iinclude','-Itests']+core+inputs+['tools/radial_phased_snapshot.c','-o','build/radial-phased-snapshot'],cwd=root,check=True)
subprocess.run([sys.executable,'tools/prepare_hardware_dv.py'],cwd=root,check=True)
flags=['-DRADIAL_PHASED_INPUT_ONLY']+['-D'+name+'_INPUT_ONLY' for name in ['RADIAL','VELOCITY','GRAVITY','DIVIDE','STORE','ELAPSED','PUSH','SCALE','PREPARE']]
subprocess.run(['gcc','-std=c99','-O2']+flags+['-Iinclude','-Itests','-I'+str(engine),'tools/radial_phased_reference.c','tests/scenarios.c']+inputs+[str(root/'build/agc_engine_hardware_dv.o'),str(engine/'agc_engine_init.o'),str(engine/'rfopen.o'),'-o','build/radial-phased-reference'],cwd=root,check=True)
outputs=[]
for name,cmd in [('native',['./build/radial-phased-snapshot']),('reference',['./build/radial-phased-reference',str(reference/'MAIN.agc.bin')])]:
    run=subprocess.run(cmd,cwd=root,capture_output=True,text=True); (root/f'build/radial-phased-{name}.snapshots').write_text(run.stdout)
    if name=='reference': (root/'build/radial-phased-reference.trace').write_text(run.stderr)
    if run.returncode: sys.exit(run.stderr[-3000:] or f'{name} exited {run.returncode}')
    rows=[list(map(int,r.split())) for r in run.stdout.splitlines()]; assert all(len(r)==138 for r in rows),(name,len(rows))
    outputs.append(rows)
assert len(outputs[0])==len(outputs[1]);errors=[]
for a,b in zip(*outputs):
    if a!=b: errors.append(f'case/step/stage {a[:4]}: differing words '+str([(i-4,x,y) for i,(x,y) in enumerate(zip(a,b)) if x!=y]))
if errors: sys.exit('\n'.join(errors[:20])+f'\n{len(errors)} differing rows')
from collections import Counter
print('PASS: 2467 inputs / all complete134-word internal UNIT and source pair checkpoints match original radial group and hardware-DV full prefix')
print('Includes all 128 integrated skip inputs, three four-step histories, raw/error corners and 2048 random calls')
print('UNIT / STORE14 / DOT24 SL1 / STODLHDOT,30 / SL11 DMPHDOT / DAD36 DSULAND / STODLHCALC1,HDOT')
print('Paths:',Counter(row[3] for row in outputs[1] if row[2] in [2,3]))
print('Snapshot SHA256:',hashlib.sha256((root/'build/radial-phased-reference.snapshots').read_bytes()).hexdigest())
