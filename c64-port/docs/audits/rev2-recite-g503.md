# Second review: re-cite cm-p61-p67 (Comanche055/P61-P67.agc)

Second certificate reviewer, 2026-10-15. Lane G's #503 (merged as 80342904; reviewed at its head 91080f77) refroze cm-p61-p67. It was complete on docs/audits/rev2-recite-g457.md. **Verdict: re-cited.**

## What changed

- **`returns` key.** The spec now has `'returns': {'GOFLASH': 3, 'GOFLASHR': 4, 'GOPERF1R': 4}`, so every word a display call returns to is a compiled routine. That closes the CM entry holes P61.3, P62.2+11 and P62.1+4 found by D1's flight sweep.
- **Return counts against DISPLAY_INTERFACE_ROUTINES.agc.**
  - CALL CADR +1 is terminate, +2 proceed and +3 enter/recycle.
  - GOFLASHR and GOPERF1R (through GODSPRS1) also return at once to CALL CADR +4.
- **New entries.** There are 13, all in bank 26 (200 in all).
  - The file has 7 GOFLASH/GOFLASHR/GOPERF1R call sites: 2225, 2271, 2307, 2332, 2343, 2462 and 2513.
  - Every return word of each site now has an entry, and each is a reachable TC instruction, not a data word.
  - No entry was removed.
- **Unchanged.** The tools are not changed, and `interp_compile.py --check` passes for all 80 specs. The domain, the external continuations and the source revision are unchanged. Only the scope changes, from 188 to 200 routines, now naming the display returns.

## Evidence at 91080f77 (reproduced)

| Check | Result |
|---|---|
| Hashes | All 5 files match SHA256.json and the certificate |
| Differential | 51,200 cases equal yaAGC (200 routines); case set = proof |
| Host + sanitizer | 400 checks, 0 failures each |
| VICE | 10 PRGs byte-identical; 10/10 PASS, 267 checks = vice_targets |
| `--cover-check` | verified, cases_match; host 1,430, C64 1,430, 0 missing |
| `cert_selfcheck cm-p61-p67` | OK, no gaps; 667/667 rows equal on yaAGC; gcov 1,459/1,459 |
| `cert_precheck` | 0 FAIL (vice-manifest WARN; vice_targets checked by hand) |

The generated indices are rebuilt after merge.
