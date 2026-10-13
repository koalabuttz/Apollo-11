---
id: waitlist-entries
title: WAITLIST entries reached from other files in the transfer ABI
date: 2026-10-13T00:30Z
lane: A
---
The WAITLIST points other files enter are now registered transfer routines at their original addresses: the T3RUPT body of both missions (01,3407 and 01,3416), DLY2 -1 of both (05211 and 05255, entered by DELAYJOB) and Luminary NOQBRSM +1 (05275, entered by the P-axis autopilot). 1,437 cases match yaAGC on all 2048 erasable words, the superbank and the interrupt state through `agc_transfer_run`; the DLY2 -1 cases run DELAYJOB and the Waitlist together, half of them on a sane waitlist so the task is inserted and the job sleeps (45 LM, 52 CM), and 57 more LM cases with the new task due 1 cs before a queued one, so WAIT2's list search ends on CCS -0. The 1,446 strict and sanitizer checks pass, and the C64 rows take every host branch arc. See `docs/WAITLIST-ENTRIES.md`.
