#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Freeze RTB sign agreement and fixed CM transpose groups, partial files."""
import pathlib,gzip,lzma,json,hashlib,re,shutil
r=pathlib.Path(__file__).resolve().parents[1];e=r/'evidence';stem='rtb-agreement';files=[]
stats=json.loads((r/f'build/{stem}-corpus-stats.json').read_text());mapping=json.loads((r/f'build/{stem}-source-map.json').read_text());targets={};streams={}
for name,x in stats.items():
 mission=x['mission'];entry=x['entry'];tree=('lm','cm')[mission];digest=x['decoded_sha256'];length=x['complete_states']*4104
 for role in ['reference','native','sanitize']:
  with gzip.open(r/f'build/{stem}-{tree}-{entry}-{role}.snapshots.gz','rb')as f:
   h=hashlib.sha256();size=0
   while raw:=f.read(1048576):h.update(raw);size+=len(raw)
  assert size==length and h.hexdigest()==digest
 obj=e/f'{stem}-{name}-complete.snapshots.xz'
 if obj.exists():
  with lzma.open(obj,'rb')as f:assert hashlib.file_digest(f,'sha256').hexdigest()==digest
 else:
  with gzip.open(r/f'build/{stem}-{tree}-{entry}-reference.snapshots.gz','rb')as a,lzma.open(obj,'wb',preset=6)as b:shutil.copyfileobj(a,b,1048576)
 files.append('evidence/'+obj.name)
 streams[name]={role:{'object':'evidence/'+obj.name,'decoded_sha256':digest,'decoded_bytes':length}for role in ['original_assembly','strict_native','whole_ASAN_UBSAN']}
 target=stem+'-'+name
 for suffix in ['prg','map','build.log','validation.log','vice-mailbox.bin','hardware-stack.bin','software-stack.bin','vice.log','vice-monitor.log','vice.mon']:
  path=target+('.'if suffix in ['prg','map']else'-')+suffix;shutil.copyfile(r/'build'/path,e/path);files.append('evidence/'+path)
 prg=(e/f'{target}.prg').read_bytes();load=int.from_bytes(prg[:2],'little');log=(e/f'{target}-validation.log').read_text();stack=[int(v)for v in re.findall(r'stack extent (\d+) bytes',log)];mail=(e/f'{target}-vice-mailbox.bin').read_bytes()
 assert mail==bytes([0xa5,0,0,32,mission*16+entry])and len(stack)==2
 targets[name]=dict(complete_states=32,mailbox=mail.hex(),PRG_bytes=len(prg),loaded_end=hex(load+len(prg)-3),hardware_stack=stack[0],software_stack=stack[1],compiler_ZP=0)
for suffix in ['validation.log','host-validation.log','frozen-validation.log','source-map.json','corpus-stats.json']:
 path=stem+'-'+suffix;shutil.copyfile(r/'build'/path,e/path);files.append('evidence/'+path)
review=e/f'{stem}-independent-review.txt';assert review.exists();files.append('evidence/'+review.name)
proof=dict(complete_states=sum(x['complete_states']for x in stats.values()),model_words=2048,external_words=['IRQ','SUPER'],containing_file_status='partial',qualified_containing_words={m:x['qualified_containing_words']for m,x in mapping.items()},original_source_audit='evidence/rtb-angles-source-audit.json',source_map=f'evidence/{stem}-source-map.json',logical_complete_streams=streams,actual_C64=targets,source_constants={'Comanche055/XNBEB':{'physical_fixed_word':19376,'raw_octal':'02713'},'Comanche055/XNB1EB':{'physical_fixed_word':19377,'raw_octal':'02554'}},independent_review=dict(path='evidence/'+review.name,sha256=hashlib.sha256(review.read_bytes()).hexdigest()),scope='Both mission SGNAGREE/TPAGREE and SIGNMPAC; CM TRANSP1/2 and XNBEB/XNB1EB. Source-exact raw TP carry, signed-zero sign selection, six DP swaps and actual basic returns through mission DANZIG entry. Every ordinary major/minor input and transpose word swept in varied contexts, not Cartesian. Nonnull canonical image and ordinary memory/rawA/L caller contract. NORMUNIT/NORMUNX1/VECSGNAG/PULSEIMU/CMQUALITY remain unclaimed; no whole RTB file or resident application certified.')
p=e/f'{stem}-proof.json';p.write_text(json.dumps(proof,indent=2)+'\n');files.append('evidence/'+p.name)
files+=['docs/RTB-AGREEMENT.md','include/rtb_agreement.h','src/rtb_agreement.c','tests/rtb_agreement_cases.h','tests/rtb_agreement_cases.c','tests/rtb_agreement_vectors.h','tests/test_rtb_agreement.c']
files+=['tools/'+n for n in ['rtb_agreement.mk','rtb_agreement_reference.c','rtb_agreement_snapshot.c','rtb_agreement_inputs.c','rtb_agreement_source.py','rtb_agreement_diagnostic.c','rtb_agreement_vice_test.py','freeze_rtb_agreement.py','package_rtb_agreement.py']]
files=sorted(set(files));manifest=f'evidence/{stem}-evidence.sha256';(r/manifest).write_text(''.join(hashlib.sha256((r/p).read_bytes()).hexdigest()+'  '+p+'\n'for p in files));files.append(manifest)
(r/f'build/{stem}-commit-files.txt').write_text('\n'.join(sorted(files))+'\n');print(len(files),'ready files;',proof['complete_states'],'complete states')
