---
id: lm-conic-timethet
title: Translate Luminary time-theta subroutine (TIMETHET, PARAM, GEOM, GETX, NEWSTATE)
date: 2026-10-12T07:00Z
lane: B
---
Native C for the Luminary099 CONIC_SUBROUTINES time-theta subroutine (TIMETHET, PARAM/GEOM, GETX with its iteration and series, COMMNOUT, NEWSTATE/LAMENTER, ABTCONIC; 290 words of banks 12 and 4, reusing the Kepler milestone's DELTIME) as a chain of 27 steps between original NEWOPS frontiers, with three new general interpreter bodies (ITA/STQ, TIX, TLOAD). The final 2048-word image, IRQ and SUPER equal the pinned yaAGC oracle for all 65536 sampled entries from the six callers (strict and ASAN/UBSAN), 144 intermediate phase frontiers equal original NEWOPS images, 290 assembled words map to native literals after a fresh yaYUL assembly, and four C64 phase diagnostics pass 90 frozen steps in VICE. See `docs/LM-CONIC-TIMETHET.md`.
