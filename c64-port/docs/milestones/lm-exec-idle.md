---
id: lm-exec-idle
title: Run the Luminary idle loop, SUPDXCHZ and SVCT3 as native routines
date: 2026-10-12T19:00Z
lane: A
---
Luminary099 DUMMYJOB (and DUMMYJOB+2, the restart's entry), ADVAN, NUCHANG2, SUPDXCHZ (and +1) and the Waitlist end task SVCT3 with its two resumptions are routines of the control-transfer ABI (`lm_exec_idle_routines`). A job switch that finds nothing ready now continues natively: ADVAN dispatches the next job, an interpretive job through NEWOPS, or the self-check at SELFRET, and SVCT3 reaches TASKOVER or the NOVAC and FIXDELAY routines. SVCT3 is a new native body (`lm_svct3_phase`). All 4,864 cases equal the original Luminary099 program in yaAGC in all 2,048 erasable words, SUPERBNK, the interrupt-enable bit, channel 11 and InIsr. The 9,748 strict and sanitizer checks pass, and every sixteenth case matches in VICE. See `docs/LM-EXEC-IDLE.md`.
