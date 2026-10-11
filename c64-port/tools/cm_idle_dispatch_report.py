#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Retain immutable proof artifacts for one source-qualified CM canonical service/application stage."""
import argparse,gzip,hashlib,json,pathlib,re,shutil
R=pathlib.Path(__file__).resolve().parents[1];B=R/'build';E=R/'evidence'
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('stage')
a=p.parse_args();name='cm-'+a.stage;stem=a.stage.replace('-','_');E.mkdir(exist_ok=True)
assert re.fullmatch(r'[a-z]+(?:-[a-z]+)*',a.stage)
count=int(re.search(r'^count=(\d+)$',(R/f'tools/cm_{stem}_vice_test.py').read_text(),re.M)[1])
maptext=(B/(name+'.map')).read_text();resources={'prg_bytes':(B/(name+'.prg')).stat().st_size};end=0
for section in ['text','rodata','data','bss','noinit']:
 m=re.search(r'^\s*([0-9a-f]+)\s+[0-9a-f]+\s+([0-9a-f]+)\s+1\s+\.'+section+r'$',maptext,re.M)
 if m:resources['.'+section]={'address':int(m[1],16),'bytes':int(m[2],16)};end=max(end,int(m[1],16)+int(m[2],16))
resources['image_end']=end
(B/(name+'-resources.json')).write_text(json.dumps(resources,indent=2)+'\n')
proof=json.loads((B/(name+'-proof.json')).read_text())
reference=(B/(name+'-reference.snapshots')).read_bytes();native=(B/(name+'-native.snapshots')).read_bytes()
assert reference==native
assert hashlib.sha256(reference).hexdigest()==proof['snapshot_sha256']
assert (B/(name+'-mailbox.bin')).read_bytes()==bytes([0xa5,0,0,count&255,count>>8])
assert 'PASS:' in (B/(name+'-c64-result.log')).read_text()
owned=[]
def retain(src,dst):
 target=E/dst;payload=src.read_bytes()
 if target.exists():assert target.read_bytes()==payload,'Published artifact identity changed: '+str(target)
 else:target.write_bytes(payload)
 owned.append(target.relative_to(R).as_posix())
for role,payload in [('reference',reference),('native',native)]:
 target=E/(name+'-'+role+'.snapshots.gz')
 if target.exists():assert gzip.decompress(target.read_bytes())==payload
 else:
  with target.open('wb') as f:
   with gzip.GzipFile(fileobj=f,filename='',mode='wb',mtime=0) as z:z.write(payload)
 owned.append(target.relative_to(R).as_posix())
for suffix in ['proof.json','resources.json','map','prg','mailbox.bin','hardware-stack.bin','software-stack.bin','c64-result.log','monitor.log','vice.log','source-validation.log','build-validation.log','build.log','reference.trace','native.trace']:
 src=B/(name+('.'+suffix if suffix in ('map','prg') else '-'+suffix))
 if src.exists():retain(src,src.name)
retain(B/'cm-finish-source-audit.json',name+'-source-audit.json')
retain(R/'deps/cm-finish-fixture/INTERPRETER.agc.indentation.patch',name+'-INTERPRETER.agc.indentation.patch')
source='RESTARTS_ROUTINE.agc'
if a.stage=='restart-select':
 retain(B/'cm-restart-types-source-audit.json','cm-restart-types-source-audit.json')
else:
 if a.stage in ('entry-tasks','clocktask','tig5'):
  retain(B/'cm-entry-tasks-source-audit.json',name+'-source-audit.json')
  F=R/'deps/cm-entry-tasks-fixture'
  for source in ['CM_ENTRY_DIGITAL_AUTOPILOT.agc','P40-P47.agc']:
   retain(F/(source+'.indentation.patch'),name+'-'+source+'.indentation.patch')
for part,base,top in [('hardware',0x100,0x1f2),('software',0xc100,0xd000)]:
 values=(B/(name+'-'+part+'-stack.bin')).read_bytes();modified=[i for i,v in enumerate(values) if v!=0xcd]
 proof[part+'_stack_bytes']=top-base-min(modified) if modified else 0
proof['c64_cases']=count;proof['mailbox_hex']=(B/(name+'-mailbox.bin')).read_bytes().hex()
proof['target']=json.loads((B/(name+'-resources.json')).read_text())
report=E/(name+'-report.json');payload=(json.dumps(proof,indent=2)+'\n').encode()
if report.exists():assert report.read_bytes()==payload
else:report.write_bytes(payload)
owned.append(report.relative_to(R).as_posix())
files=[]
for pattern in [f'include/cm_{stem}.h',f'src/cm_{stem}.c',f'tests/cm_{stem}*',f'tests/test_cm_{stem}.c',f'platform/host/cm_{stem}_snapshot.c',f'platform/c64/cm_{stem}_diagnostic.c',f'tools/cm_{stem}*',f'tools/cm_freeze_{stem}.py',f'docs/COMANCHE_{stem.upper()}.md']:
 files.extend(x.relative_to(R).as_posix() for x in R.glob(pattern) if x.is_file())
if a.stage=='restart-select':files+=['include/cm_restart_table_data.h','tools/cm_restart_types_prepare_reference.py']
if a.stage=='entry-tasks':files+=['tools/cm_entry_tasks_prepare_reference.py']
attempt=B/(name+'-full-target-attempt')
if attempt.exists():
 for src in sorted(attempt.iterdir()):retain(src,name+'-full-target-attempt-'+src.name)
validation=B/'cm-representative-target-build.log'
if a.stage in ('restart-timing','longcall','restart-select','clocktask') and validation.exists():retain(validation,name+'-subset-build-validation.log')
files+=owned
files+=['tools/cm_finish_prepare_reference.py','tests/cm_basic_finish_cases.c','tests/cm_basic_finish_cases.h']
manifest=E/(name+'-evidence.sha256')
manifest.write_text(''.join(hashlib.sha256((R/f).read_bytes()).hexdigest()+'  '+f+'\n' for f in sorted(set(files))))
files.append(manifest.relative_to(R).as_posix())
(B/(name+'-commit-files.txt')).write_text('\n'.join(sorted(set(files)))+'\n')
print(json.dumps(proof,indent=2));print('Commit list:',B/(name+'-commit-files.txt'))
