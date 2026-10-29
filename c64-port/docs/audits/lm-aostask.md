# Certificate review: Luminary099/AOSTASK_AND_AOSJOB.agc

Reviewed 2026-10-14 by the certificate reviewer, against milestone
`lm-aostask` (Lane D2).

- **First review:** at 3309df78, after #343. It was rejected in issue #348.
- **Re-review:** at b19baf2e, after #352.

Verdict: **accepted**.

## What the first review found

The DAP-running branch was never compared. The random cases never drew
`T5ADR = GENADR PAXIS`, so `BZF +2`, `CCS INGTS` and `TC IBNKCALL TIMEGMBL`
never ran (src/lm_aostask.c:1227-1257).

## What was checked at b19baf2e

- **The DAP-running branch is fixed.** 1,435 of 2,816 cases draw
  T5ADR = 02213, so the BZF goes both ways.
  - `CCS INGTS` takes its positive, +0 and −0 arms. The +0 arm reaches
    IBNKCALL TIMEGMBL, and the −0 arm falls through to DOCKTEST.
  - All of these cases equal yaAGC, both on the host and in the C64 rows.
- **The domain text matches the code.** Only a negative non-zero INGTS is
  refused, and the domain excludes it. Every routine uses `basic_sane`, so
  NEWJOB takes any value. cert_precheck's NEWJOB WARN comes from the unused
  `sane()` helper.
- **The oracle reproduces.** 2,816 cases equal yaAGC. The proof and source
  map regenerate byte-identical, and the vectors are unchanged.
- **Host and sanitizer:** 88 checks each, 0 failures.
- **VICE:** 38 checks, 0 failures, equal to `vice_targets`. The PRG hashes
  match the logs.
- **Per-object cover.** An independent `gcov -b` run shows the C64 rows
  taking all 704 host branch outcomes.
- **Ledger checks.** The evidence hashes match, the four categories cite
  distinct files, `cert_precheck.py` reports 0 FAIL, and
  `ledgers.py --validate` passes.
