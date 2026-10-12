---
id: lanec-di-play
title: Translate the DISPLAY_INTERFACE display job selection
date: 2026-10-12T16:00Z
lane: C
---
Native C for the display job selection of DISPLAY_INTERFACE_ROUTINES in both Luminary099 and Comanche055: MAKEPLAY, the PRIO/NORM/MARK selection, MAKEPRIO through KEEPPRIO, IFLEGAL through PRIOPLAY, LINUSCHR, WITCHONE and XCHTOEND/XCHNYLOC. 2,208 complete states are identical to the unchanged original programs in Virtual AGC, in strict and ASan/UBSan builds, and 246 of them (every entry, both missions) match the oracle on a VICE PAL C64. MAKEMARK and COPIES2 are shared with lanec-di-mark and lanec-di-dsp. See `docs/LANEC-DI-PLAY.md`.
