# Certificate review: Lane G refreezes in #457

Reviewed 2026-10-14 by the second certificate reviewer, at e31cbd0c (#457).
Each milestone was refrozen for rule #414 (BOVB TCDANZIG runs natively) and
its domain text; this review replaces the Lane G review each file cited.

| File | Milestone | Was cited | Verdict |
|---|---|---|---|
| Comanche055/ORBITAL_INTEGRATION | `cm-orbital-integration` | lane-g-359.md | re-confirmed |
| Comanche055/TIME_OF_FREE_FALL | `cm-time-free-fall` | lane-g-359.md | re-confirmed |
| Comanche055/P61-P67 | `cm-p61-p67` | lane-g-107.md | re-confirmed |
| Comanche055/INTEGRATION_INITIALIZATION | `cm-integration-init` | lane-g-417.md | re-confirmed |

## What was checked

- **Precheck.** `tools/evidence.py fetch` and `verify` pass, and
  `tools/cert_precheck.py` gives 0 FAIL. The one WARN on each is the VICE
  manifest, which the proxy would not fetch, so `vice_targets` was checked by
  hand against the logs. Every cited sha256 matches its file.
- **yaAGC comparison, rerun by the reviewer** at each proof's frozen
  `--cases`, with the case set hashing to `cases_sha256`.
- **`tools/cert_selfcheck.py`** (#441): every header row matched to its
  frozen case and re-run on yaAGC, and per-group KEEP-mask gcov takes every
  host branch outcome. OK on all four.
- **Strict `--cover-check --cases 256`**: cases match, 0 missing.
- **Host and sanitizer**, and **VICE**: every rebuilt PRG is byte-identical
  to its recorded sha256, and the VICE PASS total equals `vice_targets`,
  the proof's C64 rows and the header's rows.

| Milestone | Cases | Rows re-run | Host + sanitizer | VICE PASS | PRGs |
|---|---|---|---|---|---|
| cm-orbital-integration | 85,248 | 1,091 (1 resumed, 11 truncated) | 666 | 425 | 47 |
| cm-time-free-fall | 30,976 | 1,158 | 968 | 190 | 7 |
| cm-p61-p67 | 47,872 | 625 | 374 | 251 | 11 |
| cm-integration-init | 61,696 | 813 | 482 | 331 | 12 |

## Rule #414

Every BOVB TCDANZIG site calls `cm_integration_tcdanzig` and continues
natively; the refusal after it is never taken. Both arms run on C64:

| Site | Continues with | Overflow arm, host cases | Overflow arm, C64 rows |
|---|---|---|---|
| ORBITAL_INTEGRATION 11,2455 BOVB DAD | `right(s)` | 13,101 of 18,015 | 12, in 7 groups |
| P61-P67 26,2604 BOVB VLOAD | `right(s)` | 192 of 776 | 2 |
| TIME_OF_FREE_FALL 27,3045 DMP BOVB | `next(s)` | 878 of 1,505 | 5 |
| TIME_OF_FREE_FALL 27,3116 DCOMP BOVB | `next(s)` | 1,436 of 2,244 | 8 |

INTEGRATION_INITIALIZATION has no BOVB. There are no other TCDANZIG sites.

## Domain

- **TFF's oracle setting.** Only TFF's spec gets `IRC_RTB_IN_FILE`. TFF's
  ENDT(X) is basic code inside the file, and the reference runner then skips
  its stop at DANZIG only when ENDT(X) was entered from outside it (the BOVB
  path); a case starting at ENDT(X) still stops there. yaAGC itself never
  stops at DANZIG. Without the setting 2,314 of 30,976 cases differ, so it
  carries weight. The in-file ENDT(X) path does what `cm_integration_tcdanzig`
  does (TC DANZIG, same registers).
- **"any EB (cases draw EB 0-7)"** is true: the cases draw EB at random and
  the entry checks accept 0-7. The old "EB 6" text was wrong before #457.
  Nothing in the domains excludes flight inputs. #403 and #385 do not apply
  (no task `sane_nv` entries, no RESUME).
- **cm-integration-init.** Only its source map and domain text changed. The
  map change is six hunks, one per pending pair, each moving the right-hand
  order's operand words to the routine keyed at the RTB operand word. The
  generated C is unchanged.
- **Tools.** `interp_compile.py --check` passes for all 79 specs. P61-P67's
  spec gains `index_carry16`, `image_w` and `trusted_image`; `index_carry16`
  keeps the end-around carry in `ixw` on the 6502.

## Not blocking

1. TFF's domain says the routine "goes on with the pair's right-hand order";
   both TFF BOVBs are already the right-hand order, so it goes on with the
   next pair.
2. ORBITAL_INTEGRATION's c64-cover record pins the pre-#408
   `tests/interp_case.c` (a shared source; the PRGs are byte-identical and
   the cover check verifies).
3. ORBITAL_INTEGRATION's doc says "every control-flow edge" while giving
   3,745 of 3,752, and no longer explains the truncated rows.
4. The reference runner ignores `IRC_RTB_IN_FILE` when a spec also has
   `pending_danzig`; no spec has both today.
