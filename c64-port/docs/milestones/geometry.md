---
id: geometry
title: Translate LEM_GEOMETRY and CSM_GEOMETRY
date: 2026-10-12T17:10Z
lane: D1
---
All of Luminary099 LEM_GEOMETRY (113 words: the downlink state vectors, the rendezvous radar angles RRANGLES and RRNB) and 105 of the 253 Comanche055 CSM_GEOMETRY words (SVDWN1/2, LOG and EARROT) are native C on the control-transfer ABI, as 21 routines. `tools/interp_compile.py` now also compiles basic instructions (CCS, TCF, TC with returns, DXCH, DAS, TS skips, EXTEND/INDEX), and BOVB. All 10,752 cases equal the original programs in yaAGC. The strict and sanitizer host tests pass 357 frozen cases, and seven C64 diagnostics pass 57 cases in VICE. The host vectors and C64 cases are chosen to reach every branch outcome that any case reaches; the cases include edge values (±0, 1/2, axis vectors, tiny vectors). The Luminary file has a complete certificate drafted for the certificate reviewer. The Comanche sextant routines wait for a Comanche MXV body and native TRANSP1/2. See `docs/GEOMETRY.md`.

Update 2026-10-13: the Comanche sextant routines SXTNB, SXTLOGIC, CALCSXA and SXTANG are native too, so CSM_GEOMETRY is native in every word (204 compiled, 49 constants, 91 routines) with a complete certificate drafted. SXTLOGIC and the SHORTMP multiply it calls are hand-written (`src/geometry_rtb.c`); `tools/interp_compile.py` gains the `native_calls` spec key for a TC to such a function. All 76,288 cases equal yaAGC, the host tests pass 658 frozen cases, and eight C64 diagnostics pass 197 cases in VICE, reaching all 1,671 host branch outcomes.
