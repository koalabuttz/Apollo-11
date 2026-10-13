# Certificate review: Lane E R30 after PR #235

Reviewed 2026-10-13 by the second certificate reviewer at `port/c64-native-c`
f7b699e4 (PR #235 merged). This is a re-review after issue #213.

| File | Milestone | Verdict |
|---|---|---|
| Luminary099/R30 | `r30` | accepted |
| Comanche055/R30 | `r30` | accepted |

## What #213 found

`lm_interp_orders.c` is compiled into both the LM and the CM program
(`cm_interp_orders.c` includes it). The cover keyed arcs by source file and
line, so an outcome a CM row took also credited the LM program. Ten host
outcomes were never taken by a C64 row of their own program:

- 9 in the LM object: `genscr_tail`/`genscr1`, `sqrtsub`, `sqrt_body` and
  `lmi_bhiz`.
- 1 in the CM object: `overflow`.

## What changed in #235

- The cover now keys arcs by (object, source) for every source two linked
  objects compile. This builds on steward #215, and the split follows every
  pair gcov reports.
- The C64 rows were re-picked: 260 rows in 6 groups.

## What was checked

- `tools/cert_precheck.py` gives 0 FAIL on both files after
  `tools/evidence.py fetch r30`. Both `source_revision`s are 911e5c02.
- **yaAGC comparison, rerun by the reviewer.**
  `tools/interp_routine_differential.py r30 --cases 512` shows that all
  112,640 cases over 220 routines equal the original programs. The
  regenerated case set hashes to the proof's `cases_sha256` (d6668a19…). The
  committed vectors came out unchanged.
- **Host and sanitizer.** `make test-r30 sanitize-r30` gives 880 checks, with
  0 failures each.
- **VICE.** The log's sha256 equals the certificate's, and its PASS totals
  equal `vice_targets` (260 checks) with 0 failures. This is the lane's
  local run. CI's PR plan skipped the c64 job, so the nightly vice-manifest
  is the first CI run of these PRGs. `cert_precheck` checks the PRG hashes
  against that manifest once it exists.
- **Independent C64 branch check (every case, per object, per group).** I
  regenerated every case and matched all 260 rows by edit list, none
  unmatched. Per-object gcov `-b` over every linked source, all host cases
  against the rows, finds 0 missing in every object. That includes
  `lm_interp_orders.c` in the LM object (124 outcomes) and in the CM object
  (123), and `lm_interp_steps.c` in each (44). I also built each of the 6
  groups with its own KEEP flags and ran only that group's rows
  (`cov_groups.py`). All 1,394 branch outcomes and 2,925 lines the host cases
  take are taken by a C64 row. The ten outcomes #213 named are among them.
