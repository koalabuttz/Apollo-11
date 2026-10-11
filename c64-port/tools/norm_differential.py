#!/usr/bin/env python3
"""Compare native TP-input SQRT with source-exact original assembly."""
import hashlib,pathlib,subprocess,sys
from collections import Counter
root=pathlib.Path(__file__).resolve().parents[1];engine=root/'deps/virtualagc/yaAGC'
assert (root.parent/'Luminary099/INTERPRETER.agc').read_bytes()==(root/'deps/fixture/INTERPRETER.agc').read_bytes()
subprocess.run([sys.executable,'tools/prepare_hardware_dv.py'],cwd=root,check=True)
base=['gcc','-std=c99','-O2','-Iinclude','-Itests']
subprocess.run(base+['tests/norm_snapshot.c','src/guidance_norm.c','src/agc_word.c','src/agc_multiword.c','-o','build/norm-native'],cwd=root,check=True)
subprocess.run(base+['-I'+str(engine),'tools/norm_reference.c','tests/scenarios.c','build/agc_engine_hardware_dv.o',str(engine/'agc_engine_init.o'),str(engine/'rfopen.o'),'-o','build/norm-reference'],cwd=root,check=True)
outputs=[]
for name,cmd in [('native',['build/norm-native']),('reference',['build/norm-reference','deps/fixture/MAIN.agc.bin'])]:
 run=subprocess.run(cmd,cwd=root,capture_output=True,text=True,check=True);(root/f'build/norm-{name}.snapshots').write_text(run.stdout);outputs.append([list(map(int,l.split()))for l in run.stdout.splitlines()])
assert len(outputs[0])==len(outputs[1])==132096
errors=[(a,b)for a,b in zip(*outputs)if a!=b]
for a,b in errors[:16]:print('DIFF',a,b)
if errors:sys.exit(f'{len(errors)} differing cases')
print('PASS: 132096 SQRT/SQRTSUB cases, all 20 modeled raw words, normal and pre-1302 boundaries match original assembly/hardware DV')

print("Paths root/pre-alarm1302:",dict(sorted(Counter(r[1] for r in outputs[1]).items())))
print("Snapshot SHA256:",hashlib.sha256((root/"build/norm-reference.snapshots").read_bytes()).hexdigest())
