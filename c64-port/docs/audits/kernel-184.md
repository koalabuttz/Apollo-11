# Certificate review: Kernel lane PR #184 certificates

Reviewed 2026-10-13 by the certificate reviewer at `port/c64-native-c` 952bec48,
against the 15 pending certificates that Kernel lane PR #184 added.

## Verdicts

| File | Milestones | Verdict |
|---|---|---|
| Luminary099/EXECUTIVE.agc | exec-closure (lm-exec-*) | **accepted** |
| Comanche055/EXECUTIVE.agc | exec-closure (cm-exec-*) | **accepted** |
| Luminary099/INTERRUPT_LEAD_INS.agc | rupt-lead-ins | **accepted** |
| Luminary099/KEYRUPT_UPRUPT.agc | keyrupt-uprupt | **accepted** |
| Comanche055/KEYRUPT_UPRUPT.agc | keyrupt-uprupt | **accepted** |
| Luminary099/INTER-BANK_COMMUNICATION.agc | interbank-oracle | **accepted** |
| Luminary099/WAITLIST.agc, Comanche055/WAITLIST.agc | exec-closure | rejected |
| Comanche055/INTERRUPT_LEAD_INS.agc | rupt-lead-ins | rejected |
| Luminary099/T6-RUPT_PROGRAMS.agc | lm-t6rupt | rejected |
| Comanche055/INTER-BANK_COMMUNICATION.agc | interbank-oracle | rejected |
| Luminary099/SERVICE_ROUTINES.agc, Comanche055/SERVICE_ROUTINES.agc | service-transfer | rejected |
| Luminary099/SYSTEM_TEST_STANDARD_LEAD_INS.agc, Comanche055/SYSTEM_TEST_STANDARD_LEAD_INS.agc | standard-lead-oracle | rejected |

The rejections and what each lane must do are in the reviewer's GitHub issue for
#184. The main one is that the #144 BRUPT check in the RESUME family (WAITLIST
words, native in `rupt_lead_ins.c` and `lm_t6rupt.c`) takes the superbank from
BANKRUPT. After KEYRUPT, T5RUPT or T6RUPT, BANKRUPT holds only BB, so any
interrupt of code in banks 040-043 is refused once a `fixed_data` reader is
present. The resident kernel supplies one; yaAGC resumes.

## What was checked for the accepted files

- **Evidence.** All four evidence categories are distinct records. Each
  independent comparison is a real yaAGC run of the unmodified rope.
  `cert_precheck.py` reports 0 FAIL, and every committed evidence hash matches.
- **Reproduced at 952bec48.**
  - Host and sanitizer targets give 0 failures.
  - Every yaAGC differential is identical to native: exec-* 4864/9216/9208/5120/6144/7648 cases, rupt-lead-ins 3072, keyrupt-uprupt 3072, interbank-oracle 2146.
  - Frozen vectors are unchanged, and the source maps re-derive byte for byte.
  - VICE targets give 0 failures, with check counts equal to the certificates' vice_targets: lm-exec-idle 310, lm-exec-jobs 596, lm-exec-services 581, cm-exec-idle 325, cm-exec-jobs 388, cm-exec-services 487, rupt-lead-ins 193, keyrupt-uprupt 192, interbank-oracle 723.
  - `lane_a_c64_cover.py --check` passes.
  - Independent `gcc -O0 --coverage` runs over the host and C64 case sets find no host-only arc.
  - PRG and proof manifest hashes differ only through shared-source drift.
- **Transfer ABI.** Every reference to these files' labels from the other
  `.agc` files of each mission lands on a registered routine at its original
  address. This includes:
  - the interrupt vectors;
  - KEYRUPT1/UPRUPT and their LODSAMPT returns;
  - KEYCOM (Comanche SXTMARK POSTJUMP);
  - ACCEPTUP's NOVAC;
  - the BANKCALL/IBNKCALL/POSTJUMP/USPRCADR sites (448 LM).
  NEWJOB ≠ 0 paths are compared, not refused.
- **#143.** T3RUPT and QXCH accept any Q. Data-valued Q and QRUPT cases, and a 16-bit Q run made for this review, equal yaAGC.
- **Refusal sites.** The ones cert_precheck lists (`lm_exec_idle.c`, `rupt_lead_ins.c` 31-71, `keyrupt_uprupt.c` 81-90) are NULL, interrupt-state, bank or domain guards. No flight producer reaches them.

## Notes for the lane (not blocking)

- EXECUTIVE: the CM "unreached" text says the CCSHOLE traps on CCS NEWJOB at SPECTEST, but line 181 (+0) is reached and compared. Only line 182 is unreached.
- `agc_resume_exact` returns true when no `fixed_data` reader is present. State this in docs/NATIVE_TRANSFER_ABI.md.
- The ABI doc also says interrupt hardware always saves exactly the word at ZRUPT-1 in BRUPT. After an unextended INDEX, yaAGC saves the indexed instruction instead. Correct the text.
- The LM KEYRUPT entry_contracts says "KEYCOM (Comanche SXTMARK's key path)". No Luminary code references KEYCOM.
- LM INTER-BANK: consider adding two BANKJUMP cases with E 0377 = ±0. No flight site produces them.
