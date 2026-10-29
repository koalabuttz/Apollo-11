#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Freeze original-source whole RCS-update numerical/control qualification."""
import pathlib,hashlib,gzip,lzma,json,shutil,re
r=pathlib.Path(__file__).resolve().parents[1];e=r/'evidence';files=[];stats=json.loads((r/'build/cm-rcs-matrix-corpus-stats.json').read_text());audit=json.loads((r/'build/cm-rcs-matrix-source-audit.json').read_text());mapping=json.loads((r/'build/cm-rcs-matrix-source-map.json').read_text());digest=stats['decoded_sha256'];size=stats['complete_states']*4102
for key in ['reference','native','sanitize']:
 with gzip.open(r/f'build/cm-rcs-matrix-{key}.snapshots.gz','rb')as f:
  h=hashlib.sha256();length=0
  while raw:=f.read(1024*1024):h.update(raw);length+=len(raw)
 assert length==size and h.hexdigest()==digest
obj=e/'cm-rcs-matrix-complete.snapshots.xz'
if obj.exists():
 with lzma.open(obj,'rb')as f:assert hashlib.file_digest(f,'sha256').hexdigest()==digest
else:
 with gzip.open(r/'build/cm-rcs-matrix-reference.snapshots.gz','rb')as a,lzma.open(obj,'wb',preset=6)as b:shutil.copyfileobj(a,b,1024*1024)
files.append('evidence/'+obj.name)
def keep(src,dst):shutil.copyfile(r/src,e/dst);files.append('evidence/'+dst)
for suffix in ['validation.log','host-validation.log','frozen-validation.log','preparation.log','source-audit.json','source-map.json','corpus-stats.json']:keep('build/cm-rcs-matrix-'+suffix,'cm-rcs-matrix-'+suffix)
for suffix in ['prg','map','build.log','validation.log','vice-mailbox.bin','hardware-stack.bin','software-stack.bin','vice.log','vice-monitor.log','vice.mon']:
 name='cm-rcs-matrix0'+('.'if suffix in ['prg','map']else'-')+suffix;keep('build/'+name,name)
log=(r/'build/cm-rcs-matrix0-validation.log').read_text();prg=(r/'build/cm-rcs-matrix0.prg').read_bytes();load=int.from_bytes(prg[:2],'little');stack=[int(v)for v in re.findall(r'stack extent (\d+) bytes',log)];mail=(r/'build/cm-rcs-matrix0-vice-mailbox.bin').read_bytes();assert mail==bytes([0xa5,0,0,64,0]);assert len(stack)==2
x=dict(stats,original_source_file=mapping['source_file'],original_source_sha256=mapping['original_sha256'],whole_containing_emitted_words=33,source_map='evidence/cm-rcs-matrix-source-map.json',source_audit='evidence/cm-rcs-matrix-source-audit.json',support_rope_sha256=audit['rope_sha256'],logical_complete_streams={k:{'object':'evidence/'+obj.name,'decoded_sha256':digest}for k in ['original_assembly','strict_native','whole_ASAN_UBSAN']},model_words=2048,external_IRQ_word=1,decoded_bytes=size,actual_C64={'complete_states':64,'mailbox':mail.hex(),'PRG_bytes':len(prg),'loaded_end':hex(load+len(prg)-3),'hardware_stack':stack[0],'software_stack':stack[1],'compiler_ZP':0,'source_constant_QUADANGL':660},scope='All33 original containing-source words mapped: two FLAGWRD6 gates, all source trig calls/offset/products/stores, actual ENDOFJOB transfer and QUADANGLconstant. Stops at external ENDOFJOB05217 before native Executive completion. Original RCS job entrybank22,3447 requires E6 and coherentBB,REGZERO0. No arbitrarywrongbank/producercoherence, completeRCSautopilot or eitherfullflight application claimed.',independent_review_completed=True,independent_review={"path":"evidence/cm-rcs-matrix-independent-review.txt","sha256":hashlib.sha256((r/"evidence/cm-rcs-matrix-independent-review.txt").read_bytes()).hexdigest()})
p=e/'cm-rcs-matrix-proof.json';p.write_text(json.dumps(x,indent=2)+'\n');files.append('evidence/'+p.name)
doc=f'''# Comanche RCS matrix-update source

The byte-original `RCS-CSM_DAP_EXECUTIVE_PROGRAMS.agc` has85 lines and33 emitted words. Native `cm_rcs_matrix` maps all32 executable words plus source QUADANGL660. The source comments describe AMGB/AMBG, but this containing file computes only the listed AMGB elements and CAPSI; no additional AMBG/divide stage is invented.

Actual AMBGUPDT placement is bank22,3447 because SETLOC DAPS8 selects that bank despite the earlier BANK20 directive. The source entry assumes E6 (KMPAC/AMGB scratch), coherent BB and REGZERO0. Native code preserves raw A/L/Q, ordinary MODE, source scratch, every unowned physical image word and external IRQ. The negative-or-zero FLAGWRD6 gate reaches ENDOFJOB immediately; a positive flag without BIT14 reaches it after MASK. Otherwise four original SPSIN2/SPCOS2/SPCOS1/SPSIN1 aliases compute source AMGB1,CAPSI,AMGB8,AMGB4,AMGB5,AMGB7. Original modulo MSU/COM forms CDUX minus7.25degrees with exact raw-zero behavior; polynomial helpers and products retain original coefficients and scale. The final source transfer reaches actual ENDOFJOB05217, where the external native Executive owns job completion.

All131,072 complete original/native/full-ASAN-UBSAN 2048-word-plus-IRQ states match. The corpus exercises every ordinary flag, everyCDUX/CDUZ count in active updates, retained MODE, raw initial A/L overflow patterns, four distinct nonzero unowned-image patterns and both IRQ contexts. Source branch counts are32,770 negative/zero,16,382 non-RCS and81,920 updates. Every unowned source word is compared. Decoded SHA256 is `{digest}`; one deterministic XZ object serves the three byte-identical logical streams.

One actual C64 PRG passes64 complete frozen source states plus original QUADANGLconstant. Loaded end is ${load+len(prg)-3:04x}; measured hardware/software stacks are{stack[0]}/{stack[1]}bytes, compiler-reserved zero-page0. Source-only lossless fixture deltas reconstruct expected images; native computations never consume expectedflight results. Invalid banks/PC/REGZERO reject unchanged. The constant accessor rejects unsupported reads unchanged.

The fixture overlays byte-original containing RCS and original MYSUBS with zero edits and retains byte-original CM declaration/data rope `{audit['rope_sha256']}`. Original containing-file SHA256 is `{mapping['original_sha256']}`. Source alias declarations independently confirm SPSIN1=SPSIN2=SPSIN and SPCOS1=SPCOS2=SPCOS. The containing-source word map and alias source audit are retained; the unchanged original CM polynomial/TCQ native helpers are qualified again in this complete caller composition. No old pinned-domain packet is broadened by this new proof.

There is no Executive check within the basic job. Its canonical image and shared scratch/input producers remain exclusively owned until actual ENDOFJOB. An independent read-only review by /root/general_division found no defect in the complete original containing-source arithmetic/control mapping; the retained review supports whole-file inventory certification. It does not certify a complete resident RCS autopilot or either complete Apollo application.

Reproduce with `python tools/cm_rcs_matrix_prepare_reference.py`, `python tools/cm_rcs_matrix_source.py`, and `make -f tools/cm_rcs_matrix.mk all test sanitize check-host`. Regenerate source-only target fixtures with `python tools/freeze_cm_rcs_matrix.py`; C64/VICE targets use MOS_CC, VICE, C64_ROM_DIR and VICE_DATA_DIR. `tools/package_cm_rcs_matrix.py` verifies and packages all streams and actual target PRG/map/mailbox/stack logs. The native code has no opcode decoder or host floating point.
'''
(r/'docs/COMANCHE-RCS-MATRIX.md').write_text(doc);files.extend(['docs/COMANCHE-RCS-MATRIX.md','evidence/cm-rcs-matrix-independent-review.txt'])
files.extend(['include/cm_rcs_matrix.h','src/cm_rcs_matrix.c','tests/cm_rcs_matrix_cases.h','tests/cm_rcs_matrix_cases.c','tests/cm_rcs_matrix_vectors.h','tests/test_cm_rcs_matrix.c','tools/cm_rcs_matrix.mk','tools/cm_rcs_matrix_reference.c','tools/cm_rcs_matrix_snapshot.c','tools/cm_rcs_matrix_inputs.c','tools/cm_rcs_matrix_prepare_reference.py','tools/cm_rcs_matrix_source.py','tools/cm_rcs_matrix_diagnostic.c','tools/cm_rcs_matrix_vice_test.py','tools/freeze_cm_rcs_matrix.py','tools/package_cm_rcs_matrix.py']);files=sorted(set(files));manifest='evidence/cm-rcs-matrix-evidence.sha256';(r/manifest).write_text(''.join(hashlib.sha256((r/p).read_bytes()).hexdigest()+'  '+p+'\n'for p in files));files.append(manifest);(r/'build/cm-rcs-matrix-commit-files.txt').write_text('\n'.join(sorted(files))+'\n');print(len(files),'ready files',digest)
