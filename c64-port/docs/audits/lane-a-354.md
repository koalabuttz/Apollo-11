# Certificate review: Luminary099 P30_P37 after PR #354

Reviewed 2026-10-14 by the second certificate reviewer at `port/c64-native-c`
c905232b (PR #354 merged), with the yaAGC reference rebuilt from the tip.

| File | Milestone | Verdict |
|---|---|---|
| Luminary099/P30_P37 | `lm-p30-file` | accepted |

## What was checked

- **Precheck.** `tools/evidence.py fetch` passes and `tools/cert_precheck.py`
  gives 0 FAIL (the one WARN is the uncached nightly VICE manifest). The
  `source_revision` is 911e5c02. The 40 refusal sites are the `basic_sane`
  and `sane` entry guards, TC Q/TC E guards and dispatcher defaults.
- **yaAGC comparison, rerun by the reviewer.**
  `tools/interp_routine_differential.py lm-p30-file --cases 512` prints PASS
  with all 17,920 cases equal across 35 routines. The case set hashes to the
  proof's `cases_sha256` (9c817dd9), and the committed vectors came out
  unchanged.
- **Host and sanitizer.** `make test-lm-p30-file sanitize-lm-p30-file` gives
  1,127 checks with 0 failures each.
- **VICE.** The log's sha256 equals the certificate's and it has no FAIL. Its
  PASS lines sum to 57 checks with 0 failures, equal to `vice_targets`, the
  proof's C64 rows and the rows in `tests/lm_p30_file_c64_vectors.h`.
- **Independent C64 branch check.** All 57 header rows match a regenerated
  case. Per-object gcov `-b` with every host case shows 0 missing outcomes and
  lines, and the per-group build (2 groups, each with its own KEEP flags)
  takes all 688 host outcomes and 1,125 lines, 0 missing. The strict
  `--cover-check` also verifies the record (660 of 660).
- **Domain.** P30 is started only by V37 (`FRESH_START_AND_RESTART.agc` 1087
  FCADR P30), through SPVAC and FINDVAC, so the interpretive entries see a
  VAC FIXLOC and NEWJOB +0 or positive. S30.1 is called only from P30
  itself (P30_P37.agc 76), and the file has no phase change, so no restart
  re-enters an interpretive entry from another job state. The display waits
  (VNPOOH) resume in the same job. The basic entries take any NEWJOB and
  FIXLOC.
