#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Freeze whole byte-original LM reaction-control failure monitor evidence."""
import pathlib,hashlib,gzip,lzma,json,shutil,re
r=pathlib.Path(__file__).resolve().parents[1];e=r/'evidence';stem='lm-rcs-monitor';files=[]
stats=json.loads((r/f'build/{stem}-corpus-stats.json').read_text());audit=json.loads((r/f'build/{stem}-source-audit.json').read_text());mapping=json.loads((r/f'build/{stem}-source-map.json').read_text());digest=stats['decoded_sha256'];size=stats['complete_states']*4106
assert stats['complete_states']==131072 and mapping['emitted_words']==69
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
 keep('build/lm_rcs_monitor-'+suffix,stem+'0-'+suffix)
log=(r/f'build/{stem}0-validation.log').read_text();prg=(r/f'build/{stem}0.prg').read_bytes();load=int.from_bytes(prg[:2],'little');stack=[int(v)for v in re.findall(r'stack extent (\d+) bytes',log)];mail=(r/'build/lm_rcs_monitor-vice-mailbox.bin').read_bytes();assert mail==bytes([0xa5,0,0,192,0])and len(stack)==2
review=e/f'{stem}-independent-review.txt';assert review.exists()
x=dict(stats,original_source_file=mapping['source_file'],original_source_sha256=mapping['original_sha256'],whole_containing_emitted_words=69,source_map=f'evidence/{stem}-source-map.json',source_audit=f'evidence/{stem}-source-audit.json',support_rope_sha256=audit['rope_sha256'],logical_complete_streams={k:{'object':'evidence/'+obj.name,'decoded_sha256':digest}for k in ['original_assembly','strict_native','whole_ASAN_UBSAN']},model_words=2048,external_words=['IRQ','SUPER','channel32'],decoded_bytes=size,actual_C64=dict(complete_states=192,mailbox=mail.hex(),PRG_bytes=len(prg),loaded_end=hex(load+len(prg)-3),hardware_stack=stack[0],software_stack=stack[1],compiler_ZP=0,containing_source_constants=18),scope='All69 byte-original containing source words: actual06,3156 RCSMON/RCSMONIT entry, channel32 highest-changed-bit selection, both valve-mask directions, actual NOVAC admission and all18 containing data words. IRQ 0/canonical ordinary image, coherentEB/BB and valid Executive pool contract. Stops before external RESUME 05270 or BAILOUT1 05716; neither external service nor admitted ACCJOB executes here. Complete current/PVAL low8 combination sweep twice; other contexts varied, not Cartesian. No complete resident application claimed.',independent_review=dict(path='evidence/'+review.name,sha256=hashlib.sha256(review.read_bytes()).hexdigest()))
p=e/f'{stem}-proof.json';p.write_text(json.dumps(x,indent=2)+'\n');files.append('evidence/'+p.name);files.append('evidence/'+review.name)
doc=f'''# Luminary reaction-control failure monitor

Native `lm_rcs_monitor` covers all 69 emitted words of byte-original `RCS_FAILURE_MONITOR.agc`: 51 instructions and 18 containing data words (two inline CADR words and 16 table words). RCSMON/RCSMONIT actually enters bank 06,3156 because SETLOC RCSMONT overrides the nominal BANK23 directive. CH5MASK/CH6MASK/PVALVEST are unbanked E2 words, so all coherent incoming EB values are retained.

The routine accepts an explicit stable inverted channel32 word. It compares its logical low8 bits with PVALVEST, processes only the highest changed bit, sets/clears the corresponding DAP inhibit-mask bits, and updates only that recorded valve bit. Existing inhibit bits receive the source disjoint ADS/complement behavior. No-change keeps source A/L/Q debris. A change admits the literal 1/ACCJOB descriptor 02454/040106 at PRIO27 using the verified Executive NOVAC implementation and the source return helper. Every source erasable exchange, oldcore bank in L, EXECTEM1/2 and return Q 06743 is preserved.

Normal execution stops before external RESUME 05270 with IRQ 0, channel32 and SUPER retained. A full pool stops before actual BAILOUT1 05716 with the source alarm 1202 descriptor still unexecuted and raw bank/register debris preserved. The module executes neither RESUME nor BAILOUT nor the admitted ACCJOB. Caller owns the interrupt and excludes input/mask/Executive producers throughout this basic routine; there is no internal Executive safe point or suspended native frame. The public header admits canonical ordinary image words/raw16-bit A/L, coherent banks, PVALVEST 0..0377 and valid initialized Executive contexts. Priority+0 is excluded because its source CCS skip executes the embedded NO.CORES word as an instruction. First-free-core0/NEWJOB+0 is excluded because SPECTEST calls external CCSHOLE. Both are inconsistent initialized Executive states.

All 131,072 original-machine/strict-native/full-ASAN-UBSAN complete 2048-word-plus-IRQ/SUPER/channel32 states match. Both cohorts exhaust all 65,536 current/PVALVEST low8 combinations. The corpus includes 512 no-change states, 65,280 opening and 65,280 closing states, 118,832 RESUME handoffs and 12,240 resource-exhaustion handoffs. It varies all eight EB/SUPER values, all eight first-free slots and a full pool, active/dormant/idle priority contexts, every ordinary CH5/CH6 mask in one context, raw incoming A/L, high channel32 bits and two distinct nonzero unowned-memory contexts. Every unowned image word is compared. These contexts are not an exhaustive multidimensional Cartesian product. Decoded SHA256 is `{digest}`; one deterministic XZ object serves all three identical logical streams.

One actual C64 PRG passes 192 source-only complete frozen states and all18 containing constants, loaded end${load+len(prg)-3:04x}. Measured hardware/software stacks are {stack[0]}/{stack[1]} bytes and compiler zero-page allocation 0. Expected images use lossless original-source delta fixtures; native computations receive only canonical inputs. Strict host checks reject unsupported controls/IRQ/PVALVEST/core states without mutation and exhaust the constant accessor over all 65,536 physical addresses including null/unsupported-output preservation.

The isolated fixture overlays the byte-original source with zero edits and retains original LM declaration/data rope `{audit['rope_sha256']}`. Original file SHA256 is `{mapping['original_sha256']}`. All 69 emitted words are mapped and every containing data word is checked against the original rope. Supporting source provenance is explicitly inherited from the frozen LM POWERED source-audit; this does not enlarge that packet's scope. Independent read-only review found no defect and is retained here. This is complete containing-file coverage under the declared interrupt/Executive contract; complete interrupt recovery and flight-application residency remain separate requirements.

Reproduce with `python tools/lm_rcs_monitor_prepare_reference.py`, `python tools/lm_rcs_monitor_source.py`, `make -f tools/lm_rcs_monitor.mk all test sanitize`, `python tools/freeze_lm_rcs_monitor.py`, and `make -f tools/lm_rcs_monitor.mk check-host c64 vice-test` with MOS_CC/VICE/C64_ROM_DIR/VICE_DATA_DIR. Packaging retains actual PRG/map/mailbox/stack logs and the logical full-state source/native/SAN streams.
'''
(r/'docs/LUMINARY-RCS-MONITOR.md').write_text(doc);files.append('docs/LUMINARY-RCS-MONITOR.md')
files+=['include/lm_rcs_monitor.h','src/lm_rcs_monitor.c','tests/lm_rcs_monitor_cases.h','tests/lm_rcs_monitor_cases.c','tests/lm_rcs_monitor_vectors.h','tests/test_lm_rcs_monitor.c']
files+=['tools/'+n for n in ['lm_rcs_monitor.mk','lm_rcs_monitor_reference.c','lm_rcs_monitor_snapshot.c','lm_rcs_monitor_inputs.c','lm_rcs_monitor_prepare_reference.py','lm_rcs_monitor_source.py','lm_rcs_monitor_diagnostic.c','lm_rcs_monitor_vice_test.py','freeze_lm_rcs_monitor.py','package_lm_rcs_monitor.py']]
files=sorted(set(files));manifest=f'evidence/{stem}-evidence.sha256';(r/manifest).write_text(''.join(hashlib.sha256((r/p).read_bytes()).hexdigest()+'  '+p+'\n'for p in files));files.append(manifest);(r/f'build/{stem}-commit-files.txt').write_text('\n'.join(sorted(files))+'\n');print(len(files),'ready files',digest)
