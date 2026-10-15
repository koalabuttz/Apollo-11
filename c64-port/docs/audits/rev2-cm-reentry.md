# Second review: cm-reentry (Comanche055/REENTRY_CONTROL.agc) and r60-r62 re-cite

Second certificate reviewer, 2026-10-15. Reviewed at #478's merge 7c35462c. **Verdicts: cm-reentry accepted, and #461 is resolved. r60-r62 (Luminary099 and Comanche055 R60_62.agc) is re-cited.**

## cm-reentry: domain

- **LEWD (E7,1724-1725).** This was the #461 blocker. The spec's new `cell_pairs` draws any positive double word: a high word 1..037777 over a low word of either sign, or a +0 or -0 high word over a positive low word. These are the values STARTENT, LEWDOVFL and LWDSTORE (raw DAD and STORE) leave. supported_domain names LWDSTORE.
- **Effect on other specs.** `cell_pairs` draws from the rng only for specs that have it, and only cm-reentry does, so no other spec's cases change.
- **Code and earlier fixes.** There is no source change since the #457 review. The #413 source map, the #414 BOVB TCDANZIG sites and the entry context (SERVICER's AVERAGEG job, so #403 and #385 do not apply) are unchanged from that review.

## cm-reentry: evidence (reproduced)

| Check | Result |
|---|---|
| Hashes | All 5 files match SHA256.json and the certificate |
| Differential | 100,352 cases equal yaAGC (392 routines) |
| Host + sanitizer | 784 checks, 0 failures each |
| VICE | 46 PRGs byte-identical; 46/46 PASS, 543 checks = vice_targets |
| `--cover-check` | verified, cases_match; host 2,002, C64 2,002, 0 missing |
| `cert_selfcheck cm-reentry` | OK, no gaps; rows 1,327/1,327 equal on yaAGC; fresh gcov 2,031/2,031 |
| `cert_precheck` | 0 FAIL (vice-manifest WARN: not fetchable here; vice_targets checked by hand) |

## r60-r62: what changed

- **C64 rows.** `c64_rows: 'cover'` adds rows, so the C64 cases take every gcov arc, including agc_transfer.c:26.
- **TOBALLC's CCS A (LM 26,2164).** This is the return from TC G+N,AUTO. It is now drawn from {+0, 01000, 020000}, with -0 and negative values left out. That is right: READ/MASK give no negative value.
- **Nit, not blocking: the "not G+N" value.** G+N,AUTO returns it through `CCS A` after `MASK BIT10` (R60_62.agc 226-230). So A comes back as BIT10 - 1 = 0777, not BIT10. Any positive A takes the same CCS branch, and TOBALLA's CAF then overwrites A.
- **Salted run at the flight values.** I ran `IRC_SALT=rv2toballc --cases 256` with A in {0777, 020000, +0}. PASS: 50,176 cases equal yaAGC (196 routines). Fix the value and the domain text at the next refreeze.

## r60-r62: evidence (reproduced)

| Check | Result |
|---|---|
| Hashes | All 7 files match SHA256.json and both certificates |
| Differential | 100,352 cases equal yaAGC (196 routines) |
| Host + sanitizer | 832 checks, 0 failures each |
| VICE | 9 PRGs byte-identical; 9/9 PASS, 268 checks = vice_targets |
| `--cover-check` | verified, cases_match; host 1,572, C64 1,572, 0 missing |
| `cert_selfcheck r60-r62` | OK, no gaps; rows 1,100/1,100 equal; NOTE only for the shared agc_image_ops.c / interp_case.c pins (#450) |
| `cert_precheck` (both files) | 0 FAIL |
