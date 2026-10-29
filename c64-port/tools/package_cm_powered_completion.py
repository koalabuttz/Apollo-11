#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Freeze source/native/full-sanitizer identities and actual C64 completion packets."""
import pathlib,hashlib,gzip,lzma,json,shutil,re
r=pathlib.Path(__file__).resolve().parents[1];e=r/'evidence'
packets=[
 ('cm_smcdure',18,8192,147456,'SMCDURES', 'COMANCHE-SMCDURES.md',
  'Explicit source DLOAD/DMP, PDDL, BDSU, DDV, STORE, SL1, STODL and RVQ phases compute all three DCDU increments from DTHETASM and half-amplitude CDU sine/cosine pairs. The 18 native phase returns preserve source scratch, registers, MODE, stack pushes/pops, stores and controls. Every returned DANZIG with EDOP0 is a genuine Executive check; pending second operations retain canonical ownership. Producer exclusion spans each original operation, and no numerical return itself invents a scheduler yield. Final RVQ accepts all ordinary fixed FCADR banks and derives INTBIT15/A from the saved return. The new domain extension requalifies lower fixed-bank returns without changing the frozen permanent-interface packet.'),
 ('cm_nbsm',6,8192,188416,'legacy NBSM', 'COMANCHE-NBSM.md',
  'The legacy NBSM source saves QPRET in X2, loads the S1-indexed six-word CDU angle vector, stores it into CDUSPOT, loads VAC32D, calls the qualified TRG*NBSM composition and finally performs source STCALL32D/X2. Source angle arrays cover sixteen physical addresses across all erasable-bank contexts, including actual P51 MARKDOWN/MARK2DWN addresses. The saved indirect fixed FCADR must be >=004000 because source GOTO classifies lower words as erasable indirection. The input includes the actual P51 return036052 and P20 return047126. INTBIT15/A are computed for both lower and upper fixed banks. The source interpreter genuine EDOP0 checks in the leading stages are exposed; pending VLOAD and internal CALL/NEWOPS remain under the same canonical owner.'),
 ('cm_powered_quick',2,32768,65536,'QUICTRIG and TC A', 'COMANCHE-POWERED-QUICK.md',
  'The existing immutable QUICTRIG integer polynomial implementation is qualified anew against the byte-original declaration rope. A separate generic cm_powered_quick_return executes the actual final TC A at23,3605, setting PC to saved A and Q1 before caller code executes. Source INHINT/RELINT, raw register and scratch effects are included. Every 15-bit input count, ordinary MODE, all eight EB contexts and all fixed basic saved Q02000..07777 occur. There is no Executive check inside the basic routine. This generic return is separate from the qualified CLOCKJOB SWRETURN/BUF2 composition. The five source POWERED constants are also checked on both target images.'),
 ('cm_powered_fixed_return',4,32768,131072,'fixed RVQ return closure', 'COMANCHE-POWERED-FIXED-RETURN.md',
  'A separate extension completes actual RVQ from CDUTRIG, CD*TR*G, transformed VLOAD/pendingRVQ and SMCDURES return boundaries. It qualifies every one of32768 ordinary saved FCADR words at each of four source origins, including fixed banks0/1: unlike indirect GOTO, RVQ explicitly enters GOTO+4 and skips erasable classification. It restores EB from BANKSET, records POLISH, derives FB/BB/BANKSET/LOC/INTBIT15, normalizes the raw L register exactly as source dispatch does, and returns at NEWOPS06037. The older immutable permanent-interface proof remains scoped to its >=040000 saved-return domain. Pending RVQ retains canonical ownership; the completed SMCDURES EDOP0 boundary permits the external source Executive check before this call.')]
allfiles=[];proofs=[]
def keep(src,dst,files):shutil.copyfile(r/src,e/dst);files.append('evidence/'+dst)
for stem,count,calls,states,title,docname,scope in packets:
 short=stem.replace('_','-');files=[];digest=None
 for key in ['reference','native','sanitize']:
  with gzip.open(r/f'build/{short}-{key}.snapshots.gz','rb')as f:
   h=hashlib.sha256();size=0
   while raw:=f.read(1024*1024):h.update(raw);size+=len(raw)
  assert size==states*4102,(short,key,size)
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
 if stem=='cm_powered_quick':
  keep('build/cm-powered-quick-sanitize-validation.log','cm-powered-quick-sanitize-validation.log',files)
 target=[]
 for n in range(count):
  pre=f'{short}{n}'
  for suf in ['prg','map','build.log','validation.log','vice-mailbox.bin','hardware-stack.bin','software-stack.bin','vice.log','vice-monitor.log','vice.mon']:
   name=pre+('.'if suf in ['prg','map']else'-')+suf;keep('build/'+name,name,files)
  log=(r/f'build/{pre}-validation.log').read_text();prg=(r/f'build/{pre}.prg').read_bytes();load=int.from_bytes(prg[:2],'little');stack=[int(x)for x in re.findall(r'stack extent (\d+) bytes',log)];mail=(r/f'build/{pre}-vice-mailbox.bin').read_bytes()
  assert mail==bytes([0xa5,0,0,32,n]);assert len(stack)==2
  target.append({'native_phase':n,'complete_states':32,'mailbox':mail.hex(),'prg_bytes':len(prg),'loaded_end_hex':hex(load+len(prg)-3),'hardware_stack_bytes':stack[0],'software_stack_bytes':stack[1],'compiler_ZP_bytes':0})
 proof={'source_file':'Comanche055/POWERED_FLIGHT_SUBROUTINES.agc','source_file_sha256':'dbf2f3db7c28220733b86b30b994cfccf29f4cfce0c4e59084c13bdbeccc5766','source_audit_dependency':{'path':'evidence/cm-powered-trig-source-audit.json','sha256':hashlib.sha256((e/'cm-powered-trig-source-audit.json').read_bytes()).hexdigest()},'rope_sha256':'6e3150ae9b43535237e10f565205d1f65e6d60cafbe8935be598e03e9810b061','native_group':title,'calls':calls,'complete_states':states,'model_words':2048,'external_IRQ_word':1,'decoded_state_bytes':size,'whole_strict_and_sanitizer_streams_match':True,'logical_complete_streams':{k:{'object':'evidence/'+obj.name,'decoded_sha256':digest}for k in ['original_assembly','strict_native','whole_ASAN_UBSAN']},'target_images':target,'scope':scope}
 path=e/f'{short}-proof.json';path.write_text(json.dumps(proof,indent=2)+'\n');files.append('evidence/'+path.name);proofs.append(str(path.relative_to(r)))
 doc=f'''# Comanche {title}

{scope}

The independent fixture uses byte-original CM declarations/data rope `6e3150ae9b43535237e10f565205d1f65e6d60cafbe8935be598e03e9810b061`, including original ALTVAR01357 and R53FLBIT=BIT8. The source audit discloses only indentation repairs needed by yaYUL; all containing-source assembled words retain the original rope identity. Earlier packets on a different support rope remain unchanged.

All {states:,} complete 2048-word-plus-IRQ checkpoints from {calls:,} inputs match original assembly, strict native and whole ASAN/UBSAN execution. Decoded SHA256 is `{digest}`. One lossless XZ object represents the three identical logical streams; every unowned image word is compared. Source-only frozen fixtures reconstruct the complete expected images. Invalid bank/PC/REGZERO entries reject unchanged in host tests.

{count} actual C64 PRGs pass {count*32} complete frozen states. Maximum loaded end is ${max(int(t['loaded_end_hex'],16)for t in target):04x}; measured hardware/software stack maxima are {max(t['hardware_stack_bytes']for t in target)}/{max(t['software_stack_bytes']for t in target)} bytes, with compiler-reserved zero-page0. These diagnostics prove native target execution and retained memory. They do not certify a complete resident flight application.

Reproduce after `python tools/cm_powered_prepare_reference.py` and the original hardware-DV oracle prerequisites with `make -f tools/{stem}.mk test sanitize check-host`. `python tools/freeze_{stem}.py` accepts build gzip or the committed XZ source corpus. C64/VICE targets accept SELECTOR0..{count-1} (default0), MOS_CC, VICE, C64_ROM_DIR and VICE_DATA_DIR. Native functions execute explicitly compiled source operations without reading an AGC opcode, host floating point or persistent private continuation state. External control preserves producer exclusion and implements only original source scheduling points.
'''
 (r/'docs'/docname).write_text(doc);files.append('docs/'+docname)
 for f in [f'include/{stem}.h',f'src/{stem}.c',f'tests/{stem}_cases.h',f'tests/{stem}_cases.c',f'tests/{stem}_vectors.h',f'tests/test_{stem}.c',f'tools/{stem}_reference.c',f'tools/{stem}_snapshot.c',f'tools/{stem}_inputs.c',f'tools/{stem}.mk',f'tools/{stem}_diagnostic.c',f'tools/{stem}_vice_test.py',f'tools/freeze_{stem}.py']:
  if stem=='cm_powered_quick' and f in ['include/cm_powered_quick.h','src/cm_powered_quick.c']:f=f.replace('cm_powered_quick.','cm_powered_quick_return.')
  assert (r/f).exists(),f;files.append(f)
 files=sorted(set(files));manifest=f'evidence/{short}-evidence.sha256';(r/manifest).write_text(''.join(hashlib.sha256((r/p).read_bytes()).hexdigest()+'  '+p+'\n'for p in files));files.append(manifest);allfiles.extend(files);(r/f'build/{short}-commit-files.txt').write_text('\n'.join(sorted(files))+'\n');print(short,len(files),'files',digest,flush=True)
(r/'build/cm-powered-completion-packet-files.txt').write_text('\n'.join(sorted(set(allfiles)))+'\n')
(r/'build/cm-powered-completion-proof-list.json').write_text(json.dumps(proofs,indent=2)+'\n')
