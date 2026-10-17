# Second review: LM and CM INTERPRETER after #517

Second certificate reviewer, 2026-10-17. The Interpreter lane's #517 (merged as c07d6de1) refroze `lm-interpreter` and `cm-interpreter`. It keeps every label segment the basic subroutines flow through, adds the indexed TC targets as entries, and adds the matching states. **Verdict: both certificates accepted and re-cited.** The previous record is interpreter-367.md.

## Mechanical checks

- `ledgers.py --validate` passes. `evidence.py fetch` passes for both specs, and all 12 evidence sha256 values match port-coverage.json. `cert_precheck.py` reports 0 FAIL; its warnings are the ones accepted in #367.
- The differential logs show LM 28,148 cases over 55 routines and CM 25,592 over 50, all equal. Closure is LM 769/772 and CM 658/659. VICE runs LM 105 and CM 101 targets, matching `vice_targets`.
- The reviewer rebuilt the host tests, plain and under ASan/UBSan: LM 1777 and CM 1611 checks, 0 failures.

## Domain

Each entry checks only `basic_sane()` at run time, so every pin has to admit every value flight can hold. All of them do.

- **OVERFLOW (A ±1, L in {0,1,3,5}).** Every caller arrives after a DAS or TS overflow: SETOVF, ENDDOT+6, TAD's TCF SETOVF and VSSL (LM lines 808, 991, 885, 1598). So A is ±1 and L is +0, or 3/5 from OVERFLWY/OVERFLWZ. No caller produces L=1, but admitting it does no harm.
- **MAXDVSW (EQUALS MATINC) in {0,-1}.** Every path into MAXTEST writes it first: DDV/BDDV, MPAC+, /MPAC+ and UNIT LARGE2. A positive value or -0 would execute BIASHI as an instruction.
- **MAXTEST-1..-14 and -21.** These are exactly the indexed targets in the source:
  - DVNORMCT gives -1..-14.
  - UNIT gives -1..-14 and -21 (`CS SRDDV`, DEC 20).
  - The C entries sit at 00,2540 and 00,2547-2564.
- **SQRTSUB** admits any MPAC. Its only callers are UNIT and SQRT.
- **SQRTSUB's CAF POSMAX (LM 00,3452 / CM 00,3451) is unreached.** The reviewer ran ARGHI through DAS MPAC for all 2^27 inputs from 10000 00000 to 17777 37777, which covers both the SMPAC+ and NORMTEST paths. No case overflows; the largest sum is 2^28-1. Mixed signs only shrink the dividend, and ARGLO roots stay below .71. The two DPAGREE words (LM 03,3255-3256) remain justified as in #367.

## Code

- **POODOO paths are faithful.** On LM, `DXCH LOC; TC POODOO1` leaves Q=03404 and Z=05726. On CM, `TC POODOO` leaves Q=03403 and Z=05721.
- **Translation spot checks are faithful.** The reviewer checked:
  - BUFPOS/BUFNEG, with the impossible CCS paths kept literally.
  - The DDV front end: BUFZERO, BUF+, MPAC+, and the TPAGREE and FINALDV+3 returns.
  - OVERFLOW's INDEX A / CS LIMITS read from rope, the INDEX L ADS steps and the TS skips.
  - SQRTSUB's ARGHI sequence.

## Notes (non-blocking)

1. docs/INTERPRETER-FILE.md says LM refuses 13 cases, but the differential log says 12. The CM figure of 8 is correct.
2. The closure's `unreached` reason in tools/mk/{lm,cm}-interpreter.mk argues from the largest argument. It could cite the exhaustive result instead.
3. Shared-source drift, NOTE only (#450): the rope path change in tests/*_interpreter_target.c, and the stops array in interp_routine_reference.c growing from 16 to 64.
