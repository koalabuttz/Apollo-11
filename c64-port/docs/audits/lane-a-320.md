# Certificate review: Lane A whole-file certificates after PR #320

Reviewed 2026-10-14 by the second certificate reviewer (reviewer 1 handed
these over) at `port/c64-native-c` f2c54a98, in a detached worktree with no
build products carried over. The yaAGC reference and the ropes were rebuilt
from the tip. This covers four new pending certificates: the LM and CM P21
files, LM P31 (GENERAL_LAMBERT_AIMPOINT_GUIDANCE) and LM P76.

| File | Milestone | Verdict |
|---|---|---|
| Luminary099/GROUND_TRACKING_DETERMINATION_PROGRAM | `lm-ground-track` | accepted |
| Comanche055/GROUND_TRACKING_DETERMINATION_PROGRAM | `cm-ground-track` | accepted |
| Luminary099/GENERAL_LAMBERT_AIMPOINT_GUIDANCE | `lm-lambert-aim` | accepted |
| Luminary099/P76 | `lm-p76-file` | accepted |

## What was checked

- **Precheck.** `tools/evidence.py fetch` passes all four manifests, and
  `tools/cert_precheck.py` gives 0 FAIL on all four. Every `source_revision`
  is 911e5c02.
  - The only WARN is that the nightly VICE manifest could not be fetched;
    `vice_targets` was checked against the logs instead (below).
  - The refusal-site NOTEs are entry guards (`basic_sane` at basic entries,
    `at_newops`/`sane` at interpretive ones, one per routine), TC Q/TC K
    guards, the basic dispatcher default and null guards, and `lmi_switch`
    refusals after SET and CLEAR orders. Those cannot be reached: each switch
    word is a literal that decodes to the DANZIG return
    (`src/lm_interp_steps.c:273-274`).
- **yaAGC comparison, rerun by the reviewer.**
  `tools/interp_routine_differential.py <m> --cases 512` prints PASS with
  every case equal on all four. Every regenerated case set hashes to the
  proof's `cases_sha256`, and the committed vectors came out unchanged.
- **Host and sanitizer.** `make test-<m> sanitize-<m>` passes with 0 failures
  on all four, with the same counts as the committed host logs.
- **VICE.** Each log's sha256 matches its certificate, and none contains
  FAIL. Each log's check count equals `vice_targets`, the proof's C64 rows and
  the rows in `tests/<m>_c64_vectors.h`.
- **Independent C64 branch check.** Every case regenerated from
  `case_points`, every header row matched to a case, per-object gcov `-b`,
  then each C64 group built with its own KEEP flags and run on only its rows.
  0 outcomes and 0 lines are missing on all four. The strict
  `--cover-check` also passes on lm-lambert-aim and lm-p76-file.

  | Milestone | Cases equal | Host checks | VICE checks (PRGs) | Outcomes | Lines |
  |---|---|---|---|---|---|
  | lm-ground-track | 27,136 | 1,708 | 102 (22) | 879 | 1,376 |
  | cm-ground-track | 25,088 | 1,582 | 95 (10) | 786 | 1,354 |
  | lm-lambert-aim | 16,384 | 1,028 | 55 (1) | 551 | 876 |
  | lm-p76-file | 26,112 | 1,638 | 77 (2) | 753 | 1,233 |

  Every spec sets `index_carry16`, and no generated source calls `ixw`.

## Domain review

- **NEWJOB and FIXLOC at the interpretive entries.** P21, P31 and P76 are
  started only by V37 (FCADRMM1). V37XEQ starts them with SPVAC, which takes
  the FINDVAC path and sets FIXLOC to a VAC base (EXECUTIVE 253-255). NEWJOB
  is -0 only in DUMMYJOB, so a running job never sees it. P76's
  `PHASCHNG OCT 04024` is a type C job restart with the old priority
  PHSPRDT4, which V37XEQ sets positive, so a restart comes back as a FINDVAC
  job at 13,2320, a basic entry that accepts any FIXLOC and NEWJOB. The P21
  and P31 files contain no phase changes.
- **INTWAKE1 is noreturn.** INTWAKE1 (INTEGRATION_INITIALIZATION.agc 814-828)
  ends at `GOBAC TC INTPRET; RVQ`, which returns through QPRET. P76 sets
  QPRET to OUT (P76.agc 142-146), so control comes back interpretively at
  13,2332, which is registered as an interpretive entry
  (`src/lm_p76_file.c:754`), never as basic code after the CADR.
- **P21's DDV of P21VEL.** `P21VSAVE+13` is the DDV ASIN pair (LM 24,3606,
  CM 37,2102), a resume key with MPAC holding the dividend; half its cases
  draw the joint edge operands. Over the host cases the shared DDV body takes
  every arc (zero dividend, ±0 divisor, overflow, equal majors,
  normalisation), and every one is also taken on the C64.
- **A at return entries.** No entry follows SPSIN or SPCOS, and the first
  instruction at every basic entry is CAF, TC or TCF, none of which reads A.
  At interpretive CALL returns MPAC zero vectors of both signs are drawn.
- **Contract text.** The entry contracts match the generated guards.

## Notes (not blocking)

- The lm-lambert-aim entry contract names "BANKSET, FB and INTBIT15
  agreeing" at NEWOPS; the guard also requires SUPERBNK bit 7 clear
  (`sb_ok`, `src/lm_lambert_aim.c:64-65`), as the cm-ground-track text says.
- PRGs were not rebuilt and VICE was not rerun; the PRG hashes were checked
  against the logs only.
