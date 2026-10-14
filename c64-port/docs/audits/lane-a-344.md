# Certificate review: Comanche055 P76 after PR #344

Reviewed 2026-10-14 by the second certificate reviewer at `port/c64-native-c`
9d115e46 (PR #344 merged), with the yaAGC reference rebuilt from the tip. This
is the Comanche counterpart of `lm-p76-file`, accepted in `lane-a-320.md`.

| File | Milestone | Verdict |
|---|---|---|
| Comanche055/P76 | `cm-p76-file` | accepted |

## What was checked

- **Precheck.** `tools/evidence.py fetch` passes and `tools/cert_precheck.py`
  gives 0 FAIL (the one WARN is the uncached nightly VICE manifest). The
  `source_revision` is 911e5c02. The 55 refusal sites are the `basic_sane`
  and `sane` entry guards, TC Q/TC E guards and dispatcher defaults.
- **yaAGC comparison, rerun by the reviewer.**
  `tools/interp_routine_differential.py cm-p76-file --cases 512` prints PASS
  with all 24,576 cases equal across 48 routines. The case set hashes to the
  proof's `cases_sha256` (a1a585a2), and the committed vectors came out
  unchanged.
- **Host and sanitizer.** `make test-cm-p76-file sanitize-cm-p76-file` gives
  1,543 checks with 0 failures each.
- **VICE.** The log's sha256 equals the certificate's and it has no FAIL. Its
  PASS lines sum to 76 checks with 0 failures, equal to `vice_targets`, the
  proof's C64 rows and the rows in `tests/cm_p76_file_c64_vectors.h`.
- **Independent C64 branch check.** All 76 header rows match a regenerated
  case. Per-object gcov `-b` with every host case shows 0 missing outcomes and
  lines, and the per-group build (2 groups, each with its own KEEP flags)
  takes all 665 host outcomes and 1,217 lines, 0 missing. The strict
  `--cover-check` also verifies the record (637 of 637).
- **Domain.** The entries mirror the LM file. P76 is started by V37
  (`FRESH_START_AND_RESTART.agc` FCADR P76) through SPVAC and FINDVAC, so
  interpretive entries see a VAC FIXLOC and NEWJOB +0 or positive.
  `PHASCHNG OCT 04024` (P76.agc 129-130) restarts as a job at the basic entry
  2177, which takes any NEWJOB and FIXLOC. INTWAKE1
  (`INTEGRATION_INITIALIZATION.agc` 831-849) returns through GOBAC's RVQ to
  QPRET, which P76 sets to OUT (P76.agc 139-143), registered as the
  interpretive entry 2211.
