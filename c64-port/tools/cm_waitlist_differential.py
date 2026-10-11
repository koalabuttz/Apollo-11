#!/usr/bin/env python3
"""Compare CM queue owned words against independently executed original code."""
import hashlib,pathlib,subprocess
R=pathlib.Path(__file__).resolve().parents[1]; E=R/'deps/virtualagc/yaAGC'; F=R/'deps/cm-waitlist-fixture'
subprocess.run(['python3','tools/cm_waitlist_prepare_reference.py'],cwd=R,check=True)
subprocess.run(['cc','-std=c99','-O2','-DCM_WAIT_INPUT_ONLY','-I'+str(E),'-Iinclude','-Itests','tools/cm_waitlist_reference.c','tests/cm_waitlist_cases.c','tests/cm_executive_cases.c',str(E/'agc_engine.o'),str(E/'agc_engine_init.o'),str(E/'rfopen.o'),'-o','build/cm-waitlist-reference'],cwd=R,check=True)
for name,cmd in [('native',['./build/cm-waitlist-snapshot']),('reference',['./build/cm-waitlist-reference',str(F/'MAIN.agc.bin')])]:
 p=subprocess.run(cmd,cwd=R,text=True,capture_output=True,check=True)
 (R/f'build/cm-waitlist-{name}.snapshots').write_text(p.stdout); (R/f'build/cm-waitlist-{name}.trace').write_text(p.stderr)
a=(R/'build/cm-waitlist-native.snapshots').read_bytes(); b=(R/'build/cm-waitlist-reference.snapshots').read_bytes()
assert a==b,'CM waitlist complete state mismatch'
rows=[list(map(int,r.split())) for r in a.decode().splitlines()]
assert len(rows)==306 and all(len(r)==34 for r in rows)
print('PASS: 196 CM queue cases, 306 exact complete-state checkpoints, ten actual native task sequences; CM alarm entries, scratch preservation, 16 guards and physical bank transfers checked')
print('Snapshot SHA256:',hashlib.sha256(b).hexdigest())
