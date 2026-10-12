---
id: lanec-di-nvdsp
title: Translate the DISPLAY_INTERFACE display start-up
date: 2026-10-12T19:00Z
lane: C
---
Native C for PINBRNCH through V50PASTE of DISPLAY_INTERFACE_ROUTINES in both Luminary099 and Comanche055: PINBRNCH, NORMBNCH, NVDSP (GOPLAY), NV50DSP and its returns, BLANKCHK, PERFCHEK, GOANIDLE, ISITN00, 1STOR2ND and V50PASTE, translated block by block. 3,486 complete states are identical to the unchanged original programs in Virtual AGC, in strict and ASan/UBSan builds, 114 with an out-of-range COPINDEX are refused unchanged, and 277 cases (every entry, both missions) match the oracle on a VICE PAL C64. See `docs/LANEC-DI-NVDSP.md`.
