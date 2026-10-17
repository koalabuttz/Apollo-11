# Second review: lm-trim-gimbal on the fast-ops build (#570)

Second certificate reviewer, 2026-10-17. Lane E's #570 (head 37109284) moves GTS onto `include/agc_fast_redirect.h`, the compile-time redirect already used by Q_R-AXIS, TJET_LAW and DAPIDLER (0f7b8572, 2d94572e). **Verdict: accepted and re-cited.** The previous record is lane-f-211.md.

## Code

- **The change.** `src/lm_trim_gimbal.c` gains only the redirect setup: `AFR_EBN = LM_TG_EB >> 8`, the include, `W` as `AFR_S(s)->words`, and `AFR_BIND(s)` in `entry()`.
- **Entry check.** It is unchanged: FB match, EB = LM_TG_EB, and non-null image and io.
- **What the redirect does.** A plain EBANK6 operand goes to the fast helper of the same name. Every other operand (indexed cells, central registers, SR) goes to the `agc_image_ops` wrapper. The redirect's own differential (`tools/agc_fast_ops_differential.py`, `make test-agc-fast-ops`) checks that the two have the same effects.
- **Generic build.** `-DLM_TG_GENERIC` keeps the plain `agc_image_ops` build.
- **Build files.** The mk sources and headers add agc_fast_ops/agc_fast_window. `native_files` adds `src/agc_fast_ops.c` and `src/agc_fast_window.c`.

## Evidence

- **Unchanged files.** The yaAGC differential (16,211 cases, 173 GTS refusals), `tests/lm_trim_gimbal_vectors.h`, the proof and the host log are byte-identical to base.
- **Host and sanitizer run.** I rebuilt and ran `make test-lm-trim-gimbal sanitize-lm-trim-gimbal` on the fast build at 37109284: 33,031 checks, 0 failures, matching the committed host log. The CA table still equals the certified rope.
- **C64.** The rows were re-picked over the fast-op sources, covering 378 edges (was 324). VICE runs 256 checks with 0 failures on PRG cb94a740. vice_targets stays at 256.
- **Checks.** `ledgers.py --validate` passes. `cert_precheck` reports 0 FAIL. Its 2 WARNs (346 refused cases, 9 refusal sites) and the uncached vice manifest are the same as on base, and lane-f-211.md accepted them.

## Notes (non-blocking)

- PRG sha and heap_start drift come from the fast-op runtime. This is a NOTE because the targets pass.
