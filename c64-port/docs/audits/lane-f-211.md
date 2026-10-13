# Certificate re-confirmation: three Lane F files after PR #211

Reviewed 2026-10-13 by the second certificate reviewer at `port/c64-native-c`
7113b20e (PR #211 merged). These files were accepted in `lane-f-151.md`.
PR #211 replaced their evidence, so this record re-checks them against the new
evidence and becomes their `independent_review`.

| File | Milestone | Verdict |
|---|---|---|
| P-AXIS_RCS_AUTOPILOT | `lm-p-axis` | accepted (re-confirmed) |
| Q_R-AXIS_RCS_AUTOPILOT | `lm-q-r-axis` | accepted (re-confirmed) |
| TRIM_GIMBAL_CONTROL_SYSTEM | `lm-trim-gimbal` | accepted (re-confirmed) |

## What changed in #211

- **P-AXIS and Q_R** are split into part functions, with 13 part keys each
  for the flight store.
- **The C64 rows** were re-chosen: 128 for P-AXIS, 128 for Q_R, 256 for TRIM.
- **TRIM GTS** now reads DOSHIFT's indexed `CA` through the program's
  `fixed_data` (the rope), not a compiled-in table. The table moved to
  `tests/lm_trim_gimbal_ca_table.inc` as the test program's `fixed_data`.
  Without a `fixed_data` hook, GTS refuses and restores its cells and channel
  12.

## What was checked

- `tools/cert_precheck.py` gives 0 FAIL on all three.
- **yaAGC comparison, rerun by the reviewer** (`make <m>-differential`). The
  frozen vectors and C64 vectors came out byte-identical
  (`git diff --exit-code`). The equal and refused counts are unchanged from
  `lane-f-151.md`:

  | Milestone | Equal | Refused |
  |---|---|---|
  | lm-p-axis | 16,235 | 149 |
  | lm-q-r-axis | 15,547 | 837 |
  | lm-trim-gimbal | 16,211 | 173 |

- **Host and sanitizer.** `make test-<m> sanitize-<m>` gives 33,056, 33,796
  and 33,031 checks, with 0 failures each.
- **VICE.** Each log's sha256 equals the certificate's, and its PASS total
  equals `vice_targets` with 0 failures: 128, 128 and 256 checks.
- **Independent C64 branch check** (`lanef_cov.py`, gcc `--coverage` with
  `gcov -b` per object). Every host case ran in the full build, and only the
  C64 rows ran in the C64 build. Every branch outcome and line the host cases
  take is also taken by a C64 row:

  | Milestone | Outcomes | Lines | Missing |
  |---|---|---|---|
  | lm-p-axis | 393 | 614 | 0 |
  | lm-q-r-axis | 413 | 565 | 0 |
  | lm-trim-gimbal | 371 | 446 | 0 |

## Note (not blocking)

- In a flight build, GTS's indexed `CA` into fixed memory needs the module's
  `fixed_data`. `platform/c64/flight_table_module.c` sets it only when the
  module defines `FS_FIXED_DATA`, and no module defines it yet. When the
  flight store registers TRIM_GIMBAL, it has to supply that hook, or GTS will
  refuse on those words. The certificate's `supported_domain` says this.
