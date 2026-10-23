# Translate Comanche STABLE_ORBIT (P38-P39)

Lane G (CM flight). Milestone `cm-stable-orbit`. How the routines are made and checked: `docs/INTERP-COMPILE.md` (whole-file basic code: `tools/basic_compile.py`).

`Comanche055/STABLE_ORBIT.agc` (`src/cm_stable_orbit.c`) is native C in full: The stable-orbit rendezvous programs P38 and P39 (and the P78 and P79 entries) with their precision updates, recycle tests and V06 displays, basic and interpretive. Each routine runs the file from its entry, or from where it resumes, to the point where control leaves the file. Every original word is one call of the basic-instruction primitives (`src/agc_image_ops.c`) under a label named for its address, with TS overflow skips, CCS four-way skips and INDEX on the whole word kept; computed transfers (TC Q, indexed TCF) go through one dispatch on the target address.

## Scope

104 routines, keyed by physical address in `cm_stable_orbit_basic` and `cm_stable_orbit_interpretive`:

| Entry | Kind | Why it is a routine |
|---|---|---|
| 04,2742 P38 | basic | entry P38 (FRESH_START_AND_RESTART.agc FCADR P38) |
| 04,2743  | basic | return from AVFLAGA |
| 04,2744 P78 | basic | entry P78 (FRESH_START_AND_RESTART.agc FCADR P78) |
| 04,2745  | basic | return from AVFLAGP |
| 04,2746  | basic | return from P20FLGON |
| 04,2750  | basic | TC 02750 stored in E4,1767 |
| 04,2753  | basic | return from BANKCALL |
| 04,2754  | basic | resumed from outside |
| 04,2755  | basic | resumed from outside |
| 04,2756  | basic | resumed from outside |
| 04,2760  | basic | return from BLANKET |
| 04,2770  | basic | return from BANKCALL |
| 04,2771  | basic | resumed from outside |
| 04,2772  | basic | resumed from outside |
| 04,2773  | basic | resumed from outside |
| 04,2775  | basic | return from BLANKET |
| 04,2777  | interpretive | after TC INTPRET |
| 04,3002  | interpretive | CHANG2 resume |
| 04,3004  | interpretive | CHANG2 resume |
| 04,3007  | interpretive | CHANG2 resume |
| 04,3010  | interpretive | CHANG2 resume |
| 04,3013 OPTN1 | interpretive | BHIZ target |
| 04,3016  | interpretive | CHANG2 resume |
| 04,3020  | interpretive | CALL return |
| 04,3023  | interpretive | CHANG2 resume |
| 04,3024  | interpretive | CHANG2 resume |
| 04,3025  | interpretive | CHANG2 resume |
| 04,3026  | basic | after EXIT |
| 04,3031  | basic | return from BANKCALL |
| 04,3032  | basic | resumed from outside |
| 04,3033  | basic | resumed from outside |
| 04,3034  | basic | resumed from outside |
| 04,3036  | basic | return from BLANKET |
| 04,3041  | basic | TC 03041 stored in E4,1767 |
| 04,3042 JUNCTN1 | interpretive | after TC INTPRET |
| 04,3045 RECYCLE | interpretive | GOTO target |
| 04,3047  | interpretive | CALL return |
| 04,3053  | interpretive | CHANG2 resume |
| 04,3055  | interpretive | STCALL return |
| 04,3057  | interpretive | CHANG2 resume |
| 04,3061  | interpretive | CHANG2 resume |
| 04,3063  | interpretive | CHANG2 resume |
| 04,3065  | interpretive | CHANG2 resume |
| 04,3067  | interpretive | CHANG2 resume |
| 04,3071 OPTN2 | interpretive | switch branch target |
| 04,3074  | interpretive | CHANG2 resume |
| 04,3075 JUNCTN2 | interpretive | STCALL target |
| 04,3100  | interpretive | CHANG2 resume |
| 04,3101 MAINRTNE | interpretive | CHANG2 resume |
| 04,3103  | interpretive | STCALL return |
| 04,3105  | interpretive | CHANG2 resume |
| 04,3106  | interpretive | CHANG2 resume |
| 04,3112  | interpretive | CHANG2 resume |
| 04,3114 TEST3979 | interpretive | SSP TEST3979 |
| 04,3121  | interpretive | CHANG2 resume |
| 04,3123 P39P79 | interpretive | switch branch target |
| 04,3124  | basic | after EXIT |
| 04,3125 MAINRTN1 | interpretive | switch branch target |
| 04,3127  | interpretive | CHANG2 resume |
| 04,3131  | interpretive | CHANG2 resume |
| 04,3133  | interpretive | CHANG2 resume |
| 04,3135  | interpretive | CHANG2 resume |
| 04,3140  | interpretive | CALL return |
| 04,3142  | interpretive | CALL return |
| 04,3143  | interpretive | CHANG2 resume |
| 04,3147 DSPLY58 | interpretive | switch branch target |
| 04,3150  | basic | after EXIT |
| 04,3152 DSPLY81 | basic | TC 03152 stored in E4,1767 |
| 04,3154  | basic | TC 03154 stored in E4,1767 |
| 04,3155  | interpretive | after TC INTPRET |
| 04,3160  | interpretive | CHANG2 resume |
| 04,3162  | interpretive | STCALL return |
| 04,3166 P39 | basic | entry P39 (FRESH_START_AND_RESTART.agc FCADR P39) |
| 04,3167  | basic | return from AVFLAGA |
| 04,3172 P79 | basic | entry P79 (FRESH_START_AND_RESTART.agc FCADR P79) |
| 04,3173  | basic | return from AVFLAGP |
| 04,3177  | basic | return from P20FLGON |
| 04,3200  | interpretive | after TC INTPRET |
| 04,3203 P39/P79B | interpretive | switch branch target |
| 04,3204  | interpretive | pending DANZIG after RTB LOADTIME (DAD KT on its right; entered at DANZIG with LOC at the operand word, through the CM DANZIG marker) |
| 04,3206  | interpretive | after RTB LOADTIME |
| 04,3207  | interpretive | CHANG2 resume |
| 04,3211  | interpretive | STCALL return |
| 04,3213  | interpretive | CALL return |
| 04,3216 PREC/TT | interpretive | CALL target |
| 04,3221  | interpretive | CHANG2 resume |
| 04,3223  | interpretive | STCALL return |
| 04,3226  | interpretive | CHANG2 resume |
| 04,3230  | interpretive | CHANG2 resume |
| 04,3231  | interpretive | CHANG2 resume |
| 04,3232  | interpretive | CHANG2 resume |
| 04,3234  | interpretive | CHANG2 resume |
| 04,3236  | interpretive | CHANG2 resume |
| 04,3240  | interpretive | CHANG2 resume |
| 04,3242  | interpretive | STCALL return |
| 04,3244  | interpretive | CALL return |
| 04,3247 INTRPVP | interpretive | STCALL target |
| 04,3253  | interpretive | CHANG2 resume |
| 04,3255  | interpretive | CALL return |
| 04,3257 OTHERV | interpretive | switch branch target |
| 04,3261  | interpretive | CALL return |
| 04,3271  | basic | return from BANKCALL |
| 04,3272  | basic | resumed from outside |
| 04,3273  | basic | resumed from outside |
| E4,1767 (erasable) | continuation | the TC words the file stores there: 02750, 03041, 03152, 03154 |

Destinations outside the file: AVFLAGA, AVFLAGP, BANKCALL, BLANKET, CHANG2, ENDOFJOB, GOTOPOOH, P20FLGON, RTB LOADTIME. Lane A has not yet registered the Comanche Executive and Waitlist routines (WAITLIST, NOVAC, FINDVAC, TASKOVER, ENDOFJOB, IBNKCALL with ISWRETRN, POSTJUMP, BANKCALL, RESUME) or the T5RUPT/T6RUPT vectors that dispatch T5LOC/T6LOC; until it does, a transfer to them stops with EXEC_UNMAPPED at the frontier.

## Domain

Canonical 2048-word image (every word random, the counters, `TIME1`-`TIME6`, the CDUs and the save registers at 010-061 included) under the routine's `EBANK=`, FB of the routine's bank, any A, L and Q unless stated, and any values of the other erasables. `NEWJOB` is +0 or, in a quarter of the cases, positive: a job waiting, so any `DANZIG` goes to `CHANG2`, where the routine stops; every interpretive word that can follow a `DANZIG` is a `CHANG2` resume key of its own. Any values of the program erasables; interpretive routines entered at NEWOPS with FIXLOC a VAC area, resume points starting from the state the preceding orders leave. Return addresses outside the file (QPRET and the other outside CADRs) are never a word of the file, a key it registers or DANZIG, and at the file's basic entries EDOP holds no right-hand order still to run while LOC (under BANKSET) points at such a word, other than the RTB operand pairs (#553: either state would run the file's own words as orders, which no flight caller leaves).

## Evidence

Hashes: `evidence/cm-stable-orbit/SHA256.json`; all files are in the tree.

- **Source map.** `cm-stable-orbit-source-map.json` lists all 225 words of the file with the routine that compiles each (or why a constant is never run as code); 0 are untranslated.
- **Independent assembly comparison.** `make cm-stable-orbit-differential` runs 256 states per routine (26,624 cases) through the original Comanche055 rope in yaAGC (pinned yaAGC, its ScalerCounter held at 0 (TIME1-TIME6 and the timer interrupts do not advance during a case)) and the native routines; all are equal in Z, 2048 erasable words (A, L 16-bit) and the interrupt-enable state. The cases and the stops reached per routine are in `cm-stable-orbit-proof.json` (`cm-stable-orbit-differential.log` is the run). One case in 128 is frozen in `tests/cm_stable_orbit_vectors.h`.
- **Host and sanitizer results.** `make test-cm-stable-orbit sanitize-cm-stable-orbit` runs the 208 frozen cases (`cm-stable-orbit-host.log`).
- **C64 execution results.** `make vice-cm-stable-orbit` runs 3 diagnostic programs with 140 cases (`cm-stable-orbit-vice.log`, which starts with each PRG's SHA-256). The cases are chosen so the C64 runs take every control-flow edge (both outcomes of each branch) of every linked source that the 26,624 differential cases take: 1733 of 1733 edges and comparison outcomes (clang -O0 trace-pc-guard,trace-cmp) and 64 of 64 compiled labels. Checked again with gcov -b over gcc -O0 `--coverage` builds of the same sources: the C64 cases take 881 of the 881 branch outcomes all the cases take (`c64_gcov_branches` in the proof).

## Flight build

The spec sets `trusted_image`. In flight builds (`RK_FIXED_IMAGE`), the entry checks skip their scan of the whole erasable image, which costs about 121,000 cycles a routine on the C64. Host tests and C64 diagnostics keep the scan.
