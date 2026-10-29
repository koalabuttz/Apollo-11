---
id: exec-closure
title: Map every word of EXECUTIVE.agc and WAITLIST.agc to the native routines that run it
date: 2026-10-12T18:00Z
lane: A
---
Every assembled word of EXECUTIVE.agc and WAITLIST.agc in both missions now maps to the native routine milestones whose comparisons with yaAGC ran it. A new tracer (`tools/oracle_trace/`) links into any yaAGC reference and records the fixed words the original executes. The exec milestones' own differentials were rerun under it, and they still pass with unchanged vectors.

Every instruction word of all four files is in a native routine's compared path, apart from named exceptions: the original's own CCSHOLE traps and its CAN'T GET HERE NOOPs.

For the last two LM paths, `lm_basic_wake` now admits a job asleep in core set 0 (a task's JOBWAKE under the dummy job): WAKETEST to SPECTEST. The LM NOVAC and JOBWAKE cases were widened to reach the dormant-job scan and every wake slot. Certificates for all four files are drafted for review. See `docs/EXEC-CLOSURE.md`.
