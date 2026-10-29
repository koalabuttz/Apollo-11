#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Retain complete source proof, target artifacts, and exact identity ledger."""
import hashlib,json,pathlib,shutil,subprocess
R=pathlib.Path(__file__).resolve().parents[1]
OWNED=['include/lm_interbank.h','src/lm_interbank.c','tests/lm_interbank_cases.h','tests/lm_interbank_cases.c','tests/lm_interbank_inline_data.h','tests/lm_interbank_target.h','tests/lm_interbank_target.c','tests/lm_interbank_vectors.h','tests/test_lm_interbank.c','platform/c64/lm_interbank_diagnostic.c','tools/lm_interbank_compare.py','tools/lm_interbank_verify_frozen.py','tools/lm_interbank_dump.h','tools/lm_interbank_input_dump.c','tools/lm_interbank_snapshot.c','tools/lm_interbank_reference.c','tools/lm_interbank_source.py','tools/lm_interbank_host_test.py','tools/freeze_lm_interbank.py','tools/lm_interbank_vice_test.py','tools/lm_interbank_port.mk','tools/package_lm_interbank.py','docs/LUMINARY-INTERBANK.md']
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def descriptor(path):return dict(path=path,sha256=sha(R/path))
def main():
 pairs={
 'lm-interbank-native-v2.snapshots.bin.xz':'lm-interbank-source-base.snapshots.bin.xz',
 'lm-interbank-native-v3.snapshots.bin.xz':'lm-interbank-source-extra.snapshots.bin.xz',
 'lm-interbank-native-v2.log':'lm-interbank-source-base.log',
 'lm-interbank-extra.log':'lm-interbank-source-extra.log',
 'lm-interbank-source-audit.json':'lm-interbank-source-audit.json',
 'lm-interbank-native-v3-frozen-results.json':'lm-interbank-strict-results.json',
 'lm-interbank-sanitize-frozen-results.json':'lm-interbank-sanitize-results.json',
 'lm-interbank-strict-final.log':'lm-interbank-strict.log',
 'lm-interbank-sanitize.log':'lm-interbank-sanitize.log',
 'lm-interbank-tests.log':'lm-interbank-tests.log',
 'lm-interbank-target-source.snapshots.bin.xz':'lm-interbank-target-source.snapshots.bin.xz',
 'lm-interbank-target-results.json':'lm-interbank-target-results.json',
 'lm-interbank-vice-results.json':'lm-interbank-vice-results.json',
 'lm-interbank-c64.log':'lm-interbank-c64.log',
 'lm-interbank.prg':'lm-interbank.prg','lm-interbank.map':'lm-interbank.map',
 'lm-interbank-mailbox.bin':'lm-interbank-mailbox.bin','lm-interbank-monitor.log':'lm-interbank-monitor.log',
 'lm-interbank-vice.log':'lm-interbank-vice.log','lm-interbank-vice.mon':'lm-interbank-vice.mon'}
 files=OWNED.copy()
 for old,new in pairs.items():
  path='evidence/'+new;shutil.copyfile(R/'build'/old,R/path);files.append(path)
 review='evidence/lm-interbank-independent-review.txt';assert (R/review).is_file();files.append(review)
 source=json.loads((R/'evidence/lm-interbank-source-audit.json').read_text());strict=json.loads((R/'evidence/lm-interbank-strict-results.json').read_text());san=json.loads((R/'evidence/lm-interbank-sanitize-results.json').read_text());target=json.loads((R/'evidence/lm-interbank-vice-results.json').read_text())
 assert strict['states']==san['states']==921088 and strict['decoded_sha256']==san['decoded_sha256']=='e8b88d7c729a249635a0a997a88e82944be89624c249886b4be58c96bad3cd4c'
 assert target['states']==448 and target['mailbox']=='a5c00100'
 dependencies=['include/agc_word.h','src/agc_word.c','include/lm_fixed_compat.h','src/lm_fixed_compat.c','src/lm_fixed_compat_words.inc','tests/frozen_words.h','tests/frozen_words.c','tools/frozen_words.py','evidence/passive-storage-proof.json','evidence/passive-storage-SHA256.json','evidence/standard-lead-source-audit.json']
 proof=dict(source_file=source['source_path'],source_sha256=source['source_sha256'],source_edits=0,original_rope_sha256=source['rope_sha256'],emitted_words=76,complete_source_line_map='evidence/lm-interbank-source-audit.json',public_source_entries=14,source_states=921088,all_raw_A_L_per_entry=True,context_cross_products='sampled; fixed and erasable callers, allEB/FB/SUPER/IRQ variants represented',negative_overflow_bankjump_excluded=False,negative_overflow_fault_frontier=dict(pc=2,raw_Q=0o177776,next_word_execution='outside file, explicitly not decoded'),aggregate_decoded_sha256=strict['decoded_sha256'],source_corpora=[descriptor('evidence/lm-interbank-source-base.snapshots.bin.xz'),descriptor('evidence/lm-interbank-source-extra.snapshots.bin.xz')],full_sanitizer_states=san['states'],target=target,expected_state_comparison='every2048 physical word plusSUPER/IRQ exactly; lossless sparse constructor/source delta retains untouched words',PRIO12=dict(physical_fixed_word=0o4644,word=0o12000,all_accessor_arguments=65536),runtime_domain='ordinary canonicalRAM15, rawA/L16, ZERO0, coherentbanks, ordinarycompiled Q0100..07775/savedTC0100..07777, ordinaryinline physicalERA>=0100, scratch/operand/interruptproducersquiesced',external_continuations='actual compiled callees/callers; source interpreted-code handoff; invalid registerQ fault frontier',instruction_decoder=False,runtime_AGC_emulator=False,integrated_application_complete=False,dependencies=[descriptor(p) for p in dependencies])
 proofpath='evidence/lm-interbank-proof.json';(R/proofpath).write_text(json.dumps(proof,indent=2)+'\n');files.append(proofpath)
 revision=subprocess.check_output(['git','rev-parse','HEAD'],cwd=R,text=True).strip()
 certificate=dict(source_file=source['source_path'],source_sha256=source['source_sha256'],source_revision=revision,reviewed_all_source_paths=True,public_entries=['BANKCALL','SWCALL','SWRETURN','POSTJUMP','BANKJUMP','Q+10000','PRIO12','MAKECADR','SUPDACAL','IBNKCALL','ISWCALLL','ISWRETRN','USPRCADR','SUPERSW'],internal_paths=['ordinary/+overflow/-overflowSWCALL','all4 fixedCCS1777/MAKECADRsuccessors onBANKJUMP negativeoverflow','inline physical aliases afterstores','SUPDACAL rawoverflowbank/SUPERrestore','USPRCADR EDOPediting/TSoverflow','actualsavedTCreturn'],caller_domain=proof['runtime_domain'],external_continuations=['caller-owned compiled sourcecallee/caller','interpretedcoding nativecontinuation','PC2/Q-1 invalidregisterfaultfrontier beforeexternalwordfetch'],emitted_words=76,evidence=dict(source_to_native_map=descriptor('docs/LUMINARY-INTERBANK.md'),independent_assembly_comparison=descriptor('evidence/lm-interbank-source-audit.json'),host_and_sanitizer_results=descriptor(proofpath),c64_execution_results=descriptor('evidence/lm-interbank-vice-results.json'),independent_review=descriptor(review)),integrated_application_complete=False)
 certpath='evidence/lm-interbank-coverage.json';(R/certpath).write_text(json.dumps(certificate,indent=2)+'\n');files.append(certpath)
 for path in files:assert (R/path).stat().st_size<100*1024*1024,path
 ledger=[dict(path=p,sha256=sha(R/p),bytes=(R/p).stat().st_size) for p in sorted(files)]
 manifest='evidence/lm-interbank-SHA256.json';(R/manifest).write_text(json.dumps(ledger,indent=2)+'\n');files.append(manifest)
 (R/'build/lm-interbank-commit-files.txt').write_text('\n'.join(sorted(files))+'\n')
 print('READY',len(files),'owned files; ledger',sha(R/manifest),'proof',sha(R/proofpath),'certificate',sha(R/certpath))
if __name__=='__main__':main()
