# Certificate review: Luminary099 P34-35_P74-75 after PR #419

Reviewed 2026-10-14 by the second certificate reviewer at `port/c64-native-c`
000b88bc (PR #419 merged), with the yaAGC reference rebuilt from the tip.

| File | Milestone | Verdict |
|---|---|---|
| Luminary099/P34-35_P74-75 | `lm-p34-p35` | accepted |

## What was checked

- **Precheck.** `tools/evidence.py fetch` passes and `tools/cert_precheck.py`
  gives 0 FAIL. The one WARN is the nightly VICE manifest, which could not be
  fetched. The `source_revision` is 911e5c02.
- **Refusal sites.** There are 550:
  - the 509 `at_newops`/`at_basic` entry guards;
  - 23 `lmi_switch` guards on non-branching SET/CLR;
  - 6 INDEX bank-crossing guards in NTARGCHK;
  - the MUTABLE `fixed_index` defaults;
  - the RTB LOADTIME/DPMODE guards;
  - the TC Q/TC RTRN range guards;
  - four TS DISPDEX overflow skips at 35,3665, which cannot trigger because A
    comes from CA Z.
- **yaAGC comparison, rerun by the reviewer.**
  `tools/interp_routine_differential.py lm-p34-p35 --cases 512` prints PASS
  with all 260,608 cases equal across 509 routines. The case set hashes to the
  proof's `cases_sha256` (7c5d4742). The strict `--cover-check` verifies 597
  rows (19 truncated rows rebuilt, 6 resumed rows by digest) with 0 missing.
- **Host and sanitizer.** `make test-lm-p34-p35 sanitize-lm-p34-p35` gives 509
  checks with 0 failures each.
- **VICE.**
  - The log's sha256 equals the certificate's and it has no FAIL.
  - Its 48 PASS lines sum to 597 checks. That equals `vice_targets`, the
    proof's C64 rows and the rows in `tests/lm_p34_p35_c64_vectors.h`, group
    by group.
  - All 48 PRGs rebuilt at the tip are byte-identical to the logged ones.
- **Independent C64 branch check.**
  - All 591 non-resumed header rows match a regenerated case. The 6 resumed
    rows equal their parent case plus the recorded snapshot.
  - The 48 KEEP-group builds with `-DIRC_CUTS` take all 3,225 host outcomes,
    0 missing.
  - The 19 plain truncated rows, rerun on yaAGC, stop at their cuts in the
    recorded state.
  - For all 6 resumed rows, the snapshot was re-derived from yaAGC and all
    pairs are equal. yaAGC run from it to the cut equals the native run and
    the digest.
- **The 'end' row.**
  - Row 260624 resumes at 35,3145 (PDDL ACOS) and runs to ACOSABRT's ALARM1.
  - The yaAGC run of its parent case from 35,3062 ends in the same digest, so
    the resumed state lies on the original program's own path.
  - It is the only row that takes the ACOS alarm arm at `src/lm_p34_p35.c`
    4744.
  - The state needs PUSHLOC at 29D, which writes over 28D after the |28D| ≤ 1/2
    guard. Flight does not get there, because S33/34.1 does SETPD 0D, but the
    state is inside the certified domain (any PUSHLOC).
- **VNPOOH.**
  - The new entry 35,3712 serves `TC VNPOOH` from P30_P37, P32-P35_P72-P75
    and GENERAL_LAMBERT.
  - All of these are V37 FINDVAC jobs, so `sane()`'s VAC FIXLOC and
    non-negative NEWJOB fit.
  - No other milestone links these tables, and those files' prechecks are
    unchanged.
  - The tool's 'end' handling only touches records that contain an 'end' row,
    and no other committed record has one.
- **Accounting.** The 1,056 words in banks 04, 06, 10, 11, 23 and 35 are 1,030
  words in 509 routines, 18 2DEC/OCT operand constants and 8 inline VN
  operands.
- **Domain.**
  - P34, P35, P74 and P75 are started only by V37 (FRESH_START_AND_RESTART
    FCADRMM/PREMM1) through FINDVAC.
  - The two `PHASCHNG OCT 04024` calls are type C group-4 job restarts at
    35,3662 and 35,3670. Both are registered, and both restart through
    FINDVAC, since PHSPRDT4 is positive.
  - The file schedules no waitlist task and has no RESUME and no BOV(B), so
    #403, #385 and #414 do not apply.
  - Every label that other files call is an entry.
  - All 335 CHANG2 resume points start from flowed states.
  - The ACOS alarm arms at all five sites are compared.
- **Doc fix in this PR.** `docs/LM-P34-P35.md` and the `native_coverage` scope
  still described the earlier freeze: 178 routines, 91,136 cases, 24
  diagnostics with 344 cases, and six host-only routines. They now match the
  evidence: 509 routines, 260,608 cases, and 48 diagnostics with 597 rows (25
  truncated), with no host-only routines.
- **Not blocking.**
  - The cover-check counts 3,198/3,210 taken host/C64 arcs against the proof's
    3,200/3,200, likely a gcc version difference. Either way, 0 are missing.
  - The INTLOOP+7 MPAC pin is not named in `supported_domain`. Both arms of
    its BZE are compared.
