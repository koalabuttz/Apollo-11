# Certificate review: LM and CM INTERPRETER after PRs #361, #367 and #368

Reviewed 2026-10-14 by the second certificate reviewer, first at 1004a6dc
(PR #361; held in #366), then again at 14a3cfbd (PR #367, with the steward's
tool fixes from #368), with the yaAGC reference rebuilt from the tip.

| File | Milestone | Verdict |
|---|---|---|
| Luminary099/INTERPRETER | `lm-interpreter` | accepted |
| Comanche055/INTERPRETER | `cm-interpreter` | accepted |

The files are certified as "core + generated" (steward ruling): generated
cross-file basic subroutines in `src/{lm,cm}_interpreter.c` with evidence in
`evidence/{lm,cm}-interpreter/`, and the compact core `src/lm_interp_core.c`
(CM includes it) with evidence in `evidence/interpreter/`.

## What was checked

- **Precheck.** `tools/evidence.py fetch` passes and `tools/cert_precheck.py`
  gives 0 FAIL on both; `source_revision` 911e5c02. The NEWJOB WARN is
  CHANG2's handoff (the -0 refusal is DUMMYJOB's idle state only), and the
  overlay_reu WARN is the REU rope read path the VICE checks run.
- **yaAGC comparison, rerun by the reviewer.** `--cases 512` prints PASS:
  LM 21,500 cases across 42 routines (cases_sha256 082e71a1), CM 18,940
  across 37 (e57c9618), 4 refused each (#224: LOC at an operand word, now
  outside the stated domain). The stock strict `--cover-check` verifies LM
  643 of 643 and CM 597 of 597 after #368.
- **Host and sanitizer.** LM 1,348 and CM 1,189 checks, 0 failures each.
- **VICE.** The logs' sha256 equal the certificates'; no FAIL. LM 69+2 = 71
  and CM 67, equal to `vice_targets`, the proofs' C64 rows and the header
  rows. Per-group gcov: LM 2 groups, 672 outcomes and 1,755 lines; CM 1
  group, 626 outcomes and 1,622 lines; 0 missing, and 0 per object.
- **Closure and accounting.** LM 2,083 words = 416 basic + 1,614 core + 53
  constants, closure 414 of 416: DPAGREE 03,3255-3256 is reachable only
  through POLY's negative CCS exit, which POLYCNT (N-1 down to +0) never
  takes, and no LM word transfers there. CM 2,080 words, closure 381 of 381
  (CM has `TCR DPAGREE`, P40-P47.agc 843).
- **Core.** vice-interp-core 181 checks per mission, fuzz 2 x 100,000 cases
  against the verification interpreter with 0 differ, interp-flight-vice P00
  240 of 240. The core differential against yaAGC, rerun at the tip, gives
  74,424 cases with 14 differ (the committed log, 70,536 cases, predates three
  milestones that add 0 differ). All 14 are CM FAZB1 `STORE W+54D/108D/162D,1`
  under random WIXA; flight sets WIXA to 6-54 (MEASUREMENT_INCORPORATION.agc
  252-264, 326-348), and the contract now states that indexed and push-down
  erasable addresses stay inside erasable. The other 113 of #366's 127 came
  from the harness ignoring `flow_from`; with #368 those points start from
  flowed states and agree. `lm/cm-interpreter --all`: 632 cases, 0 differ.
- **Domain.** The supported_domains name `sane` (NEWJOB not negative, FIXLOC
  one of the five VAC areas) and `basic_sane` (FB the routine's bank, any FB
  for fixed-fixed entries, FB 0 for bank-00 ones), and both match the code.
  MPTEMP 0-28 matches the spec. `trusted_image` only changes builds with
  `RK_FIXED_IMAGE`, which no cited build sets.
- **Not blocking.** Regenerate `interp-core-differential.log` at the tip and
  record its commit in the header.
