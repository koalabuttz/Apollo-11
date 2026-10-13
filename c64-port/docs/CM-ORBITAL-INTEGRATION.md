# Translate Comanche ORBITAL_INTEGRATION

Lane G (CM flight). Milestone `cm-orbital-integration`. How the routines are made and checked: `docs/INTERP-COMPILE.md` (whole-file basic code: `tools/basic_compile.py`).

`Comanche055/ORBITAL_INTEGRATION.agc` (`src/cm_orbital_integration.c`) is native C in full: Encke integration: TIMESTEP, the DIFEQ stages (DIFEQ+0/1/2), ACCOMP with the oblateness and solar/lunar terms (GAMCOMP, OBLATE, NBRANCH), RECTIFY and the end-of-step bookkeeping, all interpretive with their basic helpers. Each routine runs the file from its entry, or from where it resumes, to the point where control leaves the file. Every original word is one call of the basic-instruction primitives (`src/agc_image_ops.c`) under a label named for its address, with TS overflow skips, CCS four-way skips and INDEX on the whole word kept; computed transfers (TC Q, indexed TCF) go through one dispatch on the target address.

## Scope

333 routines, keyed by physical address in `cm_orbital_integration_basic` and `cm_orbital_integration_interpretive`:

| Entry | Kind | Why it is a routine |
|---|---|---|
| 11,2372 KEPPREP | interpretive | entry KEPPREP (INTEGRATION_INITIALIZATION.agc  KEPPREP) |
| 11,2375  | interpretive | CHANG2 resume |
| 11,2377  | interpretive | CHANG2 resume |
| 11,2401  | interpretive | CHANG2 resume |
| 11,2404  | interpretive | CHANG2 resume |
| 11,2405  | interpretive | CHANG2 resume |
| 11,2410  | interpretive | CHANG2 resume |
| 11,2413  | interpretive | CHANG2 resume |
| 11,2414  | interpretive | CHANG2 resume |
| 11,2416  | interpretive | CHANG2 resume |
| 11,2420  | interpretive | CHANG2 resume |
| 11,2422  | interpretive | CHANG2 resume |
| 11,2424  | interpretive | CHANG2 resume |
| 11,2425  | interpretive | CHANG2 resume |
| 11,2427  | interpretive | CHANG2 resume |
| 11,2431  | interpretive | CHANG2 resume |
| 11,2433  | interpretive | CHANG2 resume |
| 11,2436  | interpretive | CHANG2 resume |
| 11,2440  | interpretive | CHANG2 resume |
| 11,2442  | interpretive | CHANG2 resume |
| 11,2445  | interpretive | CHANG2 resume |
| 11,2450  | interpretive | CHANG2 resume |
| 11,2452  | interpretive | CHANG2 resume |
| 11,2455  | interpretive | CHANG2 resume |
| 11,2457  | interpretive | CHANG2 resume |
| 11,2461  | interpretive | CHANG2 resume |
| 11,2464  | interpretive | CHANG2 resume |
| 11,2470  | interpretive | CHANG2 resume |
| 11,2472 FBR3 | interpretive | GOTO target |
| 11,2476  | interpretive | CHANG2 resume |
| 11,2501  | interpretive | CHANG2 resume |
| 11,2503  | interpretive | TIX target |
| 11,2505  | interpretive | CHANG2 resume |
| 11,2506  | interpretive | CHANG2 resume |
| 11,2510  | interpretive | CHANG2 resume |
| 11,2512 ACCOMP | interpretive | GOTO target |
| 11,2515  | interpretive | CHANG2 resume |
| 11,2517  | interpretive | CHANG2 resume |
| 11,2521  | interpretive | CHANG2 resume |
| 11,2524  | interpretive | CHANG2 resume |
| 11,2525  | interpretive | CHANG2 resume |
| 11,2531  | interpretive | CHANG2 resume |
| 11,2532  | interpretive | CHANG2 resume |
| 11,2534  | interpretive | switch branch target |
| 11,2536  | interpretive | CHANG2 resume |
| 11,2540  | interpretive | CHANG2 resume |
| 11,2541  | interpretive | CHANG2 resume |
| 11,2543  | interpretive | CALL return |
| 11,2546  | interpretive | CHANG2 resume |
| 11,2550  | interpretive | CHANG2 resume |
| 11,2551  | interpretive | CHANG2 resume |
| 11,2555  | interpretive | CHANG2 resume |
| 11,2557  | interpretive | CALL return |
| 11,2562  | interpretive | CHANG2 resume |
| 11,2565  | interpretive | CHANG2 resume |
| 11,2567  | interpretive | switch branch target |
| 11,2570  | interpretive | CHANG2 resume |
| 11,2572  | interpretive | CHANG2 resume |
| 11,2573  | interpretive | CHANG2 resume |
| 11,2577  | interpretive | CHANG2 resume |
| 11,2602  | interpretive | CHANG2 resume |
| 11,2605  | interpretive | CHANG2 resume |
| 11,2606  | interpretive | CHANG2 resume |
| 11,2610 GETRPSV | interpretive | switch branch target |
| 11,2613  | interpretive | CHANG2 resume |
| 11,2617  | interpretive | CHANG2 resume |
| 11,2622  | interpretive | CHANG2 resume |
| 11,2623  | interpretive | switch branch target |
| 11,2625  | interpretive | CALL return |
| 11,2630  | interpretive | CHANG2 resume |
| 11,2632  | interpretive | CHANG2 resume |
| 11,2634  | interpretive | STCALL return |
| 11,2636 GAMCOMP | interpretive | CALL target |
| 11,2640  | interpretive | CHANG2 resume |
| 11,2642  | interpretive | CHANG2 resume |
| 11,2644  | interpretive | CHANG2 resume |
| 11,2647  | interpretive | CHANG2 resume |
| 11,2651  | interpretive | CHANG2 resume |
| 11,2652  | interpretive | CHANG2 resume |
| 11,2654  | interpretive | CHANG2 resume |
| 11,2655  | interpretive | CHANG2 resume |
| 11,2657  | interpretive | CHANG2 resume |
| 11,2660  | interpretive | CHANG2 resume |
| 11,2662  | interpretive | CHANG2 resume |
| 11,2663  | interpretive | CHANG2 resume |
| 11,2666  | interpretive | CHANG2 resume |
| 11,2671  | interpretive | CHANG2 resume |
| 11,2674  | interpretive | CHANG2 resume |
| 11,2675  | interpretive | CHANG2 resume |
| 11,2700  | interpretive | CHANG2 resume |
| 11,2701  | interpretive | CHANG2 resume |
| 11,2703  | interpretive | CHANG2 resume |
| 11,2704  | interpretive | CHANG2 resume |
| 11,2706  | interpretive | CHANG2 resume |
| 11,2707  | interpretive | CHANG2 resume |
| 11,2711  | interpretive | CHANG2 resume |
| 11,2713  | interpretive | CHANG2 resume |
| 11,2716  | interpretive | CHANG2 resume |
| 11,2720  | interpretive | CHANG2 resume |
| 11,2723  | interpretive | CHANG2 resume |
| 11,2726  | interpretive | CHANG2 resume |
| 11,2730  | interpretive | CHANG2 resume |
| 11,2731  | interpretive | CHANG2 resume |
| 11,2734  | interpretive | CHANG2 resume |
| 11,2736  | interpretive | CHANG2 resume |
| 11,2741  | interpretive | CHANG2 resume |
| 11,2742  | interpretive | CHANG2 resume |
| 11,2745  | interpretive | CHANG2 resume |
| 11,2750  | interpretive | CHANG2 resume |
| 11,2752  | interpretive | BOV target |
| 11,2755  | interpretive | CHANG2 resume |
| 11,2757  | interpretive | CHANG2 resume |
| 11,2760  | interpretive | CHANG2 resume |
| 11,2762 GOBAQUE | interpretive | BOV target |
| 11,2764  | interpretive | CHANG2 resume |
| 11,2766  | interpretive | CHANG2 resume |
| 11,2771  | interpretive | CHANG2 resume |
| 11,2773  | interpretive | CHANG2 resume |
| 11,2775  | interpretive | CHANG2 resume |
| 11,3000  | interpretive | STCALL return |
| 11,3002  | interpretive | CALL return |
| 11,3005 INT-ABRT | interpretive | BZE target |
| 11,3006  | basic | after EXIT |
| 11,3010 OBLATE | interpretive | switch branch target |
| 11,3013  | interpretive | CHANG2 resume |
| 11,3016  | interpretive | CHANG2 resume |
| 11,3022  | interpretive | CHANG2 resume |
| 11,3025  | interpretive | CHANG2 resume |
| 11,3030  | interpretive | CALL return |
| 11,3031  | interpretive | CHANG2 resume |
| 11,3034  | interpretive | CHANG2 resume |
| 11,3037  | interpretive | CHANG2 resume |
| 11,3040 COMTERM | interpretive | CHANG2 resume |
| 11,3041  | interpretive | CHANG2 resume |
| 11,3044  | interpretive | CHANG2 resume |
| 11,3046  | interpretive | CHANG2 resume |
| 11,3051  | interpretive | CHANG2 resume |
| 11,3053  | interpretive | CHANG2 resume |
| 11,3055  | interpretive | CHANG2 resume |
| 11,3060  | interpretive | CHANG2 resume |
| 11,3061  | interpretive | CHANG2 resume |
| 11,3064  | interpretive | CHANG2 resume |
| 11,3067  | interpretive | CHANG2 resume |
| 11,3070  | interpretive | CHANG2 resume |
| 11,3072  | interpretive | CHANG2 resume |
| 11,3075  | interpretive | CHANG2 resume |
| 11,3100  | interpretive | CHANG2 resume |
| 11,3103  | interpretive | CHANG2 resume |
| 11,3104  | interpretive | CHANG2 resume |
| 11,3106  | interpretive | CHANG2 resume |
| 11,3110  | interpretive | CHANG2 resume |
| 11,3112  | interpretive | CHANG2 resume |
| 11,3114  | interpretive | CHANG2 resume |
| 11,3116  | interpretive | CHANG2 resume |
| 11,3120  | interpretive | CHANG2 resume |
| 11,3122  | interpretive | CHANG2 resume |
| 11,3124  | interpretive | CHANG2 resume |
| 11,3126  | interpretive | CHANG2 resume |
| 11,3130  | interpretive | CHANG2 resume |
| 11,3133  | interpretive | BOV target |
| 11,3136  | interpretive | CHANG2 resume |
| 11,3141  | interpretive | CHANG2 resume |
| 11,3144  | interpretive | CHANG2 resume |
| 11,3146 QUALITY3 | interpretive | STCALL return |
| 11,3147  | interpretive | CHANG2 resume |
| 11,3151  | interpretive | CHANG2 resume |
| 11,3152  | interpretive | CHANG2 resume |
| 11,3155  | interpretive | CHANG2 resume |
| 11,3160  | interpretive | CHANG2 resume |
| 11,3162  | interpretive | CHANG2 resume |
| 11,3163  | interpretive | CHANG2 resume |
| 11,3166  | interpretive | CHANG2 resume |
| 11,3170  | interpretive | CHANG2 resume |
| 11,3173  | interpretive | CHANG2 resume |
| 11,3174  | interpretive | CHANG2 resume |
| 11,3177  | interpretive | CHANG2 resume |
| 11,3201  | interpretive | CHANG2 resume |
| 11,3202  | interpretive | CHANG2 resume |
| 11,3204  | interpretive | CHANG2 resume |
| 11,3206  | interpretive | CHANG2 resume |
| 11,3211  | interpretive | CHANG2 resume |
| 11,3214  | interpretive | CALL return |
| 11,3217  | interpretive | CHANG2 resume |
| 11,3220 NBRANCH | interpretive | entry NBRANCH (RTB_OP_CODES.agc  NBRANCH) |
| 11,3223  | interpretive | CHANG2 resume |
| 11,3227 COSPHIE | interpretive | switch branch target |
| 11,3231  | interpretive | CHANG2 resume |
| 11,3233  | interpretive | CHANG2 resume |
| 11,3240 TIMESTEP | interpretive | entry TIMESTEP (INTEGRATION_INITIALIZATION.agc  TIMESTEP) |
| 11,3244  | interpretive | CALL return |
| 11,3246 RECTEST | interpretive | switch branch target |
| 11,3250  | interpretive | CHANG2 resume |
| 11,3252  | interpretive | CHANG2 resume |
| 11,3255  | interpretive | CHANG2 resume |
| 11,3260  | interpretive | CHANG2 resume |
| 11,3263  | interpretive | CHANG2 resume |
| 11,3266  | interpretive | CHANG2 resume |
| 11,3270  | interpretive | CHANG2 resume |
| 11,3272  | interpretive | CHANG2 resume |
| 11,3274 CALLRECT | interpretive | BOV target |
| 11,3276 INTGRATE | interpretive | GOTO target |
| 11,3300  | interpretive | CHANG2 resume |
| 11,3302  | interpretive | CHANG2 resume |
| 11,3303  | interpretive | CHANG2 resume |
| 11,3305 DIFEQ0 | interpretive | STCALL target |
| 11,3311  | interpretive | CHANG2 resume |
| 11,3313  | interpretive | CHANG2 resume |
| 11,3314  | interpretive | CHANG2 resume |
| 11,3320 CHKSWTCH | interpretive | entry CHKSWTCH (INTEGRATION_INITIALIZATION.agc  CHKSWTCH) |
| 11,3324  | interpretive | CHANG2 resume |
| 11,3327  | interpretive | CALL return |
| 11,3332  | interpretive | switch branch target |
| 11,3333 RPQOK | interpretive | switch branch target |
| 11,3336  | interpretive | CHANG2 resume |
| 11,3341  | interpretive | CHANG2 resume |
| 11,3344  | interpretive | CHANG2 resume |
| 11,3346  | interpretive | CHANG2 resume |
| 11,3350 EARSPH | interpretive | switch branch target |
| 11,3352  | interpretive | CHANG2 resume |
| 11,3355 DOSWITCH | interpretive | BMN target |
| 11,3357  | interpretive | CALL return |
| 11,3361 ORIGCHNG | interpretive | entry ORIGCHNG (INTEGRATION_INITIALIZATION.agc  ORIGCHNG) |
| 11,3364  | interpretive | CALL return |
| 11,3367  | interpretive | CHANG2 resume |
| 11,3372  | interpretive | CHANG2 resume |
| 11,3373  | interpretive | CHANG2 resume |
| 11,3375  | interpretive | CHANG2 resume |
| 11,3377  | interpretive | CALL return |
| 11,3402  | interpretive | switch branch target |
| 11,3405  | interpretive | CHANG2 resume |
| 11,3406  | interpretive | CHANG2 resume |
| 11,3410  | interpretive | CHANG2 resume |
| 11,3411  | interpretive | CHANG2 resume |
| 11,3412  | interpretive | CHANG2 resume |
| 11,3415  | interpretive | CHANG2 resume |
| 11,3421 RECTIFY | interpretive | entry RECTIFY (MEASUREMENT_INCORPORATION.agc  RECTIFY) |
| 11,3424  | interpretive | CHANG2 resume |
| 11,3427  | interpretive | CHANG2 resume |
| 11,3430  | interpretive | CHANG2 resume |
| 11,3432  | interpretive | CHANG2 resume |
| 11,3435 MINIRECT | interpretive | entry MINIRECT (P76.agc  MINIRECT) |
| 11,3436  | interpretive | CHANG2 resume |
| 11,3440  | interpretive | CHANG2 resume |
| 11,3441  | interpretive | CHANG2 resume |
| 11,3443  | interpretive | CHANG2 resume |
| 11,3444  | interpretive | CHANG2 resume |
| 11,3445  | interpretive | CHANG2 resume |
| 11,3446 DIFEQ+0 | interpretive | CADR DIFEQ+0 |
| 11,3450  | interpretive | CHANG2 resume |
| 11,3452 DIFEQ+1 | interpretive | CADR DIFEQ+1 |
| 11,3454  | interpretive | CHANG2 resume |
| 11,3456  | interpretive | CHANG2 resume |
| 11,3457  | interpretive | CHANG2 resume |
| 11,3461  | interpretive | CHANG2 resume |
| 11,3463 DIFEQ+2 | interpretive | CADR DIFEQ+2 |
| 11,3466  | interpretive | CHANG2 resume |
| 11,3470  | interpretive | CHANG2 resume |
| 11,3472  | interpretive | CHANG2 resume |
| 11,3475  | interpretive | CHANG2 resume |
| 11,3477  | interpretive | CHANG2 resume |
| 11,3501  | interpretive | CHANG2 resume |
| 11,3502  | interpretive | CHANG2 resume |
| 11,3504  | interpretive | CHANG2 resume |
| 11,3505  | interpretive | CHANG2 resume |
| 11,3511  | interpretive | CALL return |
| 11,3514  | interpretive | CHANG2 resume |
| 11,3515  | interpretive | CHANG2 resume |
| 11,3516  | interpretive | CHANG2 resume |
| 11,3520  | interpretive | CHANG2 resume |
| 11,3522  | interpretive | CHANG2 resume |
| 11,3523  | interpretive | CHANG2 resume |
| 11,3525  | interpretive | CALL return |
| 11,3531  | interpretive | CHANG2 resume |
| 11,3534  | interpretive | CHANG2 resume |
| 11,3537  | interpretive | CALL return |
| 11,3542 NEXTCOL | interpretive | CHANG2 resume |
| 11,3544  | interpretive | CALL return |
| 11,3547  | interpretive | CHANG2 resume |
| 11,3550  | interpretive | CHANG2 resume |
| 11,3551  | interpretive | CHANG2 resume |
| 11,3554  | interpretive | CHANG2 resume |
| 11,3555  | interpretive | CHANG2 resume |
| 11,3557 ENDSTATE | interpretive | switch branch target |
| 11,3562  | interpretive | CHANG2 resume |
| 11,3564  | interpretive | CHANG2 resume |
| 11,3565  | interpretive | CHANG2 resume |
| 11,3572  | interpretive | CHANG2 resume |
| 11,3573  | basic | after EXIT |
| 11,3575  | basic | return from PHASCHNG |
| 11,3577  | basic | return from UPFLAG |
| 11,3600  | interpretive | after TC INTPRET |
| 11,3603  | interpretive | CHANG2 resume |
| 11,3607 AMOVED | interpretive | SSP AMOVED |
| 11,3613  | interpretive | CHANG2 resume |
| 11,3620  | interpretive | CHANG2 resume |
| 11,3622 RELOADSV | interpretive | TIX target |
| 11,3624  | interpretive | CHANG2 resume |
| 11,3626 DIFEQCOM | interpretive | STCALL return |
| 11,3631  | interpretive | CHANG2 resume |
| 11,3634  | interpretive | CHANG2 resume |
| 11,3635  | interpretive | CHANG2 resume |
| 11,3637  | interpretive | CHANG2 resume |
| 11,3642  | interpretive | CHANG2 resume |
| 11,3644  | interpretive | CHANG2 resume |
| 11,3645  | interpretive | CHANG2 resume |
| 11,3651 WMATEND | interpretive | BOV target |
| 11,3654  | interpretive | CHANG2 resume |
| 11,3656  | interpretive | CHANG2 resume |
| 11,3660  | basic | after EXIT |
| 11,3662  | basic | return from ALARM |
| 11,3663  | interpretive | after TC INTPRET |
| 11,3665 DOW.. | interpretive | switch branch target |
| 11,3670  | interpretive | CHANG2 resume |
| 11,3672  | interpretive | STCALL return |
| 11,3673  | interpretive | CHANG2 resume |
| 11,3677  | interpretive | CHANG2 resume |
| 11,3702  | interpretive | CHANG2 resume |
| 11,3704  | interpretive | STCALL return |
| 11,3707  | interpretive | switch branch target |
| 11,3711  | interpretive | CHANG2 resume |
| 11,3713 DOW..1 | interpretive | STCALL target |
| 11,3715  | interpretive | CHANG2 resume |
| 11,3717  | interpretive | CHANG2 resume |
| 11,3721  | interpretive | CHANG2 resume |
| 11,3723  | interpretive | CHANG2 resume |
| 11,3726  | interpretive | CHANG2 resume |
| 11,3727  | interpretive | CHANG2 resume |
| 11,3730  | interpretive | CHANG2 resume |
| 11,3733  | interpretive | CHANG2 resume |
| 11,3734  | interpretive | CHANG2 resume |
| 11,3735  | interpretive | CHANG2 resume |
| 11,3740  | interpretive | CHANG2 resume |
| 11,3743  | interpretive | CHANG2 resume |

Destinations outside the file: ALARM, BOVB ENDT(X), CHANG2, PHASCHNG, POODOO, UPFLAG. Lane A has not yet registered the Comanche Executive and Waitlist routines (WAITLIST, NOVAC, FINDVAC, TASKOVER, ENDOFJOB, IBNKCALL with ISWRETRN, POSTJUMP, BANKCALL, RESUME) or the T5RUPT/T6RUPT vectors that dispatch T5LOC/T6LOC; until it does, a transfer to them stops with EXEC_UNMAPPED at the frontier.

## Domain

Canonical 2048-word image (every word random, the counters, `TIME1`-`TIME6`, the CDUs and the save registers at 010-061 included) under EB 6 (the file's `EBANK=`), FB of the routine's bank, any A, L and Q unless stated, and any values of the other erasables. `NEWJOB` is +0 or, in a quarter of the cases, positive: a job waiting, so any `DANZIG` goes to `CHANG2`, where the routine stops; every interpretive word that can follow a `DANZIG` is a `CHANG2` resume key of its own. PBODY 0 (Earth) or 2 (Moon); X1 and X2, S1 and S2 the values the stage loops set; DIFEQCNT 0 or a stage count; COLREG one of the W-matrix column offsets; PUSHLOC on the pushdown list. Mid-flow resume points start from the state the preceding orders leave (MODE, MPAC, PUSHLOC). Other erasables take any value.

## Evidence

Hashes: `evidence/cm-orbital-integration/SHA256.json`; all files are in the tree.

- **Source map.** `cm-orbital-integration-source-map.json` lists all 816 words of the file with the routine that compiles each (or why a constant is never run as code); 0 are untranslated.
- **Independent assembly comparison.** `make cm-orbital-integration-differential` runs 256 states per routine (85,248 cases) through the original Comanche055 rope in yaAGC (pinned yaAGC, its ScalerCounter held at 0 (TIME1-TIME6 and the timer interrupts do not advance during a case)) and the native routines; all are equal in Z, 2048 erasable words (A, L 16-bit) and the interrupt-enable state. The cases and the stops reached per routine are in `cm-orbital-integration-proof.json` (`cm-orbital-integration-differential.log` is the run). One case in 128 is frozen in `tests/cm_orbital_integration_vectors.h`.
- **Host and sanitizer results.** `make test-cm-orbital-integration sanitize-cm-orbital-integration` runs the 666 frozen cases (`cm-orbital-integration-host.log`).
- **C64 execution results.** `make vice-cm-orbital-integration` runs 25 diagnostic programs with 411 cases (`cm-orbital-integration-vice.log`, which starts with each PRG's SHA-256). The cases are chosen so the C64 runs take every control-flow edge (both outcomes of each branch) of every linked source that the 85,248 differential cases take: 3622 of 3724 edges and comparison outcomes (clang -O0 trace-pc-guard,trace-cmp) and 6 of 6 compiled labels. Checked again with gcov -b over gcc -O0 `--coverage` builds of the same sources: the C64 cases take 1673 of the 1725 branch outcomes all the cases take (`c64_gcov_branches` in the proof).
