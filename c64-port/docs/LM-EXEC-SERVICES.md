# Run Luminary Executive and Waitlist service entries as native routines

Lane A. Milestone `lm-exec-services`. Interface: `docs/NATIVE_TRANSFER_ABI.md`.

The Luminary099 service entries that jobs, tasks and restarts call through an inline 2CADR are now routines of the control-transfer ABI:
- NOVAC and FINDVAC (Executive),
- WAITLIST and TWIDDLE (Waitlist),
- the LONGCALL chain.

The 2CADR (or ADRES) at Q comes from a site table compiled from the source, or from erasable memory when the caller stored it there, as RESTARTS does at GOLOC. RESTARTS therefore runs through the job or task admission it selects, to SWRETURN.

## Scope

- `Luminary099/EXECUTIVE.agc`
  - NOVAC 05072 and FINDVAC 05105, as entered by `TC NOVAC`/`TC FINDVAC` with the 2CADR at Q.
  - The admission itself is `lm_admission_image` (`docs/LM-CANONICAL-ADMISSION.md`). It covers a free core set and VAC area, the job's return to Q+2, and BAILOUT1 with 1201 or 1202.
- `Luminary099/WAITLIST.agc`
  - WAITLIST 05203 and TWIDDLE 05173. The scheduling is `lm_waitlist_image` (`docs/LM-CANONICAL-WAITLIST.md`), through the caller return or BAILOUT1 1203 and POODOO1 1204.
  - The whole LONGCALL chain: LONGCALL 05277, LGCL2CDR, LNGCALL2 01,3452 with the delta-time check, LONGPOOH, LONGCYCL 01,3466, MUCHTIME, LASTTIME, LONGRTRN 01,3506 and its TCF at 01,3515, GETCADR 01,3516 and TSKOVCDR.
- `Luminary099/RESTARTS_ROUTINE.agc` (`lm-restart-select`): RESTARTS now composes with these services rather than test stubs.
- Not covered here:
  - Other Executive entries: SPVAC, JOBSLEEP, JOBWAKE, PRIOCHNG, CHANG1/CHANG2, ENDOFJOB.
  - The T3RUPT/TASKOVER dispatch, VARDELAY and FIXDELAY.
  - The bodies of BAILOUT1 and POODOO1 (ALARM_AND_ABORT).
  - The jobs and tasks the 2CADRs name.

  Each stops the trampoline with `EXEC_UNMAPPED` at its address until its file registers a routine.

Refused inputs (`EXEC_OUT_OF_SCOPE`, image unchanged), with no original producer:
- A fixed Q that is not a genuine call site, or a Q above 07775.
- LONGCYCL, LNGCALL2, LONGRTRN or GETCADR outside BB 02003.
- A LONGEXIT or LONGCADR address of 0 or above 07777.
- The two LONGCYCL branches that would execute the constant DPBIT14 words: a positive-zero or negative high word after a nonpositive low word.

  The oracle confirms all 38 such task cases reach 01,3477 or 01,3500.
- Priorities and delays outside the qualified domains of `lm_admission_image` and `lm_waitlist_image`.

  The source's dummy 1.2SPOT (shared by groups 2 and 3) is one such input. Its second part is `DEC 100` with a positive `2CADR TASKOVER`, so RESTARTS would admit TASKOVER as a job of priority 0144. The test checks the refusal; the composition cases skip that phase.

## Source map

| AGC | Native |
|---|---|
| NOVAC 05072, FINDVAC 05105 | `novac`, `findvac` → `lm_admission_image` |
| WAITLIST 05203, TWIDDLE 05173 | `waitlist_call`, `twiddle` → `lm_waitlist_image` |
| LONGCALL 05277 (DXCH LONGTIME, NDX Q DCA 0, DXCH LONGCADR, DCA LGCL2CDR, DTCB) | `longcall` |
| LNGCALL2 01,3452, delta check, LONGPOOH 01,3521 | `lngcall2` |
| LONGCYCL 01,3466, MUCHTIME 01,3511, LASTTIME 01,3501 | `longcycl` |
| LONGRTRN 01,3506, TCF LONGRTRN 01,3515, TSKOVCDR | `longrtrn` |
| GETCADR 01,3516 | `getcadr` |
| Inline 2CADR/ADRES after every genuine TC NOVAC/FINDVAC/WAITLIST/LONGCALL/TWIDDLE | `lm_exec_sites` (`src/lm_exec_sites.inc`, `tools/agc_sites_generate.py lm-exec`) |

`include/lm_exec_routines.h` registers ten entries in `lm_exec_routines`.

The site generator excludes PINBALL's TCNOVAC, TCWAIT and TCFINDVC words, which are constants stored into erasable rather than calls. It includes STARTSIM's `TC FINDVAC` with its `OCT 77777` pair.

## Evidence

Hashes: `evidence/lm-exec-services/SHA256.json`. Bulk files are assets of the `evidence-lm-exec-services` release
(`python3 tools/evidence.py fetch lm-exec-services`).

- **Independent assembly comparison.** `make lm-exec-services-differential` runs the unmodified Luminary099 rope in yaAGC (host-only, `tools/lm_exec_services_reference.c`). Each case runs from its entry to its stop: SWRETURN, the caller's return, TASKOVER, the task 2CADR, BAILOUT1/POODOO1, or DPBIT14. Every case equals the native run in its outcome and all 2,048 erasable words, and per-case digests are frozen into `tests/lm_exec_services_vectors.h`.

  | Cohort | Returned | Alarm | Refused |
  |---|---|---|---|
  | 4,096 restart compositions | 3,272 | 824 | 0 |
  | 4,088 call-site runs | 3,014 | 1,074 | 0 |
  | 1,024 task runs | 849 | 137 | 38 |

  A third of the NOVAC and FINDVAC call-site runs pass a dormant (sleeping) job in the core-set scan, which runs NOVAC3's negative-priority branch.

  Results: `evidence/lm-exec-services/lm-exec-services-proof.json`. `evidence/lm-exec-services/lm-exec-services-sites.json` lists each site word checked against the source text and the rope.
- **Host and sanitizer results.** `make test-lm-exec-services sanitize-lm-exec-services` (`evidence/lm-exec-services/lm-exec-services-host.log`) checks:
  - the digests;
  - that all ten routines and alarms 1201, 1202, 1203, WAITLIST 1204 and LONGPOOH 1204 are reached;
  - the refusals above.
- **C64 execution results.** `make vice-lm-exec-services` (`evidence/lm-exec-services/lm-exec-services-vice.log`) runs every sixteenth case (576).
