#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Freeze the real C64 resident CM task/service composition artifacts."""
import pathlib,hashlib,json,re,gzip
R=pathlib.Path(__file__).resolve().parents[1];B=R/'build';E=R/'evidence';name='cm-mission-overlay';E.mkdir(exist_ok=True)
assert (B/(name+'-vice-mailbox.bin')).read_bytes()==bytes([0xa5,0,0,82,6])
assert 'PASS:' in (B/(name+'-c64-result.log')).read_text()
proof=json.loads((B/'cm-mission-loop-proof.json').read_text());reference=(B/'cm-mission-loop-reference.snapshots').read_bytes();assert reference==(B/'cm-mission-loop-native.snapshots').read_bytes();assert hashlib.sha256(reference).hexdigest()==proof['snapshot_sha256']
modules=json.loads((B/(name+'-modules.json')).read_text());assert len(modules)==7
for m in modules:
 raw=(B/m['prg']).read_bytes();assert len(raw)-2==m['byte_count'] and hashlib.sha256(raw).hexdigest()==m['sha256'] and m['byte_count']<=16384
rows=[list(map(int,r.split())) for r in reference.decode().splitlines() if r.startswith('320 ')];raw=(B/(name+'-vice-shared.bin')).read_bytes();assert len(raw)==4096;assert [int.from_bytes(raw[i:i+2],'little') for i in range(0,4096,2)]==rows[-1][4:2052]
stack=(B/(name+'-vice-stack.bin')).read_bytes();modified=[i for i,v in enumerate(stack) if v!=0xcd];extent=2048-min(modified) if modified else 0;assert extent<2048
report={'source_cases':proof['cases'],'source_checkpoints':proof['checkpoints'],'source_snapshot_sha256':proof['snapshot_sha256'],'canonical_words':2048,'c64_continuous_cases':12,'c64_checkpoints':82,'c64_full_samples':3,'mailbox_hex':'a500005206','software_stack_bytes':extent,'resident_prg_bytes':(B/(name+'.prg')).stat().st_size,'modules':modules,'scope':'actual automatic IRQ/task services/hardware RESUME/DANZIG to genuine selected foreground continuation; counter drivers zero/scaler frozen; no whole-flight completion claim'}
owned=[]
def retain(p,n):
 dst=E/n;raw=p.read_bytes()
 if dst.exists():assert dst.read_bytes()==raw,'Published artifact identity changed:'+str(dst)
 else:dst.write_bytes(raw)
 owned.append(dst.relative_to(R).as_posix())
for p in [B/(name+'.prg'),B/(name+'.map'),B/(name+'.d64'),B/(name+'-modules.json'),B/'cm_mission_overlay_modules.h',B/'cm-mission-sdk/overlay-sdk-runtime/provenance.txt']+[B/(name+'-'+s) for s in ['build-validation.log','c64-result.log','vice-mailbox.bin','vice-shared.bin','vice-stack.bin','vice-monitor.log','vice.log']]:retain(p,p.name if p.name!='provenance.txt' else name+'-sdk-provenance.txt')
for m in modules:
 for ext in ['prg','map']:p=B/pathlib.Path(m['prg']).with_suffix('.'+ext);retain(p,p.name)
p=E/(name+'-report.json');data=(json.dumps(report,indent=2)+'\n').encode()
if p.exists():assert p.read_bytes()==data
else:p.write_bytes(data)
owned.append(p.relative_to(R).as_posix())
for role in ['reference','native']:
 target=E/('cm-mission-loop-'+role+'.snapshots.gz');payload=(B/('cm-mission-loop-'+role+'.snapshots')).read_bytes()
 if target.exists():assert gzip.decompress(target.read_bytes())==payload
 else:
  with target.open('wb')as f:
   with gzip.GzipFile(fileobj=f,filename='',mode='wb',mtime=0)as z:z.write(payload)
 owned.append(target.relative_to(R).as_posix())
for suffix in ['proof.json','source-validation.log','build-validation.log','reference.trace','native.trace']:retain(B/('cm-mission-loop-'+suffix),'cm-mission-loop-'+suffix)
retain(B/'cm-mission-source-audit.json','cm-mission-source-audit.json')
for source in json.loads((B/'cm-mission-source-audit.json').read_text())['sources']:retain(R/'deps/cm-mission-fixture'/(source+'.indentation.patch'),'cm-mission-'+source+'.indentation.patch')
files=owned+[p.relative_to(R).as_posix() for pattern in ['platform/c64/cm_mission_overlay*','tools/cm_mission_overlay*','docs/COMANCHE_MISSION_LOOP.md','include/cm_mission_loop.h','src/cm_mission_loop.c','src/cm_mission_builtin.c','tests/cm_mission_loop*','tests/test_cm_mission_loop.c','tests/cm_mission_overlay_vectors.h','platform/host/cm_mission_loop_snapshot.c','tools/cm_mission_loop*','tools/cm_freeze_mission_loop.py','tools/cm_mission_prepare_reference.py'] for p in R.glob(pattern) if p.is_file()]
files+=['src/cm_service_loop.c','include/cm_service_loop.h','include/overlay_runtime.h','src/overlay_runtime.c','platform/c64/overlay_platform.c','platform/c64/overlay_platform.h','platform/c64/overlay_resident.ld','platform/c64/overlay_payload.ld','platform/c64/overlay_entry.S','tools/overlay_runtime_library.py','tools/overlay_build.py','evidence/cm-service-loop-evidence.sha256','evidence/cm-danzig-evidence.sha256','evidence/cm-basic-finish-evidence.sha256','evidence/cm-task-interrupt-evidence.sha256']
manifest=E/(name+'-evidence.sha256');manifest.write_text(''.join(hashlib.sha256((R/f).read_bytes()).hexdigest()+'  '+f+'\n' for f in sorted(set(files))));files.append(manifest.relative_to(R).as_posix());(B/(name+'-commit-files.txt')).write_text('\n'.join(sorted(set(files)))+'\n');print(json.dumps(report,indent=2));print('Commit list:',B/(name+'-commit-files.txt'))
