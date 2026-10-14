# Certificate review: cm-p37-p70 after the #414 refreeze (#465)

Reviewed 2026-10-14 by the second certificate reviewer, at 68b7b019 (#465).
Lane E refroze Comanche055/P37_P70 so GAMDV45's BOVB TCDANZIG runs natively
(steward rule #414). This review replaces `lane-e-372.md` as the file's
independent review. Verdict: **re-confirmed**.

## What was checked

| Check | Result |
|---|---|
| `evidence.py fetch`, cited sha256 | all match (source map, proof, host log, VICE log, cover) |
| `cert_precheck.py` | 0 FAIL; the VICE manifest WARN (proxy) checked by hand; 635 refusal sites (+1, the new tcdanzig guard) |
| `ledgers.py --validate` | PASS |
| `cert_selfcheck.py cm-p37-p70` | OK: 304,640 of 304,640 cases equal yaAGC over 595 routines (case set 16d6ec0a = proof), resume_flow 409/409, 1,882 rows re-run on yaAGC, host 2,450 = C64 2,450 branch outcomes |
| Strict `--cover-check` | verified, cases match, 0 missing; 3 truncated rows (1 rebuilt, 2 resumed by digest) |
| Host and sanitizer | 1,190 checks each, 0 failures |
| VICE 3.7.1 | 51 of 51 PRGs byte-identical and PASS, 692 checks = `vice_targets` = proof rows = header rows = cover rows |

## Rule #414

GAMDV45's `BOVB TCDANZIG` (P37_P70.agc 1694-1695, 32,3563) calls
`cm_integration_tcdanzig` (`src/cm_p37_p70.c` 5089) and continues natively
with the pair's right-hand order. The spec keys the rtb under `TCDANZIG` and
`ENDT(X)`, includes `cm_integration_moves.h` and links
`src/cm_integration_moves.c` on host and C64, as in the accepted sibling
files. Both arms are compared:

| Arm | Host frozen cases | C64 rows |
|---|---|---|
| overflow | 51 (2,315 executions) | group 34 row 2 |
| no overflow | 24 (451 executions) | group 37 row 1 |

The external 057352 is gone (10 externals to 9). There is no other
TCDANZIG or DANZIG transfer: the file's six other BOV sites branch to
interpretive labels.

## Domain

The certificate's entry, erasable and pin clauses are unchanged word for
word from the version accepted in #394. The only supported_domain change is
the truncated-row sentence (now three rows, two resumed at 32,3451 and
32,3560), which matches the cover record. The new path reads only Z and FB,
and no pin was added. docs/CM-P37-P70.md agrees with the evidence.

## Not blocking

1. The spec gains `trusted_image`, which drops the 15-bit image scan in
   `sane()` only under `RK_FIXED_IMAGE` (the flight build). The host, sanitizer
   and VICE builds keep it, and the flight build compiles clean. Neither the
   PR nor the doc mentions it.
2. The cover record pins `tests/interp_case.c` from before #408 (a shared
   source; the steward's #450 rule makes it a NOTE, and the PRGs are
   byte-identical).
