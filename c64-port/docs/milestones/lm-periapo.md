---
id: lm-periapo
title: Translate the Luminary apsides display subroutines (PERIAPO, SHIFTR1, MAXCHK)
date: 2026-10-12T15:00Z
lane: E
---
Native C for the shared apsides display subroutines of the Luminary099 targeting programs: RPAD, PERIAPO1, PERIAPO, SETRAD, LEMSTORE/ACTIVE, CSMSTORE/PASSIVE, VECSHIFT and SHIFTR1 from P34-35_P74-75 (bank 23) and MAXCHK/MAXNM from R30 (bank 22), 73 words, as ten steps between original NEWOPS frontiers that run the lm-conic-timerad milestone's APSIDES steps for PERIAPO. The final 2048-word image, IRQ and SUPER equal the pinned yaAGC oracle for all 65536 sampled entries from the routines' Luminary call returns (strict and ASAN/UBSAN), 118 intermediate phase frontiers equal original NEWOPS images, every assembled word maps to a native literal after a fresh yaYUL assembly, and four C64 phase diagnostics pass 63 frozen steps in VICE. See `docs/LM-PERIAPO.md`.
