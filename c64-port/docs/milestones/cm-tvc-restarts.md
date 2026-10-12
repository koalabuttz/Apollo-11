---
id: cm-tvc-restarts
title: Translate Comanche TVCRESTARTS
date: 2026-10-12T22:00Z
lane: G
---
All of Comanche055 TVCRESTARTS (90 words) is native C on the control-transfer ABI, as 7 routines compiled whole-file by `tools/basic_compile.py`. All 1,792 cases equal the original program in yaAGC. The strict and sanitizer host tests pass 56 frozen cases, and 1 C64 diagnostic passes 19 cases in VICE that take every branch outcome the yaAGC-compared cases take. The complete certificate is drafted for the certificate reviewer. See `docs/CM-TVC-RESTARTS.md`.
