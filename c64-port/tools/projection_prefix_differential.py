#!/usr/bin/env python3
"""Compare full RODCOMP prefix through AFC against original source, pinned hardware DV."""
import hashlib,pathlib,re,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[1];reference=pathlib.Path(sys.argv[1]).resolve();engine=root/'deps/virtualagc/yaAGC'
for name in ('INTERPRETER.agc','LUNAR_LANDING_GUIDANCE_EQUATIONS.agc'):
    assert (root.parent/'Luminary099'/name).read_bytes()==(reference/name).read_bytes()
listing=(reference/'assembly.lst').read_text()
for name,address in [('VAD','6751'),('VSU','6747'),('VXSC','7401'),('STADR','6354'),('DANZIG','6061'),('R','E7,1520'),('V','E7,1526'),('VBIAS','E5,1630')]:
    assert re.search(r'(?<!\w)'+re.escape(name)+r'\s+'+address+r'\b',listing),name
inputs=['tests/prepare_cases.c','tests/scale_cases.c','tests/push_cases.c','tests/elapsed_cases.c','tests/store_cases.c','tests/divide_cases.c','tests/gravity_cases.c','tests/velocity_cases.c','tests/radial_cases.c','tests/unit_cases.c','tests/trig_cases.c','tests/projection_cases.c']
core=['src/agc_word.c','src/agc_multiword.c','src/guidance.c','src/guidance_prepare.c','src/guidance_scale.c','src/guidance_push.c','src/guidance_elapsed.c','src/guidance_store.c','src/guidance_divide.c','src/guidance_gravity.c','src/guidance_vector.c','src/guidance_product.c','src/guidance_shift.c','src/guidance_velocity.c','src/guidance_dp_divide.c','src/guidance_norm.c','src/guidance_unit.c','src/guidance_dot.c','src/guidance_sum.c','src/guidance_radial.c','src/guidance_acceleration.c','src/guidance_trig.c','src/guidance_rotation.c','src/guidance_projection.c']
subprocess.run(['gcc','-std=c99','-O2','-Iinclude','-Itests']+core+inputs+['tools/projection_prefix_snapshot.c','-o','build/projection-prefix-snapshot'],cwd=root,check=True)
subprocess.run([sys.executable,'tools/prepare_hardware_dv.py'],cwd=root,check=True)
flags=['-D'+name+'_INPUT_ONLY' for name in ['PROJECTION','TRIG','UNIT','RADIAL','VELOCITY','GRAVITY','DIVIDE','STORE','ELAPSED','PUSH','SCALE','PREPARE']]
subprocess.run(['gcc','-std=c99','-O2']+flags+['-Iinclude','-Itests','-I'+str(engine),'tools/projection_prefix_reference.c','tests/scenarios.c']+inputs+[str(root/'build/agc_engine_hardware_dv.o'),str(engine/'agc_engine_init.o'),str(engine/'rfopen.o'),'-o','build/projection-prefix-reference'],cwd=root,check=True)
outputs=[]
for name,cmd in [('native',['./build/projection-prefix-snapshot']),('reference',['./build/projection-prefix-reference',str(reference/'MAIN.agc.bin')])]:
    run=subprocess.run(cmd,cwd=root,capture_output=True,text=True); (root/f'build/projection-prefix-{name}.snapshots').write_text(run.stdout)
    if name=='reference': (root/'build/projection-prefix-reference.trace').write_text(run.stderr)
    if run.returncode: sys.exit(run.stderr[-3000:] or f'{name} exited {run.returncode}')
    rows=[list(map(int,r.split())) for r in run.stdout.splitlines()]; assert len(rows)==14856 and all(len(r)==170 for r in rows),(name,len(rows))
    outputs.append(rows)
errors=[]
for a,b in zip(*outputs):
    if a!=b: errors.append(f'case/step/stage {a[:4]}: differing words '+str([(i-4,x,y) for i,(x,y) in enumerate(zip(a,b)) if x!=y]))
if errors: sys.exit('\n'.join(errors[:20])+f'\n{len(errors)} differing rows')
print('PASS: 2467 inputs / 14856 full-prefix checkpoints through projection match unchanged source, 166 raw state words')
print('Snapshot SHA256:',hashlib.sha256((root/'build/projection-prefix-reference.snapshots').read_bytes()).hexdigest())
