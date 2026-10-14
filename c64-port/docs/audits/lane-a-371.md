# Certificate review: Luminary099 R31 after PR #371

Reviewed 2026-10-14 by the second certificate reviewer at `port/c64-native-c`
4c522060 (PR #371 merged), with the yaAGC reference rebuilt from the tip.

| File | Milestone | Verdict |
|---|---|---|
| Luminary099/R31 | `lm-r31-file` | accepted |

## What was checked

- **Precheck.** `tools/evidence.py fetch` passes and `tools/cert_precheck.py`
  gives 0 FAIL (the one WARN is the uncached nightly VICE manifest). The
  `source_revision` is 911e5c02. The 97 refusal sites are the `sane` and
  `basic_sane` entry guards, SET/CLEAR switch guards, the LOADTIME and
  NORMUNX1 RTB refusals, TC Q/TC E guards and the dispatcher default.
- **yaAGC comparison, rerun by the reviewer.**
  `tools/interp_routine_differential.py lm-r31-file --cases 512` prints PASS
  with all 41,984 cases equal across 82 routines. The case set hashes to the
  proof's `cases_sha256` (df8318a4), and the vectors came out unchanged. The
  strict `--cover-check` verifies the record and rebuilds the one truncated
  row, 0 missing.
- **Host and sanitizer.** `make test-lm-r31-file sanitize-lm-r31-file` gives
  2,637 checks with 0 failures each.
- **VICE.** The log's sha256 equals the certificate's and it has no FAIL. Its
  22 PASS lines sum to 139 checks, equal to `vice_targets`, the proof's C64
  rows and the rows in `tests/lm_r31_file_c64_vectors.h`. All 22 PRGs rebuilt
  at the tip are byte-identical to the logged ones.
- **Independent C64 branch check.** All 139 header rows match a regenerated
  case. Per-object gcov over every host case shows 0 missing, and the 22
  KEEP-group builds with `-DIRC_CUTS` take all 1,277 host outcomes and 1,979
  lines. The truncated row (case 33816, cut at 37,3352) gives the same state
  on yaAGC and native, equal to the recorded digest; uncut, the run goes on,
  so the cut is real.
- **Closure.** The 204 words are 53 executed basic words, 143 interpreted
  words and 8 constants, none unreached ("53/143" is that split).
- **Domain.** R31 is started only by V83 (EXTENDED_VERBS.agc 966-971), through
  WAITLIST and FINDVAC, so the interpretive entries see a VAC FIXLOC and
  NEWJOB +0 or positive. DISPN5X (NOVAC) is basic only. The file has no phase
  change, and a restart clears EXTVBACT. The MUNFLAG fall-through at 37,3347
  is the V83-in-P63/P12 path (AVEGFLAG and MUNFLAG both set).
- **Doc fix in this PR.** `docs/LM-R31-FILE.md` carried a stale second copy of
  its body that contradicted the evidence (138 cases, one outcome host only).
  The copy is removed. Not blocking: `entry_contracts` omits 37,3144 DOCMBASE
  from its list of GOTO targets (the 82-routine count is right).
