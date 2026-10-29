#!/usr/bin/env python3
"""Compare complete native CM state to normalized historical CM Executive."""
import hashlib,pathlib,subprocess
ROOT=pathlib.Path(__file__).resolve().parents[1]; ENGINE=ROOT/'deps/virtualagc/yaAGC'
FIXTURE=ROOT/'deps/comanche-fixture'
assert (FIXTURE/'EXECUTIVE.original.agc').read_bytes()==(ROOT.parent/'Comanche055/EXECUTIVE.agc').read_bytes()
subprocess.run(['make','build/cm-executive-snapshot'],cwd=ROOT,check=True)
subprocess.run(['cc','-std=c99','-O2','-I'+str(ENGINE),'-Iinclude','-Itests',
 'tools/cm_executive_reference.c','tests/cm_executive_cases.c',
 str(ENGINE/'agc_engine.o'),str(ENGINE/'agc_engine_init.o'),str(ENGINE/'rfopen.o'),
 '-o','build/cm-executive-reference'],cwd=ROOT,check=True)
rows=[]
for name,cmd in [('native',['./build/cm-executive-snapshot']),('reference',['./build/cm-executive-reference',str(FIXTURE/'MAIN.agc.bin')])]:
 r=subprocess.run(cmd,cwd=ROOT,text=True,capture_output=True,check=True)
 (ROOT/f'build/cm-executive-{name}.snapshots').write_text(r.stdout)
 if name=='reference': (ROOT/'build/cm-executive-reference.trace').write_text(r.stderr)
 rows.append([list(map(int,line.split())) for line in r.stdout.splitlines()])
assert all(len(x)==1536 for x in rows)
for n,(native,reference) in enumerate(zip(*rows)):
 assert len(native)==len(reference)==314,(n,'fields')
 assert native==reference,(n,[(i,a,b) for i,(a,b) in enumerate(zip(native,reference)) if a!=b])
print('PASS: 1536 Comanche admission/release cases; 312 owned state words and result agree; 16 guards preserved')
print('Snapshot SHA256:',hashlib.sha256((ROOT/'build/cm-executive-reference.snapshots').read_bytes()).hexdigest())
print('Scope: ENDFIND / BAILOUT entry / CHANJOB-2 / DUMMYJOB entry; seven CM cores, five VACs')
