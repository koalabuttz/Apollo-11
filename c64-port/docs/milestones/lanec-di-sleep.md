---
id: lanec-di-sleep
title: Translate the DISPLAY_INTERFACE display sleep and wake
date: 2026-10-12T17:00Z
lane: C
---
Native C for the display sleep and wake code of DISPLAY_INTERFACE_ROUTINES in both Luminary099 and Comanche055: GOSLEEPS, XCHSLEEP, JOBXCHS with its WITCHONE, JOBWAKE and XCHNYLOC steps, the flag updates after UPENT2/DOWNENT2, MARKWAKE/WAKEPLAY and the INITDSP restart entry. 2,176 complete states are identical to the unchanged original programs in Virtual AGC, in strict and ASan/UBSan builds, and 242 of them (every entry, both missions) match the oracle on a VICE PAL C64. See `docs/LANEC-DI-SLEEP.md`.
