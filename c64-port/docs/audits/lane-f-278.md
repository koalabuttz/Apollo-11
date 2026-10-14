# Certificate re-confirmation: four Lane F files after PR #278

Reviewed 2026-10-13 by the second certificate reviewer at `port/c64-native-c`
1816baab (PR #278 merged). `lane-f-233.md` and `lane-f-236.md` re-confirmed
these files after their move onto `agc_fast_ops`. PR #278 moved their
plain-cell window accesses into `src/agc_fast_window.c` and refreshed their
evidence, so this record re-checks them and becomes their
`independent_review`. None of their sources, vectors or makefiles changed
between 1816baab and 96fb4fcd.

| File | Milestone | Verdict |
|---|---|---|
| P-AXIS_RCS_AUTOPILOT | `lm-p-axis` | accepted (re-confirmed) |
| Q_R-AXIS_RCS_AUTOPILOT | `lm-q-r-axis` | accepted (re-confirmed) |
| TJET_LAW | `lm-tjet-law` | accepted (re-confirmed) |
| DAPIDLER_PROGRAM | `lm-dapidler` | accepted (re-confirmed) |

## What was checked

- `tools/cert_precheck.py` gives 0 FAIL on all four, after
  `tools/evidence.py fetch`. Its refusal WARNs on P-AXIS and Q_R report 298
  and 1,674 cases; that double-counts the per-entry breakdown. The proofs
  still give 149 and 837 refused cases, the same cases `lane-f-151.md`
  argued.
- **yaAGC comparison, rerun by the reviewer** (`make <m>-differential`, with
  the pinned llvm-mos v23.3.0 as `MOS_CC`). The frozen vectors and C64
  vectors came out byte-identical (`git diff --exit-code`). The counts match
  the earlier records:

  | Milestone | Equal | Refused |
  |---|---|---|
  | lm-p-axis | 16,235 | 149 |
  | lm-q-r-axis | 15,547 | 837 |
  | lm-tjet-law | 16,384 | 0 |
  | lm-dapidler | 9,216 | 0 |

- **Host and sanitizer.** `make test-<m> sanitize-<m>` gives 33,056, 33,796,
  16,443 and 9,260 checks, with 0 failures each.
- **VICE.** Each log's sha256 equals the certificate's, and its PASS total
  equals `vice_targets` with 0 failures: 128, 128, 256 and 192 checks. Each
  rebuilt PRG has the sha256 given in its log.
- **Independent C64 branch check** (`lanef_cov.py`, gcc `--coverage` with
  `gcov -b` per object; `agc_fast_ops.c` and `agc_fast_window.c` are linked).
  Every branch outcome and line the host cases take is also taken by a C64
  row:

  | Milestone | Outcomes | Lines | Missing |
  |---|---|---|---|
  | lm-p-axis | 379 | 571 | 0 |
  | lm-q-r-axis | 428 | 589 | 0 |
  | lm-tjet-law | 229 | 298 | 0 |
  | lm-dapidler | 325 | 438 | 0 |

  Line counts grow by the new window wrapper lines, which have no branches.
  TJET_LAW's counts fall because its indexed and MP operands moved off the
  image ops.

## Also reviewed: RCS_FAILURE_MONITOR (`lm-rcsmon`, PR #251)

Rejected in issue #285. Its evidence reproduces (1,024 cases equal, 71 host
checks, VICE 18 checks, 197 outcomes and 249 lines with 0 missing), but
`sane()` in `src/lm_rcsmon.c` refuses NEWJOB -0 (set by DUMMYJOB while
idling) and a NOVAC job's FIXLOC 0110, both of which T4RUPT reaches. Its
certificate stays pending.
