# Run Comanche Executive and Waitlist service entries as native routines

Lane A. Milestone `cm-exec-services`. Interface: `docs/NATIVE_TRANSFER_ABI.md`. Luminary counterpart: `docs/LM-EXEC-SERVICES.md` and `docs/LM-EXEC-JOBS.md`.

The Comanche055 service entries that jobs and tasks call through an inline operand, and the task services, are now routines of the control-transfer ABI:
- NOVAC and FINDVAC (Executive),
- WAITLIST and TWIDDLE (Waitlist),
- the LONGCALL chain,
- FIXDELAY, VARDELAY, TASKOVER and the T3RUPT vector.

The 2CADR (ADRES after TWIDDLE, DEC after FIXDELAY) at Q comes from a site table compiled from the source, or from erasable memory when the caller stored it there. A CM file that calls one of these services can now reach it through the trampoline instead of stopping at its address.

## Scope

- `Comanche055/EXECUTIVE.agc`
  - NOVAC 05134 and FINDVAC 05147, as entered by `TC NOVAC`/`TC FINDVAC` with the 2CADR at Q.
  - The admission itself is `cm_admission_zero` (`docs/COMANCHE_ADMISSION_ZERO.md`). It covers a free core set and VAC area, the job's return to Q+2, and BAILOUT with 1201 or 1202.
- `Comanche055/WAITLIST.agc`
  - WAITLIST 05245 and TWIDDLE 05235. The scheduling is `cm_waitlist_extended` (`docs/COMANCHE_WAITLIST_EXTENDED.md`), through the caller return or BAILOUT 1203 and POODOO 1204.
  - The whole LONGCALL chain: LONGCALL 05342, LGCL2CDR, LNGCALL2 01,3461, LONGCYCL 01,3465, MUCHTIME, LASTTIME, LONGRTRN 01,3505 and its TCF at 01,3514, GETCADR 01,3515 and TSKOVCDR. Comanche's LNGCALL2 has no delta-time check (Luminary's LONGPOOH): it falls straight into LONGCYCL.
  - FIXDELAY 05267 and VARDELAY 05272 from a task, through `cm_waitlist_internal` (`docs/COMANCHE_WAITLIST_INTERNAL.md`), to TASKOVER or BAILOUT 1203.
  - TASKOVER 05324. RUPTAGN +1 dispatches the next due task through T3RUPT2 (`cm_task_transfer`). RUPTAGN -0 runs the RESUME sequence and the RESUME instruction: Z becomes ZRUPT-1 and the interrupt ends (`program->hardware->in_interrupt` cleared).
  - The T3RUPT vector 04014 and T3RUPT 01,3416 lead-in (A,L to ARUPT, BB with SUPERBNK to BANKRUPT, Q to QRUPT), then T3RUPT2.
- Not covered here:
  - Other Executive entries: SPVAC, SPVACIN, JOBSLEEP, JOBWAKE, PRIOCHNG, CHANG1/CHANG2, ENDOFJOB.
  - The bodies of BAILOUT and POODOO (ALARM_AND_ABORT).
  - The SVCT3 end-of-list task, DLY2-1, and the RESUME/NOQRSM/NOQBRSM entries other interrupts use.
  - The jobs and tasks the 2CADRs name, and RESTARTS (now lane A2's).

  Each stops the trampoline with `EXEC_UNMAPPED` at its address until its file registers a routine.

Refused inputs (`EXEC_OUT_OF_SCOPE`, image and io unchanged):
- Inputs with no original producer:
  - a fixed Q that is not a genuine call site, or a Q above 07775;
  - LONGCYCL, LNGCALL2, LONGRTRN or GETCADR outside BB 02003;
  - a LONGEXIT or LONGCADR address of 0 or above 07777;
  - TASKOVER or the vector outside an interrupt, or a RUPTAGN other than +1 or -0;
  - priorities and delays outside the qualified domains of the bodies above.
- Two source paths that would execute a word that is not an instruction. The oracle confirms both, and the case vectors include them:
  - The two LONGCYCL branches that would execute the constant DPBIT14 words (01,3476 or 01,3477): a positive-zero or negative high word after a nonpositive low word. Comanche reaches them from LONGCALL itself with a zero or negative delta.
  - The WTLST5 path whose last CCS sees +0, which executes the erasable word 01203 (`TC 1203`). It happens when later Waitlist intervals are +0 (tasks due together) and the new task falls after all of them. `cm_waitlist_internal` already stopped there. `cm_waitlist_extended` did not: in this state it scheduled the task (or raised 1203), where the original runs into erasable memory. It now returns `CM_WL_SOURCE_OVERFLOW` with the image unchanged. Its own tests still pass, and no released vector reached this state.

## Source map

| AGC | Native |
|---|---|
| NOVAC 05134, FINDVAC 05147 | `novac`, `findvac` → `cm_admission_zero` |
| WAITLIST 05245, TWIDDLE 05235 | `waitlist_call`, `twiddle` → `cm_waitlist_extended` |
| LONGCALL 05342 (DXCH LONGTIME, NDX Q DCA 0, DXCH LONGCADR, DCA LGCL2CDR, DTCB) | `longcall` |
| LNGCALL2 01,3461 | `lngcall2` |
| LONGCYCL 01,3465, MUCHTIME 01,3510, LASTTIME 01,3500 | `longcycl` |
| LONGRTRN 01,3505, TCF LONGRTRN 01,3514, TSKOVCDR | `longrtrn` |
| GETCADR 01,3515 | `getcadr` |
| FIXDELAY 05267, VARDELAY 05272 | `fixdelay`, `vardelay` → `cm_waitlist_internal` |
| TASKOVER 05324, RESUME 05341 | `taskover` → `cm_task_transfer` |
| T3RUPT vector 04014, T3RUPT 01,3416 | `t3rupt` → `cm_task_transfer` |
| Inline operand after every genuine TC NOVAC/FINDVAC/WAITLIST/LONGCALL/TWIDDLE/FIXDELAY | `cm_exec_sites` (`src/cm_exec_sites.inc`, `tools/agc_sites_generate.py cm-exec`) |

`include/cm_exec_routines.h` registers fourteen entries in `cm_exec_routines`. The site table holds 267 words: 129 service sites and 14 FIXDELAY sites. The generator excludes PINBALL's TCNOVAC, TCWAIT and TCFINDVC words, which are constants stored into erasable rather than calls. It includes FRESH_START_AND_RESTART's `TC FINDVAC` with its `OCT 77777` pair.

## Evidence

Hashes: `evidence/cm-exec-services/SHA256.json`.

- **Independent assembly comparison.** `make cm-exec-services-differential` runs the unmodified Comanche055 rope in yaAGC (host-only, `tools/cm_exec_services_reference.c`). Each case runs from its entry to its stop: the caller's return, TASKOVER, the task 2CADR, the next task, the end of the interrupt, BAILOUT/POODOO, DPBIT14 or 01203. Every case equals the native run in its outcome and, unless refused, all 2,048 erasable words, SUPERBNK, the interrupt-enable bit and InIsr. Per-case digests are frozen into `tests/cm_exec_services_vectors.h`.

  | Cohort | Cases | Returned | Alarm | Refused |
  |---|---|---|---|---|
  | Every call site, 32 states | 4,128 | 2,923 | 1,147 | 58 |
  | LONGCYCL and GETCADR tasks | 1,024 | 839 | 136 | 49 |
  | Every FIXDELAY site, 32 states | 448 | 317 | 128 | 3 |
  | VARDELAY | 512 | 439 | 57 | 16 |
  | TASKOVER resume | 512 | 512 | 0 | 0 |
  | TASKOVER and T3RUPT dispatch | 1,024 | 1,024 | 0 | 0 |

  Results: `evidence/cm-exec-services/cm-exec-services-proof.json`. `evidence/cm-exec-services/cm-exec-services-sites.json` lists each site word checked against the source text and the rope.
- **Host and sanitizer results.** `make test-cm-exec-services sanitize-cm-exec-services` (`evidence/cm-exec-services/cm-exec-services-host.log`) checks:
  - the digests;
  - that all fourteen routines and alarms 1201, 1202, 1203 and 1204 are reached;
  - the refusals above, with the image unchanged.
- **C64 execution results.** `make vice-cm-exec-services` (`evidence/cm-exec-services/cm-exec-services-vice.log`) runs every sixteenth case (478).
