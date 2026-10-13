---
id: cm-reentry
title: Translate Comanche REENTRY_CONTROL
date: 2026-10-13
lane: G
---
All but 2 words of Comanche055 REENTRY_CONTROL (1172 words) is native C on the control-transfer ABI, as 390 routines compiled whole-file by `tools/basic_compile.py`. All 99,840 cases equal the original program in yaAGC. The strict and sanitizer host tests pass 780 frozen cases, and 47 C64 diagnostics pass 544 cases in VICE that take every branch outcome the yaAGC-compared cases take. The file stays partial (see the doc). See `docs/CM-REENTRY.md`.
