# Second review: cm-p34-p35 (Comanche055/P34-35_P74-75.agc)

Second certificate reviewer, 2026-10-15. Reviewed at #488's head c2283298 and checked unchanged at its merge 7ce3abe9. **Verdict: accepted.** Rejection #483 (VNPOOH's way back over only some callers) is resolved.

## Domain

- **VNPOOH callers.** The listing has exactly 12 `TC VNPOOH` sites: FB 35 at 2005, 2071, 2147, 2755, 2776 and 3156; FB 04 at 3340, 3350, 3415, 3444 and 3546; FB 37 at 2641. The spec's joint states give the (FB, return) pair of each, with `out` set on exactly the 7 sites outside the file. No other transfer names 04575-04613.
- **RTRN (E4,1767)** writers: VNPOOH's QXCH, INTINT/PREC/TT STQ in the same job before the call, and STABLE_ORBIT P38/39 under its own major mode.
- **TBASE5 (1063)** writers: VNPOOH's TS, P11's TS at launch, and NEWPHASE group 5 with TBASE in SERVICER (average-G). Neither average-G nor P11 runs during a P3x VNPOOH display.
- So the 12 joint pairs are the flight set. SETMUER vac+046 and MUTABLE [2,10] are unchanged and faithful.
- Rules #403, #385 and #414 do not apply: there is no task, waitlist or interrupt entry, no RESUME and no BOVB.

## Shared tool change

The first #488 head pinned joint-state words in `held`, which `relate()` reads, so it changed the frozen cases of cm-powered-subs, cm-extended-verbs-file and cm-conics. c2283298 keeps them in their own `jpinned` set. At c2283298 `--cover-check` verifies with cases_match for cm-powered-subs (24,064), cm-extended-verbs-file (93,184), cm-conics (113,920) and cm-p34-p35.

## Evidence (reproduced)

| Check | Result |
|---|---|
| Differential | 262,656 cases equal yaAGC (513 routines); case sha = proof; resume_flow 328/328 |
| Host + sanitizer | 513 checks, 0 failures each |
| VICE | 44 PRGs byte-identical to the log; 44/44 PASS, 584 checks = vice_targets = proof rows |
| Truncated rows | 10: rebuilt 4, resumed digest 6, failed 0 |
| `--cover-check` | verified; host 2,171, C64 2,183, 0 missing |
| `cert_selfcheck cm-p34-p35` | OK, no gaps; gcov 2,171/2,171 |
| `cert_precheck` | 0 FAIL (vice-manifest WARN; vice_targets checked by hand) |
| Release assets | vice 3266c42a, host 11e132f2, differential 141c1aca present |

Nit, not claimed: r4610 has dead code after `W[5]=04662;return EXEC_OK;`.
