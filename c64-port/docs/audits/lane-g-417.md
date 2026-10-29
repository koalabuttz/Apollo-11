# Certificate review: Comanche055 INTEGRATION_INITIALIZATION after PR #417

Reviewed 2026-10-14 by the second certificate reviewer at `port/c64-native-c`
e230d399 (PR #417 merged), with the yaAGC reference rebuilt from the tip.
The certificate was accepted in `lane-g-258.md` and re-confirmed in
`lane-g-370.md`. #417 makes two changes:
- INTSTALL, its EXIT and GOBAC's RVQ now take a NOVAC job's FIXLOC 0110
  (`novac`).
- The pending right-hand orders of the six RTB-left pairs are now keyed at
  DANZIG (`pending_danzig`).

| File | Milestone | Verdict |
|---|---|---|
| Comanche055/INTEGRATION_INITIALIZATION | `cm-integration-init` | re-confirmed (`vice_targets` corrected in this PR) |

## What was checked

- **Precheck.** `tools/evidence.py fetch` passes and `tools/cert_precheck.py`
  gives 0 FAIL. The one WARN is the VICE manifest, which could not be
  fetched. All five cited evidence hashes match, and `source_revision` is
  911e5c02.
- **What #417 changed.**
  - **NOVAC.** `sane()` accepts FIXLOC 0110 (`src/cm_integration_init.c`
    67). About a quarter of the INTSTALL, INTSTALL+1 and GOBAC+1 cases take
    it. 0110 is NOVAC's LOC(MPAC +6) - LOC(QPRET) (EXECUTIVE.agc 37-39).
    V37's ROC (FRESH_START_AND_RESTART.agc 913-916) and V56
    (EXTENDED_VERBS.agc 950-953) call INTSTALL from NOVAC jobs.
  - **Other callers.** The file's basic routines, including the STATEINT
    task, use `basic_sane()`, which checks neither FIXLOC nor NEWJOB. Its
    other interpretive callers run in FINDVAC jobs.
  - **Pending keys.** The six keys are the RTB operand words: 13,2732 BON,
    13,3352 SL, 13,3570 DAD, 13,3646 DAD, 13,3662 BDSU and 35,3727 SSP. Their
    EDOP codes (0163, 0116, 0162, 0162, 0156, 046) equal the listing's pair
    words, and their resume words are right. The oracle continues through
    DANZIG only at those keys with that EDOP.
- **Steward rule #414.** The file has no BOVB, TCDANZIG or BOV to ENDT(X).
  Its three BOVs branch to interpretive code, and BOV ABS runs ABS natively
  on the not-taken path. Every RTB-left pair's right-hand order now runs
  natively.
- **yaAGC comparison, rerun by the reviewer.**
  `tools/interp_routine_differential.py cm-integration-init --cases 256` (the
  frozen size) prints PASS: all 61,696 cases are equal across 241 routines.
  The case set hashes to the proof's `cases_sha256` (60f7fdf3), and the stop
  table is identical to the frozen log. The strict `--cover-check` verifies
  331 rows and 1,400 of 1,400 host outcomes, 0 missing.
- **Host and sanitizer.** `make test-cm-integration-init
  sanitize-cm-integration-init` gives 482 checks with 0 failures each.
- **VICE.** The log's sha256 equals the certificate's, and it has no FAIL.
  Its 12 PASS lines sum to 331, equal to the proof's C64 rows and the header
  rows. All 12 PRGs rebuilt at e230d399 are byte-identical to the logged
  ones.
- **Independent C64 branch check.** All 331 header rows match a regenerated
  case, in the proof's order, and all 331 digests equal a fresh yaAGC run.
  Per-object gcov shows 0 missing, and the 12 KEEP-group builds take all
  1,428 host outcomes and 3,163 lines, 0 missing. There are no truncated or
  resumed rows. The C64 rows include each pending key and each NOVAC entry.
- **Fixed in this PR.** `vice_targets` still gave 322 checks, the count from
  the 9-PRG freeze. It now gives 331. No evidence hash changes.
- **Not blocking.**
  - The 0110 acceptance is in the file-wide `sane()`, so all 204
    interpretive routines accept it, while only INTSTALL and GOBAC+1 compare
    it.
  - The domain text still opens with "FIXLOC a VAC area".
  - The spec comment names two of the six pairs.
