# Certificate review: Comanche055 and Luminary099 PHASE_TABLE_MAINTENANCE.agc

The certificate reviewer reviewed these on 2026-10-12 at `port/c64-native-c` 9b1bdb76, against the certificates drafted by milestones `cm-phase-table` and `lm-phase-table` (PR #62).

Verdict: **accepted** for both files.

## What was checked

- **Evidence.** The certificates have four distinct records: the milestone doc, the yaAGC proof, the host log and the VICE log. All hashes match.
- **Transfer ABI.**
  - The fixed-fixed entries are registered routines: NEWMODEX/A, MMDSPLAY, CHECKMM, 2PHSCHNG, PHASCHNG and PHSCHNGA.
  - The bank-10 targets are registered at their original addresses: SETUPDSP (BANKJUMP CADR), PHSCHNG2 (DTCB 2CADR) and CON2 (DTCB GENADR), plus SETUPDSP's NOVAC return.
  - Every outgoing transfer ends in the trampoline. The phase operands of every genuine caller are compiled site tables, and the NOVAC 2CADR for DSPMMJOB is among them.
- **Host log.** The cm-phase-table host log was regenerated after commit cd3b2c07. That commit replaced vacuous step-limited refusal checks with single calls, which makes the checks stricter. The log reproduces.

## Reproduced

| Mission | Differential | Host and sanitizer (`test-`, `sanitize-`) | VICE (`vice-`) |
|---|---|---|---|
| CM | 4,096 cases; proof and sites JSON rebuild byte for byte | 4,673 checks, 0 failures | 256 checks, 0 failures; PRG hash equals the log |
| LM | 4,096 cases; proof and sites JSON rebuild byte for byte | 4,586 checks, 0 failures | 256 checks, 0 failures; PRG hash equals the log |

gcov reaches 137/138 lines of `src/cm_phase_table.c` and 142/143 lines of `src/lm_phase_table.c`. The untaken arms are refusals, with one exception, described below.

## Notes for the lanes

- **The certificate wording does not match the code.** The certificates and the docs say a TEMPSW of TB+LB (060000) is refused. The code at `src/cm_phase_table.c:157` and `src/lm_phase_table.c:167` instead follows the source on `CCS` -0 into BELOW3 and sets LONGBASE. Nothing compares that path with yaAGC.
  - The path is unreachable. No compiled operand and no V37RET value sets both bits, so this does not block the certificate.
  - Fix one side or the other:
    - make the code refuse (`if(c==3u)return EXEC_OUT_OF_SCOPE;`), or
    - reword the certificate and docs to "executed, uncompared, no producer".
