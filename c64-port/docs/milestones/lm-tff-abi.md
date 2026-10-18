---
id: lm-tff-abi
title: Compile Luminary TIME_OF_FREE_FALL for the flight store
date: 2026-10-18T01:00Z
lane: E
---
Luminary099 TIME_OF_FREE_FALL is compiled as 123 routines of the control-transfer ABI (`src/lm_tff_abi.c`) for the LM flight store, alongside the complete hand-written `src/lm_time_free_fall.c`, so V82 and the other interpretive callers stay native (the LM DOI scenario's 95 + 16 bridged points go to 0). All 31,488 cases equal yaAGC; 984 frozen cases pass the strict and sanitizer host tests, and 196 C64 cases pass in VICE, taking all 2,332 host edges. See `docs/LM-TFF-ABI.md`.
