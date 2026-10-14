---
id: lm-aotmark
title: LM AOT mark system as registered native routines
date: 2026-10-13T16:30Z
lane: D1
---
Luminary099 AOTMARK (AOTMARK, GETDAT, OPTAXIS, MARKRUPT, MARKCHEX, AVESTAR, SURFSTAR, OANB and the rest, 511 words in banks 07, 04 and 15) is compiled by `tools/interp_compile.py` into 122 routines of the control-transfer ABI: every entry, every return from a display or POSTJUMP, and every interpretive word after a DANZIG. All 62,464 cases equal the original program in yaAGC, from flight-realistic VAC, MARKSTAT, MKDEX and mark-counter states. The host and sanitizer tests pass 2,030 frozen cases, and 21 C64 diagnostics pass 302 cases in VICE that take all 1,638 gcov arcs the host cases take and 18,691 of the 18,693 per-site branch outcomes (the two left are shared-code branches the C64 cases take from other call sites). Channel 016 (NAVKEYIN) is now a latched input. A complete certificate is drafted for the reviewer. See `docs/LM-AOTMARK.md`.
