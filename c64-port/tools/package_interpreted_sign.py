#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Freeze generic source-qualified LM/CM COMP and SIGN arithmetic."""
import pathlib,json,gzip,lzma,hashlib,shutil,re
r=pathlib.Path(__file__).resolve().parents[1];e=r/'evidence';stem='interpreted-sign';files=[]
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
for suffix in ['validation.log','host-validation.log','frozen-validation.log','source-audit.json','source-audit.log','corpus-stats.json','input-coverage.json','input-coverage.log']:
 keep(f'build/{stem}-{suffix}',f'{stem}-{suffix}')
actual=[]
for selector in range(4):
 for suffix in ['prg','map','build.log','validation.log','vice-mailbox.bin','hardware-stack.bin','software-stack.bin','vice.log','vice-monitor.log','vice.mon']:
  name=f'{stem}{selector}'+('.'if suffix in ('prg','map')else'-')+suffix;keep('build/'+name,name)
 prg=(r/f'build/{stem}{selector}.prg').read_bytes();load=int.from_bytes(prg[:2],'little');log=(r/f'build/{stem}{selector}-validation.log').read_text();stack=[int(v)for v in re.findall(r'stack extent (\d+) bytes',log)];mail=(r/f'build/{stem}{selector}-vice-mailbox.bin').read_bytes();assert mail==bytes([0xa5,0,0,64,selector])and len(stack)==2
 actual.append(dict(selector=selector,complete_states=64,mailbox=mail.hex(),PRG_bytes=len(prg),loaded_end=hex(load+len(prg)-3),hardware_stack=stack[0],software_stack=stack[1],compiler_ZP=0))
review=e/f'{stem}-independent-review.txt';assert review.exists();files.append('evidence/'+review.name)
proof=dict(input_coverage=json.loads((r/f'build/{stem}-input-coverage.json').read_text()),missions=stats,total_complete_states=262144,source_footprint={name:dict(original_source_file=name+'/INTERPRETER.agc',original_sha256=record['original_source_sha256'],emitted_words=record['emitted_words'],original_lines=record['sign_source_lines'])for name,record in audit.items()},source_audit=f'evidence/{stem}-source-audit.json',model_words=2048,external_IRQ_word=1,actual_C64=actual,total_actual_C64_states=256,scope='All27 emitted SIGN/COMP group words in each original mission interpreter. Ordinary MPAC/operand words, valid source MODE except negativezero, coherent EB/BB/sourceentryPC/REGZERO0. Explicit stable operand is source effective ordinary DP pair; caller owns fixed/erasable addressing binding. Ends at source DANZIG entry without executing dispatch or NEWJOB. Every unowned canonical word and external IRQ retained. No whole INTERPRETER or whole application certificate.',independent_review=dict(path='evidence/'+review.name,sha256=hashlib.sha256(review.read_bytes()).hexdigest()))
p=e/f'{stem}-proof.json';p.write_text(json.dumps(proof,indent=2)+'\n');files.append('evidence/'+p.name)
doc='''# Original LM and CM SIGN and COMP

The native `interpreted_sign` API covers all27 emitted words of the original SIGN/COMP interpreter group in each mission. LM entries are SIGN07655/COMP07670; CM entries are SIGN07624/COMP07637. It finishes at actual DANZIG entry06061/06030 before dispatch executes, retaining LOC, EDOP, Q, source banks and external IRQ. This numeric return is not an invented scheduler check; original completed-pair scheduling remains the canonical interpreter continuation owner's responsibility.

SIGN inspects the stable source DP operand's high word first. A negative nonzero high word complements immediately, a positive nonzero high word leaves MPAC unchanged, and either high-word zero tests the low word. Only a negative nonzero low word then complements. The last CCS leaves its diminished absolute value in A on unchanged paths. COMP complements the first MPAC pair unconditionally. Nonnegative MODE complements MPAC+2; negative nonzero MODE complements the remaining vector pairs at offsets3/5, preserving MPAC+2. Source DCS/DXCH exchanges leave the old pair in A/L, including both signed zeros. Every ordinary MODE except invalid negativezero is admitted; negativezero would skip the source vector EXTEND and is deliberately rejected unchanged.

The explicit sign_operand contains ordinary15-bit source effective operand words. Its canonical addressing/fixed-constant binding belongs to the compiled caller; it is not an arbitrary interpreter address decoder. Inputs/shared scratch producers are excluded during the operation. Raw incoming16-bit L receives original instruction-time normalization, while source A/L output values and every other2048-word image field match exactly. Source entry PC, coherent EB/BB and REGZERO0 are checked before any mutation.

Both missions match all131072 complete2048-word-plus-IRQ original-machine/strict-native/full-ASAN-UBSAN states, totaling262144. Each entry uses a65536-case cohort including every ordinary MPAC high word after its boundary cohort, varied MODE and other components, stable source SIGN operand signs/zeros, raw A/L overflow bits, all coherent EB/FB combinations, externally retained IRQ and distinct nonzero unowned-image patterns. The4096-case boundary grid exercises every branch with signed zeros and extrema. Other multidimensional combinations are sampled, not an exhaustive Cartesian product.

Four actual C64 PRGs pass256 full frozen source states, covering each mission/entry separately. PRGs are5671/5844bytes, hardware stack14bytes, software stacks48/50bytes and compiler zero-page0; all load below$C000. Source-only lossless fixture deltas reconstruct expected canonical images. Native arithmetic does not read expected outputs. Strict host rejection tests verify unsupported PC/bank/mode and missing SIGN operands leave the image unchanged.

Source audit maps every original group word and verifies the unchanged qualified fixtures' original INTERPRETER source identities, declaration/data ropes and disclosed CM indentation-only repairs. Independent read-only review is retained with the proof. This is a complete SIGN/COMP source-group milestone, not a whole interpreter, arbitrary operand-addressing dispatcher, alarm/scheduler continuation or either complete Apollo application.

Reproduce with `python tools/interpreted_sign_source.py`, `make -f tools/interpreted_sign.mk all test sanitize`, `python tools/freeze_interpreted_sign.py`, `make -f tools/interpreted_sign.mk check-host`, and c64/vice-test SELECTOR0..3 with MOS_CC/VICE/C64_ROM_DIR/VICE_DATA_DIR. `tools/package_interpreted_sign.py` retains deterministic XZ objects shared by identical source/native/sanitized logical streams and every actual target artifact. The implementation uses integer source operations without an opcode decoder or host floating point.
'''
(r/'docs/INTERPRETED-SIGN.md').write_text(doc);files.append('docs/INTERPRETED-SIGN.md')
files+=['include/interpreted_sign.h','src/interpreted_sign.c','tests/interpreted_sign_cases.h','tests/interpreted_sign_cases.c','tests/interpreted_sign_vectors.h','tests/test_interpreted_sign.c']
files+=['tools/'+name for name in ['interpreted_sign.mk','interpreted_sign_reference.c','interpreted_sign_snapshot.c','interpreted_sign_inputs.c','interpreted_sign_source.py','interpreted_sign_diagnostic.c','interpreted_sign_vice_test.py','freeze_interpreted_sign.py','package_interpreted_sign.py','interpreted_sign_coverage.py','interpreted_sign_coverage.c']]
files=sorted(set(files));manifest=f'evidence/{stem}-evidence.sha256';(r/manifest).write_text(''.join(hashlib.sha256((r/p).read_bytes()).hexdigest()+'  '+p+'\n'for p in files));files.append(manifest);(r/f'build/{stem}-commit-files.txt').write_text('\n'.join(sorted(files))+'\n');print(len(files),'ready files',proof['total_complete_states'])
