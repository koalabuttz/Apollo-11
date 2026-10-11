#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import hashlib,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[1];engine=root/'deps/virtualagc/yaAGC'
subprocess.run([sys.executable,'tools/prepare_hardware_dv.py'],cwd=root,check=True)
base=['gcc','-std=c99','-O2','-Iinclude','-Itests']
subprocess.run(base+['src/agc_word.c','src/agc_hardware_divide.c','tests/hardware_divide_cases.c','tools/hardware_divide_snapshot.c','-o','build/hardware-divide-native'],cwd=root,check=True)
subprocess.run(base+['-I'+str(engine),'tools/hardware_divide_reference.c','tests/hardware_divide_cases.c','tests/scenarios.c','src/agc_word.c','build/agc_engine_hardware_dv.o',str(engine/'agc_engine_init.o'),str(engine/'rfopen.o'),'-o','build/hardware-divide-reference'],cwd=root,check=True)
outputs=[]
for name in ['native','reference']:
 p=subprocess.run(['build/hardware-divide-'+name],cwd=root,capture_output=True,text=True,check=True);(root/f'build/hardware-divide-{name}.snapshots').write_text(p.stdout);outputs.append(p.stdout.splitlines())
assert len(outputs[0])==len(outputs[1])==327680
bad=[(a,b)for a,b in zip(*outputs)if a!=b]
for a,b in bad[:12]:print('DIFF',a,b)
if bad:sys.exit(f'{len(bad)} differences')
print('PASS:327680 full16-bit hardwareDV states, including zero/mixedsign/overflow operands')
print('Snapshot SHA256:',hashlib.sha256((root/'build/hardware-divide-reference.snapshots').read_bytes()).hexdigest())
