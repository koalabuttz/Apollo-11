---
id: cm-tvc-roll
title: Translate Comanche TVCROLLDAP (CM roll DAP during TVC: phase plane, firing time, roll jets)
date: 2026-10-12T17:50Z
lane: G
---
Native C for all of Comanche055 TVCROLLDAP (bank 16 `3272`-`3717`, 278 words): the ROLLDAP waitlist task (roll rate from OGANOW/OGAPAST, the six-region phase-plane switching logic, the firing-time division and limits, roll jet selection on channel 6 and the T6 hand-off) and the NOROLL1 T6 task, from both entry points to TASKOVER or NOQRSM. The final image and channels 6 and 13 equal unchanged Comanche055 in the pinned yaAGC for all 65536 sampled entry states, strict and ASAN/UBSAN; all 278 words, 47 fixed constants and the NOROL1T6/LIMITS words are checked against the rope; a C64 diagnostic passes 128 frozen oracle rows in VICE. See `docs/CM-TVC-ROLL.md`.
