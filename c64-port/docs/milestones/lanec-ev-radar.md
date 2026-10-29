---
id: lanec-ev-radar
title: Translate the Luminary radar test verbs (R04, R77)
date: 2026-10-12T22:00Z
lane: C
---
Native C for R77 through R77END of Luminary099 EXTENDED_VERBS (V63 R04 radar sampling and display, V78 R77 LR spurious-return test, V79 R77END), translated block by block. 2,448 complete states are identical to the unchanged original program in Virtual AGC, in strict and ASan/UBSan builds, and 223 of them (every entry) match the oracle on a VICE PAL C64. See `docs/LANEC-EV-RADAR.md`.
