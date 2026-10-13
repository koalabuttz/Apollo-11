---
id: cm-tvc-exec
title: Translate Comanche TVCEXECUTIVE
date: 2026-10-13
lane: G
---
All of Comanche055 TVCEXECUTIVE (144 words) is native C on the control-transfer ABI, as 12 routines compiled whole-file by `tools/basic_compile.py`. All 3,072 cases equal the original program in yaAGC. The strict and sanitizer host tests pass 96 frozen cases, and 1 C64 diagnostic passes 25 cases in VICE that take every branch outcome the yaAGC-compared cases take. The complete certificate is drafted for the certificate reviewer. See `docs/CM-TVC-EXEC.md`.
