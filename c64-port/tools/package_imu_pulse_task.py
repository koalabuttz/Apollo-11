#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Retain source-only IMUPULSE proof and actual C64 diagnostics."""
import pathlib,lzma,json,hashlib,re,shutil
r=pathlib.Path(__file__).resolve().parents[1];e=r/'evidence';stem='imu-pulse-task';files=[]
stats=json.loads((r/'build/imu-pulse-task-corpus-stats.json').read_text());mapping=json.loads((r/'build/imu-pulse-task-source-map.json').read_text());targets={};streams={}
for name,x in stats.items():
 mission=x['mission'];stage=x['stage'];augment=x['augment'];tree=('lm','cm')[mission];digest=x['decoded_sha256'];length=x['complete_states']*4112;archive=r/f'build/imu-pulse-task-{tree}-{augment}-{stage}.states.xz'
 with lzma.open(archive,'rb')as f:
  h=hashlib.sha256();size=0
  while raw:=f.read(1048576):h.update(raw);size+=len(raw)
 assert size==length and h.hexdigest()==digest
 obj=e/f'{stem}-{name}-complete.states.xz';shutil.copyfile(archive,obj);files.append('evidence/'+obj.name)
 streams[name]={role:dict(object='evidence/'+obj.name,decoded_sha256=digest,decoded_bytes=length)for role in ['original_assembly','strict_native','whole_ASAN_UBSAN']}
 target=stem+'-'+name
 for suffix in ['prg','map','build.log','validation.log','vice-mailbox.bin','hardware-stack.bin','software-stack.bin','vice.log','vice-monitor.log','vice.mon']:
  path=target+('.'if suffix in ['prg','map']else'-')+suffix;shutil.copyfile(r/'build'/path,e/path);files.append('evidence/'+path)
 prg=(e/f'{target}.prg').read_bytes();load=int.from_bytes(prg[:2],'little');log=(e/f'{target}-validation.log').read_text();stack=[int(v)for v in re.findall(r'stack extent (\d+) bytes',log)];mail=(e/f'{target}-vice-mailbox.bin').read_bytes()
 assert mail==bytes([0xa5,0,0,32,mission*32+augment*4+stage])and len(stack)==2
 targets[name]=dict(complete_states=32,mailbox=mail.hex(),PRG_bytes=len(prg),loaded_end=hex(load+len(prg)-3),hardware_stack=stack[0],software_stack=stack[1],compiler_ZP=0)
for suffix in ['full-validation.log','host-validation.log','frozen-validation.log','source-map-validation.log','source-map.json','corpus-stats.json']:
 path=stem+'-'+suffix;shutil.copyfile(r/'build'/path,e/path);files.append('evidence/'+path)
review=e/'imu-pulse-task-independent-review.txt';assert review.exists();files.append('evidence/'+review.name)
proof=dict(complete_states=sum(x['complete_states']for x in stats.values()),model_words=2048,external_words=['IRQ','SUPER','CHAN11','CHAN12','CHAN14'],source_group_words={m:x['source_group_words']for m,x in mapping.items()},containing_file_status='partial',source_map=f'evidence/{stem}-source-map.json',logical_complete_streams=streams,actual_C64=targets,source_branches={name:x['source_branches']for name,x in stats.items()},source_audit_dependency='evidence/imu-pulse-source-audit.json',source_fixture_tool='tools/imu_pulse_prepare_reference.py',independent_review=dict(path='evidence/'+review.name,sha256=hashlib.sha256(review.read_bytes()).hexdigest()),immutable_service_dependencies={p:hashlib.sha256((r/p).read_bytes()).hexdigest()for p in ['src/lm_waitlist_image.c','src/cm_waitlist_image.c','src/waitlist.c','src/cm_waitlist.c','src/cm_native_image.c']},qualification='Both original deferred STRTGYRO/8192AUG through literal WAITLIST/JOBWAKE/IMUBAD, plus successful next-task WAITLIST registration and channel14 activation before literal TCF TASKOVER. Ordinary canonical banks/phase0..3/E4..E7 command pairs, idle output counters. Augment input activity01000 clear; no active-channel or asynchronous pulse-consumption claim. Empty initialized queues with variedTIME3 corpus. No callee wake/alarm/scheduler continuation or whole IMU/RTB/file/application claim.',target_warning_exception='Frozen WAITLIST helper enum status signedness warnings remain visible under -Wno-error=sign-conversion; all host warnings remain errors.',decoded_stream_identity='Each of eight originals is retained once as deterministic XZ. Strict-native and full-sanitized streams compared directly against decoded original bytes with pipefail; no duplicate raw full streams retained.')
p=e/'imu-pulse-task-proof.json';p.write_text(json.dumps(proof,indent=2)+'\n');files.append('evidence/'+p.name)
files+=['docs/NATIVE-GYRO-TASKS.md','include/imu_pulse_task.h','src/imu_pulse_task.c','src/imu_pulse_task_register.c','tests/imu_pulse_task_cases.h','tests/imu_pulse_task_cases.c','tests/imu_pulse_task_vectors.h','tests/test_imu_pulse_task.c']
files+=['tools/'+n for n in ['imu_pulse_task.mk','imu_pulse_task_reference.c','imu_pulse_task_snapshot.c','imu_pulse_task_inputs.c','imu_pulse_task_source.py','imu_pulse_task_diagnostic.c','imu_pulse_task_vice_test.py','freeze_imu_pulse_task.py','package_imu_pulse_task.py']]
files=sorted(set(files));manifest='evidence/imu-pulse-task-evidence.sha256';(r/manifest).write_text(''.join(hashlib.sha256((r/p).read_bytes()).hexdigest()+'  '+p+'\n'for p in files));files.append(manifest)
(r/'build/imu-pulse-task-commit-files.txt').write_text('\n'.join(sorted(files))+'\n');print(len(files),'ready files;',proof['complete_states'],'complete states')
