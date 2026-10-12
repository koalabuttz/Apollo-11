# Auditor review: Luminary099/RESTARTS_ROUTINE.agc

Reviewed 2026-10-12 by the port auditor against the certificate drafted by
milestone `lm-restart-select` (PR #9), re-reviewed after PR #40 (8f2c1f00)
fixed issue #29.

Verdict: **accepted**.

## Issue #29, now fixed

PR #40 added 1024 targeted oracle cases. The auditor instrumented yaAGC to
record the successor of every CCS and BZMF in 01,3526-3775 over all 7168
compared cases (old cohort / new cohort):

- LONGCALL time left: CCS LONGTIME (:249) + 185/93, +0 0/32, - 232/107,
  -0 1/408; CCS LONGTIME+1 (:253) + 0/122, +0 0/20, - 1/118, -0 0/180. The
  NOOP marked "CAN'T GET HERE" (:255) is reached by 20 compared cases.
- FINDTIME: CCS (:126) + 674/42, +0 0/42, - 844/84, -0 0/88; CCS (:132)
  + 1453/126, +0 0/22, - 65/64, -0 0/44, with positive and negative overflow.
- CHKNOVAC: BZMF (:157) taken 801/128, not taken 1563/0; priorities +0 and
  -0 each 0/64.
- TIMETEST: CCS at 01,3574 with a +0 direct delta, 0/64.

Outcomes never reached are impossible from the code (CCS after a MASK at
3537/3552/3702; +0 at 3563 since (phase&7)+NEG2 gives -0; -0 at 3652; no
CADRTAB word is 0 or 077777) or excluded by the input domain (3541/3542 need
phase 0, a negative phase or -0, refused at `src/lm_restart_select.c:124`).
gcov on the compared native binary: 85/85 lines; untaken arms are only
input-refusal guards.

## Reproduced (a611930a)

- Host and sanitizer: 7168 selection and 4096 composition cases, 40 table
  entries, 13727 checks, 0 failures.
- yaAGC differential: all 7168 + 4096 cases equal the original program;
  regenerated proof matches the committed one (select 844a3a1a…, compose
  2cc63c69…, rope 1f5326e0…).
- VICE x64sc 3.7.1 PAL: 704 checks, 0 failures.
- All certificate hashes match, for this file and RESTART_TABLES.

## Open notes for the lane (not blocking)

- Add the phase restriction (TEMPPHS 1..01777) to `supported_domain`.
- Cite the real lm-exec-services round trip instead of the recording stubs
  in `tests/lm_restart_routines_cases.c`.
- 600SECS (`src/lm_restart_select.c:101`) is a correct literal rather than a
  fixed-data read.
- The release host log names commit eaf06b09…, which is not on origin, and
  the VICE evidence PRG is 2 bytes smaller than a rebuild; recapture both
  logs from the merged commit.
