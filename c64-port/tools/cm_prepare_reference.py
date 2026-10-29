#!/usr/bin/env python3
"""Prepare Comanche Executive fixture; normalize five historical pseudo-labels."""
import difflib,hashlib,pathlib,re,shutil,subprocess
ROOT=pathlib.Path(__file__).resolve().parents[1]
PIN='1b5d6474841a7ab53237ead070f76f35f162f68d'
V=ROOT/'deps/virtualagc'; F=ROOT/'deps/comanche-fixture'
assert subprocess.check_output(['git','-C',str(V),'rev-parse','HEAD'],text=True).strip()==PIN
F.mkdir(parents=True,exist_ok=True)
for p in (V/'Comanche055').glob('*.agc'): shutil.copyfile(p,F/p.name)
original=(ROOT.parent/'Comanche055/EXECUTIVE.agc').read_text()
(F/'EXECUTIVE.original.agc').write_text(original)
patched=original
changes=[('\t+2\tCAF',' +2\t\tCAF'),('\t-2\tLXCH',' -2\t\tLXCH'),
         ('\t-1\tCAE',' -1\t\tCAE'),('\t+4\tINDEX',' +4\t\tINDEX'),
         ('\n+1\t\tEXTEND','\n +1\t\tEXTEND')]
for before,after in changes:
    assert patched.count(before)==1,before
    patched=patched.replace(before,after)
(F/'EXECUTIVE.agc').write_text(patched)
(F/'executive-indentation.patch').write_text(''.join(difflib.unified_diff(original.splitlines(True),patched.splitlines(True),fromfile='Comanche055/EXECUTIVE.agc',tofile='fixture/EXECUTIVE.agc')))
with (F/'assembly.lst').open('w') as log:
 subprocess.run([str(V/'yaYUL/yaYUL'),'MAIN.agc'],cwd=F,stdout=log,stderr=subprocess.STDOUT,check=True)
listing=(F/'assembly.lst').read_text()
for name,address in [('NOVAC','5134'),('FINDVAC','5147'),('ENDFIND','5222'),
 ('BAILOUT','7755'),('CHANJOB','01,2727'),('ENDJOB1','01,3124'),('DUMMYJOB','01,3223'),
 ('MPAC','0154'),('VAC1USE','0400'),('PRIORITY','0167'),('NEWJOB','0067'),('FIXLOC','0120'),('OVFIND','0121')]:
 assert re.search(r'(?<!\w)'+re.escape(name)+r'\s+'+address+r'\b',listing),name
# Independently assemble the pinned corrected Executive. All rope words must
# agree, proving the narrow normalization changes only transcription layout.
rope=(F/'MAIN.agc.bin').read_bytes()
shutil.copyfile(V/'Comanche055/EXECUTIVE.agc',F/'EXECUTIVE.agc')
with (F/'pinned-assembly.lst').open('w') as log:
 subprocess.run([str(V/'yaYUL/yaYUL'),'MAIN.agc'],cwd=F,stdout=log,stderr=subprocess.STDOUT,check=True)
assert rope==(F/'MAIN.agc.bin').read_bytes(),'normalized/pinned rope mismatch'
(F/'EXECUTIVE.agc').write_text(patched)
assert (F/'EXECUTIVE.original.agc').read_bytes()==(ROOT.parent/'Comanche055/EXECUTIVE.agc').read_bytes()
print('PASS: five indentation-only normalizations; complete rope equals pinned corrected Comanche 055')
print('Virtual AGC pin:',PIN)
print('Original Comanche Executive SHA256:',hashlib.sha256(original.encode()).hexdigest())
print('Normalized Executive SHA256:',hashlib.sha256(patched.encode()).hexdigest())
print('Rope SHA256:',hashlib.sha256(rope).hexdigest())
print('Fixture:',F)
