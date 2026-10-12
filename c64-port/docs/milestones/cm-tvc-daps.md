---
id: cm-tvc-daps
title: Translate Comanche TVCDAPS
date: 2026-10-12T22:00Z
lane: G
---
All of Comanche055 TVCDAPS (489 words) is native C on the control-transfer ABI, as 9 routines compiled whole-file by `tools/basic_compile.py`. All 2,304 cases equal the original program in yaAGC. The strict and sanitizer host tests pass 72 frozen cases, and 1 C64 diagnostic passes 18 cases in VICE that take every branch outcome the yaAGC-compared cases take. The complete certificate is drafted for the certificate reviewer. See `docs/CM-TVC-DAPS.md`.
