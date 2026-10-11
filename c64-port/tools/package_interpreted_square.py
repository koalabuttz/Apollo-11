#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Freeze generic source-qualified LM/CM COMP and SIGN arithmetic."""
import pathlib,json,gzip,lzma,hashlib,shutil,re
r=pathlib.Path(__file__).resolve().parents[1];e=r/'evidence';stem='interpreted-square';files=[]
stats=json.loads((r/f'build/{stem}-corpus-stats.json').read_text());audit=json.loads((r/f'build/{stem}-source-audit.json').read_text())
for s in stats:
 name=s['mission'];digest=s['decoded_sha256'];size=s['complete_states']*4102
 assert s['complete_states']==131072
 for role in ('reference','native','sanitize'):
  with gzip.open(r/f'build/{stem}-{name}-{role}.snapshots.gz','rb')as f:
   h=hashlib.sha256();length=0
   while raw:=f.read(1048576):h.update(raw);length+=len(raw)
  assert length==size and h.hexdigest()==digest
 obj=e/f'{stem}-{name}-complete.snapshots.xz'
 if obj.exists():
  with lzma.open(obj,'rb')as f:assert hashlib.file_digest(f,'sha256').hexdigest()==digest
 else:
  with gzip.open(r/f'build/{stem}-{name}-reference.snapshots.gz','rb')as a,lzma.open(obj,'wb',preset=6)as b:shutil.copyfileobj(a,b,1048576)
 files.append('evidence/'+obj.name)
 s['logical_complete_streams']={role:dict(object='evidence/'+obj.name,decoded_sha256=digest)for role in ('original_assembly','strict_native','whole_ASAN_UBSAN')};s['decoded_bytes']=size

def keep(src,dst):shutil.copyfile(r/src,e/dst);files.append('evidence/'+dst)
for suffix in ['validation.log','host-validation.log','frozen-validation.log','source-audit.json','source-audit.log','corpus-stats.json']:
 keep(f'build/{stem}-{suffix}',f'{stem}-{suffix}')
actual=[]
for selector in range(4):
 for suffix in ['prg','map','build.log','validation.log','vice-mailbox.bin','hardware-stack.bin','software-stack.bin','vice.log','vice-monitor.log','vice.mon']:
  name=f'{stem}{selector}'+('.'if suffix in ('prg','map')else'-')+suffix;keep('build/'+name,name)
 prg=(r/f'build/{stem}{selector}.prg').read_bytes();load=int.from_bytes(prg[:2],'little');log=(r/f'build/{stem}{selector}-validation.log').read_text();stack=[int(v)for v in re.findall(r'stack extent (\d+) bytes',log)];mail=(r/f'build/{stem}{selector}-vice-mailbox.bin').read_bytes();assert mail==bytes([0xa5,0,0,64,selector])and len(stack)==2
 actual.append(dict(selector=selector,complete_states=64,mailbox=mail.hex(),PRG_bytes=len(prg),loaded_end=hex(load+len(prg)-3),hardware_stack=stack[0],software_stack=stack[1],compiler_ZP=0))
review=e/f'{stem}-independent-review.txt';assert review.exists();files.append('evidence/'+review.name)
proof=dict(missions=stats,total_complete_states=262144,source_footprint={name:dict(original_source_file=name+'/INTERPRETER.agc',original_sha256=record['original_source_sha256'],emitted_words=record['emitted_words'],original_lines=dict(DSQ=record['DSQ_source_lines'],DSQSUB=record['DSQSUB_source_lines']))for name,record in audit.items()},source_audit=f'evidence/{stem}-source-audit.json',model_words=2048,external_IRQ_word=1,actual_C64=actual,total_actual_C64_states=256,scope='All17 emitted DSQ/DSQSUB group words in each original mission interpreter. Ordinary MPAC/operand words, any ordinary source MODE, coherent EB/BB/sourceentryPC/REGZERO0. FB0 bank0 direct source entries3174/3300. DSQ ends at source DANZIG entry without executing dispatch/NEWJOB; DSQSUB completes TCQ to savedfixedQ02000..07777 with Q3. Every unowned canonical word and external IRQ retained. No whole INTERPRETER or whole application certificate.',independent_review=dict(path='evidence/'+review.name,sha256=hashlib.sha256(review.read_bytes()).hexdigest()))
p=e/f'{stem}-proof.json';p.write_text(json.dumps(proof,indent=2)+'\n');files.append('evidence/'+p.name)
doc="# Original LM and CM DSQ and DSQSUB\n\nThe native `interpreted_square` API covers the complete17-word scalar DSQ/DSQSUB source group in each original mission interpreter. Both entries share bank0 addresses03174/03300. DSQ performs source squaring and reaches DANZIG06061/06030 before dispatch executes. Direct DSQSUB finishes the actual TCQ transfer at its saved fixed return Q02000..07777, with Q3; it does not run the instruction at that returned PC. External IRQ is retained and no Executive check is invented.\n\nThe source arithmetic keeps high(minor squared), forms the unrounded doubled cross term with original low-limb overflowcarry, then adds the major square. The first DAS indicator is stored by source XCH before final SQUARE/DAS; MPAC+2 keeps the source unrounded third limb. A retains the final net DAS overflowindicator and L is positivezero. MODE, OVFIND and every other source scratch field remain unchanged. Raw zeros, disagreeing limb signs and cross-term/major overflow are retained without floating point or an opcode decoder.\n\nBoth missions match all131072 complete2048-word-plus-IRQ original/strict/full-ASAN-UBSAN states, totaling262144. Each entry's65536-case cohort includes every ordinary major/minor raw word in varied contexts after its4096-case boundary grid, signed zeros, HALF/extrema, raw initial A/L, retained MODE, coherent EB/BB, all3072 fixed return Q values for directsub, IRQ contexts and distinct nonzero unowned-image patterns. Every unowned canonical word is compared. The multidimensional combinations are sampled, not an exhaustive Cartesian product.\n\nFour actual C64 PRGs pass256 complete frozen source states. PRGs are6549/6641bytes, hardware stack17bytes, software stack53/55bytes and compiler zero-page0. All load below$C000. Lossless source-only fixture deltas reconstruct complete expected images; native arithmetic consumes only ordinary input images. Strict host rejection tests ensure unsupported PC/bank/REGZERO/direct return leave the image unchanged.\n\nSource audit maps both DSQ entry words and all15 DSQSUB words in each mission, preserving original INTERPRETER/declaration rope identities and the previously disclosed CM indentation-only repairs. Shared AGC integer MP/DAS helpers are requalified through the complete original caller/subroutine composition. Independent read-only review finds no defect and is retained with exact reviewed hashes. The canonical source input/scratch producers remain excluded until the actual return boundary.\n\nThis is complete arithmetic-group coverage, not whole INTERPRETER, arbitrary erasable return, native Executive continuation or either complete Apollo application. Reproduce with `python tools/interpreted_square_source.py`, `make -f tools/interpreted_square.mk all test sanitize`, `python tools/freeze_interpreted_square.py`, `make -f tools/interpreted_square.mk check-host`, and c64/vice-test SELECTOR0..3 with MOS_CC/VICE/C64_ROM_DIR/VICE_DATA_DIR. `tools/package_interpreted_square.py` retains deterministic XZ objects for all three byte-identical logical streams and all actual target artifacts.\n"
(r/'docs/INTERPRETED-SQUARE.md').write_text(doc);files.append('docs/INTERPRETED-SQUARE.md')
files+=['include/interpreted_square.h','src/interpreted_square.c','tests/interpreted_square_cases.h','tests/interpreted_square_cases.c','tests/interpreted_square_vectors.h','tests/test_interpreted_square.c']
files+=['tools/'+name for name in ['interpreted_square.mk','interpreted_square_reference.c','interpreted_square_snapshot.c','interpreted_square_inputs.c','interpreted_square_source.py','interpreted_square_diagnostic.c','interpreted_square_vice_test.py','freeze_interpreted_square.py','package_interpreted_square.py']]
files=sorted(set(files));manifest=f'evidence/{stem}-evidence.sha256';(r/manifest).write_text(''.join(hashlib.sha256((r/p).read_bytes()).hexdigest()+'  '+p+'\n'for p in files));files.append(manifest);(r/f'build/{stem}-commit-files.txt').write_text('\n'.join(sorted(files))+'\n');print(len(files),'ready files',proof['total_complete_states'])
