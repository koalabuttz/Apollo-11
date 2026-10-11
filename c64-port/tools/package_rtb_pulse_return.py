#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,lzma,json,hashlib,re,shutil
r=pathlib.Path(__file__).resolve().parents[1];e=r/'evidence';stem='rtb-pulse-return';files=[]
stats=json.loads((r/'build/rtb-pulse-return-corpus-stats.json').read_text());mapping=json.loads((r/'build/rtb-pulse-return-source-map.json').read_text());targets={};streams={}
for name,x in stats.items():
 mission=x['mission'];digest=x['decoded_sha256'];length=x['complete_states']*4106;archive=r/f'build/rtb-pulse-return-{name}.states.xz'
 with lzma.open(archive,'rb')as f:
  h=hashlib.sha256();size=0
  while raw:=f.read(1048576):h.update(raw);size+=len(raw)
 assert size==length and h.hexdigest()==digest
 obj=e/f'{stem}-{name}-complete.states.xz';shutil.copyfile(archive,obj);files.append('evidence/'+obj.name)
 streams[name]={role:dict(object='evidence/'+obj.name,decoded_sha256=digest,decoded_bytes=length)for role in ['original_assembly','strict_native','whole_ASAN_UBSAN']}
 target=stem+'-'+name
 for suffix in ['prg','map','build.log','validation.log','vice-mailbox.bin','hardware-stack.bin','software-stack.bin','vice.log','vice-monitor.log','vice.mon']:
  path=target+('.'if suffix in ['prg','map']else'-')+suffix;shutil.copyfile(r/'build'/path,e/path);files.append('evidence/'+path)
 prg=(e/f'{target}.prg').read_bytes();load=int.from_bytes(prg[:2],'little');log=(e/f'{target}-validation.log').read_text();stack=[int(v)for v in re.findall(r'stack extent (\d+) bytes',log)];mail=(e/f'{target}-vice-mailbox.bin').read_bytes();assert mail==bytes([0xa5,0,0,32,mission])and len(stack)==2
 targets[name]=dict(complete_states=32,mailbox=mail.hex(),PRG_bytes=len(prg),loaded_end=hex(load+len(prg)-3),hardware_stack=stack[0],software_stack=stack[1],compiler_ZP=0)
for suffix in ['host-validation.log','frozen-validation.log','source-map-validation.log','source-map.json','corpus-stats.json']:
 path=stem+'-'+suffix;shutil.copyfile(r/'build'/path,e/path);files.append('evidence/'+path)
review=e/'rtb-pulse-return-independent-review.txt';assert review.exists();files.append('evidence/'+review.name)
proof=dict(complete_states=sum(x['complete_states']for x in stats.values()),model_words=2048,external_words=['IRQ','SUPER'],source_group_words={m:x['source_group_words']for m,x in mapping.items()},source_map=f'evidence/{stem}-source-map.json',logical_complete_streams=streams,actual_C64=targets,source_audit_dependency='evidence/rtb-angles-source-audit.json',independent_review=dict(path='evidence/'+review.name,sha256=hashlib.sha256(review.read_bytes()).hexdigest()),qualification='Original trailing PULSEIMU TCF DANZIG after actual external return; ordinary canonical/source bank coherent registers, rawA/L, externalIRQ/SUPER, excluded producers. Pending interpreter controls retained; DANZIG body and external IMUPULSE not executed.',decoded_stream_identity='Each mission original stream is retained once as deterministic XZ. Strict and full sanitized streams directly compared with decoded original under pipefail.')
p=e/'rtb-pulse-return-proof.json';p.write_text(json.dumps(proof,indent=2)+'\n');files.append('evidence/'+p.name)
files+=['docs/NATIVE-RTB-PULSE-RETURN.md','include/rtb_pulse_return.h','src/rtb_pulse_return.c','tests/rtb_pulse_return_cases.h','tests/rtb_pulse_return_cases.c','tests/rtb_pulse_return_vectors.h','tests/test_rtb_pulse_return.c']
files+=['tools/'+n for n in ['rtb_pulse_return.mk','rtb_pulse_return_reference.c','rtb_pulse_return_snapshot.c','rtb_pulse_return_inputs.c','rtb_pulse_return_source.py','rtb_pulse_return_diagnostic.c','rtb_pulse_return_vice_test.py','freeze_rtb_pulse_return.py','package_rtb_pulse_return.py']]
files=sorted(set(files));manifest='evidence/rtb-pulse-return-evidence.sha256';(r/manifest).write_text(''.join(hashlib.sha256((r/p).read_bytes()).hexdigest()+'  '+p+'\n'for p in files));files.append(manifest)
(r/'build/rtb-pulse-return-commit-files.txt').write_text('\n'.join(sorted(files))+'\n');print(len(files),'ready files;',proof['complete_states'],'complete states')
