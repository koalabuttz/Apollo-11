#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Freeze the real C64 resident CM task/service composition artifacts."""
import pathlib,hashlib,json,re
R=pathlib.Path(__file__).resolve().parents[1];B=R/'build';E=R/'evidence';name='cm-service-overlay';E.mkdir(exist_ok=True)
assert (B/(name+'-vice-mailbox.bin')).read_bytes()==bytes([0xa5,0,0,58,6])
assert 'PASS:' in (B/(name+'-c64-result.log')).read_text()
proof=json.loads((B/'cm-service-loop-proof.json').read_text());reference=(B/'cm-service-loop-reference.snapshots').read_bytes();assert reference==(B/'cm-service-loop-native.snapshots').read_bytes();assert hashlib.sha256(reference).hexdigest()==proof['snapshot_sha256']
modules=json.loads((B/(name+'-modules.json')).read_text());assert len(modules)==6
for m in modules:
 raw=(B/m['prg']).read_bytes();assert len(raw)-2==m['byte_count'] and hashlib.sha256(raw).hexdigest()==m['sha256'] and m['byte_count']<=16384
rows=[list(map(int,r.split())) for r in reference.decode().splitlines() if r.startswith('320 ')];raw=(B/(name+'-vice-shared.bin')).read_bytes();assert len(raw)==4096;assert [int.from_bytes(raw[i:i+2],'little') for i in range(0,4096,2)]==rows[-1][4:2052]
stack=(B/(name+'-vice-stack.bin')).read_bytes();modified=[i for i,v in enumerate(stack) if v!=0xcd];extent=2048-min(modified) if modified else 0;assert extent<2048
report={'source_cases':proof['cases'],'source_checkpoints':proof['checkpoints'],'source_snapshot_sha256':proof['snapshot_sha256'],'canonical_words':2048,'c64_continuous_cases':12,'c64_checkpoints':58,'c64_full_samples':3,'mailbox_hex':'a500003a06','software_stack_bytes':extent,'resident_prg_bytes':(B/(name+'.prg')).stat().st_size,'modules':modules,'scope':'actual source tasks/services to05341 before hardware RESUME; counter drivers zero/scaler frozen; no whole-flight completion claim'}
owned=[]
def retain(p,n):
 dst=E/n;raw=p.read_bytes()
 if dst.exists():assert dst.read_bytes()==raw,'Published artifact identity changed:'+str(dst)
 else:dst.write_bytes(raw)
 owned.append(dst.relative_to(R).as_posix())
for p in [B/(name+'.prg'),B/(name+'.map'),B/(name+'.d64'),B/(name+'-modules.json'),B/'cm_service_overlay_modules.h',B/'cm-service-sdk/overlay-sdk-runtime/provenance.txt']+[B/(name+'-'+s) for s in ['build-validation.log','c64-result.log','vice-mailbox.bin','vice-shared.bin','vice-stack.bin','vice-monitor.log','vice.log']]:retain(p,p.name if p.name!='provenance.txt' else name+'-sdk-provenance.txt')
for m in modules:
 for ext in ['prg','map']:p=B/pathlib.Path(m['prg']).with_suffix('.'+ext);retain(p,p.name)
p=E/(name+'-report.json');data=(json.dumps(report,indent=2)+'\n').encode()
if p.exists():assert p.read_bytes()==data
else:p.write_bytes(data)
owned.append(p.relative_to(R).as_posix())
files=owned+[p.relative_to(R).as_posix() for pattern in ['platform/c64/cm_service_overlay*','tools/cm_service_overlay*','docs/COMANCHE_SERVICE_OVERLAY.md'] for p in R.glob(pattern) if p.is_file()]
files+=['src/cm_service_loop.c','include/cm_service_loop.h','include/overlay_runtime.h','src/overlay_runtime.c','platform/c64/overlay_platform.c','platform/c64/overlay_platform.h','platform/c64/overlay_resident.ld','platform/c64/overlay_payload.ld','platform/c64/overlay_entry.S','tools/overlay_runtime_library.py','tools/overlay_build.py','evidence/cm-service-loop-evidence.sha256']
manifest=E/(name+'-evidence.sha256');manifest.write_text(''.join(hashlib.sha256((R/f).read_bytes()).hexdigest()+'  '+f+'\n' for f in sorted(set(files))));files.append(manifest.relative_to(R).as_posix());(B/(name+'-commit-files.txt')).write_text('\n'.join(sorted(set(files)))+'\n');print(json.dumps(report,indent=2));print('Commit list:',B/(name+'-commit-files.txt'))
