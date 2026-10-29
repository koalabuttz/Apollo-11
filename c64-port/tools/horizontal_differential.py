#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import hashlib,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[1];engine=root/'deps/virtualagc/yaAGC';fixture=root/'deps/fixture'
assert (root.parent/'Luminary099/INTERPRETER.agc').read_bytes()==(fixture/'INTERPRETER.agc').read_bytes()
base=['gcc','-std=c99','-O2','-Iinclude','-Itests','-DUNIT_INPUT_ONLY','-DDIVIDE_INPUT_ONLY','-DSTORE_INPUT_ONLY','-DELAPSED_INPUT_ONLY','-DPUSH_INPUT_ONLY','-DSCALE_INPUT_ONLY','-DPREPARE_INPUT_ONLY']
inputs=['tests/prepare_cases.c','tests/scale_cases.c','tests/push_cases.c','tests/elapsed_cases.c','tests/store_cases.c','tests/divide_cases.c','tests/unit_cases.c','tests/horizontal_cases.c']
subprocess.run(base+['src/agc_word.c','src/agc_multiword.c','src/guidance_product.c','src/guidance_vector.c','src/guidance_sum.c','src/guidance_shift.c','src/guidance_dp_divide.c','src/guidance_norm.c','src/guidance_unit.c','src/guidance_horizontal.c','tools/horizontal_snapshot.c']+inputs+['-o','build/horizontal-native'],cwd=root,check=True)
subprocess.run(base+['-DHORIZONTAL_INPUT_ONLY','-I'+str(engine),'tools/horizontal_reference.c','tests/scenarios.c']+inputs+['build/agc_engine_hardware_dv.o',str(engine/'agc_engine_init.o'),str(engine/'rfopen.o'),'-o','build/horizontal-reference'],cwd=root,check=True)
outputs=[]
for name,cmd in [('native',['build/horizontal-native']),('reference',['build/horizontal-reference','deps/fixture/MAIN.agc.bin'])]:
 run=subprocess.run(cmd,cwd=root,capture_output=True,text=True);(root/f'build/horizontal-{name}.snapshots').write_text(run.stdout);(root/f'build/horizontal-{name}.trace').write_text(run.stderr)
 if run.returncode:sys.exit(run.stderr[-3000:])
 outputs.append([list(map(int,r.split()))for r in run.stdout.splitlines()])
assert len(outputs[0])==len(outputs[1])
errors=[(a,b)for a,b in zip(*outputs)if a!=b]
for a,b in errors[:16]:print('DIFF',a[:3],[(i,x,y)for i,(x,y)in enumerate(zip(a[3:],b[3:]))if x!=y])
if errors:sys.exit(f'{len(errors)} differing rows')
print('PASS8192 complete120word phased horizontal cases match original assembly/hardware DV')
print('Snapshot SHA256:',hashlib.sha256((root/'build/horizontal-reference.snapshots').read_bytes()).hexdigest())

for name,cmd in [('native',['build/horizontal-native','--inputs']),('reference',['build/horizontal-reference','deps/fixture/MAIN.agc.bin','--inputs'])]:
 run=subprocess.run(cmd,cwd=root,capture_output=True,text=True,check=True);(root/f'build/horizontal-{name}.inputs').write_text(run.stdout)
assert (root/'build/horizontal-native.inputs').read_bytes()==(root/'build/horizontal-reference.inputs').read_bytes()
