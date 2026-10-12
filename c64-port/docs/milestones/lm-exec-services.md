---
id: lm-exec-services
title: Run Luminary Executive and Waitlist service entries as native routines
date: 2026-10-12T04:30Z
lane: A
---
Luminary099 NOVAC, FINDVAC, WAITLIST, TWIDDLE and the WAITLIST.agc LONGCALL chain (LONGCALL, LNGCALL2, LONGCYCL, LONGRTRN, GETCADR) are routines of the control-transfer ABI. They take their inline 2CADR or ADRES from a site table compiled from the source (146 call sites, 268 words), so RESTARTS now reaches real job and task admission instead of stubs. All 9,208 cases equal the original Luminary099 program in all 2,048 erasable words. The cases are 4,096 RESTARTS-to-SWRETURN compositions, 28 states at every genuine call site, and 1,024 LONGCYCL and GETCADR task runs, and they reach alarms 1201, 1202, 1203 and both 1204s. The 18,441 strict and sanitizer checks pass, and every sixteenth case matches in VICE. See `docs/LM-EXEC-SERVICES.md`.
