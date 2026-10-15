# Translate Comanche SERVICER207

Lane G (CM flight). Milestone `cm-servicer`. How the routines are made and checked: `docs/INTERP-COMPILE.md` (whole-file basic code: `tools/basic_compile.py`).

`Comanche055/SERVICER207.agc` (`src/cm_servicer.c`) is native C in full: The average-G servicer: PREREAD, READACCS, PIPSDONE, SERVICER, CALCGRAV/CALCRVG, AVGEND, NORMLIZE and the restart points, basic and interpretive. Each routine runs the file from its entry, or from where it resumes, to the point where control leaves the file. Every original word is one call of the basic-instruction primitives (`src/agc_image_ops.c`) under a label named for its address, with TS overflow skips, CCS four-way skips and INDEX on the whole word kept; computed transfers (TC Q, indexed TCF) go through one dispatch on the target address.

## Scope

80 routines, keyed by physical address in `cm_servicer_basic` and `cm_servicer_interpretive`:

| Entry | Kind | Why it is a routine |
|---|---|---|
| 37,2701 PREREAD | basic | entry PREREAD (RESTART_TABLES.agc -2CADR PREREAD) |
| 37,2705 REDO5.31 | basic | return from NOVAC |
| 37,2712  | basic | return from FINDVAC |
| 37,2716  | basic | return from WAITLIST |
| 37,2721  | basic | return from NEWPHASE |
| 37,2722 PREREAD1 | basic | entry PREREAD1 (P11.agc CADR PREREAD1) |
| 37,2744 READACCS | basic | entry READACCS (P11.agc 2CADR READACCS) |
| 37,2745 PIPSDONE | basic | GENADR PIPSDONE |
| 37,2751 REDO5.5 | basic | entry REDO5.5 (RESTART_TABLES.agc -2CADR REDO5.5) |
| 37,3005  | basic | return from WAITLIST |
| 37,3024  | basic | return from WAITLIST |
| 37,3027  | basic | return from NEWPHASE |
| 37,3033 CHEKAVEG | basic | entry CHEKAVEG (RESTART_TABLES.agc -2CADR CHEKAVEG) |
| 37,3043 MAKESERV | basic | return from WAITLIST |
| 37,3047  | basic | return from FINDVAC |
| 37,3052  | basic | return from NEWPHASE |
| 37,3067 QUIKREAD | basic | 2CADR QUIKREAD |
| 37,3104 SERVICER | basic | entry SERVICER (RESTART_TABLES.agc 2CADR SERVICER) |
| 37,3121  | basic | return from ALARM |
| 37,3131  | basic | return from PHASCHNG |
| 37,3133 DVTOTUP | basic | 2CADR DVTOTUP |
| 37,3134  | interpretive | after TC INTPRET |
| 37,3136  | interpretive | CHANG2 resume |
| 37,3140  | basic | after EXIT |
| 37,3145  | basic | return from PHASCHNG |
| 37,3146  | interpretive | after TC INTPRET |
| 37,3150  | interpretive | CALL return |
| 37,3151  | basic | after EXIT |
| 37,3153  | basic | return from PHASCHNG |
| 37,3157  | basic | return from GENTRAN |
| 37,3162  | basic | return from PHASCHNG |
| 37,3165 AVGEND | basic | 2CADR AVGEND |
| 37,3171  | basic | return from UPFLAG |
| 37,3175  | basic | return from 2PHSCHNG |
| 37,3176  | interpretive | after TC INTPRET |
| 37,3200  | interpretive | CALL return |
| 37,3201  | basic | after EXIT |
| 37,3206  | basic | return from BANKCALL |
| 37,3214  | basic | return from DOWNFLAG |
| 37,3216  | basic | return from DOWNFLAG |
| 37,3225  | basic | return from 2PHSCHNG |
| 37,3227 SERVEXIT | basic | entry SERVEXIT (P40-P47.agc 2CADR SERVEXIT) |
| 37,3231  | basic | return from PHASCHNG |
| 37,3236 NORMLIZE | basic | entry NORMLIZE (RESTART_TABLES.agc 2CADR NORMLIZE) |
| 37,3242  | basic | return from GENTRAN |
| 37,3244  | interpretive | after TC INTPRET |
| 37,3247  | interpretive | CALL return |
| 37,3251  | interpretive | CHANG2 resume |
| 37,3252  | interpretive | CHANG2 resume |
| 37,3253  | basic | after EXIT |
| 37,3305 REREADAC | basic | entry REREADAC (RESTART_TABLES.agc -2CADR REREADAC) |
| 37,3353 CALCGRAV | interpretive | entry CALCGRAV (P40-P47.agc  CALCGRAV) |
| 37,3354  | interpretive | CHANG2 resume |
| 37,3355  | interpretive | CHANG2 resume |
| 37,3360  | interpretive | CHANG2 resume |
| 37,3362  | interpretive | CHANG2 resume |
| 37,3364  | interpretive | CHANG2 resume |
| 37,3366  | interpretive | CHANG2 resume |
| 37,3371  | interpretive | CHANG2 resume |
| 37,3372  | interpretive | CHANG2 resume |
| 37,3374  | interpretive | CHANG2 resume |
| 37,3376  | interpretive | CHANG2 resume |
| 37,3401  | interpretive | CHANG2 resume |
| 37,3403  | interpretive | CHANG2 resume |
| 37,3405  | interpretive | CHANG2 resume |
| 37,3407 ITISMOON | interpretive | BMN target |
| 37,3412  | interpretive | CHANG2 resume |
| 37,3415  | interpretive | CHANG2 resume |
| 37,3417  | interpretive | CHANG2 resume |
| 37,3420 CALCRVG | interpretive | CALL target |
| 37,3423  | interpretive | CHANG2 resume |
| 37,3425  | interpretive | CHANG2 resume |
| 37,3426  | interpretive | CHANG2 resume |
| 37,3427  | interpretive | CHANG2 resume |
| 37,3431  | interpretive | CHANG2 resume |
| 37,3434  | interpretive | CHANG2 resume |
| 37,3437  | interpretive | CHANG2 resume |
| 37,3441  | interpretive | STCALL return |
| 37,3442  | interpretive | CHANG2 resume |
| 37,3444  | interpretive | CHANG2 resume |

Destinations outside the file: 2PHSCHNG, ALARM, BANKCALL, CHANG2, DOWNFLAG, ENDOFJOB, FINDVAC, FIXDELAY, GENTRAN, NEWPHASE, NOVAC, PHASCHNG, POSTJUMP, TASKOVER, UPFLAG, WAITLIST. Lane A has not yet registered the Comanche Executive and Waitlist routines (WAITLIST, NOVAC, FINDVAC, TASKOVER, ENDOFJOB, IBNKCALL with ISWRETRN, POSTJUMP, BANKCALL, RESUME) or the T5RUPT/T6RUPT vectors that dispatch T5LOC/T6LOC; until it does, a transfer to them stops with EXEC_UNMAPPED at the frontier.

## Domain

Canonical 2048-word image (every word random, the counters, `TIME1`-`TIME6`, the CDUs and the save registers at 010-061 included) under EB 6 (the file's `EBANK=`), FB of the routine's bank, any A, L and Q unless stated, and any values of the other erasables. `NEWJOB` is +0 or, in a quarter of the cases, positive: a job waiting, so any `DANZIG` goes to `CHANG2`, where the routine stops; every interpretive word that can follow a `DANZIG` is a `CHANG2` resume key of its own. Any values of the servicer erasables; interpretive routines entered at NEWOPS with FIXLOC a VAC area.

## Evidence

Hashes: `evidence/cm-servicer/SHA256.json`; all files are in the tree.

- **Source map.** `cm-servicer-source-map.json` lists all 375 words of the file with the routine that compiles each (or why a constant is never run as code); 0 are untranslated.
- **Independent assembly comparison.** `make cm-servicer-differential` runs 256 states per routine (20,480 cases) through the original Comanche055 rope in yaAGC (pinned yaAGC, its ScalerCounter held at 0 (TIME1-TIME6 and the timer interrupts do not advance during a case)) and the native routines; all are equal in Z, 2048 erasable words (A, L 16-bit) and the interrupt-enable state, the I/O channels 11 and the 31A/31B/32 trap flags. The cases and the stops reached per routine are in `cm-servicer-proof.json` (`cm-servicer-differential.log` is the run). One case in 128 is frozen in `tests/cm_servicer_vectors.h`.
- **Host and sanitizer results.** `make test-cm-servicer sanitize-cm-servicer` runs the 160 frozen cases (`cm-servicer-host.log`).
- **C64 execution results.** `make vice-cm-servicer` runs 10 diagnostic programs with 122 cases (`cm-servicer-vice.log`, which starts with each PRG's SHA-256). The cases are chosen so the C64 runs take every control-flow edge (both outcomes of each branch) of every linked source that the 20,480 differential cases take: 1765 of 1765 edges and comparison outcomes (clang -O0 trace-pc-guard,trace-cmp) and 240 of 240 compiled labels. Checked again with gcov -b over gcc -O0 `--coverage` builds of the same sources: the C64 cases take 989 of the 989 branch outcomes all the cases take (`c64_gcov_branches` in the proof).

## Flight build

The spec sets `trusted_image`, `image_w` and `index_carry16`. In flight builds (`RK_FIXED_IMAGE`), the entry checks skip their scan of the whole erasable image, which costs about 121,000 cycles a routine on the C64. That scan was the top cost of the CM entry window in VICE (lane-g/cm-entry-profile-80342904.md in the project files). Host tests and C64 diagnostics keep the scan.
