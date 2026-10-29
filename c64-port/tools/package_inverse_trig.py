#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Freeze original inverse-trig proofs and complete native target results."""
import pathlib,hashlib,gzip,lzma,json,shutil,re
r=pathlib.Path(__file__).resolve().parents[1];e=r/'evidence';files=[]
def keep(src,dst):shutil.copyfile(r/src,e/dst);files.append('evidence/'+dst)
stats=json.loads((r/'build/inverse-trig-corpus-stats.json').read_text());audit=json.loads((r/'build/inverse-trig-source-audit.json').read_text())
proofs=[]
for mission,info in enumerate(stats):
 name=info['mission'];digest=info['decoded_sha256'];size=info['complete_states']*4108
 for key in ['reference','native','sanitize']:
  with gzip.open(r/f'build/inverse-trig-{name}-{key}.snapshots.gz','rb')as f:
   h=hashlib.sha256();length=0
   while raw:=f.read(1024*1024):h.update(raw);length+=len(raw)
  assert length==size and h.hexdigest()==digest
 obj=e/f'inverse-trig-{name}-complete.snapshots.xz'
 if obj.exists():
  with lzma.open(obj,'rb')as f:assert hashlib.file_digest(f,'sha256').hexdigest()==digest
 else:
  with gzip.open(r/f'build/inverse-trig-{name}-reference.snapshots.gz','rb')as a,lzma.open(obj,'wb',preset=6)as b:shutil.copyfileobj(a,b,1024*1024)
 files.append('evidence/'+obj.name);keep(f'build/inverse-trig-{name}-reference.trace',f'inverse-trig-{name}-reference.trace')
 targets=[]
 for phase in range(4):
  n=mission*4+phase;pre=f'inverse-trig{n}'
  for suffix in ['prg','map','build.log','validation.log','vice-mailbox.bin','hardware-stack.bin','software-stack.bin','vice.log','vice-monitor.log','vice.mon']:
   filename=pre+('.'if suffix in ['prg','map']else'-')+suffix;keep('build/'+filename,filename)
  log=(r/f'build/{pre}-validation.log').read_text();prg=(r/f'build/{pre}.prg').read_bytes();load=int.from_bytes(prg[:2],'little');stack=[int(v)for v in re.findall(r'stack extent (\d+) bytes',log)];mail=(r/f'build/{pre}-vice-mailbox.bin').read_bytes();assert mail==bytes([0xa5,0,0,32,n]);assert len(stack)==2
  targets.append({'selector':n,'source_phase':phase,'complete_states':32,'mailbox':mail.hex(),'PRG_bytes':len(prg),'loaded_end':hex(load+len(prg)-3),'hardware_stack':stack[0],'software_stack':stack[1],'compiler_ZP':0})
 key='Luminary099'if mission==0 else'Comanche055'
 proof=dict(info,source_audit='evidence/inverse-trig-source-audit.json',original_source_sha256=audit[key]['original_source_sha256'],support_rope_sha256=audit[key]['support_rope_sha256'],calls=65536,model_words=2048,external_IRQ_word=1,decoded_bytes=size,logical_complete_streams={k:{'object':'evidence/'+obj.name,'decoded_sha256':digest}for k in ['original_assembly','strict_native','whole_ASAN_UBSAN']},target_images=targets,constants_checked_on_every_target=36,scope='Explicit source ARCSIN/ARCCOS stages through math/zero/clamp DANZIG entry, and pre1301 alarm service boundary. Basic internal returns retain ownership; no invented source Executive check. The source alarm handler and post-alarm continuation, arbitrary erasable code dispatch, and entire INTERPRETER/application are outside this packet. SQRT1302 status is exposed for boundary-compatible invalid arguments but no produced case reaches it. Every native canonical phase accepts only its stated source continuation and ordinary coherent bank registers. Raw overflow/signedzero/TPthird scratch effects remain source exact.')
 path=e/f'inverse-trig-{name}-proof.json';path.write_text(json.dumps(proof,indent=2)+'\n');files.append('evidence/'+path.name);proofs.append(str(path.relative_to(r)));print(name,info['complete_states'],digest,flush=True)
for suffix in ['validation.log','host-validation.log','frozen-validation.log','source-audit.json','corpus-stats.json']:keep('build/inverse-trig-'+suffix,'inverse-trig-'+suffix)
files.extend(['include/inverse_trig.h','src/inverse_trig.c','tests/inverse_trig_cases.h','tests/inverse_trig_cases.c','tests/inverse_trig_constants.h','tests/inverse_trig_vectors.h','tests/test_inverse_trig.c','tools/inverse_trig.mk','tools/inverse_trig_reference.c','tools/inverse_trig_snapshot.c','tools/inverse_trig_inputs.c','tools/inverse_trig_source.py','tools/inverse_trig_diagnostic.c','tools/inverse_trig_vice_test.py','tools/freeze_inverse_trig.py','tools/package_inverse_trig.py'])
doc='''# Original Luminary and Comanche inverse trigonometry

Native canonical `inverse_trig_prepare/root/polynomial/finish` follows original ARCSIN/ARCCOS in each mission's INTERPRETER. The first stage handles source triple-sign branching, signed zeros, magnitude/clamp tests, negative identity and exact ESCAPE/ESCAPE2 instruction-word values. It returns before normalized SQRTSUB or at source completed DANZIG/pre1301 alarm boundary. The root stage composes independently qualified normalized square root and source DP VSHRRND. The polynomial stage performs all seven DMPSUB-plus-DAS iterations with the eight original coefficients, preserving the third word without intermediate rounding. Final DMPSUB and original negative/arcsine epilogues preserve raw A/L, Q, scratch, MODE and all bank controls.

Each mission runs 65,536 inputs, covering every ordinary high-word input for both routines, signed-zero/minor/third-word corners, every ordinary MODE, all eight EB contexts, initial raw A/L overflow patterns, pending EDOP and retained IRQ. Each returns 158,320 complete 2048-word-plus-IRQ checkpoints: 65,536 preparation states and 30,928 states each after root, polynomial and finish. All 316,640 original/strict/full-ASAN-UBSAN checkpoints match, including every unowned physical word. There are 33,784 pre1301 boundaries per mission; zero/clamp cases and completed math account for the remaining source paths. Decoded stream identities and one deterministic lossless XZ object per mission are retained under the proofs.

Eight actual C64 PRGs pass 256 complete frozen source states. Every diagnostic also reads and compares all 36 original degree/coefficient/NEG12 constants across both missions. Source-only lossless fixtures reconstruct expected images from original execution; expected flight values are never supplied to native arithmetic. Native code uses no AGC opcode decoder or host floating point. Unsupported bank/PC/REGZERO/escape continuations reject unchanged. Numerical constants are an explicit readonly source accessor.

The LM fixture uses byte-original INTERPRETER with zero edits. CM uses byte-original INTERPRETER with the 36 indentation-only repairs disclosed in the frozen POWERED trig source audit; no containing inverse-group line is edited. Both fixtures retain the byte-original declaration/data domains. The source audit records original/assembled SHA identities, rope identity and every emitted inverse-group word (LM85/CM83). It covers the whole inverse arithmetic group, not the whole INTERPRETER source file.

The native calls expose basic arithmetic implementation boundaries. Internal C returns retain canonical ownership; an external Executive check is allowed only at a genuine completed-source DANZIG/EDOP0 boundary. Producers and shared scratch must remain excluded through each original operation. This packet stops before the original 1301 alarm handler; external code owns actual ALARM1/ALARM and its post-alarm continuation. The exposed 1302 status is not reached by the preparation corpus and is not a separately qualified synthetic SQRT-abort entry. Complete resident scheduling,16KB overlay allocation and either flight application remain separate acceptance work.

Reproduce the original fixtures with `tools/lm_powered_prepare_reference.py` and `tools/cm_powered_prepare_reference.py`; run `python tools/inverse_trig_source.py`, then `make -f tools/inverse_trig.mk all test sanitize check-host`. The freezer `tools/freeze_inverse_trig.py` uses the complete original gzip or committed XZ corpus. C64/VICE targets accept SELECTOR0..7 (default0), MOS_CC, VICE, C64_ROM_DIR and VICE_DATA_DIR. `tools/package_inverse_trig.py` verifies and packages all complete streams and actual target PRGs/maps/mailboxes/stacks.
'''
(r/'docs/INVERSE-TRIG.md').write_text(doc);files.append('docs/INVERSE-TRIG.md');files=sorted(set(files));manifest='evidence/inverse-trig-evidence.sha256';(r/manifest).write_text(''.join(hashlib.sha256((r/p).read_bytes()).hexdigest()+'  '+p+'\n'for p in files));files.append(manifest);(r/'build/inverse-trig-commit-files.txt').write_text('\n'.join(sorted(files))+'\n');print(len(files),'ready files')
