# Run Luminary Executive job switches and Waitlist task services as native routines

Lane A. Milestone `lm-exec-jobs`. Interface: `docs/NATIVE_TRANSFER_ABI.md`. Follows `lm-exec-services` (`docs/LM-EXEC-SERVICES.md`).

The rest of the Luminary099 Executive's job entries and the Waitlist's task-side entries are now routines of the control-transfer ABI. Each routine composes an image body that an earlier milestone qualified against the original program, and ends where the original continues:
- a job switch ends at the selected job's basic entry, at NEWOPS 06070 with LOC and BANKSET set for an interpretive job, or at ADVAN 01,3214 (the idle loop) with COMP ACTY turned off;
- SPVAC, JOBWAKE and a PRIOCHNG that keeps the job end at the caller's return;
- FIXDELAY and VARDELAY end at TASKOVER; TASKOVER ends back in the interrupted program (RUPTAGN -0) or at the next due task (+1); the T3 vector ends at the dispatched task;
- the alarms end at BAILOUT1 or POODOO1 with the source code.

## Scope

- `Luminary099/EXECUTIVE.agc`, in `lm_exec_job_routines` (`src/lm_exec_jobs.c`):

  | Entry | Body |
  |---|---|
  | SPVACIN 05112, SPVAC 05116 | `lm_admission_image` (`docs/LM-CANONICAL-ADMISSION.md`) |
  | CHANG1 05122 | `lm_basic_change` (`docs/LM-BASIC-CHANGE.md`) |
  | CHANG2 05126 | `lm_interpreter_change` (`docs/NATIVE-CANONICAL-EXCHANGE.md`) |
  | JOBSLEEP 05133 | `lm_basic_sleep` (`docs/LM-BASIC-SLEEP.md`) |
  | JOBWAKE 05137 | `lm_basic_wake` |
  | PRIOCHNG 05146 | `lm_basic_priority_change` |
  | ENDOFJOB 05155 | `lm_basic_finish` (`docs/NATIVE-SOURCE-DISPATCH.md`) |

  JOBSLEEP and PRIOCHNG reach CHANJOB through the same EJSCAN choice as ENDOFJOB, so they can switch to an interpretive job. Their image bodies select only basic jobs. The routines therefore turn the winning core set's negative LOC into the INTRSM resume address first (ENDPRCHG's COM, AD ONE, TS LOC), run the basic body, and then apply INTRSM's BANKSET, INTBIT15 and EDOP setup and go to NEWOPS. ENDOFJOB's body already did this. The oracle matches all 320 such cases.
- `Luminary099/WAITLIST.agc`, in `lm_exec_task_routines` (`src/lm_exec_tasks.c`):
  - FIXDELAY 05221 and VARDELAY 05224 through `lm_delay_image` (the DLY2/WAIT2 insertion, LVWTLIST to TASKOVER, or 1203/1204). FIXDELAY's `DEC` delay comes from the compiled site table at all 16 genuine call sites (`tools/agc_sites_generate.py lm-exec` now includes FIXDELAY), or from erasable when Q is erasable.
  - TASKOVER 05261: RUPTAGN -0 through `lm_taskover_resume` (restore SUPERBNK, QRUPT, BBANK and ARUPT, RELINT, RESUME at ZRUPT-1, which clears the interrupt flag); RUPTAGN +1 through T3RUPT2 (`lm_waitlist_dispatch`).
- `Luminary099/INTERRUPT_LEAD_INS.agc`: the T3RUPT vector 04014 and T3RUPT through `lm_waitlist_dispatch`, or `lm_waitlist_irq_dispatch` when interrupts are enabled.
- The ABI gains `agc_hardware` (`program->hardware`): channel 11 and the interrupt-in-progress flag. JOBSLEEP, ENDOFJOB, TASKOVER and the T3 vector refuse without it.
- Not covered here:
  - In EXECUTIVE: the DUMMYJOB/ADVAN idle loop, NUDIRECT and NUCHANG2.
  - In WAITLIST: the SVCT3 end-of-list task, and the RESUME, NOQRSM and NOQBRSM entries that other interrupt programs exit through.
  - The other interrupt vectors, BAILOUT1 and POODOO1, and the jobs and tasks that are switched to.

Refused inputs (`EXEC_OUT_OF_SCOPE`, image unchanged), with no original producer:
- Inputs outside the image bodies' qualified domains.
- No `program->hardware` for a routine that needs it.
- A fixed FIXDELAY Q that is not a genuine call site.
- An interpretive successor LOC whose resume address would pass 07777.

## Source map

| AGC | Native |
|---|---|
| SPVACIN 05112 (DXCH NEWLOC ... FINDVAC2), SPVAC 05116 (Q-2, SPVACIN) | `spvac` |
| CHANG1 05122 → CHANJOB | `chang1` |
| CHANG2 05126 → CHANJOB -1 | `chang2` |
| JOBSLEEP 05133 → JOBSLP1, EJSCAN, CHANJOB/ENDPRCHG or ADVAN | `jobsleep` |
| JOBWAKE 05137 → JOBWAKE2, ENDFIND | `jobwake` |
| PRIOCHNG 05146 → PRIOCH2, EJSCAN, CHANJOB/ENDPRCHG or the caller | `priochng` |
| ENDOFJOB 05155 → ENDJOB1, EJSCAN, CHANJOB/ENDPRCHG or ADVAN | `endofjob` |
| ENDPRCHG negative LOC → INTRSM → NEWOPS 06070 | `interpretive_in`, `intrsm` |
| FIXDELAY 05221 (INDEX Q, CAF 0, INCR Q), VARDELAY 05224 → DLY2, WAIT2, LVWTLIST/DELAYEX | `fixdelay`, `vardelay` |
| TASKOVER 05261 (CCS RUPTAGN) → T3RUPT2 or RESUME | `taskover` |
| T3RUPT vector 04014 → T3RUPT 01,3407 → T3RUPT2 | `t3rupt` |

## Evidence

Hashes: `evidence/lm-exec-jobs/SHA256.json`. Bulk files are assets of the `evidence-lm-exec-jobs` release
(`python3 tools/evidence.py fetch lm-exec-jobs`).

- **Independent assembly comparison.** `make lm-exec-jobs-differential` runs the unmodified Luminary099 rope in yaAGC (host-only, `tools/lm_exec_jobs_reference.c`). Each case starts at its entry with the same image, SUPERBNK, interrupt-enable bit, channel 11 and InIsr, and runs until Z reaches one of the case's stops (the caller's return, a job entry, NEWOPS, ADVAN, TASKOVER, a task), the interrupt ends, or BAILOUT1/POODOO1. Every case equals the native run in its outcome, all 2,048 erasable words, SUPERBNK, the interrupt-enable bit, channel 11 and InIsr. Per-case digests are frozen into `tests/lm_exec_jobs_vectors.h`. Inputs come from the qualified image milestones' own input domains. The Waitlist domain is sampled below case 7560, where its generator's arithmetic still fits the C64's 16-bit `int`.

  | Cohort | Cases | Basic job or return | Interpretive job | Idle | Alarm |
  |---|---|---|---|---|---|
  | SPVAC, SPVACIN | 1,024 | 761 | | | 263 |
  | CHANG1 | 768 | 388 | 380 | | |
  | CHANG2 | 768 | 384 | 384 | | |
  | JOBSLEEP | 1,024 | 654 | 109 | 261 | |
  | JOBWAKE | 512 | 512 | | | |
  | PRIOCHNG | 1,024 | 813 | 211 | | |
  | ENDOFJOB | 1,024 | 475 | 289 | 260 | |
  | FIXDELAY/VARDELAY | 1,024 | 720 at TASKOVER | | | 304 |
  | FIXDELAY at its 16 call sites | 512 | 464 at TASKOVER | | | 48 |
  | TASKOVER resume | 512 | 512 | | | |
  | T3 vector and TASKOVER +1 | 1,024 | 1,024 tasks | | | |

  The JOBWAKE cases put the matching sleeping job in every core set 1-7, so WAKETEST's match path runs (exec-closure needs it).

  The alarms are 1201 (169), 1202 (94), 1203 (214) and 1204 (138). Results: `evidence/lm-exec-jobs/lm-exec-jobs-proof.json`.
- **Host and sanitizer results.** `make test-lm-exec-jobs sanitize-lm-exec-jobs` (`evidence/lm-exec-jobs/lm-exec-jobs-host.log`) checks:
  - the digests;
  - that all 12 entries and all four alarms are reached, along with interpretive successors after JOBSLEEP and PRIOCHNG, idle after JOBSLEEP and ENDOFJOB, RESUME clearing the interrupt flag, TASKOVER dispatching, and T3 dispatch in both interrupt states;
  - the refusals above, with the image, io and hardware unchanged.

  `make test-lm-exec-services` still passes with the FIXDELAY sites in the shared table.
- **C64 execution results.** `make vice-lm-exec-jobs` (`evidence/lm-exec-jobs/lm-exec-jobs-vice.log`) runs every sixteenth case (576) in three programs: the admission and change cohorts, the switch cohorts, and the task cohorts. All the job routines together do not fit below the `$c000` mailbox.
