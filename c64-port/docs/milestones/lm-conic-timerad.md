---
id: lm-conic-timerad
title: Translate Luminary time-radius and apsides subroutines (TIMERAD, APSIDES)
date: 2026-10-12T07:30Z
lane: B
---
Native C for the Luminary099 CONIC_SUBROUTINES time-radius and apsides subroutines (TIMERAD with TERMNVEC, CIRCULAR and BADR2; APSIDES with GETECC and INFINAPO; 123 words of bank 12) as 9 new steps between original NEWOPS frontiers, reusing the time-theta milestone's PARAM/GEOM, GETX, COMMNOUT and DELTIME steps. The final 2048-word image, IRQ and SUPER equal the pinned yaAGC oracle for all 65536 sampled entries from both callers (strict and ASAN/UBSAN), 180 intermediate phase frontiers equal original NEWOPS images, 123 assembled words map to native literals after a fresh yaYUL assembly, and six C64 phase diagnostics pass 108 frozen steps in VICE. See `docs/LM-CONIC-TIMERAD.md`.
