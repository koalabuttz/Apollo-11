#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Retain actual source-verified CM clock-display native disk composition."""
import pathlib,hashlib,json,gzip
R=pathlib.Path(__file__).resolve().parents[1];B=R/'build';E=R/'evidence';prefix='cm-clock-display-overlay';source='cm-clock-display-mission'
proof=json.loads((B/(source+'-proof.json')).read_text());assert proof['snapshot_sha256']=='295f2ea42a6563b411a00f9c2254c2b8b5d9c284b7309e1edc10340200f2d4c2'
reference=(B/(source+'-reference.snapshots')).read_bytes();assert reference==(B/(source+'-native.snapshots')).read_bytes();assert hashlib.sha256(reference).hexdigest()==proof['snapshot_sha256']
assert (B/(prefix+'-vice-mailbox.bin')).read_bytes()==bytes.fromhex('a500007a03')
assert 'PASS:' in (B/(prefix+'-c64-result.log')).read_text()
rows=[list(map(int,l.split())) for l in reference.decode().splitlines()];assert len(rows)==1344 and all(len(r)==2060 for r in rows)
assert not any(r[2]==103 for r in rows)
busyproof=json.loads((B/'cm-clock-display-busy-proof.json').read_text());assert busyproof['snapshot_sha256']=='ce2a6fe603dea893e6e1ba202e267b581e37e0e7949444c7da22fe606b79f5fe'
busy=(B/'cm-clock-display-busy-reference.snapshots').read_bytes();assert busy==(B/'cm-clock-display-busy-native.snapshots').read_bytes();assert hashlib.sha256(busy).hexdigest()==busyproof['snapshot_sha256']
brows=[list(map(int,l.split()))for l in busy.decode().splitlines()];assert len(brows)==1216 and sum(r[2]==103 for r in brows)==64
assert (B/(prefix+'-vice-shared.bin')).read_bytes()==b''.join(v.to_bytes(2,'little') for v in [r for r in brows if r[0]==0][-1][4:2052])
modules=json.loads((B/(prefix+'-modules.json')).read_text());assert len(modules)==9
for m in modules:
 data=(B/m['prg']).read_bytes();assert len(data)-2==m['byte_count']<=16384 and hashlib.sha256(data).hexdigest()==m['sha256']
stack=(B/(prefix+'-vice-stack.bin')).read_bytes();changed=[i for i,v in enumerate(stack) if v!=0xcd];extent=2048-min(changed) if changed else 0;assert extent<2048
owned=[]
def retain(p,name=None):
 q=E/(name or p.name);data=p.read_bytes()
 if q.exists():assert q.read_bytes()==data,'Published artifact changed: '+str(q)
 else:q.write_bytes(data)
 owned.append(q.relative_to(R).as_posix())
for suffix in ['prg','map','d64']:retain(B/(prefix+'.'+suffix))
for suffix in ['modules.json','build-validation.log','c64-result.log','vice-mailbox.bin','vice-shared.bin','vice-stack.bin','vice-monitor.log','vice.log']:retain(B/(prefix+'-'+suffix))
for m in modules:
 for ext in ['prg','map']:retain(B/pathlib.Path(m['prg']).with_suffix('.'+ext))
retain(B/'cm_clock_display_overlay_modules.h');retain(B/'cm-clock-display-sdk/overlay-sdk-runtime/provenance.txt',prefix+'-sdk-provenance.txt')
goplay=[r for r in rows if r[0]==0][-1];assert goplay[9]==0o3312
assert (B/'cm-clock-display-goplay-case0.words.bin').read_bytes()==b''.join(v.to_bytes(2,'little') for v in goplay[4:2052])
retain(B/'cm-clock-display-goplay-case0.words.bin');retain(B/'cm-clock-display-goplay-case0.io.json')
for suffix in ['proof.json','validation.log','sanitize-validation.log','freeze.log','native.trace','reference.trace']:retain(B/(source+'-'+suffix))
for suffix in ['proof.json','validation.log','sanitize-validation.log','native.trace','reference.trace']:retain(B/('cm-clock-display-busy-'+suffix))
retain(E/'cm-clock-display-overlay-timeout8.json')
for trial in sorted((E/'cm-clock-display-overlay-timeout8').glob('*')):owned.append(trial.relative_to(R).as_posix())
for label,role in [(label,role)for label in [source,'cm-clock-display-busy']for role in ['native','reference']]:
 p=B/(label+'-'+role+'.snapshots');q=E/(p.name+'.gz');data=p.read_bytes()
 if q.exists():assert gzip.decompress(q.read_bytes())==data
 else:
  with q.open('wb') as f:
   with gzip.GzipFile(fileobj=f,filename='',mode='wb',mtime=0) as z:z.write(data)
 owned.append(q.relative_to(R).as_posix())
report=dict(source_cases=128,source_checkpoints=2560,source_snapshot_sha256=proof['snapshot_sha256'],busy_source_snapshot_sha256=busyproof['snapshot_sha256'],canonical_words=2048,c64_continuous_cases=6,c64_checkpoints=122,c64_complete_terminal_samples=6,distinct_terminal_sample_contexts=3,mailbox_hex='a500007a03',software_stack_bytes=extent,resident_prg_bytes=(B/(prefix+'.prg')).stat().st_size,modules=modules,scope='actual T3/CLOKTASK-admitted CLOCKJOB/trig/MAKEPLAY/priority/wake to GOPLAY, actual sleep/restored interpreter, or matchingTEMPFLSH051366/DSPLOCK1..7 busyEND/restored interpreter; noun40 displayconversion remains separate',complete_application_qualified=False)
q=E/(prefix+'-report.json');data=(json.dumps(report,indent=2)+'\n').encode()
if q.exists():assert q.read_bytes()==data
else:q.write_bytes(data)
owned.append(q.relative_to(R).as_posix())
files=owned+[p.relative_to(R).as_posix() for pattern in ['include/cm_clock_display_mission.h','src/cm_clock_display_mission.c','src/cm_clock_display_builtin.c','platform/c64/cm_clock_display_overlay*','platform/host/cm_clock_display_mission_snapshot.c','tests/cm_clock_display_mission*','tests/cm_clock_display_overlay_vectors.h','tools/cm_clock_display_mission*','tools/cm_clock_display_overlay*','tools/cm_freeze_clock_display.py','docs/COMANCHE_CLOCK_DISPLAY_MISSION.md','platform/host/cm_clock_display_busy_snapshot.c','tests/cm_clock_display_busy_cases*','tools/cm_clock_display_busy*']for p in R.glob(pattern)if p.is_file()]
files += ['include/cm_clock_mission.h','src/cm_clock_mission.c','src/cm_clock_mission_builtin.c','include/cm_mission_loop.h','src/cm_mission_loop.c','src/cm_mission_builtin.c','include/cm_service_loop.h','src/cm_service_loop.c','src/cm_service_builtin.c','src/cm_makeplay.c','include/cm_makeplay.h','src/cm_makeplay_sleep.c','include/cm_makeplay_sleep.h','src/cm_priority_complete.c','include/cm_priority_complete.h','src/cm_basic_wake.c','include/cm_basic_wake.h','src/cm_basic_sleep.c','include/cm_basic_sleep.h','src/cm_native_image.c','include/cm_native_image.h','src/cm_dispatch.c','include/cm_dispatch.h','src/cm_executive.c','include/cm_executive.h','src/agc_word.c','include/agc_word.h','src/agc_multiword.c','include/agc_multiword.h','tests/cm_mission_loop_cases.c','tests/cm_mission_loop_cases.h','tests/cm_clock_mission_cases.c','tests/cm_clock_mission_cases.h','tests/cm_service_loop_cases.c','tests/cm_service_loop_cases.h','tools/cm_executive_reference.c','tests/cm_executive_cases.c','tests/cm_executive_cases.h','tools/prepare_cm_original_rope.py','evidence/fixed-data-cm-original.rope.bin','evidence/fixed-data-audit.json.gz','evidence/cm-clock-mission-overlay-evidence.sha256','evidence/cm-priority-complete-evidence.sha256','evidence/cm-makeplay-sleep-SHA256.json','include/overlay_runtime.h','src/overlay_runtime.c','platform/c64/overlay_platform.c','platform/c64/overlay_platform.h','platform/c64/overlay_resident.ld','platform/c64/overlay_payload.ld','platform/c64/overlay_entry.S','tools/overlay_runtime_library.py','tools/overlay_build.py']
files=[f for f in files if (R/f).is_file()]
for stem in ['cm_task_transfer','cm_entry_tasks','cm_clocktask','cm_tig5','cm_svct3','cm_waitlist_image','cm_admission_image','cm_delay_image','cm_longcall','cm_restart_select','cm_restart_timing','cm_init_display','cm_basic_priority','cm_waitlist','cm_phase','cm_task_interrupt','cm_danzig','cm_basic_finish','cm_clockjob','cm_quictrig','cm_display_control','cm_idle_dispatch']:
 for folder,suffix in [('src','.c'),('include','.h')]:
  dependency=folder+'/'+stem+suffix;assert (R/dependency).is_file();files.append(dependency)
# Assert native control identity is unchanged from the retained oversized target trial.
old=json.loads((E/'cm-clock-display-overlay-timeout8/cm-clock-display-overlay-modules.json').read_text())
assert all(m==old[i] for i,m in enumerate(modules[:7]))
ledger=E/(prefix+'-evidence.sha256');ledger.write_text(''.join(hashlib.sha256((R/f).read_bytes()).hexdigest()+'  '+f+'\n'for f in sorted(set(files))));files.append(ledger.relative_to(R).as_posix());(B/(prefix+'-commit-files.txt')).write_text('\n'.join(sorted(set(files)))+'\n')
print(json.dumps(report,indent=2));print('Commit list:',B/(prefix+'-commit-files.txt'))
