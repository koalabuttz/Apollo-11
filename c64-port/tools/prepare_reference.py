#!/usr/bin/env python3
"""Build pinned Virtual AGC and a full fixture containing UNCHANGED upstream Executive."""
import argparse
import hashlib
import pathlib
import shutil
import subprocess

ROOT=pathlib.Path(__file__).resolve().parents[1]
PIN='1b5d6474841a7ab53237ead070f76f35f162f68d'
p=argparse.ArgumentParser()
p.add_argument('--virtualagc',type=pathlib.Path)
p.add_argument('--fixture',type=pathlib.Path,default=ROOT/'deps/fixture')
a=p.parse_args()
v=(a.virtualagc or ROOT/'deps/virtualagc').resolve()
v.parent.mkdir(parents=True,exist_ok=True)
def run(args,**kw): return subprocess.run(args,check=True,**kw)
if not v.exists():
    run(['git','clone','https://github.com/virtualagc/virtualagc.git',str(v)])
    run(['git','-C',str(v),'checkout','--detach',PIN])
actual=subprocess.check_output(['git','-C',str(v),'rev-parse','HEAD'],text=True).strip()
if actual!=PIN: raise SystemExit('Virtual AGC revision mismatch: '+actual)
for sub,targets in [('yaYUL',['yaYUL']),('yaAGC',['yaAGC','libyaAGC.a'])]:
    run(['make','-C',str(v/sub),'-j4','cc=gcc','NVER=\\"'+PIN[:12]+'\\"']+targets)
fixture=a.fixture.resolve(); fixture.mkdir(parents=True,exist_ok=True)
for f in (v/'Luminary099').glob('*.agc'): shutil.copyfile(f,fixture/f.name)
original=ROOT.parent/'Luminary099/EXECUTIVE.agc'
shutil.copyfile(original,fixture/'EXECUTIVE.agc')
with (fixture/'assembly.lst').open('w') as log:
    run([str(v/'yaYUL/yaYUL'),'MAIN.agc'],cwd=fixture,stdout=log,stderr=subprocess.STDOUT)
assert (fixture/'EXECUTIVE.agc').read_bytes()==original.read_bytes()
# The adapter uses these historical addresses; fail if the support changes them.
listing=(fixture/'assembly.lst').read_text()
for name,address in [('MPAC','0154'),('PRIORITY','0167'),('VAC1USE','0400'),
                     ('ENDFIND','5160'),('BAILOUT1','5716'),('CHANG1','5122'),
                     ('CHANJOB','01,2706'),('ENDPRCHG','01,2765'),
                     ('NUCHANG2','01,3011'),('NUDIRECT','01,3225'),
                     ('ENDJOB1','01,3103'),('DUMMYJOB','01,3206'),
                     ('JOBSLEEP','5133'),('JOBWAKE','5137'),('PRIOCHNG','5146'),
                     ('JOBSLP1','01,2776'),('JOBWAKE2','01,3023'),
                     ('PRIOCH2','01,3072'),('EJSCAN','01,3120')]:
    import re
    if not re.search(r'\b'+name+r'\s+'+address+r'\b',listing):
        raise SystemExit('fixture address mismatch: '+name)
print('Fixture:',fixture)
print('Unchanged Executive SHA256:',hashlib.sha256(original.read_bytes()).hexdigest())
print('Rope SHA256:',hashlib.sha256((fixture/'MAIN.agc.bin').read_bytes()).hexdigest())
