---
id: lanec-ev-time
title: Translate the V55 clock update (R33) and the ATTCK2 task
date: 2026-10-12T16:05Z
lane: C
---
Native C for the EXTENDED_VERBS V55 clock update (Luminary099 R33, Comanche055 ALINTIME) and the Luminary099 V43 ATTCK2 meter task, translated block by block. 1,152 complete states are identical to the unchanged original program in Virtual AGC, in strict and ASan/UBSan builds, and 231 of them (every entry) match the oracle on a VICE PAL C64. The Lane C oracle now keeps Virtual AGC's simulated CDU drive away from CDUXCMD..CDUZCMD, with every earlier suite's oracle CRC unchanged. See `docs/LANEC-EV-TIME.md`.
