# KALMAN_FILTER (Luminary), compiled form for the DAP

Lane D2. Milestone `lm-kalman`. How the routines are made and checked: `docs/INTERP-COMPILE.md`.

Luminary099 KALMAN_FILTER is already complete as the hand-written `src/lm_torque.c` (`docs/LM_TORQUE.md`). It has its own `lm_torque(image, io)` API. This milestone adds a compiled form, `src/lm_kalman.c`, as one basic routine of the control-transfer ABI (RATELOOP at 16,3641), for the LM DAP's per-cycle set: P-AXIS's SUPERJOB does `TCF RATELOOP` on every DAP pass. The file's coverage certificate is unchanged.

## Spec keys

- **`ebank: 6`**, **`channels: 'all'`, `channel_inputs`**, **`index_carry16`**, **`edge_cases`, `c64_rows: 'cover'`.**
- **`fast_window` (new).** The orders go through `include/agc_fast_redirect.h`: operands in the EB6 window pass a one-byte offset to Lane F's noinline `afw_*` helpers (#278). The object is 2,208 bytes on the flight image (`RK_FIXED_IMAGE`), 2,807 bytes before.

## Evidence

Hashes: `evidence/lm-kalman/SHA256.json` (all files in the tree). 2,048 cases equal yaAGC; 64 are frozen in `tests/lm_kalman_vectors.h` and pass the strict and sanitizer host tests; `make vice-lm-kalman` passes 3 C64 cases that take all 193 gcov branch outcomes and 394 compare edges the host cases take.
