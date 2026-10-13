---
id: lm-conic-compiled
title: Compile Luminary CONIC_SUBROUTINES into transfer routines
date: 2026-10-13T02:00Z
lane: B
---
All of Luminary099 CONIC_SUBROUTINES (1,072 words) is native C on the control-transfer ABI, as 455 routines compiled by `tools/interp_compile.py`, with CHECKCTR as `lm_conic_checkctr`. Every entry (KEPLERN, TIMETHET, LAMBERT, TIMERAD, APSIDES and the rest) and every point a job resumes at after CHANG2 is a routine. All 29,120 cases equal the original program in yaAGC. The strict and sanitizer host tests pass 228 frozen cases, and 13 C64 diagnostics pass 910 cases in VICE. A complete certificate is drafted for the certificate reviewer. See `docs/LM-CONIC-COMPILED.md`.
