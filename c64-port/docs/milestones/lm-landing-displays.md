---
id: lm-landing-displays
title: Translate Luminary LANDING_ANALOG_DISPLAYS as transfer-ABI routines
date: 2026-10-12T18:30Z
lane: F
---
All of Luminary099 LANDING_ANALOG_DISPLAYS (bank 21, 467 words) runs as four native routines of the control-transfer ABI, with channels 12, 14 and 30 in `lm_flight_io`: LANDISP (the R10 task through DISINDAT, the altitude rate or extrapolation to ALTM and SPEEDRUN's velocity update), TWIDDLE's return and its INTLZE task, and VARDELAY's return (the lateral and forward velocity monitor to the cross-pointer commands, then `TC LADQSAVE` back into ALTROUT or ALTOUT). All 16,384 cases equal the original program (on yaAGC with the hardware divide) in the outcome, channels 12 and 14 and all 2,048 erasable words (strict and ASAN/UBSAN). Every routine and reachable branch is taken, every assembled word maps to its native owner after a fresh yaYUL assembly, and 128 C64 rows chosen to take every entry, outcome and branch the host cases take pass in VICE. See `docs/LM-LANDING-DISPLAYS.md`.
