---
id: lanec-ev-lmi
title: Compile the LM radar and tracking verbs' interpretive words
date: 2026-10-12T16:40Z
lane: C
---
Native C for the Luminary099 EXTENDED_VERBS interpretive words of RRLOSDSP, TRMTRAK1, ATTACHIT and TCPIN, compiled onto the native interpreter paths. 960 complete states are identical to the unchanged original program in Virtual AGC, in strict and ASan/UBSan builds, and 138 of them (every entry) match the oracle on a VICE PAL C64. The Lane C oracle can now stop at NEWOPS only when the interpreter leaves a suite's own words, and runs every DV through the pinned engine's hardware model like the other interpreter oracles, with every earlier suite's oracle CRC unchanged. See `docs/LANEC-EV-LMI.md`.
