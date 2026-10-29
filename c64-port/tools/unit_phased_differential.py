#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import hashlib,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[1];engine=root/'deps/virtualagc/yaAGC';fixture=root/'deps/fixture'
assert (root.parent/'Luminary099/INTERPRETER.agc').read_bytes()==(fixture/'INTERPRETER.agc').read_bytes()
base=['gcc','-std=c99','-O2','-Iinclude','-Itests','-DUNIT_INPUT_ONLY','-DDIVIDE_INPUT_ONLY','-DSTORE_INPUT_ONLY','-DELAPSED_INPUT_ONLY','-DPUSH_INPUT_ONLY','-DSCALE_INPUT_ONLY','-DPREPARE_INPUT_ONLY']
inputs=['tests/prepare_cases.c','tests/scale_cases.c','tests/push_cases.c','tests/elapsed_cases.c','tests/store_cases.c','tests/divide_cases.c','tests/unit_cases.c','tests/unit_phased_cases.c']
subprocess.run(base+['src/agc_word.c','src/agc_multiword.c','src/guidance_norm.c','src/guidance_unit_phased.c','tools/unit_phased_snapshot.c']+inputs+['-o','build/unit-phased-native'],cwd=root,check=True)
subprocess.run(base+['-DUNIT_PHASED_INPUT_ONLY','-I'+str(engine),'tools/unit_phased_reference.c','tests/scenarios.c']+inputs+['build/agc_engine_hardware_dv.o',str(engine/'agc_engine_init.o'),str(engine/'rfopen.o'),'-o','build/unit-phased-reference'],cwd=root,check=True)
outputs=[]
for name,cmd in [('native',['build/unit-phased-native']),('reference',['build/unit-phased-reference','deps/fixture/MAIN.agc.bin'])]:
 run=subprocess.run(cmd,cwd=root,capture_output=True,text=True);(root/f'build/unit-phased-{name}.snapshots').write_text(run.stdout);(root/f'build/unit-phased-{name}.trace').write_text(run.stderr)
 if run.returncode:sys.exit(run.stderr[-3000:])
 outputs.append([list(map(int,r.split()))for r in run.stdout.splitlines()])
assert len(outputs[0])==len(outputs[1])
errors=[(a,b)for a,b in zip(*outputs)if a!=b]
for a,b in errors[:16]:print('DIFF',a[:3],[(i,x,y)for i,(x,y)in enumerate(zip(a[3:],b[3:]))if x!=y])
if errors:sys.exit(f'{len(errors)} differing rows')
print('PASS17168 UNIT cases including256 direct negative-root alarm boundary inputs, all104 words at square/root/quotient overlay returns match original assembly/hardware DV')
print('Snapshot SHA256:',hashlib.sha256((root/'build/unit-phased-reference.snapshots').read_bytes()).hexdigest())

for name,cmd in [('native',['build/unit-phased-native','--inputs']),('reference',['build/unit-phased-reference','deps/fixture/MAIN.agc.bin','--inputs'])]:
 run=subprocess.run(cmd,cwd=root,capture_output=True,text=True,check=True);(root/f'build/unit-phased-{name}.inputs').write_text(run.stdout)
assert (root/'build/unit-phased-native.inputs').read_bytes()==(root/'build/unit-phased-reference.inputs').read_bytes()
