# Certificate review: Lane E certificates after PR #194

Reviewed 2026-10-13 by the second certificate reviewer at `port/c64-native-c`
47105def (PR #194 merged). This is a re-review after issue #161, with the
edge-case items from auditor issue #148.

| File | Milestone | Verdict |
|---|---|---|
| Comanche055/P30-P37 | `cm-p30-p37` | accepted |
| Comanche055/TPI_SEARCH | `cm-tpi-search` | accepted |
| Comanche055/P32-P33_P72-P73 | `cm-p32-p33` | accepted |
| Luminary099/P32-P35_P72-P75 | `lm-p32-p35` | accepted |
| Luminary099/R30 | `r30` | **not accepted** (#213) |
| Comanche055/R30 | `r30` | **not accepted** (#213) |

## What was checked

- `tools/cert_precheck.py --fetch` gives 0 FAIL on all six. Every
  `source_revision` is 911e5c02. The four evidence categories cite four
  distinct records, and none cites the source map.
- **yaAGC comparison, rerun by the reviewer.** I ran
  `tools/interp_routine_differential.py <m> --cases 512`. Each regenerated case
  set hashes to its proof's `cases_sha256`. The `resume_flow` specs start
  every resume LOC from the original program's state there: 56/56 for
  cm-p30-p37, 283/283 for cm-p32-p33 and 294/294 for lm-p32-p35.

  | Milestone | Cases | Routines | Result |
  |---|---|---|---|
  | cm-p30-p37 | 73,216 | 143 | all equal |
  | cm-tpi-search | 101,888 | 199 | all equal |
  | cm-p32-p33 | 208,384 | 407 | all equal |
  | lm-p32-p35 | 215,552 | 421 | all equal |
  | r30 | 112,640 | 220 | all equal |

- **Host and sanitizer.** `make test-<m> sanitize-<m>` gives 0 failures each:
  572, 796, 814, 842 and 880 checks.
- **VICE.** Each log's sha256 equals the certificate's, and its PASS totals
  equal `vice_targets` with 0 failures: 180, 248, 477, 497 and 258 checks.
- **Edge cases (#148).** `edge_cases` is on in all five specs, and every spec
  has `resume_keys`. None of the six files reads TIME1, TIME2 or a CDU, so the
  cleared 010-061 words don't limit these files. #148's cm-p37-p70 L
  divergence is not part of this review.
- **Independent C64 branch check (every case, per object, per group).** I
  regenerated every case and matched every C64 row to its case by seed and
  edit list: 180, 248, 477, 497 and 258 rows, none unmatched. Then I ran
  per-object gcov `-b` over every linked source, all host cases against the
  rows. A second pass built each C64 group with its own KEEP flags
  (`tools/mk/<m>.groups`) and `-DIRC_CUTS`, then ran only that group's rows,
  each truncated row with its cut (`cov_groups.py`, kept in the reviewer's
  notes).
  - **Accepted four:** every branch outcome and line the host cases take is
    taken by a C64 row: 934, 1,286, 1,866 and 2,759 outcomes. The only line
    missing is the harness's non-cut `irc_run` call in
    `interp_routine_native.c`.
  - **Truncated rows:** cm-tpi-search has 1, cm-p32-p33 3 and lm-p32-p35 3.
    Each runs to its cut LOC, the first NEWOPS there, and is compared against
    the yaAGC run of the same case stopped at the same LOC.
  - **r30:** 10 host outcomes are never taken on the C64. All are in
    `lm_interp_orders.c`:
    - 9 in the LM object: `genscr_tail`/`genscr1` at 446, 453 and 460;
      `sqrtsub` at 716, 717, 720 and 721; `sqrt_body` at 790; `lmi_bhiz` at
      1305.
    - 1 in the CM object: `overflow` at 288.

    `r30-c64-cover.json` reports 0 missing because it keys arcs by source
    file and line. `cm_interp_orders.c` includes `lm_interp_orders.c`, so the
    LM and CM programs' outcomes are merged there (`cm_interp_orders.c` shows
    `[0, 0]`). An outcome a CM row takes then credits the LM program, which
    is a separate C64 diagnostic. The four accepted milestones each link one
    mission, so they can't hit this.
