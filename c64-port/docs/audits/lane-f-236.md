# Certificate re-confirmation: three Lane F files after PR #236

Reviewed 2026-10-13 by the second certificate reviewer at `port/c64-native-c`
a694f89c (PR #236 merged). `lane-f-151.md` accepted these files (Q_R was
re-confirmed in `lane-f-211.md`). PR #236 moved them onto `agc_fast_ops`, the
same change #233 made for P-AXIS (`lane-f-233.md`). It refreshed their
evidence, so this record re-checks them and becomes their
`independent_review`.

| File | Milestone | Verdict |
|---|---|---|
| Q_R-AXIS_RCS_AUTOPILOT | `lm-q-r-axis` | accepted (re-confirmed) |
| TJET_LAW | `lm-tjet-law` | accepted (re-confirmed) |
| DAPIDLER_PROGRAM | `lm-dapidler` | accepted (re-confirmed) |

## What changed in #236

- Every `ai_*` call goes through `src/agc_fast_ops.c` (for plain cells) or
  through one-argument `agc_image_ops` wrappers.
- The C64 rows were re-picked by cover.
- The DAPIDLER C64 diagnostic now loads its image at $E000, with the KERNAL
  banked out, so it fits under $C000.

## What was checked

- `tools/cert_precheck.py` gives 0 FAIL on all three. The Q_R refusal warning
  covers the same refused cases `lane-f-151.md` argued.
- **yaAGC comparison, rerun by the reviewer** (`make <m>-differential`). The
  frozen vectors and C64 vectors came out byte-identical
  (`git diff --exit-code`). The counts are unchanged:

  | Milestone | Equal | Refused |
  |---|---|---|
  | lm-q-r-axis | 15,547 | 837 |
  | lm-tjet-law | 16,384 | 0 |
  | lm-dapidler | 9,216 | 0 |

- **Host and sanitizer.** `make test-<m> sanitize-<m>` gives 33,796, 16,443
  and 9,260 checks, with 0 failures each.
- **VICE.** Each log's sha256 equals the certificate's, and its PASS total
  equals `vice_targets` with 0 failures: 128, 256 and 192 checks.
- **Independent C64 branch check** (`lanef_cov.py`, gcc `--coverage` with
  `gcov -b` per object, `agc_fast_ops.c` included). Every branch outcome and
  line the host cases take is also taken by a C64 row:

  | Milestone | Outcomes | Lines | Missing |
  |---|---|---|---|
  | lm-q-r-axis | 428 | 585 | 0 |
  | lm-tjet-law | 249 | 328 | 0 |
  | lm-dapidler | 325 | 435 | 0 |
