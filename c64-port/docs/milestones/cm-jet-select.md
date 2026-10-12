---
id: cm-jet-select
title: Translate Comanche JET_SELECTION_LOGIC (CM RCS jet selection T5 task and T6 jet sequencing)
date: 2026-10-12T21:00Z
lane: G
---
Native C for all of Comanche055 JET_SELECTION_LOGIC (bank 17 `2552`-`3736`, 629 words): the JETSLECT T5 task that picks the pitch, yaw and roll jets from the translation command, rotation times and jet failures, sizes the blasts and sets up T6, and the T6START task that fires the jet words through channels 5 and 6. Both entries run to RESUME. The final image and channels 5/6/13 equal unchanged Comanche055 in the pinned yaAGC for all 65536 sampled entry states, strict and ASAN/UBSAN. All 629 words, 97 fixed constants, RCS2CADR, the seven INDEX tables and the REPLACER returns are checked against the listing and the rope. A C64 diagnostic passes 128 frozen oracle rows in VICE. See `docs/CM-JET-SELECT.md`.
