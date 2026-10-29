#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import hashlib,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[1];engine=root/'deps/virtualagc/yaAGC';fixture=root/'deps/fixture'
assert (root.parent/'Luminary099/INTERPRETER.agc').read_bytes()==(fixture/'INTERPRETER.agc').read_bytes()
base=['gcc','-std=c99','-O2','-Iinclude','-Itests','-DUNIT_INPUT_ONLY','-DDIVIDE_INPUT_ONLY','-DSTORE_INPUT_ONLY','-DELAPSED_INPUT_ONLY','-DPUSH_INPUT_ONLY','-DSCALE_INPUT_ONLY','-DPREPARE_INPUT_ONLY']
inputs=['tests/prepare_cases.c','tests/scale_cases.c','tests/push_cases.c','tests/elapsed_cases.c','tests/store_cases.c','tests/divide_cases.c','tests/unit_cases.c','tests/trig_cases.c']
subprocess.run(base+['src/agc_word.c','src/agc_multiword.c','src/guidance_product.c','src/guidance_trig.c','tools/trig_snapshot.c']+inputs+['-o','build/trig-native'],cwd=root,check=True)
subprocess.run(base+['-DTRIG_INPUT_ONLY','-I'+str(engine),'tools/trig_reference.c','tests/scenarios.c']+inputs+[str(engine/'agc_engine.o'),str(engine/'agc_engine_init.o'),str(engine/'rfopen.o'),'-o','build/trig-reference'],cwd=root,check=True)
outputs=[]
for name,cmd in [('native',['build/trig-native']),('reference',['build/trig-reference','deps/fixture/MAIN.agc.bin'])]:
 run=subprocess.run(cmd,cwd=root,capture_output=True,text=True);(root/f'build/trig-{name}.snapshots').write_text(run.stdout);(root/f'build/trig-{name}.trace').write_text(run.stderr)
 if run.returncode:sys.exit(run.stderr[-3000:])
 outputs.append([list(map(int,r.split()))for r in run.stdout.splitlines()])
assert len(outputs[0])==len(outputs[1])==65536
errors=[(a,b)for a,b in zip(*outputs)if a!=b]
for a,b in errors[:16]:print('DIFF',a[:2],[(i,x,y)for i,(x,y)in enumerate(zip(a[2:],b[2:]))if x!=y])
if errors:sys.exit(f'{len(errors)} differing rows')
print('PASS:65536 complete130word CDULOGIC/SIN/COS/CDUTRIGS states match original assembly')
print('Snapshot SHA256:',hashlib.sha256((root/'build/trig-reference.snapshots').read_bytes()).hexdigest())
