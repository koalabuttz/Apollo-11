---
id: lm-exec-jobs
title: Run Luminary Executive job switches and Waitlist task services as native routines
date: 2026-10-12T14:30Z
lane: A
---
Luminary099 SPVACIN, SPVAC, CHANG1, CHANG2, JOBSLEEP, JOBWAKE, PRIOCHNG and ENDOFJOB, plus FIXDELAY, VARDELAY, TASKOVER and the T3RUPT vector, are routines of the control-transfer ABI. A job switch now ends at the selected job's entry (or NEWOPS for an interpretive job, or the idle loop), and a task's TASKOVER ends at the next due task or back in the interrupted program. JOBSLEEP and PRIOCHNG also switch to interpretive jobs, which their image bodies did not. FIXDELAY takes its delay from the compiled site table at all 16 genuine call sites. All 9,216 cases equal the original Luminary099 program in all 2,048 erasable words, SUPERBNK, the interrupt-enable bit, channel 11 and the interrupt flag, and they reach alarms 1201, 1202, 1203 and 1204. The 18,463 strict and sanitizer checks pass, and every sixteenth case matches in VICE, in three C64 programs. See `docs/LM-EXEC-JOBS.md`.
