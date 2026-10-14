---
id: lm-ascent
title: LM ascent guidance as registered native routines
date: 2026-10-14T06:00Z
lane: D1
---
Luminary099 ASCENT_GUIDANCE (ATMAG, ASCENT, AIMER, MAINLINE, the ASCTERM and EXITVR exits, KEEPVR, ENGOFF, CUTOFF, CHECKYAW and the attitude checks, 531 words in banks 34, 30, 27, 07, 05, 14 and 33) is compiled by `tools/interp_compile.py` into 209 routines of the control-transfer ABI. All 107,008 cases equal the original program in yaAGC, from random erasable states with edge values in a quarter of them. The host and sanitizer tests pass 3,344 frozen cases. In VICE, 29 C64 diagnostics pass 250 cases, 21 of them truncated, which take all 1,728 gcov arcs the host cases take. The C64 diagnostic now runs truncated rows to their cut. A complete certificate is drafted for the reviewer. See `docs/LM-ASCENT.md`.
