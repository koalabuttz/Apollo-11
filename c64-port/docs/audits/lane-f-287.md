# Certificate review: RCS_FAILURE_MONITOR after PR #287

Reviewed 2026-10-13 by the second certificate reviewer at `port/c64-native-c`
b86de7e0 (PR #287 merged). Issue #285 rejected the first certificate because
`sane()` refused NEWJOB -0 and a NOVAC job's FIXLOC 0110, both of which
T4RUPT reaches. PR #287 adds the spec key `any_newjob` and sets `any_fixloc`,
so both routines now check with `sane_nv`, and the cases draw NEWJOB -0 in a
quarter of cases and FIXLOC anywhere in half.

| File | Milestone | Verdict |
|---|---|---|
| Luminary099/RCS_FAILURE_MONITOR | `lm-rcsmon` | accepted (vice_targets corrected) |

## Correction: vice_targets

The certificate named `vice-lm-rcsmon: 18`; the VICE log it cites (sha256
d430b681, matching the certificate) has 17 checks with 0 failures, which is
also the proof's `c64_rows` and the row count of
`tests/lm_rcsmon_c64_vectors.h`. This PR sets the count to 17.

## What was checked

- `tools/cert_precheck.py` gives 0 FAIL and 1 WARN (no nightly manifest
  cached). The 14 refusal sites are the unused TC Q/TC E helpers, the INDEX L
  table checks and `default:` arms (L is 1 to 8), the `WRAP` null check and
  the entry guard, which now takes every NEWJOB and FIXLOC the source reaches.
- **yaAGC comparison, rerun by the reviewer** (`make lm-rcsmon-differential`).
  All 1,024 cases equal yaAGC, the regenerated case set hashes to the proof's
  `cases_sha256` (66dade1b), and the committed vectors came out unchanged.
- **The #285 inputs.** The reviewer's probe reruns all 1,024 cases with
  NEWJOB = 077777 added, then with FIXLOC = 0110 added. Both give 1,024 of
  1,024 equal, with the same end states as yaAGC (512 RESUME from 06,3242;
  510 NOVAC and 2 RESUME from RCSMON).
- **Host and sanitizer.** `make test-lm-rcsmon sanitize-lm-rcsmon` gives 72
  checks with 0 failures each.
- **VICE.** The log's sha256 equals the certificate's, and it has 17 checks
  with 0 failures and no FAIL.
- **Independent C64 branch check** (every case regenerated, header rows
  matched by edit list, per-object gcov `-b`). All 17 rows match a case. The
  host cases take 220 branch outcomes and 268 lines in the linked sources;
  the C64 rows take all of them, 0 missing.
- **Domain.** The restated domain names NEWJOB +0, -0 or positive and any
  FIXLOC, which covers DUMMYJOB idling (`EXECUTIVE.agc:464-465`) and NOVAC
  jobs under T4JUMP (`T4RUPT_PROGRAM.agc:177-182`). PVALVEST 0-0377 matches
  FRESH_START's 0 and RCSMON's own low-8-bit writes.
