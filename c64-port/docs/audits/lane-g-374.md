# Certificate review: LM and CM R60_62 after PR #374

Reviewed 2026-10-14 by the second certificate reviewer at `port/c64-native-c`
151982c5 (PR #374 merged), with the yaAGC reference rebuilt from the tip.

| File | Milestone | Verdict |
|---|---|---|
| Luminary099/R60_62 | `r60-r62` | accepted |
| Comanche055/R60_62 | `r60-r62` | accepted |

## What was checked

- **Precheck.** `tools/evidence.py fetch` passes and `tools/cert_precheck.py`
  gives 0 FAIL for both files (the one WARN is the uncached nightly VICE
  manifest). The `source_revision` is 911e5c02. The 133 (LM) and 95 (CM)
  refusal sites are the `sane` entry guards (`at_newops`, `at_basic`), TS
  overflow and TC Q/TC E guards, `rtb_angles` failures and dispatcher
  defaults; no case was refused.
- **yaAGC comparison, rerun by the reviewer.**
  `tools/interp_routine_differential.py r60-r62 --cases 512` prints PASS with
  all 100,352 cases equal across 196 routines (113 LM, 83 CM). The case set
  hashes to the proof's `cases_sha256` (2edf3e44), and the rebuilt proof and
  vectors are byte-identical to the committed ones. The strict
  `--cover-check` verifies 260 rows, 1,567 of 1,567.
- **Host and sanitizer.** `make test-r60-r62 sanitize-r60-r62` gives 836
  checks with 0 failures each.
- **VICE.** The 9 PRGs rebuilt at the tip are byte-identical to the logged
  ones, and the log has no FAIL. Its PASS lines sum to 260 checks, equal to
  `vice_targets`, the proof's C64 rows and the rows in
  `tests/r60_r62_c64_vectors.h`.
- **Independent C64 branch check.** All 260 header rows match a regenerated
  case. Per-object gcov (LM and CM objects separately) shows 0 missing. The
  9 KEEP-group builds take all 3,212 host lines and all host outcomes but one:
  the not-found exit of the shared `find()` binary search
  (`src/agc_transfer.c` 26). The two rows that take it in the full build look
  up in-range non-entry addresses (26,2024, 26,2413), which the narrower group
  tables reject at the range check on line 25 with the same result. This is
  shared harness code with an identical outcome, so it does not block.
- **Channels.** READ CHAN30/31 (LM G+N,AUTO and ISITAUTO) and RXOR CHAN31
  (CM TOBALLC) read the latched inputs as yaAGC's ReadIO does (sign-extended;
  RXOR on the overflow-corrected A). The oracle sets the same uniform 15-bit
  values through the Lane D1 channel model (#259), and both arms of every
  mode-bit test are taken.
- **Domain.** R60 is called by BANKCALL from VAC jobs (LM P20-P25 R61LEM,
  P40-P47, P52, R63, the landing; CM P20-P25, P40-P47), and R62DISP only
  from V49 through FINDVAC, so `sane()`'s VAC FIXLOC holds. CHKLINUS's
  PHASCHNG 00132 restarts at RELINUS (2.13SPOT) through FINDVAC, an in-file
  entry. The display waits stop at BANKCALL, and their returns are routines
  of their own.
- **Not blocking.** `docs/milestones/r60-r62.md` says "five channel-reading
  words"; the source has three channel instructions (six words with EXTEND).
