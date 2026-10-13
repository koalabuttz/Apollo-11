---
id: waitlist-entries
title: WAITLIST entries reached from other files in the transfer ABI
date: 2026-10-13T00:30Z
lane: A
---
The WAITLIST points other files enter are now registered transfer routines at their original addresses: the T3RUPT body of both missions (01,3407 and 01,3416), DLY2 -1 of both (05211 and 05255, entered by DELAYJOB) and Luminary NOQBRSM +1 (05275, entered by the P-axis autopilot). 1,266 cases match yaAGC on all 2048 erasable words, the superbank and the interrupt state through `agc_transfer_run`; the DLY2 -1 cases run DELAYJOB and the Waitlist together. The 1,271 strict, sanitizer and C64 checks pass. See `docs/WAITLIST-ENTRIES.md`.
