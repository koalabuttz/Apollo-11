#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Freeze canonical source task/idle bridges and exact target artifacts."""
import hashlib,json,lzma,shutil,subprocess
from pathlib import Path
root=Path(__file__).resolve().parents[1];build=root/'build';evidence=root/'evidence'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
owned=['docs/LM-RECURRENCE-BRIDGES.md','tools/lm_recurrence_bridges.mk','tools/package_lm_recurrence_bridges.py','tests/test_lm_recurrence_bridge.c']
for name in ('lm_idle_start','lm_taskover_resume','lm_waitlist_dispatch'):
 owned.extend(['include/'+name+'.h','src/'+name+'.c','tests/'+name+'_cases.h','tests/'+name+'_cases.c','tools/'+name+'_reference.c','tools/'+name+'_snapshot.c','tools/'+name+'_differential.py'])
for name in ('lm_recurrence_bridge','lm_waitlist_dispatch'):
 owned.extend(['tools/'+name+'_inputs.c','tools/'+name+'_target_build.py','tools/'+name+'_vice_test.py','platform/c64/'+name+'_diagnostic.c'])
owned.append('platform/c64/lm_recurrence_bridge_fixture.h')
def retain(src,name=None):
 dest=evidence/(name or src.name);shutil.copyfile(src,dest);owned.append(str(dest.relative_to(root)))
streams={}
for name,count,words in [('lm-idle-start',8192,2052),('lm-taskover-resume',8192,2053),('lm-waitlist-dispatch',8192,2053)]:
 source=build/(name+'-reference.snapshots');digest=sha(source);assert digest==sha(build/(name+'-native.snapshots'))==sha(build/(name+'-sanitize.snapshots'));dest=evidence/(name+'-reference.snapshots.xz')
 with source.open('rb') as initial,lzma.open(dest,'wb',preset=1) as archive:shutil.copyfileobj(initial,archive)
 with lzma.open(dest,'rb') as archive:assert hashlib.sha256(archive.read()).hexdigest()==digest
 owned.append(str(dest.relative_to(root)));streams[name]={'states':count,'words':words,'sha256':digest,'strict':'PASS','full_asan_ubsan':'PASS'}
 for suffix in ('check.log','sanitize-check.log'):retain(build/(name+'-'+suffix))
for suffix in ('guards.log','guards-san.log'):retain(build/('lm-recurrence-bridge-'+suffix))
for name in ('lm-recurrence-bridge','lm-waitlist-dispatch'):
 for ext in ('prg','map','d81'):retain(build/(name+'.'+ext))
 for suffix in ('build.log','vice.log','vice-monitor.log','vice.mon','vice-mailbox.bin','vice-shared.bin','vice-stack.bin','vice-hardware-stack.bin','modules.json'):retain(build/(name+'-'+suffix))
 assert (build/(name+'-vice-mailbox.bin')).read_bytes()==bytes.fromhex('a50000400002')
 for n in range(32):
  for ext in ('prg','map'):retain(build/f'{name}-{n}.{ext}')
 retain(build/(name.replace('-','_')+'_modules.h'))
retain(build/'lm-waitlist-dispatch-c64-result.log')
sources=[p for p in owned if p.endswith('.c')and not p.endswith('_reference.c')];deps={'src/agc_word.c','src/waitlist.c','include/native_dispatch.h','deps/fixture/MAIN.agc.bin','deps/virtualagc/yaAGC/agc_engine.c','tools/reference_runner.c','platform/c64/overlay_resident.ld','platform/c64/overlay_payload.ld','platform/c64/overlay_platform.c','src/overlay_runtime.c'}
text=subprocess.check_output(['gcc','-MM','-Iinclude','-Itests','-Iplatform/c64','-Ibuild']+sources,cwd=root,text=True)
for line in text.replace('\\\n',' ').splitlines():deps.update(line.split(':',1)[1].split())
proof={'scope':'Literal canonical T3/vector+next-task, TASKOVER saved-frame RESUME, positive admitted idle/basic entry; bodies of generic selected addresses and SVCT3 excluded','host':streams,'invalid_domains_unchanged':38,'null_interfaces':10,'target_states':128,'target_words':[2052,2053],'target_mailboxes':['a50000400002','a50000400002'],'software_stack_bytes':[50,134],'hardware_stack_bytes':[33,29],'extra_zp':0,'test_fixture_ids':'isolated160..191; no production registry reservation','files':{p:sha(root/p)for p in owned},'dependencies':{p:sha(root/p)for p in sorted(deps)}}
p=evidence/'lm-recurrence-bridges-proof.json';p.write_text(json.dumps(proof,indent=2)+'\n');owned.append(str(p.relative_to(root)));(build/'lm-recurrence-bridges-commit-files.txt').write_text('\n'.join(owned)+'\n');print('Frozen',len(owned),'files,24576 original source states/128 C64 full states')
