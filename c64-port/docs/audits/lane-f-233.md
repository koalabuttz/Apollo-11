# Certificate re-confirmation: LM P-AXIS after PR #233

Reviewed 2026-10-13 by the second certificate reviewer at `port/c64-native-c`
337ece58 (PR #233 merged). `lane-f-151.md` accepted this file, and
`lane-f-211.md` re-confirmed it. PR #233 changed its code and refreshed its
evidence, so this record re-checks it and becomes its `independent_review`.

| File | Milestone | Verdict |
|---|---|---|
| Luminary099/P-AXIS_RCS_AUTOPILOT | `lm-p-axis` | accepted (re-confirmed) |

## What changed in #233

- Every `ai_*` call in `src/lm_p_axis.c` now goes through the new
  `src/agc_fast_ops.c` library (for plain cells) or through one-argument
  `agc_image_ops` wrappers. `native_files` now lists `src/agc_fast_ops.c`.
- The C64 rows were re-picked by cover, because the library adds arcs. The
  source map was regenerated after a line shift. The proof is unchanged.

## What was checked

- `tools/cert_precheck.py` gives 0 FAIL. Its refusal warning is the same 149
  refused cases `lane-f-151.md` argued.
- **yaAGC comparison, rerun by the reviewer** (`make lm-p-axis-differential`).
  16,235 cases equal and 149 are refused, as before. The frozen vectors and C64
  vectors came out byte-identical (`git diff --exit-code`).
- **Host and sanitizer.** `make test-lm-p-axis sanitize-lm-p-axis` gives
  33,056 checks, with 0 failures each.
- **VICE.** The log's sha256 equals the certificate's, and its PASS total
  equals `vice_targets` with 0 failures: 128 checks.
- **Independent C64 branch check** (`lanef_cov.py`, gcc `--coverage` with
  `gcov -b` per object, over `LM_PA_SRC` including `agc_fast_ops.c`). Every
  host case ran in the full build, and only the C64 rows ran in the C64 build.
  Every one of the 379 branch outcomes and 567 lines the host cases take is
  also taken by a C64 row.

## Note (not blocking)

- The library's own per-instruction yaAGC check (`make test-agc-fast-ops`) is
  not part of this file's evidence. It runs in CI. I did not rerun it here,
  because this worktree lacks the `deps/lm-rtb-fixture` rope it reads. The
  file-level differential above already runs every `agc_fast_ops` call P-AXIS
  makes against yaAGC.
