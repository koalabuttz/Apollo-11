---
id: lm-radar-leadin-routines
title: Translate RADAR_LEADIN_ROUTINES (compiled)
date: 2026-10-14T05:30Z
lane: D2
---
All of Luminary099 RADAR_LEADIN_ROUTINES (47 words: RADSAMP, DORSAMP, DORSAMP2 and VARADAR) is native C on the control-transfer ABI, as 7 basic routines at their original addresses compiled by tools/interp_compile.py. All 1,792 cases equal yaAGC over the sampling words' flight domain; 56 frozen cases pass the strict and sanitizer host tests, and 14 C64 cases pass in VICE, taking all 179 host branch outcomes. Pending complete certificate for the certificate reviewer. See `docs/LM-RADAR-LEADIN-ROUTINES.md`.
