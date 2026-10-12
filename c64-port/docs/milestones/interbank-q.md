---
id: interbank-q
title: INTER-BANK entries reached with an unusual Q (SWCALL -1, BANKJUMP from TC L)
date: 2026-10-12T23:00Z
lane: A
---
`SWCALL -1`, the `INCR Q` that every RTB enters through, is now a registered transfer routine in both missions, and CM BANKJUMP takes Q below 0100 as LM's does since #101 (DISPLAY_INTERFACE's SAVELOCS reaches it by `TC L`). In 1,024 cases across both missions and both entries, native runs through `agc_transfer_run` equal yaAGC on all 2048 erasable words at the callee. The 1,030 strict, sanitizer and C64 checks pass. The proof lists every native source it depends on. This addresses the INTER-BANK items of #129. See `docs/INTERBANK-Q.md`.
