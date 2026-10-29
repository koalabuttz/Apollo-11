#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Freeze selected RTB source groups; no whole-file certificate."""
import pathlib,gzip,lzma,json,hashlib,re,shutil
r=pathlib.Path(__file__).resolve().parents[1];e=r/'evidence';stem='rtb-angles';files=[];stats=json.loads((r/f'build/{stem}-corpus-stats.json').read_text());audit=json.loads((r/f'build/{stem}-source-audit.json').read_text());mapping=json.loads((r/f'build/{stem}-source-map.json').read_text());targets={};streams={}
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
 prg=(e/f'{target}.prg').read_bytes();load=int.from_bytes(prg[:2],'little');log=(e/f'{target}-validation.log').read_text();stack=[int(v)for v in re.findall(r'stack extent (\d+) bytes',log)];mail=(e/f'{target}-vice-mailbox.bin').read_bytes();assert mail==bytes([0xa5,0,0,32,mission*16+entry])and len(stack)==2
 targets[name]=dict(complete_states=32,mailbox=mail.hex(),PRG_bytes=len(prg),loaded_end=hex(load+len(prg)-3),hardware_stack=stack[0],software_stack=stack[1],compiler_ZP=0)
for suffix in ['validation.log','host-validation.log','frozen-validation.log','preparation.log','source-audit.json','source-map.json','corpus-stats.json']:
 path=stem+'-'+suffix;shutil.copyfile(r/'build'/path,e/path);files.append('evidence/'+path)
review=e/'rtb-angles-independent-review.txt';assert review.exists();files.append('evidence/'+review.name)
proof=dict(complete_states=sum(x['complete_states']for x in stats.values()),model_words=2048,external_words=['IRQ','SUPER'],containing_file_status='partial',qualified_containing_words={m:x['qualified_emitted_words']for m,x in mapping.items()},source_audit='evidence/rtb-angles-source-audit.json',source_map='evidence/rtb-angles-source-map.json',logical_complete_streams=streams,actual_C64=targets,source_constants={'Comanche055/LOCTHETA':{'physical_fixed_word':19349,'raw_octal':'01155'}},independent_review=dict(path='evidence/'+review.name,sha256=hashlib.sha256(review.read_bytes()).hexdigest()),scope='Native1TO2SUB/1STO2S/V1STO2S/2V1STO2S/CDULOGIC/CDUINC/LOADTIME bothmissions; CM INCRCDUS/READPIPS. Preserves raw DDOUBL/CCS overflow and allsource scratch/control exchanges. Ordinary canonical image/rawA/L, fixed direct Q and ordinary resolved CDUINC targets. Actual basic returns/DANZIG entry; no interpretation of next instruction. Every indicated scalar word swept in varied contexts, not Cartesian. NORMUNIT/SIGNMPAC/SGNAGREE/transposes/VECSGNAG/PULSEIMU/CMQUALITY remain outsidepacket. No complete containing file orresidentapplication claimed.')
p=e/f'{stem}-proof.json';p.write_text(json.dumps(proof,indent=2)+'\n');files.append('evidence/'+p.name)
doc='''# RTB angle, time and PIPA source groups

`rtb_angles` implements 1TO2SUB, 1STO2S, V1STO2S, 2V1STO2S, CDULOGIC, CDUINC and LOADTIME for both missions, plus Comanche INCRCDUS and READPIPS. The original SETLOC makes the source bank10 in Luminary and bank22 in Comanche. This packet maps72 of130 LM containing words and97 of206 CM words. Remaining entries are explicitly inventoried outside this packet; neither RTB/RT8 file is certified complete here.

The native integer computations retain raw DDOUBL low carry, the accumulator overflow sign used by CCS A, two’s-complement wrap through source LIMITS, DXCH minor-word exchanges and every vector staging word. CDUINC writes TEM2 before reading the ordinary resolved target, including BUF/TEM2/MPAC scratch aliases. Its final target is resolved again exactly as source. The direct fixed-return entries complete TC Q with PC equal to incoming Q and Q equal3, without executing that caller instruction. Other entries stop before the mission’s actual DANZIG entry, preserving pending LOC/EDOP/banks and source MODE changes. SOURCE LOADTIME reads TIME2/TIME1; READPIPS reads the three retained PIPA counters under INHINT then enables IRQ through the literal RELINT. The interrupt-control instructions also normalize L. Source input and canonical scratch producers remain caller-excluded; no native private continuation or instruction decoder exists.

All1,048,576 complete original-assembly/strict-native/full-ASAN-UBSAN states match, including all2048 canonical words and external IRQ/SUPER. Sixteen mission/entry contexts each contain65,536 cases. The retained upper cohort sweeps every ordinary high/minor input word for the converters, normalized increment A, vector components and TIME/PIPA counters in varied contexts. A4096-case corner cohort covers signed zeros, raw overflow signs, extrema and mixed-sign words. Direct return Q spans all3072 fixed addresses; all eight coherent EB/SUPER settings, ordinary MODE words and two distinct nonzero unowned-memory patterns are varied. CDUINC target addresses and aliased targets are varied. These are sampled joint contexts, not a Cartesian exhaustion of all multiword operands. One deterministic XZ object per identical logical stream serves original/native/SAN names; no decoded raw corpora are retained.

Sixteen actual C64 PRGs pass512 original-only complete delta-fixture states. Each retains actual PRG/map/mailbox and measured hardware/software stacks, loaded extents below$C000 and compiler zero-page allocation0. Strict host checks also reject invalid entry controls without mutation and exhaust the one qualified containing constant LOCTHETA at every65536 physical address, including unsupported/null output behavior. Only that CM constant is claimed by the accessor; remaining containing constants belong to later groups.

Both isolated fixtures overlay byte-original containing source with zero edits and retain original declaration/data ropes LM1f5326e038de5b741b2f27b01ec949dbd688cf1906994e997402587c8628f40e and CM6e3150ae9b43535237e10f565205d1f65e6d60cafbe8935be598e03e9810b061. Comanche’s original catalog filename RT8_OP_CODES.agc is assembled through MAIN’s RTB_OP_CODES.agc include; the preparation audit identifies that exact include and verifies every assembled byte equals the original source. Every containing word is independently checked against the unchanged original rope. Supporting source provenance remains disclosed through the frozen POWERED source audits. Independent read-only native/source review found no defect within the explicit nonnull/canonical/ordinary-target contract.

Reproduce with `python tools/rtb_angles_prepare_reference.py`, `python tools/rtb_angles_source.py`, `make -f tools/rtb_angles.mk all test sanitize`, `python tools/freeze_rtb_angles.py`, and `make -f tools/rtb_angles.mk check-host`. Target `c64 vice-test` additionally takes RTB_MISSION=0/1, RTB_ENTRY=0..8 (LM6/8 absent), MOS_CC/VICE/C64_ROM_DIR/VICE_DATA_DIR. Packaging retains all16 actual targets and exact source/native/SAN logical streams.
'''
for a,b in [('maps72','maps 72'),('of130','of 130'),('and97','and 97'),('of206','of 206'),('bank10','bank 10'),('bank22','bank 22'),('equal3','equal 3'),('All1,048,576','All 1,048,576'),('all2048','all 2048'),('contain65,536','contain 65,536'),('A4096-case','A 4096-case'),('all3072','all 3072'),('pass512','pass 512'),('all16','all 16'),('below$C000','below $C000'),('allocation0','allocation 0'),('every65536','all 65536')]:doc=doc.replace(a,b)
(r/'docs/RTB-ANGLES.md').write_text(doc);files.append('docs/RTB-ANGLES.md')
files+=['include/rtb_angles.h','src/rtb_angles.c','tests/rtb_angles_cases.h','tests/rtb_angles_cases.c','tests/rtb_angles_vectors.h','tests/test_rtb_angles.c']
files+=['tools/'+n for n in ['rtb_angles.mk','rtb_angles_reference.c','rtb_angles_snapshot.c','rtb_angles_inputs.c','rtb_angles_prepare_reference.py','rtb_angles_source.py','rtb_angles_diagnostic.c','rtb_angles_vice_test.py','freeze_rtb_angles.py','package_rtb_angles.py']]
files=sorted(set(files));manifest='evidence/rtb-angles-evidence.sha256';(r/manifest).write_text(''.join(hashlib.sha256((r/p).read_bytes()).hexdigest()+'  '+p+'\n'for p in files));files.append(manifest);(r/'build/rtb-angles-commit-files.txt').write_text('\n'.join(sorted(files))+'\n');print(len(files),'readyfiles',proof['complete_states'])
