---
id: cm-tvc-stroke
title: Translate Comanche TVCSTROKETEST (V68 stroke test initialization and HACK waveform)
date: 2026-10-12T16:40Z
lane: G
---
Native C for all of Comanche055 TVCSTROKETEST (bank 20 `3501`-`3611`, 73 words): the V68 STRKTSTI job (TSTINIT, STRKCHK) and the HACK waveform generator with its HACKWLST waitlist task, from each of its 4 original entry points to the next transfer out of the file (ENDOFJOB, WAITLIST, or TC BUNKER). The final image, channel 14 and interrupt enable equal unchanged Comanche055 in the pinned yaAGC for all 65536 sampled entry states, strict and ASAN/UBSAN; all 73 words, 8 fixed constants and the 7-word FCARD table are checked against the rope; a C64 diagnostic passes 128 frozen oracle rows in VICE. See `docs/CM-TVC-STROKE.md`.
