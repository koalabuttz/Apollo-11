#!/usr/bin/env python3
"""Build pinned Virtual AGC and a full fixture containing UNCHANGED upstream scheduler/alarm/restart/guidance source."""
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
for source in ['EXECUTIVE.agc','WAITLIST.agc','INTERRUPT_LEAD_INS.agc',
               'ALARM_AND_ABORT.agc','FRESH_START_AND_RESTART.agc',
               'RESTARTS_ROUTINE.agc','RESTART_TABLES.agc','PHASE_TABLE_MAINTENANCE.agc',
               'LUNAR_LANDING_GUIDANCE_EQUATIONS.agc','INTERPRETER.agc','CONTROLLED_CONSTANTS.agc']:
    shutil.copyfile(ROOT.parent/'Luminary099'/source,fixture/source)
with (fixture/'assembly.lst').open('w') as log:
    run([str(v/'yaYUL/yaYUL'),'MAIN.agc'],cwd=fixture,stdout=log,stderr=subprocess.STDOUT)
for source in ['EXECUTIVE.agc','WAITLIST.agc','INTERRUPT_LEAD_INS.agc',
               'ALARM_AND_ABORT.agc','FRESH_START_AND_RESTART.agc',
               'RESTARTS_ROUTINE.agc','RESTART_TABLES.agc','PHASE_TABLE_MAINTENANCE.agc',
               'LUNAR_LANDING_GUIDANCE_EQUATIONS.agc','INTERPRETER.agc','CONTROLLED_CONSTANTS.agc']:
    assert (fixture/source).read_bytes()==(ROOT.parent/'Luminary099'/source).read_bytes()
# The adapter uses these historical addresses; fail if the support changes them.
listing=(fixture/'assembly.lst').read_text()
for name,address in [('MPAC','0154'),('PRIORITY','0167'),('VAC1USE','0400'),
                     ('ENDFIND','5160'),('BAILOUT1','5716'),('CHANG1','5122'),
                     ('CHANJOB','01,2706'),('ENDPRCHG','01,2765'),
                     ('NUCHANG2','01,3011'),('NUDIRECT','01,3225'),
                     ('ENDJOB1','01,3103'),('DUMMYJOB','01,3206'),
                     ('JOBSLEEP','5133'),('JOBWAKE','5137'),('PRIOCHNG','5146'),
                     ('JOBSLP1','01,2776'),('JOBWAKE2','01,3023'),
                     ('PRIOCH2','01,3072'),('EJSCAN','01,3120'),
                     ('WAITLIST','5203'),('WAIT2','01,3231'),('LVWTLIST','5215'),
                     ('ENDTASK','5236'),('TASKOVER','5261'),('T3RUPT','01,3407'),
                     ('T3RUPT2','01,3414'),('LST1','E3,1400'),('LST2','E3,1410'),
                     ('TIME3','0000026'),('RUPTAGN','0734'),
                     ('ALARM1','5732'),('POODOO1','5726'),('WHIMPER','5644'),
                     ('STARTSB1','05,3116'),('GOPROG3','05,2775'),
                     ('PTBAD','05,3054'),('RESTARTS','01,3526'),
                     ('PHSNAME1','E3,1436'),('PHSPRDT1','1054'),('GOLOC','0705'),
                     ('RODCOMP','32,3275'),('RODCOUNT','E7,1746'),
                     ('RODSCAL1','E7,1756'),('VDGVERT','E7,1644'),
                     ('OLDPIPAX','E7,1763'),('THISTPIP','E7,1761'),
                     ('DELVROD','E7,1766'),('PIPATMPX','1160'),
                     ('TEMX','1254'),('MODE','0163'),('VXSC','7401'),
                     ('DANZIG','6061'),('VROUND','7140'),('DMPSUB','7107'),
                     ('KPIP1','33,2021'),('MPTEMP','0135'),('OVFIND','0121'),
                     ('LOC','0164'),('BANKSET','0165'),('ADDRWD','0116'),
                     ('PDDL','6523'),('ENDVPUSH','6540'),('PUSHLOC','0166'),
                     ('NO.WDS','6244'),('VAC1','0401'),('VAC5','0661'),('FIXLOC','0120')]:
    import re
    if not re.search(r'\b'+name+r'\s+'+address+r'\b',listing):
        raise SystemExit('fixture address mismatch: '+name)
print('Fixture:',fixture)
print('Unchanged Executive SHA256:',hashlib.sha256(original.read_bytes()).hexdigest())
for source in ['WAITLIST.agc','INTERRUPT_LEAD_INS.agc','ALARM_AND_ABORT.agc',
               'FRESH_START_AND_RESTART.agc','RESTARTS_ROUTINE.agc',
               'RESTART_TABLES.agc','PHASE_TABLE_MAINTENANCE.agc',
               'LUNAR_LANDING_GUIDANCE_EQUATIONS.agc','INTERPRETER.agc','CONTROLLED_CONSTANTS.agc']:
    print('Unchanged '+source+' SHA256:',hashlib.sha256((fixture/source).read_bytes()).hexdigest())
print('Rope SHA256:',hashlib.sha256((fixture/'MAIN.agc.bin').read_bytes()).hexdigest())
