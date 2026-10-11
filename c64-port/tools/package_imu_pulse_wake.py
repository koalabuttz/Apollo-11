#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Retain source-only IMUPULSE proof and actual C64 diagnostics."""
import pathlib,lzma,json,hashlib,re,shutil
r=pathlib.Path(__file__).resolve().parents[1];e=r/'evidence';stem='imu-pulse-wake';files=[]
stats=json.loads((r/'build/imu-pulse-wake-corpus-stats.json').read_text());mapping=json.loads((r/'build/imu-pulse-wake-source-map.json').read_text());targets={};streams={}
for name,x in stats.items():
 mission=x['mission'];stage=x['stage'];tree=('lm','cm')[mission];digest=x['decoded_sha256'];length=x['complete_states']*4112;archive=r/f'build/imu-pulse-wake-{tree}-{stage}.states.xz'
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
 assert mail==bytes([0xa5,0,0,32,mission*32+stage])and len(stack)==2
 targets[name]=dict(complete_states=32,mailbox=mail.hex(),PRG_bytes=len(prg),loaded_end=hex(load+len(prg)-3),hardware_stack=stack[0],software_stack=stack[1],compiler_ZP=0)
for suffix in ['host-validation.log','frozen-validation.log','source-map-validation.log','source-map.json','corpus-stats.json']:
 path=stem+'-'+suffix;shutil.copyfile(r/'build'/path,e/path);files.append('evidence/'+path)
review=e/'imu-pulse-wake-independent-review.txt';assert review.exists();files.append('evidence/'+review.name)
proof=dict(complete_states=sum(x['complete_states']for x in stats.values()),model_words=2048,external_words=['IRQ','SUPER','CHAN11','CHAN12','CHAN14'],source_group_words={m:x['source_group_words']for m,x in mapping.items()},containing_file_status='partial',source_map=f'evidence/{stem}-source-map.json',logical_complete_streams=streams,actual_C64=targets,source_branches={name:x['source_branches']for name,x in stats.items()},source_audit_dependency='evidence/imu-pulse-source-audit.json',source_fixture_tool='tools/imu_pulse_prepare_reference.py',independent_review=dict(path='evidence/'+review.name,sha256=hashlib.sha256(review.read_bytes()).hexdigest()),immutable_service_dependencies={p:hashlib.sha256((r/p).read_bytes()).hexdigest()for p in ['src/lm_basic_sleep.c','src/cm_basic_sleep.c','src/imu_pulse_service.c','src/lm_waitlist_image.c','src/cm_waitlist_image.c','src/executive.c','src/cm_executive.c','src/cm_dispatch.c','src/cm_native_image.c','src/waitlist.c','src/cm_waitlist.c']},qualification='Original GWAKE initialized LGYRO>=+0 resleep/free restoration body, plus actual basic Executive JOBSLEEP or WAITLIST registration through selected basic/idleADVAN/free reservation return boundaries. Valid source helper arenas/queues, ordinary coherent banks/SUPER, rawA/L, quiescent output counters and producer exclusion. No selected job body, idle loop, scheduler RESUME or physical pulse delivery; containing-file partial.',target_warning_exception='Frozen basic-wake helper enum status signedness warnings remain visible under -Wno-error=sign-conversion; all host warnings remain errors.',decoded_stream_identity='Each of four originals is retained once as deterministic XZ. Strict-native and full-sanitized streams compared directly against decoded original bytes with pipefail; no duplicate raw full streams retained.')
p=e/'imu-pulse-wake-proof.json';p.write_text(json.dumps(proof,indent=2)+'\n');files.append('evidence/'+p.name)
files+=['docs/NATIVE-GYRO-WAKE.md','include/imu_pulse_wake.h','src/imu_pulse_wake.c','tests/imu_pulse_wake_cases.h','tests/imu_pulse_wake_cases.c','tests/imu_pulse_wake_vectors.h','tests/test_imu_pulse_wake.c']
files+=['tools/'+n for n in ['imu_pulse_wake.mk','imu_pulse_wake_reference.c','imu_pulse_wake_snapshot.c','imu_pulse_wake_inputs.c','imu_pulse_wake_source.py','imu_pulse_wake_diagnostic.c','imu_pulse_wake_vice_test.py','freeze_imu_pulse_wake.py','package_imu_pulse_wake.py']]
files=sorted(set(files));manifest='evidence/imu-pulse-wake-evidence.sha256';(r/manifest).write_text(''.join(hashlib.sha256((r/p).read_bytes()).hexdigest()+'  '+p+'\n'for p in files));files.append(manifest)
(r/'build/imu-pulse-wake-commit-files.txt').write_text('\n'.join(sorted(files))+'\n');print(len(files),'ready files;',proof['complete_states'],'complete states')
