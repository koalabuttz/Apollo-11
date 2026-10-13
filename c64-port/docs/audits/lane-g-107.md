# Certificate review: fifteen Lane G Comanche055 files after PR #107

Reviewed 2026-10-13 by the second certificate reviewer at `port/c64-native-c`
b97b1b74 (PR #107 merged at c8658690). This is a re-review after issues #44,
#51, #77 and #84, and auditor issue #146.

| File | Milestone | Verdict |
|---|---|---|
| TVCMASSPROP | `cm-tvc-massprop` | accepted |
| TVCEXECUTIVE | `cm-tvc-exec` | accepted |
| TVCRESTARTS | `cm-tvc-restarts` | accepted |
| TVCSTROKETEST | `cm-tvc-stroke` | accepted |
| TVCINITIALIZE | `cm-tvc-init` | accepted |
| TVCROLLDAP | `cm-tvc-roll` | accepted |
| TVCDAPS | `cm-tvc-daps` | accepted |
| RCS-CSM_DIGITAL_AUTOPILOT | `cm-rcs-dap` | accepted |
| AUTOMATIC_MANEUVERS | `cm-auto-man` | accepted |
| JET_SELECTION_LOGIC | `cm-jet-select` | accepted |
| CM_ENTRY_DIGITAL_AUTOPILOT | `cm-entry-dap` | accepted |
| CM_BODY_ATTITUDE | `cm-body-attitude` | accepted |
| SERVICER207 | `cm-servicer` | accepted |
| P61-P67 | `cm-p61-p67` | accepted |
| P40-P47 | `cm-p40-p47` | accepted |

REENTRY_CONTROL (`cm-reentry`) stays partial with no certificate. Two words
remain: KEP's SSP after RTB P66.

## What was checked

- `tools/cert_precheck.py` gives 0 FAIL on all fifteen. Every `source_revision`
  is 911e5c02. The four evidence categories cite four distinct records, none of
  them the source map. Category 2 is now the yaAGC differential proof, which
  closes #44, #51, #77 and #84 item 1.
  - The NEWJOB WARN on the eleven basic-code files points at line 36, a guard in
    the generated `at_newops` helper. These files have no interpretive entries
    and never call it.
  - The P40-P47 WARN about `src/rtb_angles.c` is moot: `src/cm_p40_p47.c`
    makes no `rtb_` call.
- **yaAGC comparison, rerun by the reviewer.** I ran
  `tools/interp_routine_differential.py <spec> --cases 256` for all fifteen on
  my own build of the pinned reference. Every case equals yaAGC, with no
  timeout: 512, 3,072, 1,792, 1,024, 2,560, 512, 2,304, 4,352, 1,536, 512 and
  7,168 cases for the eleven basic-code files in table order, then 17,920
  (body attitude), 20,480 (SERVICER207), 47,872 (P61-P67) and 126,976 (P40-P47).
- **Host and sanitizer.** `make test-<m> sanitize-<m>` passes with 0 failures on
  all fifteen.
- **VICE.** Every log records a PRG sha256. Its totals equal `vice_targets`,
  with 0 failures: 6, 25, 19, 12, 14, 16, 18, 43, 24, 28, 79, 97, 123, 252 and
  646.
- **Independent C64 branch check (all cases, per object).** I regenerated every
  case and matched each row of `tests/<m>_c64_vectors.h` to its case by edit
  list; every row matched. I built the native runner at `-O0 --coverage` over
  every linked source, then ran `gcov -b` per object, so `cm_interp_orders.c`'s
  include of `lm_interp_orders.c` is counted separately. Every branch outcome
  the host cases take is also taken by a C64 row: 0 missing on all fifteen.
  That includes SERVICER207 and P40-P47, where the cases dropped as too big
  for one diagnostic take no outcome the rows miss.

## Prior blockers

- **Transfer ABI** (#44 item 3, #51 item 2, #77 item 2, #84 item 2). Each file is
  compiled whole by `tools/interp_compile.py`. Every entry, CADR target and
  resume point is a routine at its original address, for example MASSPROP
  05,3366 and FIXCW 05,3422. A transfer out of the file ends at its boundary
  (the externals named in each proof), not at a bare Z.
- **Raw Q returns** (#44 item 2). `tc_q` refuses a Q that is not a TC word into
  fixed memory.
- **BUNKER** (#77 item 3) is the erasable continuation E6,1657, owned by
  TVCSTROKETEST. It accepts only 2541, 2543, 2743, 2745 and TCTSKOVR.
- **ATTKALMN** (#146) is admitted from -3 to 11, and the cases draw over the
  whole range. `docs/CM-RCS-DAP.md` states this.
- **C64 rows** are chosen by edge cover. The independent check above agrees.

## Note (not blocking)

For the ten files TVCMASSPROP through TVCDAPS, AUTOMATIC_MANEUVERS,
JET_SELECTION_LOGIC and CM_ENTRY_DIGITAL_AUTOPILOT, the case set the
differential generates at this tip does not hash to the proof's
`cases_sha256`. The proofs were frozen at 76ed9fbb, before the base merge
that renamed `counters` to `seed_counters`. All of their C64 rows are still in
the tip's case set, and the tip's cases all equal yaAGC. Lane G should
re-freeze them with its next PR. RCS DAP and the four interpretive files
reproduce their hashes exactly.
