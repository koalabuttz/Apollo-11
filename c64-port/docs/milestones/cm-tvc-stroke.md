---
id: cm-tvc-stroke
title: Translate Comanche TVCSTROKETEST
date: 2026-10-12T22:00Z
lane: G
---
All of Comanche055 TVCSTROKETEST (73 words) is native C on the control-transfer ABI, as 5 routines compiled whole-file by `tools/basic_compile.py`. All 1,024 cases equal the original program in yaAGC. The strict and sanitizer host tests pass 32 frozen cases, and 1 C64 diagnostic passes 12 cases in VICE that take every branch outcome the yaAGC-compared cases take. The complete certificate is drafted for the certificate reviewer. See `docs/CM-TVC-STROKE.md`.
