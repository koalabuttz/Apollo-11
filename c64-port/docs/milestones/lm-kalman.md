---
id: lm-kalman
title: Compile KALMAN_FILTER for the DAP
date: 2026-10-14T05:30Z
lane: D2
---
Luminary099 KALMAN_FILTER's RATELOOP is compiled as one basic routine of the control-transfer ABI (`src/lm_kalman.c`) for the LM DAP's per-cycle set, alongside the complete hand-written `src/lm_torque.c`. The new spec key 'fast_window' routes its EB6 window operands to Lane F's one-byte afw_* helpers: 2,208 bytes on the flight image. All 2,048 cases equal yaAGC; 64 frozen cases pass the strict and sanitizer host tests, and 3 C64 cases pass in VICE, taking all 187 host branch outcomes. See `docs/LM-KALMAN-COMPILED.md`.
