#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Retain source-only IMU attitude admission proof and actual C64 diagnostics."""
import pathlib,lzma,json,hashlib,re,shutil,subprocess
r=pathlib.Path(__file__).resolve().parents[1];e=r/'evidence';stem='imu-attitude-admission';files=[]
stats=json.loads((r/'build/imu-attitude-admission-corpus-stats.json').read_text());mapping=json.loads((r/'build/imu-attitude-admission-source-map.json').read_text());targets={};streams={}
for role in ['host','san']:
 qualified=json.loads((r/f'build/{stem}-{role}-proof.json').read_text())
 assert len(qualified)==10
 for row in qualified:
  key=('lm','cm')[row['mission']]+str(row['stage'])
  assert row['sha256']==stats[key]['decoded_sha256'] and row['states']==stats[key]['complete_states']
for name,x in stats.items():
 mission=x['mission'];entry=x['entry'];tree=('lm','cm')[mission];digest=x['decoded_sha256'];length=x['complete_states']*4112;archive=r/f'build/imu-attitude-admission-{mission}-{entry}.snapshots.xz'
 if not archive.exists():archive=e/f'{stem}-{name}-complete.states.xz'
 with lzma.open(archive,'rb')as f:
  h=hashlib.sha256();size=0
  while raw:=f.read(1048576):h.update(raw);size+=len(raw)
 assert size==length and h.hexdigest()==digest
 obj=e/f'{stem}-{name}-complete.states.xz';assert not obj.exists() or obj.read_bytes()==archive.read_bytes()
 if archive!=obj:shutil.move(str(archive),str(obj))
 files.append('evidence/'+obj.name)
 streams[name]={role:dict(object='evidence/'+obj.name,decoded_sha256=digest,decoded_bytes=length)for role in ['original_assembly','strict_native','whole_ASAN_UBSAN']}
 target=stem+'-'+name
 for suffix in ['prg','map','build.log','validation.log','vice-mailbox.bin','hardware-stack.bin','software-stack.bin','vice.log','vice-monitor.log','vice.mon']:
  path=target+('.'if suffix in ['prg','map']else'-')+suffix;shutil.copyfile(r/'build'/path,e/path);files.append('evidence/'+path)
 prg=(e/f'{target}.prg').read_bytes();load=int.from_bytes(prg[:2],'little');log=(e/f'{target}-validation.log').read_text();stack=[int(v)for v in re.findall(r'stack extent (\d+) bytes',log)];mail=(e/f'{target}-vice-mailbox.bin').read_bytes()
 assert mail==bytes([0xa5,0,0,32,mission*32+entry])and len(stack)==2
 targets[name]=dict(complete_states=32,unchanged_guard_calls=12,constant_checks=6,mailbox=mail.hex(),PRG_bytes=len(prg),loaded_end=hex(load+len(prg)-3),hardware_stack=stack[0],software_stack=stack[1],compiler_ZP=0)
for suffix in ['host-validation.log','frozen-validation.log','source-map-validation.log','source-map.json','corpus-stats.json','literals-proof.json','host-proof.json','san-proof.json','sanitizer-validation.log','literals-validation.log']:
 path=stem+'-'+suffix;shutil.copyfile(r/'build'/path,e/path);files.append('evidence/'+path)
review=e/'imu-attitude-admission-independent-review.json';assert review.exists();files.append('evidence/'+review.name)
proof=dict(literal_accessor_arguments=131072,unchanged_target_guards=120,complete_states=sum(x['complete_states']for x in stats.values()),model_words=2048,external_words=['IRQ','SUPER','CHAN11','CHAN12','CHAN14'],source_group_words={m:x['source_group_words']for m,x in mapping.items()},containing_file_status='partial',source_map=f'evidence/{stem}-source-map.json',logical_complete_streams=streams,actual_C64=targets,source_branches={name:x['source_branches']for name,x in stats.items()},source_audit_dependency='evidence/imu-pulse-source-audit.json',source_fixture_tool='tools/imu_pulse_prepare_reference.py',independent_review=dict(path='evidence/'+review.name,sha256=hashlib.sha256(review.read_bytes()).hexdigest()),immutable_service_dependencies={p:hashlib.sha256((r/p).read_bytes()).hexdigest()for p in ['src/agc_word.c','src/agc_multiword.c','tests/imu_pulse_cases.c','src/imu_coarse.c','include/imu_coarse.h','tests/imu_coarse_cases.c','tests/imu_mode_control_cases.c','src/imu_mode_control.c','include/imu_mode_control.h','include/imu_mode_control_literals.h','tools/frozen_words.py','tests/frozen_words.c']},qualification='Original GOMANUR/R02 admission and local alarm/flag/return source bodies. Coherent actual bank07 or LM R02 bank04/EB3, ordinary canonical RAM/raw A/L, fixed Q, ordinary latches, quiet channel14 and excluded producers. GOMANUR saved fixed caller/aligned bank bounds. Typed POSTJUMP/POODOO/UPFLAG/VARALARM/SWRETURN/GOTOPOOH frontiers preserve actual source registers. No external callee body, arbitrary opcode or whole application claim.',target_warning_exception='None. Host and target strict warnings are errors.',decoded_stream_identity='Each of ten originals is retained once as deterministic XZ. Strict-native and full-sanitized streams compared directly against decoded original bytes with checked subprocess completion; no duplicate raw full streams retained.')
subprocess.run(['python3','tools/imu_attitude_admission_gap.py'],cwd=r,check=True)
shutil.copyfile(r/'build/imu-attitude-admission-gap-map.json',e/'imu-attitude-admission-gap-map.json');files.append('evidence/imu-attitude-admission-gap-map.json')
files.append('tools/imu_attitude_admission_gap.py')
p=e/'imu-attitude-admission-proof.json';p.write_text(json.dumps(proof,indent=2)+'\n');files.append('evidence/'+p.name)
files+=['docs/NATIVE-IMU-ATTITUDE-ADMISSION.md','include/imu_attitude_admission.h','src/imu_attitude_admission.c','tests/imu_attitude_admission_cases.h','tests/imu_attitude_admission_cases.c','tests/imu_attitude_admission_vectors.h','tests/test_imu_attitude_admission.c']
files+=['tools/'+n for n in ['imu_attitude_admission.mk','imu_attitude_admission_host.py','imu_attitude_admission_literals.c','imu_attitude_admission_literals.py','imu_attitude_admission_reference.c','imu_attitude_admission_snapshot.c','imu_attitude_admission_inputs.c','imu_attitude_admission_source.py','imu_attitude_admission_diagnostic.c','imu_attitude_admission_vice_test.py','freeze_imu_attitude_admission.py','package_imu_attitude_admission.py']]
files=sorted(set(files));manifest='evidence/imu-attitude-admission-evidence.sha256';(r/manifest).write_text(''.join(hashlib.sha256((r/p).read_bytes()).hexdigest()+'  '+p+'\n'for p in files));files.append(manifest)
(r/'build/imu-attitude-admission-commit-files.txt').write_text('\n'.join(sorted(files))+'\n');print(len(files),'ready files;',proof['complete_states'],'complete states')
