---
id: cm-ephemeris
title: Translate Comanche LUNAR_AND_SOLAR_EPHEMERIDES_SUBROUTINES
date: 2026-10-13
lane: G
---
All of Comanche055 LUNAR_AND_SOLAR_EPHEMERIDES_SUBROUTINES (71 words) is native C on the control-transfer ABI, as 32 routines compiled whole-file by `tools/interp_compile.py`. All 8,192 cases equal the original program in yaAGC. The strict and sanitizer host tests pass 256 frozen cases, and 2 C64 diagnostics pass 38 cases in VICE that take every branch outcome the yaAGC-compared cases take. The complete certificate is drafted for the certificate reviewer. See `docs/CM-EPHEMERIS.md`.
