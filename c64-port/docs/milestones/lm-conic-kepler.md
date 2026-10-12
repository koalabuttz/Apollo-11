---
id: lm-conic-kepler
title: Translate Luminary Kepler subroutine (KEPLERN, DELTIME, CHECKCTR)
date: 2026-10-12T06:00Z
lane: B
---
Native C for the Luminary099 CONIC_SUBROUTINES Kepler subroutine (KEPLERN, DELTIME with its S(XI)/C(XI) series, CHECKCTR; 338 words of bank 12) as a chain of 23 steps between original NEWOPS frontiers, with two new general interpreter bodies (15BITADR, BZE/GOTO) beside the 16 added earlier on this branch. The final 2048-word image, IRQ and SUPER equal the pinned yaAGC oracle for all 65536 sampled KEPPREP entries (strict and ASAN/UBSAN), 1731 intermediate phase frontiers equal original NEWOPS images, 338 assembled words map to native literals after a fresh yaYUL assembly, and four C64 phase diagnostics pass 90 frozen steps in VICE. See `docs/LM-CONIC-KEPLER.md`.
