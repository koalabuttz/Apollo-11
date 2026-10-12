---
id: cm-tvc-restarts
title: Translate Comanche TVCRESTARTS (TVC restart package, outcounter enables, EXRSTRT)
date: 2026-10-12T16:10Z
lane: G
---
Native C for all of Comanche055 TVCRESTARTS (bank 16 `3140`-`3271`, 90 words): the REDOTVC restart package, the ENABL1/ENABL2/CMDSOUT outcounter enables with their channel 12/14 writes, the PHSCHK2 and CHKSTRK restart choices, FINCOPY, TRIM/CMD and the EXRSTRT task, from each of its 6 original entry points to the next transfer out of the file (WAITLIST, RESUME, NOQRSM, POSTJUMP, IBNKCALL, ISWCALL or a TVCEXECUTIVE restart point). The final image and channels 12 and 14 equal unchanged Comanche055 in the pinned yaAGC for all 65536 sampled entry states, strict and ASAN/UBSAN; all 90 words, 14 fixed constants and the 11-word TVCCADR/TVCEXADR table are checked against the rope; a C64 diagnostic passes 128 frozen oracle rows in VICE. See `docs/CM-TVC-RESTARTS.md`.
