---
id: lm-stable-orbit
title: Compile Luminary STABLE_ORBIT into transfer routines
date: 2026-10-13T07:00Z
lane: B
---
All of Luminary099 STABLE_ORBIT (P38/P78 and P39/P79, 233 words) is native C on the control-transfer ABI, as 101 routines compiled by `tools/interp_compile.py`. Every entry, every display return and every resume point is a routine. All 25,856 cases equal the original program in yaAGC. The strict and sanitizer host tests pass 202 frozen cases, and 3 C64 diagnostics pass 122 cases in VICE that take every branch arc the host cases take. A complete certificate is drafted for the certificate reviewer. See `docs/LM-STABLE-ORBIT.md`.
