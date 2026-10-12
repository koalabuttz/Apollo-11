---
id: lanec-di-dsp
title: Translate the DISPLAY_INTERFACE display-start entries
date: 2026-10-12T14:40Z
lane: C
---
Native C for the display-start entries of DISPLAY_INTERFACE_ROUTINES in both Luminary099 and Comanche055: GODSP, GODSPR, CLEANDSP/REFLASH, REGODSP(R), CLOCPLAY, GOFLASH(R), PRIODSP(R), EXDSPRET, the GOPERF family with PURRS4, the job scheduling in GODSPRS1/VACDSP/BOTHJOBS, SAVELOCS/SAVELOCR and their MAKECADR returns, COPYNORM/COPIES, CHKPRIO and MARKPERF. 3,264 complete states are identical to the unchanged original programs in Virtual AGC, in strict and ASan/UBSan builds, and 218 of them (every entry, both missions) match the oracle on a VICE PAL C64. The Lane C symbol generator now reads constants as assembled words from the listings. Earlier Lane C C64 subsets are refrozen with a stride that reaches every entry in both missions. See `docs/LANEC-DI-DSP.md`.
