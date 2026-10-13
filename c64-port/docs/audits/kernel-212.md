# Certificate review: Kernel lane PR #212 (fixes for #198)

Reviewed 2026-10-13 by the certificate reviewer at `port/c64-native-c` 26637cbe,
against certificates the first review (docs/audits/kernel-184.md, issue #198)
sent back.

## Verdicts

| File | Milestones | Verdict |
|---|---|---|
| Comanche055/INTERRUPT_LEAD_INS.agc | rupt-lead-ins | **accepted** |
| Luminary099/T6-RUPT_PROGRAMS.agc | lm-t6rupt | **accepted** |
| Comanche055/INTER-BANK_COMMUNICATION.agc | interbank-oracle | **accepted** |
| Comanche055/SERVICE_ROUTINES.agc | service-transfer | **accepted** |
| Luminary099/SYSTEM_TEST_STANDARD_LEAD_INS.agc | standard-lead-oracle | **accepted** |
| Comanche055/SYSTEM_TEST_STANDARD_LEAD_INS.agc | standard-lead-oracle, cm-lead-tests | **accepted** |
| Luminary099/SERVICE_ROUTINES.agc | service-transfer | rejected: the certificate and docs/SERVICE-TRANSFER.md:25 say 171 site words, but the table holds 175, and the two AOSTASK `TC GENTRAN +1` callers are not described |
| Luminary099/WAITLIST.agc, Comanche055/WAITLIST.agc | exec-closure | held until the Kernel lane's TASKOVER change merges |

## What was fixed and checked

- **RESUME superbank (#198 item 1).**
  - `cm_resume` (`rupt_lead_ins.c:70`) and the LM resume (`lm_t6rupt.c:94`) now use the current superbank.
  - New rope-reader cases interrupt code in banks 030-033 under superbank 0060 and 0100, with BANKRUPT = BB. They are compared with yaAGC.
  - The only refused cases are those with a BRUPT that differs from the word at ZRUPT-1.
  - The reviewer's case (ZRUPT 02101, BANKRUPT 060003, superbank 0100) now resumes in both missions. A sweep of 8232 rope states gives no false refusal.
- **lm-t6rupt source map:** current, and all maps regenerate byte-identical.
- **CM SUPDACAL in banks 040-043 (item 2):** CM data addresses in those banks are compared on host and C64 rows.
- **GENTRAN +1 and DEBIT (items 3-4):**
  - AOSTASK's two `TC GENTRAN +1` sites are compiled site operands and are compared.
  - DEBIT with no site operand now returns EXEC_OUT_OF_SCOPE instead of reading the rope.
- **Entry counts (item 5):** 512 entry cases per mission. The CM text names BAILOUT.
- **E/JOBWAK job-found path (item 6):** BANKSET now has the JOBSLP1 form, so the path is taken and compared on host and C64. The 31 CM cases still refused have the calling job asleep, which no flight state produces.
- **vice-cm-lead-tests** is in the CM vice_targets.

## Reproduced

- Host, sanitizer and yaAGC differential runs give 0 failures. Proofs equal the evidence apart from allowed shared dependency-hash drift.
- VICE checks equal the vice_targets: interbank 722, service 569, standard-lead 477, cm-lead-tests 390, rupt-lead-ins 195, lm-t6rupt 244, keyrupt-uprupt 192.
- An independent gcov run per mission object finds that each milestone's C64 rows take every host arc.
- The files accepted in #199 (LM INTERRUPT_LEAD_INS, LM and CM KEYRUPT_UPRUPT, LM INTER-BANK) still reproduce at this tip.

## Notes (not blocking)

- docs/NATIVE_TRANSFER_ABI.md still says interrupt hardware always saves exactly the word at ZRUPT-1. After an unextended INDEX, it saves the indexed instruction.
- The lm-t6rupt cases use Q only in 0-07777 at DOT6RUPT and RESUME. The reviewer's 16-bit-Q run equals yaAGC. Widening it would put that in the evidence.
