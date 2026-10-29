---
id: inflight-align
title: Translate Luminary and Comanche INFLIGHT_ALIGNMENT_ROUTINES
date: 2026-10-12T16:14Z
lane: D1
---
Every word of INFLIGHT_ALIGNMENT_ROUTINES in both programs (184 Luminary099 words, 186 Comanche055 words, bank 23) is native C on the control-transfer ABI: CALCGTA, ARCTRIG, CALCGA with its gimbal-lock alarm and AXISGEN, as 21 routines per program. A new compiler (`tools/interp_compile.py`) emits them from the assembly listings as the compiled interpreter path of each order pair. All 21,504 cases equal the original programs in yaAGC in Z, all 2,048 erasable words and the interrupt-enable state, from each routine to the point where control leaves the file. The strict and sanitizer host tests pass 686 frozen cases, and 13 C64 diagnostics pass 65 cases in VICE. The host vectors and C64 cases are chosen to reach every branch outcome that any case reaches; the cases include edge values (±0, 1/2, axis vectors, tiny vectors). Complete certificates for both files are drafted for the certificate reviewer. See `docs/INFLIGHT-ALIGN.md`.
