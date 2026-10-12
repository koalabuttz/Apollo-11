---
id: lanec-ev-lmi
title: Compile the LM tracking verbs' interpretive words
date: 2026-10-12T16:40Z
lane: C
---
Native C for the Luminary099 EXTENDED_VERBS interpretive words of TRMTRAK1, ATTACHIT and TCPIN, compiled onto the native interpreter paths. 576 complete states are identical to the unchanged original program in Virtual AGC, in strict and ASan/UBSan builds, and 116 of them (every entry) match the oracle on a VICE PAL C64. The Lane C oracle can now stop at NEWOPS only when the interpreter leaves a suite's own words, with every earlier suite's oracle CRC unchanged. See `docs/LANEC-EV-LMI.md`.
