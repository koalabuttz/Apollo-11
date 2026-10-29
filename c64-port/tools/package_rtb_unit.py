#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Freeze original UNIT and NORM complete numerical caller composition."""
import pathlib,gzip,lzma,json,hashlib,re,shutil
r=pathlib.Path(__file__).resolve().parents[1];e=r/'evidence';stem='rtb-unit';files=[]
stats=json.loads((r/f'build/{stem}-corpus-stats.json').read_text());mapping=json.loads((r/f'build/{stem}-source-map.json').read_text());targets={};streams={}
for name,x in stats.items():
 mission=x['mission'];caller=x['caller'];phase=x['phase'];tree=('lm','cm')[mission];digest=x['decoded_sha256'];length=x['complete_states']*4106
 for role in ['reference','native','sanitize']:
  with gzip.open(r/f'build/{stem}-{tree}-{caller}-{phase}-{role}.snapshots.gz','rb')as f:
   h=hashlib.sha256();size=0
   while raw:=f.read(1048576):h.update(raw);size+=len(raw)
  assert size==length and h.hexdigest()==digest
 obj=e/f'{stem}-{name}-complete.snapshots.xz'
 if obj.exists():
  with lzma.open(obj,'rb')as f:assert hashlib.file_digest(f,'sha256').hexdigest()==digest
 else:
  with gzip.open(r/f'build/{stem}-{tree}-{caller}-{phase}-reference.snapshots.gz','rb')as a,lzma.open(obj,'wb',preset=6)as b:shutil.copyfileobj(a,b,1048576)
 files.append('evidence/'+obj.name)
 streams[name]={role:{'object':'evidence/'+obj.name,'decoded_sha256':digest,'decoded_bytes':length}for role in ['original_assembly','strict_native','whole_ASAN_UBSAN']}
 target=stem+'-'+name
 for suffix in ['prg','map','build.log','validation.log','vice-mailbox.bin','hardware-stack.bin','software-stack.bin','vice.log','vice-monitor.log','vice.mon']:
  path=target+('.'if suffix in ['prg','map']else'-')+suffix;shutil.copyfile(r/'build'/path,e/path);files.append('evidence/'+path)
 prg=(e/f'{target}.prg').read_bytes();load=int.from_bytes(prg[:2],'little');log=(e/f'{target}-validation.log').read_text();stack=[int(v)for v in re.findall(r'stack extent (\d+) bytes',log)];mail=(e/f'{target}-vice-mailbox.bin').read_bytes()
 assert mail==bytes([0xa5,0,0,32,mission*32+caller*4+phase])and len(stack)==2
 targets[name]=dict(complete_states=32,mailbox=mail.hex(),PRG_bytes=len(prg),loaded_end=hex(load+len(prg)-3),hardware_stack=stack[0],software_stack=stack[1],compiler_ZP=0)
for suffix in ['validation.log','host-validation.log','frozen-validation.log','source-map.json','corpus-stats.json']:
 path=stem+'-'+suffix;shutil.copyfile(r/'build'/path,e/path);files.append('evidence/'+path)
review=e/f'{stem}-independent-review.txt';assert review.exists();files.append('evidence/'+review.name)
proof=dict(complete_states=sum(x['complete_states']for x in stats.values()),model_words=2048,external_words=['IRQ','SUPER'],additional_observation='Source path0 normal /2 division overflow; abort1 API exists but no1302 checkpoint reached by this corpus.',source_group_words={m:x['source_group_words']for m,x in mapping.items()},containing_file_status='partial',source_map=f'evidence/{stem}-source-map.json',logical_complete_streams=streams,actual_C64=targets,immutable_numeric_dependencies={p:hashlib.sha256((r/p).read_bytes()).hexdigest()for p in ['src/guidance_length_phased.c','src/guidance_unit_phased.c','src/guidance_norm.c']},independent_review=dict(path='evidence/'+review.name,sha256=hashlib.sha256(review.read_bytes()).hexdigest()),scope='Both normal UNIT TC VECAGREE prologue and UNIT+1 numerical phases; complete NORMUNIT/NORMUNX1 callers through actual DANZIG. Ordinary canonical inputs/vector MODE-1 and initialized five VAC bases. UNIT phases retain canonical ownership; no added scheduling checks. All input cases compared through original full caller prefix at each of three subroutine phases. Generic unsupported caller modes/invalidVAC outside contract. No complete INTERPRETER file or resident application certified; RTB closure still requires IMUPULSE/hardware body and CMQUALITY groups.')
p=e/f'{stem}-proof.json';p.write_text(json.dumps(proof,indent=2)+'\n');files.append('evidence/'+p.name)
files+=['docs/RTB-UNIT.md','include/rtb_unit.h','src/rtb_unit.c','tests/rtb_unit_cases.h','tests/rtb_unit_cases.c','tests/rtb_unit_vectors.h','tests/test_rtb_unit.c']
files+=['tools/'+n for n in ['rtb_unit.mk','rtb_unit_reference.c','rtb_unit_snapshot.c','rtb_unit_inputs.c','rtb_unit_source.py','rtb_unit_diagnostic.c','rtb_unit_vice_test.py','freeze_rtb_unit.py','package_rtb_unit.py']]
files=sorted(set(files));manifest=f'evidence/{stem}-evidence.sha256';(r/manifest).write_text(''.join(hashlib.sha256((r/p).read_bytes()).hexdigest()+'  '+p+'\n'for p in files));files.append(manifest)
(r/f'build/{stem}-commit-files.txt').write_text('\n'.join(sorted(files))+'\n');print(len(files),'ready files;',proof['complete_states'],'complete states')
