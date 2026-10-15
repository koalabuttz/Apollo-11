# Second review: re-cite lm-powered-subs, cm-conics and cm-time-free-fall after #506

Second certificate reviewer, 2026-10-15. Lane B's #506 (merged as 76b64d18; reviewed at 33a224f4, and the certificates are unchanged since) adds native routines at bridged DANZIG returns that D1's flight sweep found. **Verdict: all three are re-cited.**

## New entries, checked against the source

Each new routine refuses unless EDOP holds its pair's right-hand order. It then runs that order and continues at the word the listing gives.

| File | Entry | What runs |
|---|---|---|
| Luminary099 POWERED_FLIGHT_SUBROUTINES | 10,2173 (`edop_exits` FLESHPOT+92) | FLESHPOT's `DXCH LOC` leaves LOC at its `TCF VXV` word and `TS EDOP` BIT8 (EXIT), so VXV's DANZIG exits into 10,2174 |
| Comanche055 CONIC_SUBROUTINES | 12,2306 | BRNCHCTR's `RTB BHIZ` after CHECKCTR (basic, ends in TC DANZIG) |
| | 12,2524, 12,2533, 12,3165 | DELTIME's and 1/WLOOP's `RTB PDDL` after TPMODE (pushdown list; R1; R1A) |
| Comanche055 TIME_OF_FREE_FALL | 27,3211, 27,3274 | TFFXTEST's and TFFX's `RTB DMP` after T(X), with DMP from the pushdown list |

The supported_domain, entry_contracts and external_continuations name every new entry.

## Shared tool changes

- **interp_compile.py.** Adds the `edop_exits` key and a `pending_rtb()` helper. The helper now also covers a right-hand order with no operand words.
- **interp_routine_reference.c.** It now parses `IRC_RTB_IN_FILE` when `IRC_PENDING_DANZIG` is set too. Only cm-time-free-fall sets both.
- **Other specs.** `interp_compile --check` passes on all 80 specs. `--cover-check` still gives cases_match for cm-reentry and cm-integration-init (the other pending_danzig specs) and for three control specs.

## Evidence at 33a224f4 (reproduced)

| Check | lm-powered-subs | cm-conics | cm-time-free-fall |
|---|---|---|---|
| Hashes | match | match | match |
| Cases equal yaAGC | 25,088 | 114,944 | 31,488 |
| Host + sanitizer | 1,569 / 1,569 | 3,592 / 3,592 | 984 / 984 |
| PRGs byte-identical | 1 | 30 | 8 |
| VICE checks = vice_targets | 71 | 317 / 122 / 181 | 194 |
| `--cover-check` | 548/548, 0 missing | 2,159/2,159, 0 missing | 1,047/1,047, 0 missing |
| `cert_selfcheck` | OK | OK | OK |
| `cert_precheck` | 0 FAIL | 0 FAIL | 0 FAIL |

The lm-powered-subs release logs (differential f1648b3d, host 1d4f2e47, vice c9c89d82) were published after the review, and `evidence.py fetch` now gets all three.
