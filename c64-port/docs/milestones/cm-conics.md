---
id: cm-conics
title: Translate Comanche CONIC_SUBROUTINES
date: 2026-10-13T20:30Z
lane: G
---
All of Comanche055 CONIC_SUBROUTINES (1079 words) is native C on the control-transfer ABI, as 445 routines compiled whole-file by `tools/basic_compile.py`. All 113,920 cases equal the original program in yaAGC. The strict and sanitizer host tests pass 3560 frozen cases, and 32 C64 diagnostics pass 616 cases in VICE that take every branch outcome the yaAGC-compared cases take. The complete certificate is drafted for the certificate reviewer. See `docs/CM-CONICS.md`.
