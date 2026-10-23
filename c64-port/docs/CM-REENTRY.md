# Translate Comanche REENTRY_CONTROL

Lane G (CM flight). Milestone `cm-reentry`. How the routines are made and checked: `docs/INTERP-COMPILE.md` (whole-file basic code: `tools/basic_compile.py`).

`Comanche055/REENTRY_CONTROL.agc` (`src/cm_reentry.c`) is native C in full: The entry guidance: STARTENT and the two-second steering loop (INITROLL, HUNTEST, UPCONTRL, KEP2/BALLISTIC, PREDICT3 with its reference-trajectory tables, the roll command and lift-vector logic, the G-limit and final-phase tests), with the entry displays and restart points, basic and interpretive. Each routine runs the file from its entry, or from where it resumes, to the point where control leaves the file. Every original word is one call of the basic-instruction primitives (`src/agc_image_ops.c`) under a label named for its address, with TS overflow skips, CCS four-way skips and INDEX on the whole word kept; computed transfers (TC Q, indexed TCF) go through one dispatch on the target address.

## Scope

392 routines, keyed by physical address in `cm_reentry_basic` and `cm_reentry_interpretive`:

| Entry | Kind | Why it is a routine |
|---|---|---|
| 25,2000 STARTENT | interpretive | entry STARTENT (P61-P67.agc CADR STARTENT) |
| 25,2001  | basic | after EXIT |
| 25,2010  | interpretive | after TC INTPRET |
| 25,2012  | interpretive | CHANG2 resume |
| 25,2013  | interpretive | CHANG2 resume |
| 25,2015  | interpretive | CHANG2 resume |
| 25,2016  | interpretive | CHANG2 resume |
| 25,2020  | interpretive | CHANG2 resume |
| 25,2022  | interpretive | CHANG2 resume |
| 25,2024  | interpretive | CHANG2 resume |
| 25,2026  | interpretive | CHANG2 resume |
| 25,2030  | interpretive | CHANG2 resume |
| 25,2032  | interpretive | CHANG2 resume |
| 25,2034  | interpretive | CHANG2 resume |
| 25,2036  | interpretive | STCALL return |
| 25,2041  | interpretive | CHANG2 resume |
| 25,2043  | interpretive | CHANG2 resume |
| 25,2044  | interpretive | CHANG2 resume |
| 25,2046  | interpretive | after RTB SIGNMPAC |
| 25,2050  | interpretive | CHANG2 resume |
| 25,2053  | interpretive | CHANG2 resume |
| 25,2054  | interpretive | CHANG2 resume |
| 25,2061  | interpretive | CHANG2 resume |
| 25,2063 STARTEN1 | interpretive | entry STARTEN1 (P61-P67.agc  STARTEN1) |
| 25,2066  | interpretive | CHANG2 resume |
| 25,2071  | interpretive | CHANG2 resume |
| 25,2073  | interpretive | CHANG2 resume |
| 25,2075  | interpretive | CHANG2 resume |
| 25,2077  | interpretive | STCALL return |
| 25,2100  | interpretive | CHANG2 resume |
| 25,2102  | interpretive | CHANG2 resume |
| 25,2104  | interpretive | STCALL return |
| 25,2106  | interpretive | CHANG2 resume |
| 25,2107  | interpretive | CHANG2 resume |
| 25,2115 SCALEPOP | interpretive | SSP SCALEPOP |
| 25,2117  | interpretive | CALL return |
| 25,2120 REFAZE10 | basic | entry REFAZE10 (P61-P67.agc  REFAZE10) |
| 25,2122  | basic | return from PHASCHNG |
| 25,2123  | interpretive | after TC INTPRET |
| 25,2125 TARGETNG | interpretive | CALL target |
| 25,2131  | interpretive | CHANG2 resume |
| 25,2133 GETVEL | interpretive | switch branch target |
| 25,2136  | interpretive | GOTO target |
| 25,2137 GETUNITV | interpretive | CHANG2 resume |
| 25,2141  | interpretive | CHANG2 resume |
| 25,2143  | interpretive | CHANG2 resume |
| 25,2144  | interpretive | CHANG2 resume |
| 25,2146  | interpretive | CHANG2 resume |
| 25,2150  | interpretive | CHANG2 resume |
| 25,2152  | interpretive | CHANG2 resume |
| 25,2154  | interpretive | CHANG2 resume |
| 25,2156  | interpretive | CHANG2 resume |
| 25,2160  | interpretive | CHANG2 resume |
| 25,2162 DSTORE | interpretive | CHANG2 resume |
| 25,2164  | interpretive | CHANG2 resume |
| 25,2166  | interpretive | CHANG2 resume |
| 25,2167  | interpretive | CHANG2 resume |
| 25,2173 UPDATERT | interpretive | CHANG2 resume |
| 25,2176  | interpretive | CHANG2 resume |
| 25,2200  | interpretive | STCALL return |
| 25,2203  | interpretive | CHANG2 resume |
| 25,2205  | interpretive | CHANG2 resume |
| 25,2207  | interpretive | CHANG2 resume |
| 25,2212  | interpretive | CHANG2 resume |
| 25,2215  | interpretive | BPL target |
| 25,2217 GETANGLE | interpretive | CHANG2 resume |
| 25,2222  | interpretive | CHANG2 resume |
| 25,2225  | interpretive | CHANG2 resume |
| 25,2226 THETDONE | interpretive | CHANG2 resume |
| 25,2227  | interpretive | CHANG2 resume |
| 25,2232  | interpretive | switch branch target |
| 25,2234  | interpretive | CHANG2 resume |
| 25,2237  | interpretive | CHANG2 resume |
| 25,2242  | interpretive | CHANG2 resume |
| 25,2244  | interpretive | CHANG2 resume |
| 25,2245  | interpretive | CHANG2 resume |
| 25,2246  | interpretive | CHANG2 resume |
| 25,2250  | interpretive | CHANG2 resume |
| 25,2252  | interpretive | CHANG2 resume |
| 25,2253 NOLDCALC | interpretive | BOV target |
| 25,2255 NO.05G | interpretive | BMN target |
| 25,2260 INITROLL | interpretive | SSP INITROLL |
| 25,2265  | interpretive | CHANG2 resume |
| 25,2267  | interpretive | CHANG2 resume |
| 25,2272  | interpretive | CHANG2 resume |
| 25,2275  | interpretive | after RTB P64 |
| 25,2276  | interpretive | CHANG2 resume |
| 25,2301  | interpretive | CHANG2 resume |
| 25,2303  | interpretive | CHANG2 resume |
| 25,2304  | interpretive | BMN target |
| 25,2307  | interpretive | CHANG2 resume |
| 25,2312  | interpretive | CHANG2 resume |
| 25,2316 D0EQ | interpretive | BPL target |
| 25,2321  | interpretive | CHANG2 resume |
| 25,2323  | interpretive | CHANG2 resume |
| 25,2324  | interpretive | CHANG2 resume |
| 25,2327  | interpretive | BOV target |
| 25,2331  | interpretive | CHANG2 resume |
| 25,2333  | interpretive | CHANG2 resume |
| 25,2335  | interpretive | CHANG2 resume |
| 25,2336  | interpretive | CHANG2 resume |
| 25,2341  | interpretive | CHANG2 resume |
| 25,2344  | interpretive | CHANG2 resume |
| 25,2347  | interpretive | CHANG2 resume |
| 25,2350  | interpretive | CHANG2 resume |
| 25,2351 INROLOUT | interpretive | GOTO target |
| 25,2354 KATEST | interpretive | CHANG2 resume |
| 25,2357  | interpretive | CHANG2 resume |
| 25,2362 INITRL1 | interpretive | switch branch target |
| 25,2365  | interpretive | CHANG2 resume |
| 25,2370  | interpretive | CALL return |
| 25,2373 HUNTEST | interpretive | SSP HUNTEST |
| 25,2375  | interpretive | CHANG2 resume |
| 25,2377  | interpretive | CHANG2 resume |
| 25,2401  | interpretive | CHANG2 resume |
| 25,2404  | interpretive | CHANG2 resume |
| 25,2406 A0CALC | interpretive | BMN target |
| 25,2411  | interpretive | CHANG2 resume |
| 25,2413  | interpretive | CHANG2 resume |
| 25,2415  | interpretive | CHANG2 resume |
| 25,2420  | interpretive | CHANG2 resume |
| 25,2423  | interpretive | CHANG2 resume |
| 25,2425  | interpretive | CHANG2 resume |
| 25,2427  | interpretive | CHANG2 resume |
| 25,2432  | interpretive | CHANG2 resume |
| 25,2433 V1LEAD | interpretive | BPL target |
| 25,2436  | interpretive | CHANG2 resume |
| 25,2441  | interpretive | CHANG2 resume |
| 25,2442 HUNTEST1 | interpretive | BPL target |
| 25,2445  | interpretive | CHANG2 resume |
| 25,2450  | interpretive | CHANG2 resume |
| 25,2453  | interpretive | CHANG2 resume |
| 25,2454  | interpretive | CHANG2 resume |
| 25,2457  | interpretive | CHANG2 resume |
| 25,2461  | interpretive | CHANG2 resume |
| 25,2464  | interpretive | CHANG2 resume |
| 25,2466  | interpretive | CHANG2 resume |
| 25,2467  | interpretive | CHANG2 resume |
| 25,2472  | interpretive | CHANG2 resume |
| 25,2474  | interpretive | CHANG2 resume |
| 25,2476  | interpretive | CHANG2 resume |
| 25,2477  | interpretive | CHANG2 resume |
| 25,2502  | interpretive | CHANG2 resume |
| 25,2504  | interpretive | CHANG2 resume |
| 25,2506  | interpretive | CHANG2 resume |
| 25,2511  | interpretive | CHANG2 resume |
| 25,2513  | interpretive | CHANG2 resume |
| 25,2515  | interpretive | CHANG2 resume |
| 25,2520  | interpretive | CHANG2 resume |
| 25,2522  | interpretive | CHANG2 resume |
| 25,2523  | interpretive | CHANG2 resume |
| 25,2526  | interpretive | CHANG2 resume |
| 25,2530  | interpretive | CHANG2 resume |
| 25,2532  | interpretive | CHANG2 resume |
| 25,2533 GETDHOOK | interpretive | BMN target |
| 25,2536  | interpretive | CALL return |
| 25,2537  | interpretive | CHANG2 resume |
| 25,2542  | interpretive | CHANG2 resume |
| 25,2544  | interpretive | CHANG2 resume |
| 25,2545  | interpretive | CHANG2 resume |
| 25,2550  | interpretive | CHANG2 resume |
| 25,2553  | interpretive | CHANG2 resume |
| 25,2556  | interpretive | CHANG2 resume |
| 25,2561 HUNTEST3 | interpretive | CHANG2 resume |
| 25,2562  | interpretive | CHANG2 resume |
| 25,2564  | interpretive | CHANG2 resume |
| 25,2567  | interpretive | CHANG2 resume |
| 25,2571 RANGER | interpretive | CHANG2 resume |
| 25,2572  | interpretive | CHANG2 resume |
| 25,2574  | interpretive | CHANG2 resume |
| 25,2576  | interpretive | CHANG2 resume |
| 25,2601  | interpretive | CHANG2 resume |
| 25,2604  | interpretive | CHANG2 resume |
| 25,2606  | interpretive | CHANG2 resume |
| 25,2610  | interpretive | CHANG2 resume |
| 25,2613  | interpretive | CHANG2 resume |
| 25,2614  | interpretive | CHANG2 resume |
| 25,2615  | interpretive | CHANG2 resume |
| 25,2617  | interpretive | CHANG2 resume |
| 25,2622  | interpretive | CHANG2 resume |
| 25,2623  | interpretive | CHANG2 resume |
| 25,2625  | interpretive | CHANG2 resume |
| 25,2630  | interpretive | CHANG2 resume |
| 25,2633  | interpretive | CALL return |
| 25,2636  | interpretive | CHANG2 resume |
| 25,2637  | interpretive | CHANG2 resume |
| 25,2642  | interpretive | CHANG2 resume |
| 25,2645  | interpretive | CHANG2 resume |
| 25,2647  | interpretive | CHANG2 resume |
| 25,2651  | interpretive | CHANG2 resume |
| 25,2654  | interpretive | CHANG2 resume |
| 25,2656  | interpretive | CHANG2 resume |
| 25,2660  | interpretive | CHANG2 resume |
| 25,2661  | interpretive | CHANG2 resume |
| 25,2662  | interpretive | CHANG2 resume |
| 25,2665  | interpretive | CHANG2 resume |
| 25,2666  | interpretive | CHANG2 resume |
| 25,2670  | interpretive | CHANG2 resume |
| 25,2674  | interpretive | CHANG2 resume |
| 25,2677 GETLEWD | interpretive | switch branch target |
| 25,2702  | interpretive | CHANG2 resume |
| 25,2705  | interpretive | CHANG2 resume |
| 25,2706 LWDSTORE | interpretive | CHANG2 resume |
| 25,2710  | interpretive | CHANG2 resume |
| 25,2713  | interpretive | CHANG2 resume |
| 25,2715  | interpretive | CHANG2 resume |
| 25,2716 SIDETRAK | interpretive | CHANG2 resume |
| 25,2717  | basic | after EXIT |
| 25,2725  | basic | return from PHASCHNG |
| 25,2727  | basic | return from PRIOCHNG |
| 25,2732  | interpretive | after TC INTPRET |
| 25,2735  | interpretive | CHANG2 resume |
| 25,2737  | interpretive | CHANG2 resume |
| 25,2741 LEWDOVFL | interpretive | BOV target |
| 25,2743  | interpretive | CHANG2 resume |
| 25,2745 LEWDPTR | interpretive | BMN target |
| 25,2747  | interpretive | CHANG2 resume |
| 25,2751 NEGAMA | interpretive | BMN target |
| 25,2754  | interpretive | CHANG2 resume |
| 25,2757  | interpretive | CHANG2 resume |
| 25,2762  | interpretive | CHANG2 resume |
| 25,2765  | interpretive | CHANG2 resume |
| 25,2770  | interpretive | CHANG2 resume |
| 25,2771  | interpretive | CHANG2 resume |
| 25,2773  | interpretive | CHANG2 resume |
| 25,2775  | interpretive | STCALL return |
| 25,2777  | interpretive | CHANG2 resume |
| 25,3000  | interpretive | CHANG2 resume |
| 25,3002  | interpretive | CHANG2 resume |
| 25,3004 DHOOKYQ7 | interpretive | CALL target |
| 25,3006  | interpretive | CHANG2 resume |
| 25,3010  | interpretive | CHANG2 resume |
| 25,3012  | interpretive | CHANG2 resume |
| 25,3014 PRE-HUNT | basic | entry PRE-HUNT (RESTART_TABLES.agc 2CADR PRE-HUNT) |
| 25,3015  | interpretive | after TC INTPRET |
| 25,3020  | interpretive | CALL return |
| 25,3022 FOREHUNT | interpretive | CALL target |
| 25,3024  | interpretive | CHANG2 resume |
| 25,3026  | interpretive | CHANG2 resume |
| 25,3030  | interpretive | CHANG2 resume |
| 25,3031  | interpretive | CHANG2 resume |
| 25,3033 GOTOUPSY | interpretive | BMN target |
| 25,3035 UPCONTRL | interpretive | entry UPCONTRL (P61-P67.agc  UPCONTRL) |
| 25,3040  | interpretive | CHANG2 resume |
| 25,3043  | interpretive | BMN target |
| 25,3046  | interpretive | CHANG2 resume |
| 25,3051  | interpretive | CHANG2 resume |
| 25,3054  | interpretive | CHANG2 resume |
| 25,3057 VLTEST | interpretive | CHANG2 resume |
| 25,3062  | interpretive | CHANG2 resume |
| 25,3065 CONT1 | interpretive | BPL target |
| 25,3067  | interpretive | CHANG2 resume |
| 25,3072  | interpretive | CHANG2 resume |
| 25,3075 CONT3 | interpretive | BMN target |
| 25,3100  | interpretive | CHANG2 resume |
| 25,3102  | interpretive | CHANG2 resume |
| 25,3105  | interpretive | CHANG2 resume |
| 25,3106  | interpretive | CHANG2 resume |
| 25,3111  | interpretive | CHANG2 resume |
| 25,3113  | interpretive | CHANG2 resume |
| 25,3116  | interpretive | CHANG2 resume |
| 25,3117  | interpretive | CHANG2 resume |
| 25,3122  | interpretive | CHANG2 resume |
| 25,3125  | interpretive | CHANG2 resume |
| 25,3126  | interpretive | CHANG2 resume |
| 25,3130  | interpretive | CHANG2 resume |
| 25,3133  | interpretive | CHANG2 resume |
| 25,3134 CONTINU2 | interpretive | BMN target |
| 25,3137  | interpretive | CHANG2 resume |
| 25,3142  | interpretive | CHANG2 resume |
| 25,3145  | interpretive | CHANG2 resume |
| 25,3150  | interpretive | CHANG2 resume |
| 25,3152 UPCNTRL3 | interpretive | BMN target |
| 25,3154  | interpretive | CHANG2 resume |
| 25,3157  | interpretive | CHANG2 resume |
| 25,3162  | interpretive | CHANG2 resume |
| 25,3165  | interpretive | CHANG2 resume |
| 25,3167  | interpretive | CHANG2 resume |
| 25,3171  | interpretive | CHANG2 resume |
| 25,3174  | interpretive | CHANG2 resume |
| 25,3177  | interpretive | CHANG2 resume |
| 25,3200 NEXT1 | interpretive | BMN target |
| 25,3201  | interpretive | CHANG2 resume |
| 25,3203 NEGTESTS | interpretive | CHANG2 resume |
| 25,3205  | interpretive | CHANG2 resume |
| 25,3207  | interpretive | CHANG2 resume |
| 25,3212  | interpretive | CHANG2 resume |
| 25,3214  | interpretive | CHANG2 resume |
| 25,3217  | interpretive | CHANG2 resume |
| 25,3221 DCONSTD | interpretive | STCALL return |
| 25,3223  | interpretive | CHANG2 resume |
| 25,3225  | interpretive | CHANG2 resume |
| 25,3226 BECONSTD | interpretive | CHANG2 resume |
| 25,3232 CONSTD | interpretive | after RTB KILLGRP4 |
| 25,3234  | interpretive | CHANG2 resume |
| 25,3237  | interpretive | CHANG2 resume |
| 25,3242  | interpretive | CHANG2 resume |
| 25,3245  | interpretive | CHANG2 resume |
| 25,3247  | interpretive | CHANG2 resume |
| 25,3251 CONSTD1 | interpretive | CHANG2 resume |
| 25,3253  | interpretive | CHANG2 resume |
| 25,3255  | interpretive | CHANG2 resume |
| 25,3260 DOWNCNTL | interpretive | BPL target |
| 25,3262  | interpretive | CHANG2 resume |
| 25,3265  | interpretive | CHANG2 resume |
| 25,3270  | interpretive | CHANG2 resume |
| 25,3273  | interpretive | CHANG2 resume |
| 25,3275  | interpretive | CHANG2 resume |
| 25,3300  | interpretive | CHANG2 resume |
| 25,3302  | interpretive | CHANG2 resume |
| 25,3305  | interpretive | CHANG2 resume |
| 25,3307  | interpretive | CHANG2 resume |
| 25,3311  | interpretive | CHANG2 resume |
| 25,3313 KEP | interpretive | BMN target |
| 25,3314  | interpretive | pending DANZIG after RTB P66 |
| 25,3317 KEP2 | interpretive | after RTB P66 |
| 25,3322  | interpretive | CHANG2 resume |
| 25,3325  | interpretive | CHANG2 resume |
| 25,3331  | interpretive | switch branch target |
| 25,3333 PREFINAL | interpretive | BMN target |
| 25,3337  | interpretive | after RTB P67 |
| 25,3343 PREDICT3 | interpretive | SSP PREDICT3 |
| 25,3346  | interpretive | CHANG2 resume |
| 25,3350  | basic | after EXIT |
| 25,3437  | interpretive | after TC INTPRET |
| 25,3441  | interpretive | CHANG2 resume |
| 25,3446  | interpretive | CHANG2 resume |
| 25,3451  | interpretive | CHANG2 resume |
| 25,3453 HAVDNRNG | interpretive | switch branch target |
| 25,3454  | interpretive | CHANG2 resume |
| 25,3455  | interpretive | CHANG2 resume |
| 25,3460  | interpretive | CHANG2 resume |
| 25,3463  | interpretive | CHANG2 resume |
| 25,3466  | interpretive | CHANG2 resume |
| 25,3470 GONEGLAD | interpretive | switch branch target |
| 25,3472 GOMAXL/D | interpretive | BOV target |
| 25,3473  | interpretive | pending DANZIG after RTB SIGNMPAC |
| 25,3475  | interpretive | after RTB SIGNMPAC |
| 25,3476 GLIMITER | interpretive | STCALL target |
| 25,3501  | interpretive | CHANG2 resume |
| 25,3504  | interpretive | CHANG2 resume |
| 25,3507  | interpretive | CHANG2 resume |
| 25,3512  | interpretive | CHANG2 resume |
| 25,3514  | interpretive | CHANG2 resume |
| 25,3517  | interpretive | CHANG2 resume |
| 25,3520  | interpretive | CHANG2 resume |
| 25,3523 GOPOSLAD | interpretive | BMN target |
| 25,3525 STOREL/D | interpretive | CHANG2 resume |
| 25,3526 LIMITL/D | interpretive | BPL target |
| 25,3530  | interpretive | CHANG2 resume |
| 25,3532  | interpretive | CHANG2 resume |
| 25,3535  | interpretive | CHANG2 resume |
| 25,3540 L350 | interpretive | CHANG2 resume |
| 25,3542  | interpretive | CHANG2 resume |
| 25,3545  | interpretive | CHANG2 resume |
| 25,3550  | interpretive | CHANG2 resume |
| 25,3552  | interpretive | CHANG2 resume |
| 25,3553 L353 | interpretive | BMN target |
| 25,3556  | interpretive | CHANG2 resume |
| 25,3557  | interpretive | CHANG2 resume |
| 25,3562  | interpretive | CHANG2 resume |
| 25,3565  | interpretive | CHANG2 resume |
| 25,3566 L355 | interpretive | switch branch target |
| 25,3571  | interpretive | CHANG2 resume |
| 25,3572  | interpretive | CHANG2 resume |
| 25,3575  | interpretive | CHANG2 resume |
| 25,3576 ENDEXIT | interpretive | entry ENDEXIT (P61-P67.agc  ENDEXIT) |
| 25,3577 OVERNOUT | basic | after EXIT |
| 25,3606 NODISKY | basic | return from BANKCALL |
| 25,3611 SERVNOUT | basic | entry SERVNOUT (P61-P67.agc  SERVNOUT) |
| 25,3613 STEEROFF | interpretive | BMN target |
| 25,3614  | basic | after EXIT |
| 25,3622  | basic | return from NOVAC |
| 25,3625  | basic | return from 2PHSCHNG |
| 25,3630  | interpretive | after TC INTPRET |
| 25,3632 L357 | interpretive | BMN target |
| 25,3635  | interpretive | CHANG2 resume |
| 26,3074 GETETA | interpretive | switch branch target |
| 26,3077  | interpretive | CHANG2 resume |
| 26,3102  | interpretive | CHANG2 resume |
| 26,3104  | interpretive | CHANG2 resume |
| 26,3107  | interpretive | CHANG2 resume |
| 26,3110  | interpretive | CHANG2 resume |
| 26,3114  | interpretive | CHANG2 resume |
| 26,3117 SUBETA | interpretive | switch branch target |
| 26,3122  | interpretive | CHANG2 resume |
| 26,3125 SUBETA2 | interpretive | BPL target |
| 26,3130  | interpretive | CHANG2 resume |
| 26,3133 SETMIND | interpretive | BZE target |
| 26,3136 TINYTHET | interpretive | BPL target |
| 26,3140  | interpretive | CHANG2 resume |
| 26,3142  | interpretive | CHANG2 resume |

Destinations outside the file: 2PHSCHNG, BANKCALL, CHANG1, CHANG2, NOVAC, PHASCHNG, POSTJUMP, PRIOCHNG, RTB KILLGRP4, RTB P64, RTB P65, RTB P66, RTB P67, RTB SIGNMPAC. Lane A has not yet registered the Comanche Executive and Waitlist routines (WAITLIST, NOVAC, FINDVAC, TASKOVER, ENDOFJOB, IBNKCALL with ISWRETRN, POSTJUMP, BANKCALL, RESUME) or the T5RUPT/T6RUPT vectors that dispatch T5LOC/T6LOC; until it does, a transfer to them stops with EXEC_UNMAPPED at the frontier.

## Domain

Canonical 2048-word image (every word random, the counters, `TIME1`-`TIME6`, the CDUs and the save registers at 010-061 included) under the routine's `EBANK=`, FB of the routine's bank, any A, L and Q unless stated, and any values of the other erasables. `NEWJOB` is +0 or, in a quarter of the cases, positive: a job waiting, so any `DANZIG` goes to `CHANG2`, where the routine stops; every interpretive word that can follow a `DANZIG` is a `CHANG2` resume key of its own. Each routine runs under the EBANK= the listing has in effect at its first word. Interpretive routines are entered at NEWOPS with FIXLOC a VAC area. The two pairs with an RTB on the left and an order on the right (KEP's RTB P66 / SSP at 25,3313 and RTB SIGNMPAC / DMP at 25,3472) each also have a routine keyed at the RTB's operand word, entered as the RTB's basic code leaves it at TC DANZIG: Z at DANZIG, EDOP the pair's right order, LOC at that word, BANKSET the pair's bank and FB and BBANK any bank (include/agc_transfer.h). A BOVB TCDANZIG (on overflow) runs ENDT(X)'s TC DANZIG natively (cm_integration_tcdanzig) and the routine goes on with the pair's right-hand order, as in flight (#414). PREDICT3 reads VREFER and the tables after it at JJ 0 to 91 (13 words per BACK2 pass). The repository copy of the file differs from the flight rope only in text that assembles to the same words: the label GETDLEWD where its one reference says GETLEWD (repaired to GETLEWD in the assembled listing), DEC .063296 (the rope listing has .063298; both assemble to 02015) and KT1's exponent spacing. LEWD (E7,1724-1725) is any positive double word, as STARTENT, LEWDOVFL and LWDSTORE (DAD, BMN, BOV, STORE: the words need not agree in sign) leave it: a nonzero high word over a low word of either sign, or a +0 or -0 high word over a positive low word (spec cell_pairs). Negative, LWDSTORE's BMN LEWDPTR loop never ends on the AGC either.

## Evidence

Hashes: `evidence/cm-reentry/SHA256.json`; all files are in the tree.

- **Source map.** `cm-reentry-source-map.json` lists all 1172 words of the file with the routine that compiles each (or why a constant is never run as code); 0 are untranslated.
- **Independent assembly comparison.** `make cm-reentry-differential` runs 256 states per routine (100,352 cases) through the original Comanche055 rope in yaAGC (pinned yaAGC with hardware DV and without the IMU/optics device block (tools/prepare_flight_device_engine.py), its ScalerCounter held at 0 (TIME1-TIME6 and the timer interrupts do not advance during a case)) and the native routines; all are equal in Z, 2048 erasable words (A, L 16-bit) and the interrupt-enable state. The cases and the stops reached per routine are in `cm-reentry-proof.json` (`cm-reentry-differential.log` is the run). One case in 128 is frozen in `tests/cm_reentry_vectors.h`.
- **Host and sanitizer results.** `make test-cm-reentry sanitize-cm-reentry` runs the 784 frozen cases (`cm-reentry-host.log`).
- **C64 execution results.** `make vice-cm-reentry` runs 47 diagnostic programs with 544 cases (`cm-reentry-vice.log`, which starts with each PRG's SHA-256). The cases are chosen so the C64 runs take every control-flow edge (both outcomes of each branch) of every linked source that the 100,352 differential cases take: 4227 of 4227 edges and comparison outcomes (clang -O0 trace-pc-guard,trace-cmp) and 90 of 90 compiled labels. Checked again with gcov -b over gcc -O0 `--coverage` builds of the same sources: the C64 cases take 2007 of the 2007 branch outcomes all the cases take (`c64_gcov_branches` in the proof).
