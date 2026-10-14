# Certificate review: Luminary099/CONIC_SUBROUTINES.agc

Reviewed 2026-10-14 by the certificate reviewer, against milestone
`lm-conic-compiled` (Certificate fixes lane, from Lane B). Reviewed at
f353f541, after #476.

Earlier rounds, all in issue #165:
- **ec35425b (after #329):** the MUTABLE,1 loads refused flight X1 = −10D.
- **e8f85222 (after #455):** the X1 fix held, but the KEPLOOP
  iteration-limit exit was never taken.

Verdict: **accepted**.

## What was checked

- **X1 at the MUTABLE,1 loads.** Flight's SELECTMU and INTEXIT set X1 to −2
  (Earth) or −10D (Moon).
  - KX10006 (src/lm_conic_compiled.c:51) equals the rope at 04,2006-2023,
    which is MUTABLE+2..+15.
  - All three loads run with both X1 values and are compared with yaAGC:
    :600 (62 Earth / 50 Moon cases), :1143 (415 / 468) and :1364 (84 / 92).
  - GETX's three callers are at PL 0.
- **KEPLOOP iteration-limit exit** (CONIC :770-772, 12,2267
  `BRNCHCTR … BHIZ KEPCONVG`; native :1648-1650). It is taken in 50 host
  cases (ITERCTR = 1) and by a C64 row, and it is compared with yaAGC.
- **The oracle reproduces.** 29,120 cases equal yaAGC. The refreeze is clean,
  and the proof, source map and log are byte-identical.
- **Host and sanitizer:** 228 checks, 0 failures.
- **VICE:** 516 checks, 0 failures, equal to `vice_targets`. The PRG hashes
  match.
- **Per-object cover.** An independent `gcov -b` run shows that the C64 rows
  take every host arc. The only exceptions are two harness lines
  (interp_case.c:110 and :124) that the C64 build leaves out.
- **Ledger checks.** `cert_precheck.py` reports 0 FAIL, and `cert_selfcheck`
  passes.
- **Rule #414** does not apply: the file has no BOVB, TCDANZIG or ENDT.

## Notes for the lane (not blocking)

- The spec says that flight only sets ITERCTR to 1 or 2 at BRNCHCTR. That is
  too narrow: flight resumes there with ITERCTR 1-20. Cases that start before
  BRNCHCTR still reach it with arbitrary ITERCTR, so no comparison depends on
  the claim. Correct the comment at the next refreeze.
- Eight DDV/BDDV sites never see a zero divisor. All of them are degenerate
  geometry (radius magnitudes and constants).
