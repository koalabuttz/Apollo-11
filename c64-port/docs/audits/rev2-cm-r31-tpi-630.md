# Second review: cm-r31 and cm-tpi-search trusted_image refreeze (#630)

Second certificate reviewer, 2026-10-18. The Certificate fixes lane's #630 (merged as 15734c1f) sets `trusted_image` on `cm-r31` (Comanche055/R31.agc) and `cm-tpi-search` (Comanche055/TPI_SEARCH.agc) and refreezes both. **Verdict: accepted and re-cited.** The previous records are [cm-r31.md](cm-r31.md) and [lane-e-194.md](lane-e-194.md).

## Code

- **Change.** In each file the one source change wraps `sane()`'s whole-image scan (`W[i]>AGC_MASK` for i 3..2047) in `#if !RK_FIXED_IMAGE`. Flight builds keep 15-bit words in the fixed image, so the scan can't refuse there, and it costs about 121K cycles per entry on the C64. Host and diagnostic builds keep the scan. This is the same pattern as the earlier trusted_image specs.
- **Domain.** Unchanged. No supported_domain text needs a change.

## Evidence (at 15734c1f)

- **Precheck.** `evidence.py fetch` gets 3 + 2 release files and every hash matches. `cert_precheck` gives 0 FAIL.
- **Differential.** The cases, cases_sha256, oracle_sha256 and frozen vectors are byte-identical to the previous freeze: cm-r31 has 45,568 cases and 356 vectors; cm-tpi-search has 101,888 cases and 796 vectors.
- **Host.** 356/356 and 796/796. I re-ran both locally (`make test-cm-r31`, `make test-cm-tpi-search`).
- **C64 groups.** cm-r31 goes from 12 diagnostics to 8, and cm-tpi-search from 8 to 16. The union of KEEP masks still holds all 89 and 199 routines respectively. cm-tpi-search drops `c64_host_only` (7 routines now run on the C64).
- **VICE.** cm-r31 has 117 checks and 0 failures. Its rows were reselected by c64_row_cover (118 → 117) and still take 896/896 host branch arcs. cm-tpi-search has 248 checks, 0 failures and 1,301/1,301 arcs.

## Notes (non-blocking)

1. This refreeze did not set `index_carry16` or `image_w`, which cert_precheck asks for at the next refreeze. `ixw` is used by three INDEX sites in R31 (k=0120). Without the carry fix, a negative index there refuses on the C64 (EXEC_OUT_OF_SCOPE) rather than computing a wrong value. That makes it a possible C64 stop, not a correctness hole. Set both keys at the next refreeze.
2. CM-R31.md's evidence lines are stale. It still says 22,528 cases and 704 frozen cases, but the proof has 45,568 cases and 356 vectors. TPI_SEARCH.md is similar (33,280 and 260 against 101,888 and 796). These were already wrong before #630.
