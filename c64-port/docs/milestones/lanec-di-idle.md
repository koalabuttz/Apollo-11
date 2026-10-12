---
id: lanec-di-idle
title: Translate the DISPLAY_INTERFACE ENDIDLE handling and display returns
date: 2026-10-12T18:00Z
lane: C
---
Native C for CLEANEND through TERMATE and PROCEED through ERASER of DISPLAY_INTERFACE_ROUTINES in both Luminary099 and Comanche055: REST/RESTSLEP, FLASHSUB and the ENDIDLE call, the terminate, proceed and load returns, TIMECHEK, ENDRET/ENDIT, UNSETR1/IMMEDRET, MARKRET, MARKOVER, NORMRET, MARSLEEP, LOADITIS and ERASER, translated block by block. 3,840 complete states are identical to the unchanged original programs in Virtual AGC, in strict and ASan/UBSan builds, and 226 of them (every entry, both missions) match the oracle on a VICE PAL C64. See `docs/LANEC-DI-IDLE.md`.
