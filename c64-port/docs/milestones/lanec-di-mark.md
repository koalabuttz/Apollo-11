---
id: lanec-di-mark
title: Translate the DISPLAY_INTERFACE mark displays
date: 2026-10-12T15:00Z
lane: C
---
Native C for the mark displays of DISPLAY_INTERFACE_ROUTINES in both Luminary099 and Comanche055: BLANKET, ENDMARK, CLEARMRK, MARKEND, the GOMARK entries, MAKEMARK, MARKPLAY/GOGOMARK/MARKCOP and COPYTOGO/COPYPACS. 2,944 complete states are identical to the unchanged original programs in Virtual AGC, in strict and ASan/UBSan builds, and 227 of them (every entry, both missions) match the oracle on a VICE PAL C64. The display-start paths of lanec-di-dsp are shared through `include/lanec_di_internal.h`, and `symbols.txt` WORD rows now also take equates and NAME+N. See `docs/LANEC-DI-MARK.md`.
