---
id: lm-orbital-integration
title: Translate Luminary ORBITAL_INTEGRATION
date: 2026-10-12T20:30Z
lane: B
---
All of Luminary099 ORBITAL_INTEGRATION (841 words: KEPPREP, the time step, oblateness and lunar/solar terms, rectification, the sphere-of-influence change and the W-matrix loop) is native C on the control-transfer ABI, as 353 routines compiled by `tools/interp_compile.py`. The compiler gained CGOTO, indexed fixed-constant operands and BOVB through RTB. All 45,184 cases (a quarter with a job waiting at each DANZIG) equal the original program in yaAGC. The strict and sanitizer host tests pass 353 frozen cases, and 9 C64 diagnostics pass 706 cases in VICE. A complete certificate is drafted for the certificate reviewer. See `docs/ORBITAL-INTEGRATION.md`.
