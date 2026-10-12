---
id: cm-auto-man
title: Translate Comanche AUTOMATIC_MANEUVERS (CM RCS attitude hold, rate damping and phase-plane jet timing)
date: 2026-10-12T20:00Z
lane: G
---
Native C for all of Comanche055 AUTOMATIC_MANEUVERS (bank 21 `3250`-`3764`, 333 words). This covers attitude hold with its biases, the free and rate-damping modes, the three-axis phase-plane jet-time logic and TAU limits, and the T6PROG hand-off to JETSLECT. Every entry runs to RESUME, REINIT, T6PROGM, STICKCHK, SMALLMP or the next file. The final image equals unchanged Comanche055 in the pinned yaAGC for all 65536 sampled entry states, strict and ASAN/UBSAN. All 333 words, 19 fixed constants, the double-word, 2CADR and INDEX tables, and the TC returns are checked against the listing and the rope. A C64 diagnostic passes 128 frozen oracle rows in VICE. See `docs/CM-AUTO-MAN.md`.
