---
id: cm-tvc-exec
title: Translate Comanche TVCEXECUTIVE (TVCEXEC task, roll ladders, gain/mass and trim updates)
date: 2026-10-12T15:20Z
lane: G
---
Native C for all of Comanche055 TVCEXECUTIVE (bank 16 `2720`-`3137`, 144 words): the half-second TVCEXEC task, roll ladders and FDAI error, needles, variable gains and mass, one-shot and repetitive trim correction, and the TVCRESTARTS re-entries, from each of its 12 original continuation points to the next transfer out of the file (WAITLIST, IBNKCALL or TASKOVER). The final image and IRQ equal unchanged Comanche055 in the pinned yaAGC for all 65536 sampled entry states, strict and ASAN/UBSAN (860 spin on the source's `TCF +0` for a negative VCNTR in both); all 144 words and 11 fixed constants are checked against the rope; a C64 diagnostic passes 128 frozen oracle rows in VICE. See `docs/CM-TVC-EXEC.md`.
