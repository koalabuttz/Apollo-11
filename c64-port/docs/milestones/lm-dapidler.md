---
id: lm-dapidler
title: Translate Luminary DAPIDLER_PROGRAM as transfer-ABI routines
date: 2026-10-12T17:00Z
lane: F
---
All of Luminary099 DAPIDLER_PROGRAM (banks 16 and 20, 284 words: CHEKBITS, CHEKMORE, the T5RUPT DAPIDLER with STARTDAP's initialization and SHUTDOWN, and the ALTDSPLY attitude-error needles with OVERSUB2 and the DAC loop) runs as nine native routines of the control-transfer ABI, with channels 5, 6, 12, 14, 30 and 31 in `lm_flight_io`. All 9,216 cases equal the original program in the outcome, the output channels and all 2,048 erasable words (strict and ASAN/UBSAN). Each case runs from its entry through the native Executive and inter-bank routines to RESUME, the IBNKCALL out of the file, the 1202 alarm or CHEKBITS's return. Every routine and reachable branch is taken, every assembled word maps to its native owner after a fresh yaYUL assembly, and 192 C64 rows chosen to take every entry, outcome and branch the host cases take pass in VICE. See `docs/LM-DAPIDLER.md`.
