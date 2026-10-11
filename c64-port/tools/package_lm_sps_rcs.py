#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Freeze the byte-original Luminary backup controller whole-file proof."""
import pathlib,hashlib,gzip,lzma,json,shutil,re
r=pathlib.Path(__file__).resolve().parents[1];e=r/'evidence';stem='lm-sps-rcs';files=[]
stats=json.loads((r/f'build/{stem}-corpus-stats.json').read_text());audit=json.loads((r/f'build/{stem}-source-audit.json').read_text());mapping=json.loads((r/f'build/{stem}-source-map.json').read_text());digest=stats['decoded_sha256'];size=stats['complete_states']*4102
assert stats['complete_states']==196608 and mapping['emitted_words']==82
for role in ('reference','native','sanitize'):
 with gzip.open(r/f'build/{stem}-{role}.snapshots.gz','rb')as f:
  h=hashlib.sha256();length=0
  while raw:=f.read(1048576):h.update(raw);length+=len(raw)
 assert length==size and h.hexdigest()==digest
obj=e/f'{stem}-complete.snapshots.xz'
if obj.exists():
 with lzma.open(obj,'rb')as f:assert hashlib.file_digest(f,'sha256').hexdigest()==digest
else:
 with gzip.open(r/f'build/{stem}-reference.snapshots.gz','rb')as a,lzma.open(obj,'wb',preset=6)as b:shutil.copyfileobj(a,b,1048576)
files.append('evidence/'+obj.name)
def keep(src,dst):shutil.copyfile(r/src,e/dst);files.append('evidence/'+dst)
for suffix in ['validation.log','host-validation.log','frozen-validation.log','preparation.log','source-audit.json','source-map.json','corpus-stats.json']:
 keep(f'build/{stem}-{suffix}',f'{stem}-{suffix}')
for suffix in ['prg','map','build.log','validation.log']:
 name=stem+'0'+('.'if suffix in ('prg','map')else'-')+suffix;keep('build/'+name,name)
for suffix in ['vice-mailbox.bin','hardware-stack.bin','software-stack.bin','vice.log','vice-monitor.log','vice.mon']:
 keep('build/lm_sps_rcs-'+suffix,stem+'0-'+suffix)
log=(r/f'build/{stem}0-validation.log').read_text();prg=(r/f'build/{stem}0.prg').read_bytes();load=int.from_bytes(prg[:2],'little');stack=[int(v)for v in re.findall(r'stack extent (\d+) bytes',log)];mail=(r/'build/lm_sps_rcs-vice-mailbox.bin').read_bytes();assert mail==bytes([0xa5,0,0,96,0])and len(stack)==2
review=e/f'{stem}-independent-review.txt';assert review.exists()
x=dict(stats,original_source_file=mapping['source_file'],original_source_sha256=mapping['original_sha256'],whole_containing_emitted_words=82,source_map=f'evidence/{stem}-source-map.json',source_audit=f'evidence/{stem}-source-audit.json',support_rope_sha256=audit['rope_sha256'],logical_complete_streams={k:{'object':'evidence/'+obj.name,'decoded_sha256':digest}for k in ['original_assembly','strict_native','whole_ASAN_UBSAN']},model_words=2048,external_IRQ_word=1,decoded_bytes=size,actual_C64=dict(complete_states=96,mailbox=mail.hex(),PRG_bytes=len(prg),loaded_end=hex(load+len(prg)-3),hardware_stack=stack[0],software_stack=stack[1],compiler_ZP=0,containing_source_constants=4),scope='All82 byte-original source words: SPSRCS public entry and complete source jet-sense, deadband, outer-rate, coast and inhibition paths plus4 constants. Fixed basic Q02000..07777, E6/coherentbanks, REGZERO0, axis-1/bothzeros/+1. Completes actual TCQ transfer with Q3. IRQ retained externally; no Executive check. Every raw ordinary TJ and supported axis is represented; sampled multidimensional combinations are not exhaustive Cartesian coverage. No complete resident docked DAP or complete flight application claimed.',independent_review=dict(path='evidence/'+review.name,sha256=hashlib.sha256(review.read_bytes()).hexdigest()))
p=e/f'{stem}-proof.json';p.write_text(json.dumps(x,indent=2)+'\n');files.append('evidence/'+p.name);files.append('evidence/'+review.name)
doc=f'''# Luminary SPS backup RCS controller

Native `lm_sps_rcs` covers all82 emitted words of byte-original `SPS_BACK-UP_RCS_CONTROL.agc`, including its4 containing constants. Public entry SPSRCS is bank21,3727/E6. All internal labels are composed as source branches; original external callers invoke only SPSRCS. The coherent bank/REGZERO/fixed-return-Q contract is explicit in the public header. The operation preserves external IRQ and completes TCQ with PC equal to incoming Q and Q equal3; it does not run the instruction at the returned PC.

The controller selects P/U/V through AXISCTR=-1/0/+1, accepting either raw zero for U. It computes source jet sense and deadband, outer-rate limit, coast-zone MP/BZF/BZMF, source HALF jet commands, and inhibition/reversal counters. BZMF inspects raw16-bit A before correction. The literal CTRCHECK instructions inhibit negative raw counters as well as positive counters, despite the comments describing the usual nonnegative counter domain. Raw negative-zero TJ is exercised and treated exactly as source even though the documented caller excludes it. All scratch/A/L/TCQ exchanges are retained; no decoded instruction loop or host floating point is used.

All196,608 original-machine/strict-native/full-ASAN-UBSAN complete2048-word-plus-IRQ states match. Each axis includes all32768 raw ordinary TJ values and EDOT values in a varied context, plus a4096-case boundary cohort for signed zeros, HALF, extrema, deadband and rate thresholds. Both U-axis zero encodings, drift flags, source inhibition counters, E/DKDB variations, raw incoming A/L, MODE, coherent fixed returns and distinct nonzero unowned-memory patterns are included. The comparison retains every unowned word. This proves the stated sampled contexts, not every Cartesian combination of all operands. Decoded SHA256 is `{digest}`; one deterministic XZ object serves all three identical logical streams.

One actual C64 PRG passes96 complete frozen source states plus all4 containing constants, loaded end${load+len(prg)-3:04x}. Measured hardware/software stacks are{stack[0]}/{stack[1]}bytes and compiler zero-page allocation0. Source-only lossless delta fixtures reconstruct expected full images; native computations consume only ordinary input images. Strict host checks also reject unsupported banks/PC/axis/Q without mutation and validate the constant accessor over all65536 addresses.

The isolated fixture overlays byte-original containing source with zero edits and retains original LM declaration/data rope `{audit['rope_sha256']}`. Original file SHA256 is `{mapping['original_sha256']}`. All82 emitted words and both external fixed aliases RATELIM1=CALLCODE00032 and RATEDB1=TBUILDFX00045 are mapped and checked against the original rope. Containing constants RATELIM2/UTIME are available only at their exact physical source addresses. The source input/scratch producers remain excluded throughout this basic routine; no Executive checks or private persistent state are invented.

Independent read-only review is retained with the packet. This is complete coverage of this containing source file under its declared public contract, not a complete docked autopilot or either complete Apollo application.

Reproduce with `python tools/lm_sps_rcs_prepare_reference.py`, `python tools/lm_sps_rcs_source.py`, `make -f tools/lm_sps_rcs.mk all test sanitize`, `python tools/freeze_lm_sps_rcs.py`, and `make -f tools/lm_sps_rcs.mk check-host c64 vice-test` with MOS_CC/VICE/C64_ROM_DIR/VICE_DATA_DIR. `tools/package_lm_sps_rcs.py` retains source/native/SAN streams and actual target PRG/map/mailbox/stack logs.
'''
(r/'docs/LUMINARY-SPS-RCS.md').write_text(doc);files.append('docs/LUMINARY-SPS-RCS.md')
files+=['include/lm_sps_rcs.h','src/lm_sps_rcs.c','tests/lm_sps_rcs_cases.h','tests/lm_sps_rcs_cases.c','tests/lm_sps_rcs_vectors.h','tests/test_lm_sps_rcs.c']
files+=['tools/'+n for n in ['lm_sps_rcs.mk','lm_sps_rcs_reference.c','lm_sps_rcs_snapshot.c','lm_sps_rcs_inputs.c','lm_sps_rcs_prepare_reference.py','lm_sps_rcs_source.py','lm_sps_rcs_diagnostic.c','lm_sps_rcs_vice_test.py','freeze_lm_sps_rcs.py','package_lm_sps_rcs.py']]
files=sorted(set(files));manifest=f'evidence/{stem}-evidence.sha256';(r/manifest).write_text(''.join(hashlib.sha256((r/p).read_bytes()).hexdigest()+'  '+p+'\n'for p in files));files.append(manifest);(r/f'build/{stem}-commit-files.txt').write_text('\n'.join(sorted(files))+'\n');print(len(files),'ready files',digest)
