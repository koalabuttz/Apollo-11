# Certificate review: Lane F R63, P12 and SERVICER after PR #171

Reviewed 2026-10-13 by the second certificate reviewer at `port/c64-native-c`
25cab905 (PR #171 merged at e0095945). SERVICER is a re-review after #167.

| File | Milestone | Verdict |
|---|---|---|
| Luminary099/R63 | `lm-r63` | accepted |
| Luminary099/P12 | `lm-p12` | accepted |
| Luminary099/SERVICER | `lm-servicer` | accepted |

P70-P71 (`lm-p70-p71`) stays partial with no certificate, waiting on output
channels.

## What was checked

- `tools/cert_precheck.py` gives 0 FAIL on all three. Every `source_revision`
  is 911e5c02, and the four evidence categories are distinct.
- **yaAGC comparison, rerun by the reviewer:**
  - `lm-r63` (`tools/interp_routine_differential.py --cases 512`): 15,360 cases
    over 30 routines, all equal.
  - `lm-p12`: 46,592 cases over 91 routines, all equal.
  - Both case sets hash to the proofs' `cases_sha256` (a557126f…, 957bb173…).
  - `make lm-servicer-differential`: 35,113 of 40,336 cases equal and 5,223
    refused. Each segment group's cases equal the oracle's segment runs, and the
    frozen vectors and C64 vectors come out byte-identical.
- **Host and sanitizer.** `make test-<m> sanitize-<m>`: R63 960 checks and P12
  1,456 checks, with 0 failures each.
- **VICE.** Totals equal `vice_targets` with 0 failures: R63 34, P12 112,
  SERVICER 464.
- **Independent C64 branch check:**
  - R63 and P12. I regenerated all cases and matched every row of
    `tests/<m>_c64_vectors.h` by edit list. Then I ran per-object gcov `-b`
    with every host case against the rows: 0 missing.
  - SERVICER. My `lanef_cov.py` compares the full build over all host cases
    against each of the 7 segment-group builds over its own rows. The C64
    rows take all 1,532 host branch outcomes and all 2,088 host lines.
  - #167 named 12 outcomes that the host takes only in a run crossing a
    segment end, such as the TS overflow skips at `lm_interp_orders.c`
    288/314/316, DV's zero dividend at :660, `lm_interp_steps.c` :137 and
    the work-area guard. Each is now taken on the C64, through the 16
    continuation cases (`tools/lm_servicer_chains.py`) that carry a run across
    the segment end.
- **SERVICER refusals** (`docs/LM-SERVICER.md`). Most of the 5,223 are keys
  entered inside a passage with a random work area:
  - 2,732 have a pushdown read below FIXLOC;
  - 1,525 have `33,2770`'s `BDDV*` indexed outside the named constants.

  The live job holds the pushes and the index its producers leave, so it never
  reaches either state. The remaining refusals are the producer-argued INDEX
  values, `TC Q`/`TC A` words, LOTHRUST's negative PHASE4, and EXTEND- or
  INDEX-dependent landings. cert_precheck counts 2,401 refusal outcomes. That
  is a different measure from the differential's case count.
