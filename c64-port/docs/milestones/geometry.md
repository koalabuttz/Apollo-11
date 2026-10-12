---
id: geometry
title: Translate LEM_GEOMETRY and CSM_GEOMETRY
date: 2026-10-12T17:10Z
lane: D1
---
All of Luminary099 LEM_GEOMETRY (113 words: the downlink state vectors, the rendezvous radar angles RRANGLES and RRNB) and 105 of the 253 Comanche055 CSM_GEOMETRY words (SVDWN1/2, LOG and EARROT) are native C on the control-transfer ABI, as 21 routines. `tools/interp_compile.py` now also compiles basic instructions (CCS, TCF, TC with returns, DXCH, DAS, TS skips, EXTEND/INDEX), and BOVB. All 10,752 cases equal the original programs in yaAGC. The strict and sanitizer host tests pass 336 frozen cases, and four C64 diagnostics pass 42 cases in VICE. The Luminary file has a complete certificate drafted for the certificate reviewer. The Comanche sextant routines wait for a Comanche MXV body and native TRANSP1/2. See `docs/GEOMETRY.md`.
