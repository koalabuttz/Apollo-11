# Translate Comanche INTEGRATION_INITIALIZATION

Lane G (CM flight). Milestone `cm-integration-init`. How the routines are made and checked: `docs/INTERP-COMPILE.md` (whole-file basic code: `tools/basic_compile.py`).

`Comanche055/INTEGRATION_INITIALIZATION.agc` (`src/cm_integration_init.c`) is native C in full: State-vector integration set-up and exit: INTSTALL and the INTWAKE family, SETIFLGS, STATEINT, INTEGRV, the CSM/LM state moves (MOVEACSM, PTOACSM, ...), AVETOMID, MIDTOAV1/2, TIMEINC and INTEXIT, basic and interpretive. Each routine runs the file from its entry, or from where it resumes, to the point where control leaves the file. Every original word is one call of the basic-instruction primitives (`src/agc_image_ops.c`) under a label named for its address, with TS overflow skips, CCS four-way skips and INDEX on the whole word kept; computed transfers (TC Q, indexed TCF) go through one dispatch on the target address.

## Scope

235 routines, keyed by physical address in `cm_integration_init_basic` and `cm_integration_init_interpretive`:

| Entry | Kind | Why it is a routine |
|---|---|---|
| 11,2363 ALOADED | interpretive | SSP ALOADED |
| 11,2365  | interpretive | CHANG2 resume |
| 11,2366  | interpretive | CHANG2 resume |
| 13,2627 STATEINT | basic | entry STATEINT (RESTART_TABLES.agc -GENADR STATEINT) |
| 13,2631  | basic | return from PHASCHNG |
| 13,2635  | basic | return from FINDVAC |
| 13,2636 STATINT1 | basic | entry STATINT1 (RESTART_TABLES.agc 2CADR STATINT1) |
| 13,2637  | interpretive | after TC INTPRET |
| 13,2643  | interpretive | after RTB LOADTIME |
| 13,2644  | interpretive | CHANG2 resume |
| 13,2647  | interpretive | CALL return |
| 13,2652  | interpretive | CALL return |
| 13,2656 ENDINT | interpretive | entry ENDINT (FRESH_START_AND_RESTART.agc  ENDINT) |
| 13,2660  | basic | after EXIT |
| 13,2662  | basic | return from PHASCHNG |
| 13,2667  | basic | return from LONGCALL |
| 13,2670 SETIFLGS | interpretive | entry SETIFLGS (FRESH_START_AND_RESTART.agc  SETIFLGS) |
| 13,2673  | interpretive | CHANG2 resume |
| 13,2676  | interpretive | CHANG2 resume |
| 13,2677 NOINT | interpretive | switch branch target |
| 13,2700  | basic | after EXIT |
| 13,2702  | basic | return from PHASCHNG |
| 13,2704  | basic | return from DOWNFLAG |
| 13,2705 ATOPCSM | interpretive | entry ATOPCSM (ORBITAL_INTEGRATION.agc  ATOPCSM) |
| 13,2710  | interpretive | after RTB MOVEACSM |
| 13,2713  | interpretive | CALL return |
| 13,2720 MOVEACSM | basic | entry MOVEACSM (MEASUREMENT_INCORPORATION.agc  MOVEACSM) |
| 13,2731 PTOACSM | interpretive | entry PTOACSM (EXTENDED_VERBS.agc  PTOACSM) |
| 13,2735 CLRMOON | interpretive | entry CLRMOON (ORBITAL_INTEGRATION.agc  CLRMOON) |
| 13,2741  | interpretive | CHANG2 resume |
| 13,2742 SETMOON | interpretive | entry SETMOON (ORBITAL_INTEGRATION.agc  SETMOON) |
| 13,2746  | interpretive | CHANG2 resume |
| 13,2747 MOVEPCSM | basic | entry MOVEPCSM (MEASUREMENT_INCORPORATION.agc  MOVEPCSM) |
| 13,2760 ATOPLEM | interpretive | entry ATOPLEM (ORBITAL_INTEGRATION.agc  ATOPLEM) |
| 13,2763  | interpretive | after RTB MOVEALEM |
| 13,2766  | interpretive | CALL return |
| 13,2773 MOVEALEM | basic | entry MOVEALEM (MEASUREMENT_INCORPORATION.agc  MOVEALEM) |
| 13,3004 PTOALEM | interpretive | GOTO target |
| 13,3010  | interpretive | after RTB MOVEPLEM |
| 13,3014 MOVEPLEM | basic | entry MOVEPLEM (MEASUREMENT_INCORPORATION.agc  MOVEPLEM) |
| 13,3025 USEPIOS | interpretive | switch branch target |
| 13,3030  | interpretive | CHANG2 resume |
| 13,3032  | interpretive | CHANG2 resume |
| 13,3034  | interpretive | CHANG2 resume |
| 13,3036  | interpretive | CALL return |
| 13,3040  | interpretive | CHANG2 resume |
| 13,3042  | interpretive | CHANG2 resume |
| 13,3044  | interpretive | CHANG2 resume |
| 13,3047  | interpretive | CALL return |
| 13,3052  | interpretive | CHANG2 resume |
| 13,3054  | interpretive | CHANG2 resume |
| 13,3055  | interpretive | CHANG2 resume |
| 13,3060  | interpretive | CHANG2 resume |
| 13,3071 CSMPREC | interpretive | entry CSMPREC (P34-P35,_P74-P75.agc  CSMPREC) |
| 13,3074  | interpretive | CALL return |
| 13,3077 IFLAGP | interpretive | switch branch target |
| 13,3102  | interpretive | CHANG2 resume |
| 13,3105 LEMPREC | interpretive | entry LEMPREC (P34-P35,_P74-P75.agc  LEMPREC) |
| 13,3110  | interpretive | CALL return |
| 13,3114 CSMCONIC | interpretive | entry CSMCONIC (TPI_SEARCH.agc  CSMCONIC) |
| 13,3117  | interpretive | CALL return |
| 13,3122 IFLAGC | interpretive | switch branch target |
| 13,3126 LEMCONIC | interpretive | entry LEMCONIC (TPI_SEARCH.agc  LEMCONIC) |
| 13,3131  | interpretive | CALL return |
| 13,3135 INTEGRVS | interpretive | entry INTEGRVS (GROUND_TRACKING_DETERMINATION_PROGRAM_-_P21.agc  INTEGRVS) |
| 13,3141  | interpretive | CHANG2 resume |
| 13,3146  | interpretive | switch branch target |
| 13,3151  | interpretive | CHANG2 resume |
| 13,3152  | interpretive | CHANG2 resume |
| 13,3154  | interpretive | STCALL return |
| 13,3157  | interpretive | CHANG2 resume |
| 13,3162 INTEGRV | interpretive | entry INTEGRV (FRESH_START_AND_RESTART.agc  INTEGRV) |
| 13,3164 INTEGRV1 | interpretive | CHANG2 resume |
| 13,3167 INTEGRV2 | interpretive | entry INTEGRV2 (ORBITAL_INTEGRATION.agc  INTEGRV2) |
| 13,3172  | interpretive | CHANG2 resume |
| 13,3176 A-PCHK | interpretive | BPL target |
| 13,3202  | interpretive | CALL return |
| 13,3205 ANDOUT | interpretive | switch branch target |
| 13,3210  | basic | after EXIT |
| 13,3212  | basic | return from PHASCHNG |
| 13,3214  | basic | return from UPFLAG |
| 13,3215  | interpretive | after TC INTPRET |
| 13,3220  | interpretive | CHANG2 resume |
| 13,3224 PHEXIT | interpretive | SSP PHEXIT |
| 13,3226 RECTOUT | interpretive | switch branch target |
| 13,3231  | interpretive | CALL return |
| 13,3234  | interpretive | CHANG2 resume |
| 13,3237  | interpretive | CHANG2 resume |
| 13,3242  | interpretive | CHANG2 resume |
| 13,3245  | interpretive | CHANG2 resume |
| 13,3247  | interpretive | CHANG2 resume |
| 13,3253 INTEXIT | interpretive | switch branch target |
| 13,3256  | interpretive | BOV target |
| 13,3260  | interpretive | CHANG2 resume |
| 13,3263  | interpretive | CHANG2 resume |
| 13,3265  | basic | after EXIT |
| 13,3271 RVCON | interpretive | GOTO target |
| 13,3274  | interpretive | CHANG2 resume |
| 13,3276  | interpretive | STCALL return |
| 13,3300  | interpretive | CALL return |
| 13,3303  | interpretive | CHANG2 resume |
| 13,3305 TESTLOOP | interpretive | entry TESTLOOP (ORBITAL_INTEGRATION.agc  TESTLOOP) |
| 13,3312  | interpretive | switch branch target |
| 13,3315  | interpretive | CHANG2 resume |
| 13,3317  | interpretive | CHANG2 resume |
| 13,3321  | interpretive | CHANG2 resume |
| 13,3324  | interpretive | CHANG2 resume |
| 13,3326 NORFINAL | interpretive | BMN target |
| 13,3331  | interpretive | CHANG2 resume |
| 13,3333  | interpretive | CHANG2 resume |
| 13,3335  | interpretive | CHANG2 resume |
| 13,3336  | interpretive | CHANG2 resume |
| 13,3341  | interpretive | CHANG2 resume |
| 13,3343  | interpretive | CHANG2 resume |
| 13,3346 DT/2COMP | interpretive | CHANG2 resume |
| 13,3351  | interpretive | CHANG2 resume |
| 13,3354  | interpretive | after RTB SGNAGREE |
| 13,3355  | interpretive | CHANG2 resume |
| 13,3357  | interpretive | CHANG2 resume |
| 13,3362 USEMAXDT | interpretive | CHANG2 resume |
| 13,3365  | interpretive | CHANG2 resume |
| 13,3367 MAXDT | interpretive | BOV target |
| 13,3371  | interpretive | CHANG2 resume |
| 13,3373 GETMAXDT | interpretive | BOV target |
| 13,3375  | interpretive | after RTB SIGNMPAC |
| 13,3377 P00HCHK | interpretive | BMN target |
| 13,3401  | interpretive | CHANG2 resume |
| 13,3404  | interpretive | CHANG2 resume |
| 13,3407  | interpretive | CHANG2 resume |
| 13,3411  | interpretive | BHIZ target |
| 13,3414  | interpretive | CHANG2 resume |
| 13,3417  | interpretive | CHANG2 resume |
| 13,3423  | interpretive | CHANG2 resume |
| 13,3426  | interpretive | CHANG2 resume |
| 13,3430  | interpretive | CHANG2 resume |
| 13,3432  | interpretive | CHANG2 resume |
| 13,3433  | interpretive | CHANG2 resume |
| 13,3442 INTSTALL | interpretive | entry INTSTALL (FRESH_START_AND_RESTART.agc  INTSTALL) |
| 13,3443  | basic | after EXIT |
| 13,3455 INTWAKE0 | interpretive | entry INTWAKE0 (P30,P37.agc  INTWAKE0) |
| 13,3456  | basic | after EXIT |
| 13,3457 INTWAKE | basic | entry INTWAKE (EXTENDED_VERBS.agc CADR INTWAKE) |
| 13,3470  | basic | return from PHASCHNG |
| 13,3477 INTWAKE1 | basic | entry INTWAKE1 (P76.agc CADR INTWAKE1) |
| 13,3505  | basic | return from JOBWAKE |
| 13,3522  | interpretive | after TC INTPRET |
| 13,3523 ERASTAL1 | interpretive | entry ERASTAL1 |
| 13,3524  | basic | after EXIT |
| 13,3526 ERASTAL2 | interpretive | entry ERASTAL2 |
| 13,3527  | basic | after EXIT |
| 13,3531 ERASWAK1 | basic | entry ERASWAK1 |
| 13,3533 ERASWAK2 | basic | entry ERASWAK2 |
| 13,3543 MIDTOAV2 | interpretive | entry MIDTOAV2 (P40-P47.agc  MIDTOAV2) |
| 13,3546  | interpretive | CALL return |
| 13,3551  | interpretive | CHANG2 resume |
| 13,3553 MIDTOAV1 | interpretive | entry MIDTOAV1 (P40-P47.agc  MIDTOAV1) |
| 13,3556  | interpretive | CALL return |
| 13,3561  | interpretive | after RTB LOADTIME |
| 13,3564  | interpretive | CHANG2 resume |
| 13,3567 ENTMID2 | interpretive | CALL return |
| 13,3572  | interpretive | after RTB LOADTIME |
| 13,3573 ENTMID1 | interpretive | BPL target |
| 13,3576  | interpretive | CALL return |
| 13,3601  | interpretive | CHANG2 resume |
| 13,3603  | interpretive | CALL return |
| 13,3606  | interpretive | CHANG2 resume |
| 13,3611  | interpretive | CHANG2 resume |
| 13,3614  | interpretive | BZE target |
| 13,3616  | interpretive | CHANG2 resume |
| 13,3620  | interpretive | CHANG2 resume |
| 13,3622  | interpretive | CHANG2 resume |
| 13,3623  | interpretive | CHANG2 resume |
| 13,3624  | basic | after EXIT |
| 13,3631  | basic | return from TPAGREE |
| 13,3633 CKMID2 | interpretive | entry CKMID2 (ORBITAL_INTEGRATION.agc  CKMID2) |
| 13,3637  | interpretive | after RTB LOADTIME |
| 13,3642  | interpretive | CHANG2 resume |
| 13,3645 TIMEINC | interpretive | CALL return |
| 13,3650  | interpretive | after RTB LOADTIME |
| 13,3652 MID2 | interpretive | switch branch target |
| 13,3655  | interpretive | CHANG2 resume |
| 13,3657  | interpretive | CHANG2 resume |
| 13,3661  | interpretive | CHANG2 resume |
| 13,3664  | interpretive | after RTB LOADTIME |
| 13,3667  | interpretive | CHANG2 resume |
| 13,3672  | interpretive | CHANG2 resume |
| 13,3674 NOTIME | interpretive | CALL target |
| 13,3676  | basic | after EXIT |
| 13,3701  | basic | return from ALARM |
| 13,3702  | interpretive | after TC INTPRET |
| 13,3704  | interpretive | CHANG2 resume |
| 13,3705  | interpretive | CHANG2 resume |
| 27,2671 INTWAKEU | basic | entry INTWAKEU (UPDATE_PROGRAM.agc TC INTWAKEU) |
| 27,2675  | interpretive | after TC INTPRET |
| 27,2700  | interpretive | CHANG2 resume |
| 27,2702  | interpretive | CHANG2 resume |
| 27,2704  | interpretive | CHANG2 resume |
| 27,2706  | interpretive | CALL return |
| 27,2710  | interpretive | CHANG2 resume |
| 27,2713  | interpretive | CHANG2 resume |
| 27,2717 INTWAKEM | interpretive | BZE target |
| 27,2722 INTWAKEC | interpretive | switch branch target |
| 27,2725  | interpretive | CHANG2 resume |
| 27,2727  | interpretive | CALL return |
| 27,2732 INTWAKLM | interpretive | BMN target |
| 27,2734 INTWAKEX | interpretive | GOTO target |
| 27,2736 INTWAKUP | interpretive | BZE target |
| 27,2742  | interpretive | CALL return |
| 27,2743  | basic | after EXIT |
| 27,2745  | basic | return from PHASCHNG |
| 27,2750 GRP2PC | interpretive | entry GRP2PC (P20-P25.agc  GRP2PC) |
| 27,2752  | basic | after EXIT |
| 27,2754  | basic | return from PHASCHNG |
| 27,2755  | interpretive | after TC INTPRET |
| 35,3675 AVETOMID | interpretive | entry AVETOMID (SERVICER207.agc  AVETOMID) |
| 35,3701  | interpretive | CHANG2 resume |
| 35,3704 SETCOAST | interpretive | CHANG2 resume |
| 35,3707  | interpretive | CALL return |
| 35,3713  | interpretive | switch branch target |
| 35,3716  | interpretive | CHANG2 resume |
| 35,3717  | interpretive | CHANG2 resume |
| 35,3721  | interpretive | CHANG2 resume |
| 35,3723  | interpretive | CHANG2 resume |
| 35,3726  | interpretive | CALL return |
| 35,3732  | interpretive | after RTB MOVEACSM |
| 35,3736  | interpretive | CHANG2 resume |
| 35,3740  | interpretive | switch branch target |
| 35,3744  | interpretive | CHANG2 resume |
| 35,3746  | interpretive | STCALL return |
| 35,3751  | interpretive | CALL return |
| 35,3753 INT/W | interpretive | switch branch target |
| 35,3756  | interpretive | CALL return |
| 35,3761  | interpretive | CHANG2 resume |
| 35,3764  | interpretive | CHANG2 resume |
| 35,3766  | interpretive | STCALL return |

Destinations outside the file: ALARM, BANKJUMP, CHANG2, DANZIG, DOWNFLAG, ENDOFJOB, FINDVAC, JOBSLEEP, JOBWAKE, LONGCALL, PHASCHNG, RTB LOADTIME, RTB SGNAGREE, RTB SIGNMPAC, TASKOVER, TPAGREE, UPFLAG. Lane A has not yet registered the Comanche Executive and Waitlist routines (WAITLIST, NOVAC, FINDVAC, TASKOVER, ENDOFJOB, IBNKCALL with ISWRETRN, POSTJUMP, BANKCALL, RESUME) or the T5RUPT/T6RUPT vectors that dispatch T5LOC/T6LOC; until it does, a transfer to them stops with EXEC_UNMAPPED at the frontier.

## Domain

Canonical 2048-word image (every word random, the counters, `TIME1`-`TIME6`, the CDUs and the save registers at 010-061 included) under EB 6 (the file's `EBANK=`), FB of the routine's bank, any A, L and Q unless stated, and any values of the other erasables. `NEWJOB` is +0 or, in a quarter of the cases, positive: a job waiting, so any `DANZIG` goes to `CHANG2`, where the routine stops; every interpretive word that can follow a `DANZIG` is a `CHANG2` resume key of its own. As for ORBITAL_INTEGRATION, except S1 and S2 hold saved returns here. INTBITAB and WAKESTAL indexes 0 to 2; LOCCTR one of the Executive's core-set offsets at a JOBWAKE return. Other erasables take any value.

## Evidence

Hashes: `evidence/cm-integration-init/SHA256.json`; all files are in the tree.

- **Source map.** `cm-integration-init-source-map.json` lists all 685 words of the file with the routine that compiles each (or why a constant is never run as code); 0 are untranslated.
- **Independent assembly comparison.** `make cm-integration-init-differential` runs 256 states per routine (60,160 cases) through the original Comanche055 rope in yaAGC (pinned yaAGC, its ScalerCounter held at 0 (TIME1-TIME6 and the timer interrupts do not advance during a case)) and the native routines; all are equal in Z, 2048 erasable words (A, L 16-bit) and the interrupt-enable state. The cases and the stops reached per routine are in `cm-integration-init-proof.json` (`cm-integration-init-differential.log` is the run). One case in 128 is frozen in `tests/cm_integration_init_vectors.h`.
- **Host and sanitizer results.** `make test-cm-integration-init sanitize-cm-integration-init` runs the 470 frozen cases (`cm-integration-init-host.log`).
- **C64 execution results.** `make vice-cm-integration-init` runs 9 diagnostic programs with 322 cases (`cm-integration-init-vice.log`, which starts with each PRG's SHA-256). The cases are chosen so the C64 runs take every control-flow edge (both outcomes of each branch) of every linked source that the 60,160 differential cases take: 2691 of 2691 edges and comparison outcomes (clang -O0 trace-pc-guard,trace-cmp) and 123 of 123 compiled labels. Checked again with gcov -b over gcc -O0 `--coverage` builds of the same sources: the C64 cases take 1344 of the 1344 branch outcomes all the cases take (`c64_gcov_branches` in the proof).
