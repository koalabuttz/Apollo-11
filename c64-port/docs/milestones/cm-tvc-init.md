---
id: cm-tvc-init
title: Translate Comanche TVCINITIALIZE (TVC DAP initialization, CSM/LM switch-over, LOADCOEF, S40.15)
date: 2026-10-12T17:10Z
lane: G
---
Native C for all of Comanche055 TVCINITIALIZE (bank 17 `2147`-`2551`, 259 words): TVCDAPON and MRCLEAN, TVCINIT1-4 with ATTINIT and NEEDLEIN, the DAPINIT T5 hand-off, the V46 CSM/LM switch-over (PRESWTCH, SWICHOVR, MCLEANJR), LOADCOEF and S40.15 with their coefficient tables, from each of its 9 original entry points to the next transfer out of the file. The final image, channel 11 and interrupt enable equal unchanged Comanche055 in the pinned yaAGC for all 65536 sampled entry states, strict and ASAN/UBSAN; all 259 words, 28 fixed constants and the coefficient, 2CADR and INDEX tables are checked against the rope; a C64 diagnostic passes 128 frozen oracle rows in VICE. See `docs/CM-TVC-INIT.md`.
