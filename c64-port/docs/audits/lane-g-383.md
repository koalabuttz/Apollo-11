# Certificate review: Comanche055 SXTMARK after PR #383

Reviewed 2026-10-14 by the second certificate reviewer at `port/c64-native-c`
d8ff4c1b (PR #383 merged), with the yaAGC reference rebuilt from the tip.
The certificate was accepted in `lane-d1-282-sxtmark.md`. #383 re-picked the
C64 rows and set `trusted_image`.

| File | Milestone | Verdict |
|---|---|---|
| Comanche055/SXTMARK | `cm-sxtmark` | re-confirmed |

## What was checked

- **Precheck.** `tools/evidence.py fetch` passes and `tools/cert_precheck.py`
  gives 0 FAIL. The WARNs are the existing NEWJOB refusal and the VICE
  manifest, which could not be fetched. All cited evidence hashes match.
- **What #383 changed.** `trusted_image` puts the entry checks' over-15-bit
  scan of erasable words 3..2047 (in `sane()` and `basic_sane()`) under
  `#if !RK_FIXED_IMAGE`. No cited build sets that flag, and without it the
  preprocessed `src/cm_sxtmark.c` is byte-identical to the accepted one. The
  flight builds that set it do not link SXTMARK yet. One C64 row was added
  (case 20824, DIFCHK 10,2220) for the `src/agc_transfer.c` 26 find() arc.
- **yaAGC comparison, rerun by the reviewer.**
  `tools/interp_routine_differential.py cm-sxtmark --cases 512` (the frozen
  size) prints PASS with all 21,504 cases equal across 42 routines. The case
  set hashes to the proof's `cases_sha256` (c31811ee), and the stop table is
  identical to the frozen log. The strict `--cover-check` verifies 74 rows:
  491 of 491 host arcs at the PR commit (203bb6bc) and 492 of 492 at the
  merge, 0 missing. The extra arc is the base's channel fold in
  `tests/interp_case.c` (da8833de, 737d6995).
- **Host and sanitizer.** `make test-cm-sxtmark sanitize-cm-sxtmark` gives
  679 checks with 0 failures each.
- **VICE.** The log's sha256 equals the certificate's and it has no FAIL. Its
  one PASS line (74 checks) equals `vice_targets`, the proof's C64 rows and
  the header rows. The PRG rebuilt at 203bb6bc is byte-identical to the
  logged one. At the merge it differs only in the harness's channel
  bookkeeping (`interp_case.c`, and the removed `cm_channels` global); every
  SXTMARK, target and diagnostic object compiles to identical 6502 code, and
  CI's `vice-cm-sxtmark` job passed on that tree (cfba0751).
- **Independent C64 branch check.** All 74 header rows match a regenerated
  case, in the proof's order. Per-object gcov shows 0 missing, and the single
  KEEP-group build takes all 520 host outcomes and 1,361 lines, 0 missing.
  There are no truncated or resumed rows.
- **Not blocking.** The next nightly manifest or refresh should record the
  merge-tip PRG (7a950a6b), and the proof and docs say 491 arcs where the
  tip has 492.
