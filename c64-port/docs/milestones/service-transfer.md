---
id: service-transfer
title: SERVICE_ROUTINES entries of both missions in the transfer ABI
date: 2026-10-12T23:45Z
lane: A
---
Every SERVICE_ROUTINES entry of both missions is now a registered transfer routine at its original address: UPENT2, DOWNENT2, UPFLAG, DOWNFLAG, DELAYJOB, TCGETCAD, WAKER, GENTRAN, B5OFF, and CM TRFAILOF/TRFAILON. The flag and block-transfer operands of all 323 genuine call-site words are compiled constants. TCGETCAD, WAKER and B5OFF now accept any Q, since they never read it. 1,642 cases (every site twice, plus 512 entry cases) match yaAGC on all 2048 erasable words and the interrupt inhibit, through `agc_transfer_run`. The 1,654 strict, sanitizer and C64 checks pass. This addresses the SERVICE_ROUTINES items of #129. See `docs/SERVICE-TRANSFER.md`.
