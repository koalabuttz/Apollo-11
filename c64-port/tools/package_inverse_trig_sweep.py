#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Package additive exhaustive-high-word coverage without editing frozen identities."""
import pathlib,json,gzip,lzma,hashlib,shutil,re
r=pathlib.Path(__file__).resolve().parents[1];e=r/'evidence';stem='inverse-trig-sweep';stats=json.loads((r/f'build/{stem}-corpus-stats.json').read_text());coverage=json.loads((r/f'build/{stem}-input-coverage.json').read_text());files=[]
for s,c in zip(stats,coverage):
 name=s['mission'];digest=s['decoded_sha256'];size=s['complete_states']*4108
 assert c['arccos_distinct_high_words']==c['arcsin_distinct_high_words']==32768 and not c['missing_high_words']
 for role in ['reference','native','sanitize']:
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
 s['logical_complete_streams']={role:dict(object='evidence/'+obj.name,decoded_sha256=digest)for role in ['original_assembly','strict_native','whole_ASAN_UBSAN']};s['decoded_bytes']=size

def keep(src,dst):shutil.copyfile(r/src,e/dst);files.append('evidence/'+dst)
for suffix in ['validation.log','host-validation.log','frozen-validation.log','corpus-stats.json','input-coverage.json','input-coverage.log']:
 keep(f'build/{stem}-{suffix}',f'{stem}-{suffix}')
actual=[]
for selector in range(8):
 for suffix in ['prg','map','build.log','validation.log','vice-mailbox.bin','hardware-stack.bin','software-stack.bin','vice.log','vice-monitor.log','vice.mon']:
  name=f'{stem}{selector}'+('.'if suffix in ('prg','map')else'-')+suffix;keep('build/'+name,name)
 prg=(r/f'build/{stem}{selector}.prg').read_bytes();load=int.from_bytes(prg[:2],'little');log=(r/f'build/{stem}{selector}-validation.log').read_text();stack=[int(n)for n in re.findall(r'stack extent (\d+) bytes',log)];mail=(r/f'build/{stem}{selector}-vice-mailbox.bin').read_bytes()
 assert mail==bytes([0xa5,0,0,32,selector])and len(stack)==2
 actual.append(dict(selector=selector,complete_states=32,mailbox=mail.hex(),PRG_bytes=len(prg),loaded_end=hex(load+len(prg)-3),hardware_stack=stack[0],software_stack=stack[1],compiler_ZP=0,constants_checked=36))
review=e/'inverse-trig-independent-review.txt';assert review.exists();files.append('evidence/'+review.name)
proof=dict(missions=stats,input_coverage=coverage,total_complete_states=sum(s['complete_states']for s in stats),actual_C64=actual,total_actual_C64_states=256,model_words=2048,external_IRQ_word=1,correction_to_frozen_claim='The original158320-state-per-mission corner corpus has30723 distinct highwords per function and misses2045. Its wording claiming every highword was incorrect. This additional65536-input-per-mission sweep independently supplies every32768 ordinary MPAC.high word for each function. Original numerical/source/constants/target proof identities remain byte-unchanged.',qualified_core_sha256={p:hashlib.sha256((r/p).read_bytes()).hexdigest()for p in ['src/inverse_trig.c','include/inverse_trig.h','tests/inverse_trig_cases.c','tools/inverse_trig_reference.c','tools/inverse_trig_snapshot.c']},original_source_proofs=['evidence/inverse-trig-lm-proof.json','evidence/inverse-trig-cm-proof.json'],independent_source_review=dict(path='evidence/'+review.name,sha256=hashlib.sha256(review.read_bytes()).hexdigest()),scope='Additive high-word input coverage for already qualified LM/CM ARCSIN/ARCCOS native stages only. Original alarm continuation, arbitrary producer overlap and whole INTERPRETER remain outside scope. All source/numeric states and externally retained IRQ compared at all four stages, pending EDOP and genuine DANZIG behavior unchanged. Cartesian operand combinations are sampled, not exhaustive.')
p=e/f'{stem}-proof.json';p.write_text(json.dumps(proof,indent=2)+'\n');files.append('evidence/'+p.name)
doc='''# Inverse-trig exhaustive input supplement

The frozen corner corpus originally replaced its first4096 nominal sweep cases with a16-value edge grid. Independent review found that it therefore contains30723 distinct ordinary MPAC.high inputs per function, missing2045, so the earlier report's claim of every high word was incorrect. The original arithmetic/control/constant/source identities remain unchanged.

This additive corpus restores actual MPAC.high=n>>1 after the unchanged original factory constructs its varied minor/third words, MODE, banks, pending EDOP and external IRQ. The actual emitted input images are independently inspected: each mission supplies65536 inputs and exactly32768 distinct high words for ARCSIN and for ARCCOS, with no missing word. The old boundary grid remains qualified by the original proof; the new sweep complements it without replacing its hashes. The combined proof now supports the exhaustive high-word claim, with other operands varied rather than every Cartesian combination exhausted.

Both missions match all163834 complete2048-word-plus-IRQ states at the four original stage boundaries against independent original assembly and full ASAN/UBSAN, totaling327668 supplementary states. All stages retain the original pending continuation and source controls. Eight new actual C64 phase PRGs pass256 frozen complete source states and check all36 original constants. Every PRG, map, mailbox, stack dump and source-only lossless fixture is retained. Numerical code is unchanged and no expected output is consumed by native computations.

The independent read-only source review found no concrete numerical/control discrepancy in the valid canonical/ordinary-word domain, mapped all85LM/83CM emitted words independently and exhausted constant reads over all65536 addresses per mission. Its reported coverage finding is resolved by this supplement. This remains a bounded ARCSIN/ARCCOS source milestone: actual alarm-service continuation, the whole interpreter and both complete flight applications are not certified.

Reproduce with `make -f tools/inverse_trig_sweep.mk all test sanitize`, `python tools/inverse_trig_sweep_coverage.py`, `python tools/freeze_inverse_trig_sweep.py`, `make -f tools/inverse_trig_sweep.mk check-host`, and c64/vice-test SELECTOR0..7 with MOS_CC/VICE/C64_ROM_DIR/VICE_DATA_DIR. `tools/package_inverse_trig_sweep.py` retains one deterministic XZ object for each mission's three byte-identical logical streams and packages all actual target artifacts. Source fixtures and arithmetic dependencies are the unchanged original packet's qualified ones.
'''
(r/'docs/INVERSE-TRIG-SWEEP.md').write_text(doc);files.append('docs/INVERSE-TRIG-SWEEP.md')
files+=['tests/inverse_trig_sweep_cases.c','tests/inverse_trig_sweep_vectors.h','tests/test_inverse_trig_sweep.c','tools/inverse_trig_sweep.mk','tools/inverse_trig_sweep_vice_test.py','tools/inverse_trig_sweep_coverage.py','tools/freeze_inverse_trig_sweep.py','tools/package_inverse_trig_sweep.py']
files=sorted(set(files));manifest=f'evidence/{stem}-evidence.sha256';(r/manifest).write_text(''.join(hashlib.sha256((r/p).read_bytes()).hexdigest()+'  '+p+'\n'for p in files));files.append(manifest);(r/f'build/{stem}-commit-files.txt').write_text('\n'.join(sorted(files))+'\n');print(len(files),'ready files',proof['total_complete_states'])
