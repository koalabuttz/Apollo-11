#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Retain complete source proof, target artifacts, and exact identity ledger."""
import hashlib,json,pathlib,shutil,subprocess
R=pathlib.Path(__file__).resolve().parents[1]
OWNED=['include/cm_interbank_complete.h','src/cm_interbank_complete.c','tests/cm_interbank_complete_cases.h','tests/cm_interbank_complete_cases.c','tests/cm_interbank_complete_inline_data.h','tests/cm_interbank_complete_target.h','tests/cm_interbank_complete_target.c','tests/cm_interbank_complete_vectors.h','tests/test_cm_interbank_complete.c','platform/c64/cm_interbank_complete_diagnostic.c','tools/cm_interbank_complete_compare.py','tools/cm_interbank_complete_verify_frozen.py','tools/cm_interbank_complete_dump.h','tools/cm_interbank_complete_input_dump.c','tools/cm_interbank_complete_snapshot.c','tools/cm_interbank_complete_reference.c','tools/cm_interbank_complete_source.py','tools/cm_interbank_complete_host_test.py','tools/freeze_cm_interbank_complete.py','tools/cm_interbank_complete_vice_test.py','tools/cm_interbank_complete_port.mk','tools/package_cm_interbank_complete.py','docs/COMANCHE-INTERBANK.md']
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def descriptor(path):return dict(path=path,sha256=sha(R/path))
def main():
 pairs={
 'cm-interbank-complete-native-v2.snapshots.bin.xz':'cm-interbank-complete-source-base.snapshots.bin.xz',
 'cm-interbank-complete-native-v3.snapshots.bin.xz':'cm-interbank-complete-source-extra.snapshots.bin.xz',
 'cm-interbank-complete-native-v2.log':'cm-interbank-complete-source-base.log',
 'cm-interbank-complete-extra.log':'cm-interbank-complete-source-extra.log',
 'cm-interbank-complete-final-source.log':'cm-interbank-complete-final-source.log',
 'cm-interbank-complete-native-v4-results.json':'cm-interbank-complete-final-source-results.json',
 'cm-interbank-complete-source-audit.json':'cm-interbank-complete-source-audit.json',
 'cm-interbank-complete-native-v3-frozen-results.json':'cm-interbank-complete-strict-results.json',
 'cm-interbank-complete-sanitize-frozen-results.json':'cm-interbank-complete-sanitize-results.json',
 'cm-interbank-complete-strict-final.log':'cm-interbank-complete-strict.log',
 'cm-interbank-complete-sanitize.log':'cm-interbank-complete-sanitize.log',
 'cm-interbank-complete-tests.log':'cm-interbank-complete-tests.log',
 'cm-interbank-complete-target-source.snapshots.bin.xz':'cm-interbank-complete-target-source.snapshots.bin.xz',
 'cm-interbank-complete-target-results.json':'cm-interbank-complete-target-results.json',
 'cm-interbank-complete-vice-results.json':'cm-interbank-complete-vice-results.json',
 'cm-interbank-complete-c64.log':'cm-interbank-complete-c64.log',
 'cm-interbank-complete.prg':'cm-interbank-complete.prg','cm-interbank-complete.map':'cm-interbank-complete.map',
 'cm-interbank-complete-mailbox.bin':'cm-interbank-complete-mailbox.bin','cm-interbank-complete-monitor.log':'cm-interbank-complete-monitor.log',
 'cm-interbank-complete-vice.log':'cm-interbank-complete-vice.log','cm-interbank-complete-vice.mon':'cm-interbank-complete-vice.mon'}
 files=OWNED.copy()
 for old,new in pairs.items():
  path='evidence/'+new;shutil.copyfile(R/'build'/old,R/path);files.append(path)
 review='evidence/cm-interbank-complete-independent-review.txt';assert (R/review).is_file();files.append(review)
 source=json.loads((R/'evidence/cm-interbank-complete-source-audit.json').read_text());strict=json.loads((R/'evidence/cm-interbank-complete-strict-results.json').read_text());san=json.loads((R/'evidence/cm-interbank-complete-sanitize-results.json').read_text());target=json.loads((R/'evidence/cm-interbank-complete-vice-results.json').read_text())
 assert strict['states']==san['states']==921088 and strict['decoded_sha256']==san['decoded_sha256']=='ea316c318b96d980e7d95dc875e1955ad78c7c5bd93174d01ac49bd21beb26ad'
 final_source=json.loads((R/'evidence/cm-interbank-complete-final-source-results.json').read_text());assert final_source['states']==921088 and final_source['decoded_sha256']==strict['decoded_sha256']
 assert target['states']==448 and target['mailbox']=='a5c00100'
 dependencies=['include/agc_word.h','src/agc_word.c','include/cm_fixed_compat.h','src/cm_fixed_compat.c','src/cm_fixed_compat_words.inc','tests/frozen_words.h','tests/frozen_words.c','tools/frozen_words.py','evidence/passive-storage-proof.json','evidence/passive-storage-SHA256.json','evidence/standard-lead-source-audit.json']
 proof=dict(source_file=source['source_path'],source_sha256=source['source_sha256'],source_edits=0,original_rope_sha256=source['rope_sha256'],emitted_words=76,complete_source_line_map='evidence/cm-interbank-complete-source-audit.json',public_source_entries=14,source_states=921088,all_raw_A_L_per_entry=True,context_cross_products='sampled; fixed and erasable callers, allEB/FB/SUPER/IRQ variants represented',negative_overflow_bankjump_excluded=False,negative_overflow_fault_frontier=dict(pc=2,raw_Q=0o177776,next_word_execution='outside file, explicitly not decoded'),aggregate_decoded_sha256=strict['decoded_sha256'],source_corpora=[descriptor('evidence/cm-interbank-complete-source-base.snapshots.bin.xz'),descriptor('evidence/cm-interbank-complete-source-extra.snapshots.bin.xz')],full_sanitizer_states=san['states'],fresh_final_source_replay=descriptor('evidence/cm-interbank-complete-final-source-results.json'),preserved_unowned_cm_io_fields=11,target=target,expected_state_comparison='every2048 physical word plusSUPER/IRQ exactly; lossless sparse constructor/source delta retains untouched words',PRIO12=dict(physical_fixed_word=0o4710,word=0o12000,all_accessor_arguments=65536),runtime_domain='ordinary canonicalRAM15, rawA/L16, ZERO0, coherentbanks, ordinarycompiled Q0100..07775/savedTC0100..07777, ordinaryinline physicalERA>=0100, scratch/operand/interruptproducersquiesced',external_continuations='actual compiled callees/callers; source interpreted-code handoff; invalid registerQ fault frontier',instruction_decoder=False,runtime_AGC_emulator=False,integrated_application_complete=False,dependencies=[descriptor(p) for p in dependencies])
 proofpath='evidence/cm-interbank-complete-proof.json';(R/proofpath).write_text(json.dumps(proof,indent=2)+'\n');files.append(proofpath)
 revision=subprocess.check_output(['git','rev-parse','HEAD'],cwd=R,text=True).strip()
 certificate=dict(source_file=source['source_path'],source_sha256=source['source_sha256'],source_revision=revision,reviewed_all_source_paths=True,public_entries=['BANKCALL','SWCALL','SWRETURN','POSTJUMP','BANKJUMP','Q+10000','PRIO12','MAKECADR','SUPDACAL','IBNKCALL','ISWCALL','ISWRETRN','USPRCADR','SUPERSW'],internal_paths=['ordinary/+overflow/-overflowSWCALL','all4 fixedCCS1777/MAKECADRsuccessors onBANKJUMP negativeoverflow','inline physical aliases afterstores','SUPDACAL rawoverflowbank/SUPERrestore','USPRCADR EDOPediting/TSoverflow','actualsavedTCreturn'],caller_domain=proof['runtime_domain'],external_continuations=['caller-owned compiled sourcecallee/caller','interpretedcoding nativecontinuation','PC2/Q-1 invalidregisterfaultfrontier beforeexternalwordfetch'],emitted_words=76,evidence=dict(source_to_native_map=descriptor('docs/COMANCHE-INTERBANK.md'),independent_assembly_comparison=descriptor('evidence/cm-interbank-complete-source-audit.json'),host_and_sanitizer_results=descriptor(proofpath),c64_execution_results=descriptor('evidence/cm-interbank-complete-vice-results.json'),independent_review=descriptor(review)),integrated_application_complete=False)
 certpath='evidence/cm-interbank-complete-coverage.json';(R/certpath).write_text(json.dumps(certificate,indent=2)+'\n');files.append(certpath)
 for path in files:assert (R/path).stat().st_size<100*1024*1024,path
 ledger=[dict(path=p,sha256=sha(R/p),bytes=(R/p).stat().st_size) for p in sorted(files)]
 manifest='evidence/cm-interbank-complete-SHA256.json';(R/manifest).write_text(json.dumps(ledger,indent=2)+'\n');files.append(manifest)
 (R/'build/cm-interbank-complete-commit-files.txt').write_text('\n'.join(sorted(files))+'\n')
 print('READY',len(files),'owned files; ledger',sha(R/manifest),'proof',sha(R/proofpath),'certificate',sha(R/certpath))
if __name__=='__main__':main()
