---
id: cm-exec-idle
title: Run the Comanche idle loop, SUPDXCHZ and SVCT3 as native routines
date: 2026-10-12T18:30Z
lane: A
---
Comanche055 DUMMYJOB (and DUMMYJOB+2, the restart's entry), ADVAN, NUCHANG2, SUPDXCHZ (and +1) and the Waitlist end task SVCT3 with its two resumptions are routines of the control-transfer ABI (`cm_exec_idle_routines`). A job switch that finds nothing ready now continues natively: ADVAN dispatches the next job, an interpretive job through NEWOPS, or the self-check at SELFRET, and SVCT3 reaches TASKOVER or the NOVAC and FIXDELAY routines whose operands are already in the `cm-exec-services` site table. All 5,120 cases, drawn from the qualified idle dispatch, idle recheck, super dispatch and SVCT3 generators, equal the original Comanche055 program in yaAGC in all 2,048 erasable words, SUPERBNK, the interrupt-enable bit and channel 11. The 10,260 strict and sanitizer checks pass, and every sixteenth case matches in VICE. See `docs/CM-EXEC-IDLE.md`.
