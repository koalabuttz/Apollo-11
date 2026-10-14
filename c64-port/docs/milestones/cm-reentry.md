---
id: cm-reentry
title: Translate Comanche REENTRY_CONTROL
date: 2026-10-14T13:00Z
lane: G
---
All of Comanche055 REENTRY_CONTROL (1172 words) is native C on the control-transfer ABI, as 392 routines compiled whole-file by `tools/basic_compile.py`. All 100,352 cases equal the original program in yaAGC. The strict and sanitizer host tests pass 784 frozen cases, and 49 C64 diagnostics pass 547 cases in VICE that take every branch outcome the yaAGC-compared cases take. The complete certificate is drafted for the certificate reviewer. See `docs/CM-REENTRY.md`.
