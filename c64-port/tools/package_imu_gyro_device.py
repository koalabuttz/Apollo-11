#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Retain source-only IMUPULSE proof and actual C64 diagnostics."""
import pathlib,lzma,json,hashlib,re,shutil
r=pathlib.Path(__file__).resolve().parents[1];e=r/'evidence';stem='imu-gyro-device';files=[]
stats=json.loads((r/'build/imu-gyro-device-corpus-stats.json').read_text());audit=json.loads((r/'build/imu-gyro-device-source-audit.json').read_text());targets={};streams={}
for name,x in stats.items():
 mission=x['mission'];mode=x['mode'];tree=('lm','cm')[mission];digest=x['decoded_sha256'];length=x['complete_states']*4156;archive=r/f'build/imu-gyro-device-{tree}-{mode}.states.xz'
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
 assert mail==bytes([0xa5,0,0,32,mission*32+mode])and len(stack)==2
 targets[name]=dict(complete_states=32,mailbox=mail.hex(),PRG_bytes=len(prg),loaded_end=hex(load+len(prg)-3),hardware_stack=stack[0],software_stack=stack[1],compiler_ZP=0)
for suffix in ['host-validation.log','frozen-validation.log','guard-validation.log','irq-validation.log','source-audit.json','corpus-stats.json']:
 path=stem+'-'+suffix;shutil.copyfile(r/'build'/path,e/path);files.append('evidence/'+path)
review=e/'imu-gyro-device-independent-review.txt';assert review.exists();files.append('evidence/'+review.name)
proof=dict(complete_states=sum(x['complete_states']for x in stats.values()),model_words=2048,external_words=['IRQ','SUPER','CHAN11','CHAN12','CHAN14','phase32','pending','selection','event_count','events18'],source_groups_credited=0,peripheral_source_audit=f'evidence/{stem}-source-audit.json',logical_complete_streams=streams,actual_C64=targets,event_counts={name:x['event_counts']for name,x in stats.items()},rejection_invariants_per_context=6,source_audit_dependency='evidence/imu-pulse-source-audit.json',source_fixture_tool='tools/imu_pulse_prepare_reference.py',independent_review=dict(path='evidence/'+review.name,sha256=hashlib.sha256(review.read_bytes()).hexdigest()),host_only_reference_dependency={audit['peripheral_source']:audit['source_sha256']},qualification='Native simulated gyro peripheral transport; one eligible reference-peripheral step, explicit pending/selection/phase and eventbuffer. Timed and defaultburst backend modes match unchanged reference with originalCAFZERO pending CPU observation and noCPU canonicalwrites. Ordinary positivecounter0..037777, legalmode state, callerproducer exclusion/clock ownership. No decoder/privatepersistentstate, source instruction/wallclock timeline or physical output claim; no containingfile sourcecredit.',decoded_stream_identity='Each of four originals is retained once as deterministic XZ. Strict-native and full-sanitized streams compared directly against decoded original bytes with pipefail; no duplicate raw full streams retained.')
p=e/'imu-gyro-device-proof.json';p.write_text(json.dumps(proof,indent=2)+'\n');files.append('evidence/'+p.name)
files+=['docs/NATIVE-GYRO-DEVICE.md','include/imu_gyro_device.h','src/imu_gyro_device.c','tests/imu_gyro_device_cases.h','tests/imu_gyro_device_cases.c','tests/imu_gyro_device_vectors.h','tests/test_imu_gyro_device.c']
files+=['tools/'+n for n in ['imu_gyro_device.mk','imu_gyro_device_reference.c','imu_gyro_device_snapshot.c','imu_gyro_device_inputs.c','imu_gyro_device_diagnostic.c','imu_gyro_device_vice_test.py','freeze_imu_gyro_device.py','package_imu_gyro_device.py']]
files=sorted(set(files));manifest='evidence/imu-gyro-device-evidence.sha256';(r/manifest).write_text(''.join(hashlib.sha256((r/p).read_bytes()).hexdigest()+'  '+p+'\n'for p in files));files.append(manifest)
(r/'build/imu-gyro-device-commit-files.txt').write_text('\n'.join(sorted(files))+'\n');print(len(files),'ready files;',proof['complete_states'],'complete states')
