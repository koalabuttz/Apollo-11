---
id: cm-stable-orbit
title: Translate Comanche STABLE_ORBIT (P38-P39)
date: 2026-10-13T20:15Z
lane: G
---
All of Comanche055 STABLE_ORBIT (225 words) is native C on the control-transfer ABI, as 104 routines compiled whole-file by `tools/basic_compile.py`. All 26,368 cases equal the original program in yaAGC. The strict and sanitizer host tests pass 206 frozen cases, and 3 C64 diagnostics pass 139 cases in VICE that take every branch outcome the yaAGC-compared cases take. The complete certificate is drafted for the certificate reviewer. See `docs/CM-STABLE-ORBIT.md`.
