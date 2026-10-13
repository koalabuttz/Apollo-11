---
id: cm-exec-services
title: Run Comanche Executive and Waitlist service entries as native routines
date: 2026-10-12T17:00Z
lane: A
---
Comanche055 NOVAC, FINDVAC, WAITLIST, TWIDDLE, the LONGCALL chain (LONGCALL, LNGCALL2, LONGCYCL, LONGRTRN, GETCADR), FIXDELAY, VARDELAY, TASKOVER and the T3RUPT vector are routines of the control-transfer ABI. They take their inline operand from a site table compiled from the source (143 call sites, 267 words), so CM files can now reach the Executive and Waitlist through the trampoline. All 7,648 cases equal the original Comanche055 program in all 2,048 erasable words, SUPERBNK, the interrupt-enable bit and InIsr. The cases are 32 states at every genuine call site, the LONGCYCL and GETCADR tasks, VARDELAY, and TASKOVER and T3RUPT resume and dispatch. They reach alarms 1201, 1202, 1203 and 1204, and the two paths where the original executes a non-instruction word (DPBIT14, and erasable 01203). The second path exposed a divergence in `cm_waitlist_extended`, now fixed: it scheduled the task where the original executes 01203. The 15,329 strict and sanitizer checks pass, and every sixteenth case matches in VICE. See `docs/CM-EXEC-SERVICES.md`.
