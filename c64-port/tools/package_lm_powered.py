#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Freeze all original LM POWERED groups and actual target diagnostics."""
import pathlib,hashlib,gzip,lzma,json,shutil,re
r=pathlib.Path(__file__).resolve().parents[1];e=r/'evidence'
source_sha='379505fe1429c8b0d6ef20d85bad9d8fb991083fd7202dc89c46e584f0ba92b1'
rope_sha='1f5326e038de5b741b2f27b01ec949dbd688cf1906994e997402587c8628f40e'
packets=[
('lm_powered_trig',10,65536,655360,4,'CDUTRIGS and CD*TR*GS','The canonical native basic trig loop computes all three CDU sine/cosine pairs, including source two-complement conversion and preservation of incoming MPAC. Original LM SINE/COSINE and CDULOGIC scratch/control addresses are retained. All 32768 CDU words occur for both live-CDU and stored-angle entry. Internal polynomial and basic-call returns retain the same owner; pending second operations do not create Executive checks.'),
('lm_powered_axis',2,65536,131072,4,'AX*SR*T','The source three-plane axis transformation accepts the documented +3/-3 directions, arbitrary raw vector/trig words and ordinary MODE. LM TERM1TMP aliases MPAC+3/+4 rather than the CM BUF2 pair. The source saved-return TC is separately executed. DIM preserves sign and reaches negative zero from either direction, so the positive-zero TC RTNSAVER alternate cannot execute under this entry contract.'),
('lm_powered_quick',2,32768,65536,3,'QUICTRIG and TC A','Every 15-bit CDU word is qualified through original SPSIN/SPCOS with LM TEMK01076/SQ01077 aliases and the source INHINT/RELINT effects. The final TC A is a separate compiled source return. Canonical ordinary MODE is retained. Every basic saved-Q address02000..07777 is exercised. Both C64 diagnostics also read and compare all six original POWERED constants.'),
('lm_powered_interface',8,65536,868352,4,'Eight permanent interfaces','Selectors0..7 implement CDUTRIG, CD*TR*G, CDU*SMNB, TRG*SMNB, CDU*NBSM, TRG*NBSM, *SMNB*, and *NBSM*. Explicit EXIT, basic call, source axis, INTPRET reentry, VLOAD/pendingRVQ and fixed RVQ stages preserve all canonical words and bank controls. All ordinary saved FCADR banks are accepted. Basic internal returns retain ownership, and VLOAD returns EDOP0161: an Executive handoff there would be an invented source check.'),
('lm_powered_fixed_return',4,32768,131072,3,'All-bank fixed RVQ closure','All32768 ordinary saved FCADR words are qualified at four original origins: CDUTRIG, CD*TR*G, transformed VLOAD/pendingRVQ, and CALCSMSC. RVQ enters GOTO+4 and skips erasable classification, so even fixed banks0/1 are valid. The source return restores EB from BANKSET, normalizes raw L, derives FB/BB/BANKSET/LOC/INTBIT15 and POLISH, and reaches original NEWOPS06070. CALCSMSC uses bank10/INTBIT15=0. Erasable indirect return is outside this fixed-entry contract.'),
('lm_fleshpot',4,16384,65536,3,'FLESHPOT matrix construction','The native basic source constructs the body-to-stable-member matrix from arbitrary raw source trig pairs. Four explicit phases cover first row, other row, the containing-source six-product VXV cross computation, and final source EXIT/doubling/stores through pre-SWRETURN. Ordinary destination ECADRs0400..03756 must retain an eighteen-word within-bank range and exclude the stable trig inputs. All destination banks and incoming EB contexts are exercised, including actual XNB E5,1664. The VXV proof covers this complete source composition, not a general cross-product API. Unrounded products, raw overflow/BUF/OVFIND effects and source A/BANKSET exchanges are preserved.'),
('lm_fleshpot_interface',8,8192,65536,3,'CALCSMSC and FLESHPOT-1 return composition','The original bank10 interpreted CALCSMSC source executes EXIT, actual BANKCALL and FLESHPOT-1 default ECADR selection, constructs XNB, executes actual SWRETURN using BUF2, reenters INTPRET and performs all-bank fixed RVQ. The canonical source state exposes eight explicit stage checkpoints, including the body and pending EXIT stages. Saved fixed basic addresses02000..07777 and coherent bank words are the source return domain. FLESHPOT-1 is also a standalone default-entry API. No numerical stage return invents a scheduling check. The standalone bank-return API is a FLESHPOT_FINISH-output continuation: its A/L registers must already be normalized as that original body produces. Arbitrary raw-register overflow inputs at this return boundary are not part of the qualification.')]
allfiles=[];proofs=[]
def keep(src,dst,files):
 shutil.copyfile(r/src,e/dst);files.append('evidence/'+dst)
for stem,phases,calls,states,header,title,scope in packets:
 short=stem.replace('_','-');files=[];digest=None
 for key in ['reference','native','sanitize']:
  with gzip.open(r/f'build/{short}-{key}.snapshots.gz','rb')as f:
   h=hashlib.sha256();size=0
   while raw:=f.read(1024*1024):h.update(raw);size+=len(raw)
  assert size==states*(2048+header)*2,(short,key,size)
  if digest is None:digest=h.hexdigest()
  assert h.hexdigest()==digest,(short,key)
 obj=e/f'{short}-complete.snapshots.xz'
 if obj.exists():
  with lzma.open(obj,'rb')as f:assert hashlib.file_digest(f,'sha256').hexdigest()==digest
 else:
  with gzip.open(r/f'build/{short}-reference.snapshots.gz','rb')as a,lzma.open(obj,'wb',preset=6)as b:shutil.copyfileobj(a,b,1024*1024)
 files.append('evidence/'+obj.name)
 for suffix in ['validation.log','host-validation.log','frozen-validation.log']:
  keep(f'build/{short}-{suffix}',f'{short}-{suffix}',files)
 trace=r/f'build/{short}-reference.trace'
 if trace.exists():keep(str(trace.relative_to(r)),f'{short}-reference.trace',files)
 targets=[]
 for n in range(phases):
  pre=f'{short}{n}'
  for suffix in ['prg','map','build.log','validation.log','vice-mailbox.bin','hardware-stack.bin','software-stack.bin','vice.log','vice-monitor.log','vice.mon']:
   name=pre+('.'if suffix in ['prg','map']else'-')+suffix;keep('build/'+name,name,files)
  log=(r/f'build/{pre}-validation.log').read_text();prg=(r/f'build/{pre}.prg').read_bytes();load=int.from_bytes(prg[:2],'little');stack=[int(x)for x in re.findall(r'stack extent (\d+) bytes',log)];mail=(r/f'build/{pre}-vice-mailbox.bin').read_bytes()
  assert mail==bytes([0xa5,0,0,32,n]);assert len(stack)==2
  targets.append({'native_phase':n,'complete_states':32,'mailbox':mail.hex(),'prg_bytes':len(prg),'loaded_end_hex':hex(load+len(prg)-3),'hardware_stack_bytes':stack[0],'software_stack_bytes':stack[1],'compiler_ZP_bytes':0})
 proof={'source_file':'Luminary099/POWERED_FLIGHT_SUBROUTINES.agc','source_file_sha256':source_sha,'source_audit_dependency':'evidence/lm-powered-source-audit.json','rope_sha256':rope_sha,'native_group':title,'calls':calls,'complete_states':states,'model_words':2048,'external_IRQ_word':1,'header_words':header,'decoded_state_bytes':size,'whole_strict_and_sanitizer_streams_match':True,'logical_complete_streams':{k:{'object':'evidence/'+obj.name,'decoded_sha256':digest}for k in ['original_assembly','strict_native','whole_ASAN_UBSAN']},'target_images':targets,'scope':scope}
 path=e/f'{short}-proof.json';path.write_text(json.dumps(proof,indent=2)+'\n');files.append('evidence/'+path.name);proofs.append(str(path.relative_to(r)))
 docname=short.upper()+'.md'
 doc=f'''# Luminary {title}

{scope}

The fixture overlays byte-original LM POWERED, INTERPRETER, RTB_OP_CODES and SINGLE_PRECISION sources with zero edits. It preserves original declaration/data rope `{rope_sha}`. The containing file SHA256 is `{source_sha}`.

All {states:,} complete 2048-word-plus-IRQ checkpoints from {calls:,} inputs match original assembly, strict native and whole ASAN/UBSAN execution. Every unowned physical image word is compared. Decoded SHA256 is `{digest}`. One lossless XZ object represents the three identical logical streams. Source-only frozen fixtures reconstruct all expected target images; expected flight values never enter the native numerical routines.

{phases} actual C64 PRGs pass {phases*32} complete frozen states. Maximum loaded end is ${max(int(t['loaded_end_hex'],16)for t in targets):04x}; measured hardware/software stack maxima are {max(t['hardware_stack_bytes']for t in targets)}/{max(t['software_stack_bytes']for t in targets)} bytes, with compiler-reserved zero-page0. These are standalone native diagnostics. Resident16KB overlay allocation, application Executive composition and sensor/actuator integration remain separate acceptance work.

Reproduce after `python tools/lm_powered_prepare_reference.py` and the original hardware-DV oracle prerequisites with `make -f tools/{stem}.mk test sanitize check-host`. `python tools/freeze_{stem}.py` reconstructs the frozen fixtures. C64/VICE targets accept SELECTOR0..{phases-1} (default0), MOS_CC, VICE, C64_ROM_DIR and VICE_DATA_DIR. Native code follows explicitly compiled source operations; it does not decode AGC opcodes or use host floating point. External control retains producer exclusion and canonical scratch ownership across internal stages.
'''
 (r/'docs'/docname).write_text(doc);files.append('docs/'+docname)
 for f in [f'include/{stem}.h',f'src/{stem}.c',f'tests/{stem}_cases.h',f'tests/{stem}_cases.c',f'tests/{stem}_vectors.h',f'tests/test_{stem}.c',f'tools/{stem}_reference.c',f'tools/{stem}_snapshot.c',f'tools/{stem}_inputs.c',f'tools/{stem}.mk',f'tools/{stem}_diagnostic.c',f'tools/{stem}_vice_test.py',f'tools/freeze_{stem}.py']:
  assert (r/f).exists(),f;files.append(f)
 if stem=='lm_powered_interface':files.append('tests/lm_powered_interface_apply.h')
 if stem=='lm_powered_quick':files.extend(['include/lm_powered_constants.h','src/lm_powered_constants.c','tests/test_lm_powered_constants.c'])
 files=sorted(set(files));manifest=f'evidence/{short}-evidence.sha256';(r/manifest).write_text(''.join(hashlib.sha256((r/p).read_bytes()).hexdigest()+'  '+p+'\n'for p in files));files.append(manifest);allfiles.extend(files);(r/f'build/{short}-commit-files.txt').write_text('\n'.join(sorted(files))+'\n');print(short,len(files),'files',digest,flush=True)
keep('build/lm-powered-source-audit.json','lm-powered-source-audit.json',allfiles)
keep('build/lm-powered-preparation.log','lm-powered-preparation.log',allfiles)
keep('build/lm-powered-file-source-map.json','lm-powered-file-source-map.json',allfiles)
allfiles.extend(['tools/lm_powered_prepare_reference.py','tools/lm_powered_file_review.py','tools/package_lm_powered.py'])
(r/'build/lm-powered-packet-files.txt').write_text('\n'.join(sorted(set(allfiles)))+'\n')
(r/'build/lm-powered-proof-list.json').write_text(json.dumps(proofs,indent=2)+'\n')
