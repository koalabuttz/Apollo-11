# Certificate review: refrozen C64 rows after #430, #438, #446 and #448

Reviewed 2026-10-14 by the second certificate reviewer. Each milestone was
reviewed at its merge commit: inflight-align at 42b254ba (#430), geometry at
e6964bfb (#448, the fix for #443), lm-p12 at 7e19e923 (#438), lm-rcsmon at
7e19e923 and again at e94e4410 (#451, its cover and proof refrozen at the tip),
lm-r63 at 88d8dc0e (#446). The refreezes record each milestone's C64 rows or
tighten its domain. The geometry, inflight-align and P12 specs also gain
`index_carry16`, `image_w` and `trusted_image`.

| File | Milestone | Verdict |
|---|---|---|
| Luminary099/INFLIGHT_ALIGNMENT_ROUTINES | `inflight-align` | re-confirmed |
| Comanche055/INFLIGHT_ALIGNMENT_ROUTINES | `inflight-align` | re-confirmed |
| Luminary099/LEM_GEOMETRY | `geometry` | re-confirmed |
| Comanche055/CSM_GEOMETRY | `geometry` | re-confirmed after #443 (rejected at 42b254ba) |
| Luminary099/P12 | `lm-p12` | re-confirmed |
| Luminary099/RCS_FAILURE_MONITOR | `lm-rcsmon` | re-confirmed (#449 withdrawn) |
| Luminary099/R63 | `lm-r63` | re-confirmed |

P51-P53 from #430 was rejected for the LM file (#452, GRABGRAV task domain)
and is not re-cited here.

## What was checked

For every milestone:

- **Precheck.** `tools/evidence.py fetch` passes. `tools/cert_precheck.py`
  gives 0 FAIL. The one WARN is the VICE manifest, which the proxy would not
  fetch, so `vice_targets` was checked by hand against the logs. Every cited
  evidence sha256 matches its file.
- **yaAGC comparison, rerun by the reviewer** at each proof's frozen
  `--cases`. Each regenerated case set hashes to the proof's `cases_sha256`,
  and a `--freeze` leaves the committed vectors unchanged.
- **Strict `--cover-check`**: 0 missing.
- **Host and sanitizer**: `make test-<m> sanitize-<m>`, 0 failures.
- **VICE**: every rebuilt PRG is byte-identical to its recorded sha256. PASS
  total = `vice_targets` = the proof's C64 rows = the header's rows.
- **All cases on the C64 groups**: the reviewer's scripts (or
  `tools/cert_selfcheck.py`, #441) match every header row to its frozen
  case, re-run it on yaAGC, and diff gcov branch outcomes per object and per
  KEEP-mask group against all host cases: 0 missing.

| Milestone | Cases | Routines | Host vectors | C64 rows (PRGs) | Branch outcomes host = C64 |
|---|---|---|---|---|---|
| inflight-align | 92,160 | 180 | 792 | 219 (14) | 1,649 |
| geometry | 76,288 | 149 | 655 | 194 (8) | 1,621 |
| lm-p12 | 46,592 | | 1,456 | 110 (7) | 1,024 |
| lm-rcsmon | 1,024 | | 72 | 17 (1) | 196 |
| lm-r63 | 15,360 | 30 | 960 | 35 (1) | 442 |

## Domain

- **geometry (#443).** At 42b254ba the EARROT1→OVERADAY states pinned
  DTEAROT to 0. Every flight call passes a nonzero DTEAROT
  (REENTRY_CONTROL.agc 139-143 and 257-265, P61-P67.agc 1100). The long
  OVERADAY/EARROT2 loops come from a large OVFIND: BOV(B) counts it down
  (`CCS OVFIND / TS OVFIND`, INTERPRETER.agc 2932) rather than clearing it.
  In flight OVFIND is only +0 or ±1.
  - #448 pins OVFIND to {+0, +0, +0, +1, −1} at the ten EARROT1..OVERADAY
    points that can loop back to EARROT2, and leaves DTEAROT and MPAC free.
    In the frozen set those points hold 5,120 cases, and DTEAROT is 0 in only
    120 of them. 687 run in the flight EBANK 7, from 1 s to 30 days.
  - A separate yaAGC probe at flight values matched 1,053/1,053: ±500 s,
    1 s to 23 h, 1 to 30 days, OVFIND in {0, ±1}, and entries EARROT1,
    EARROT2 and OVERADAY. Large OVFIND words diverge, so the pin is
    justified.
  - Steward rule #414: EARROT2's BOVB TCDANZIG runs natively
    (`src/cm_geometry.c` 1156). The BOVB arm runs in host and C64 cases and
    equals yaAGC. The spec maps both `TCDANZIG` and `ENDT(X)`.
  - Nit: supported_domain says that from any other OVFIND word the loop runs
    for thousands of passes. Small words converge and match yaAGC, so this
    holds only for large words.
- **lm-rcsmon.** Both entries (`src/lm_rcsmon.c` 122, 247) use `sane_nv`.
  The spec sets `any_fixloc` and `any_newjob`, and the C64 rows include
  NEWJOB −0 and a FIXLOC outside any VAC area. At 7e19e923 the cover
  record's pins on `src/cm_interp_orders.c` and `tests/interp_case.c` were
  stale, because other lanes changed those shared files, and the cover
  re-derived to 196 of 196 outcomes against 194 recorded. Under the
  steward's rule (#450) that is not a reject reason, so #449 was withdrawn.
  #451 then refroze the cover and proof at the tip. `cert_selfcheck.py
  lm-rcsmon` at e94e4410 gives OK: 1,024 cases equal yaAGC, 89 of 89 rows
  re-run equal, and the C64 takes all 225 outcomes. The pins are current,
  the VICE log and PRG (f4440410) are unchanged, and cert_precheck gives
  0 FAIL.
- **lm-p12.** It runs as a job, so `sane()` is the right domain. P12's one
  RTB (LOADTIME) runs natively.
- **lm-r63.** V89 runs as a VAC job (V89PERF → FINDVAC at PRIO10), so the
  `sane()` VAC check is the right domain. The case set is unchanged from the
  last review (`cases_sha256` a557126f).
- **inflight-align.** There are no tasks, no RESUME and no BOVB in either
  file.
- **New spec keys.**
  - `trusted_image` drops only the 15-bit erasable scan in `sane()`, and only
    under `RK_FIXED_IMAGE` (the flight build). The host, sanitizer and VICE
    builds that produce the evidence keep the scan, and the generated files
    compile clean with `-DRK_FIXED_IMAGE=1 -Werror`.
  - `image_w` takes W from `agc_image_w.h`.
  - The `index_carry16` end-around carry is correct for 16-bit unsigned. An
    exhaustive 2^32 check on lm-r63's form found 0 mismatches.

## Fixed in this PR

- `docs/INFLIGHT-ALIGN.md` now gives the current C64 figures: 14
  diagnostics, 219 cases and 1,780 outcomes, replacing 23, 224 and 1,774.
  It also gives the source map's real `--freeze` path: it is written to
  `build/` and stored as a release asset.

## Not blocking

- native_files for inflight-align leave out `agc_hardware_divide.c`,
  `agc_word.c` and `agc_multiword.c`, which are linked and measured. This
  predates #430.
