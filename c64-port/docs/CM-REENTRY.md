# Translate Comanche REENTRY_CONTROL

Lane G (CM flight). Milestone `cm-reentry`. How the routines are made and checked: `docs/INTERP-COMPILE.md` (whole-file basic code: `tools/basic_compile.py`).

`Comanche055/REENTRY_CONTROL.agc` (`src/cm_reentry.c`) is native C but for 2 words: The entry guidance: STARTENT and the two-second steering loop (INITROLL, HUNTEST, UPCONTRL, KEP2/BALLISTIC, PREDICT3 with its reference-trajectory tables, the roll command and lift-vector logic, the G-limit and final-phase tests), with the entry displays and restart points, basic and interpretive. Each routine runs the file from its entry, or from where it resumes, to the point where control leaves the file. Every original word is one call of the basic-instruction primitives (`src/agc_image_ops.c`) under a label named for its address, with TS overflow skips, CCS four-way skips and INDEX on the whole word kept; computed transfers (TC Q, indexed TCF) go through one dispatch on the target address.

Not yet translated: SSP GOTOADDR KEP2 (25,3315-3316), the order KEP leaves in EDOP across RTB P66: it runs from the interpreter's DANZIG/OPJUMP when P66 returns, a point the transfer ABI does not key yet (interpretive routines are keyed by LOC at NEWOPS). The file stays partial until that point is keyed; no complete certificate is drafted.

## Scope

102 routines, keyed by physical address in `cm_reentry_basic` and `cm_reentry_interpretive`:

| Entry | Kind | Why it is a routine |
|---|---|---|
| 25,2000 STARTENT | interpretive | entry STARTENT (P61-P67.agc CADR STARTENT) |
| 25,2001  | basic | after EXIT |
| 25,2010  | interpretive | after TC INTPRET |
| 25,2036  | interpretive | STCALL return |
| 25,2046  | interpretive | after RTB SIGNMPAC |
| 25,2063 STARTEN1 | interpretive | entry STARTEN1 (P61-P67.agc  STARTEN1) |
| 25,2077  | interpretive | STCALL return |
| 25,2104  | interpretive | STCALL return |
| 25,2115 SCALEPOP | interpretive | SSP SCALEPOP |
| 25,2117  | interpretive | CALL return |
| 25,2120 REFAZE10 | basic | entry REFAZE10 (P61-P67.agc  REFAZE10) |
| 25,2122  | basic | return from PHASCHNG |
| 25,2123  | interpretive | after TC INTPRET |
| 25,2125 TARGETNG | interpretive | CALL target |
| 25,2133 GETVEL | interpretive | switch branch target |
| 25,2136  | interpretive | GOTO target |
| 25,2162 DSTORE | interpretive | GOTO target |
| 25,2173 UPDATERT | interpretive | GOTO target |
| 25,2200  | interpretive | STCALL return |
| 25,2215  | interpretive | BPL target |
| 25,2226 THETDONE | interpretive | GOTO target |
| 25,2232  | interpretive | switch branch target |
| 25,2253 NOLDCALC | interpretive | BOV target |
| 25,2255 NO.05G | interpretive | BMN target |
| 25,2260 INITROLL | interpretive | SSP INITROLL |
| 25,2275  | interpretive | after RTB P64 |
| 25,2304  | interpretive | BMN target |
| 25,2316 D0EQ | interpretive | BPL target |
| 25,2327  | interpretive | BOV target |
| 25,2351 INROLOUT | interpretive | GOTO target |
| 25,2354 KATEST | interpretive | BMN target |
| 25,2362 INITRL1 | interpretive | switch branch target |
| 25,2370  | interpretive | CALL return |
| 25,2373 HUNTEST | interpretive | SSP HUNTEST |
| 25,2406 A0CALC | interpretive | BMN target |
| 25,2433 V1LEAD | interpretive | BPL target |
| 25,2442 HUNTEST1 | interpretive | BPL target |
| 25,2533 GETDHOOK | interpretive | BMN target |
| 25,2536  | interpretive | CALL return |
| 25,2561 HUNTEST3 | interpretive | GOTO target |
| 25,2633  | interpretive | CALL return |
| 25,2677 GETLEWD | interpretive | switch branch target |
| 25,2706 LWDSTORE | interpretive | GOTO target |
| 25,2717  | basic | after EXIT |
| 25,2725  | basic | return from PHASCHNG |
| 25,2727  | basic | return from PRIOCHNG |
| 25,2732  | interpretive | after TC INTPRET |
| 25,2741 LEWDOVFL | interpretive | BOV target |
| 25,2745 LEWDPTR | interpretive | BMN target |
| 25,2751 NEGAMA | interpretive | BMN target |
| 25,2775  | interpretive | STCALL return |
| 25,3004 DHOOKYQ7 | interpretive | CALL target |
| 25,3014 PRE-HUNT | basic | entry PRE-HUNT (RESTART_TABLES.agc 2CADR PRE-HUNT) |
| 25,3015  | interpretive | after TC INTPRET |
| 25,3020  | interpretive | CALL return |
| 25,3022 FOREHUNT | interpretive | CALL target |
| 25,3033 GOTOUPSY | interpretive | BMN target |
| 25,3035 UPCONTRL | interpretive | entry UPCONTRL (P61-P67.agc  UPCONTRL) |
| 25,3043  | interpretive | BMN target |
| 25,3065 CONT1 | interpretive | BPL target |
| 25,3075 CONT3 | interpretive | BMN target |
| 25,3134 CONTINU2 | interpretive | BMN target |
| 25,3152 UPCNTRL3 | interpretive | BMN target |
| 25,3200 NEXT1 | interpretive | BMN target |
| 25,3203 NEGTESTS | interpretive | GOTO target |
| 25,3221 DCONSTD | interpretive | STCALL return |
| 25,3226 BECONSTD | interpretive | BMN target |
| 25,3232 CONSTD | interpretive | after RTB KILLGRP4 |
| 25,3251 CONSTD1 | interpretive | GOTO target |
| 25,3260 DOWNCNTL | interpretive | BPL target |
| 25,3313 KEP | interpretive | BMN target |
| 25,3317 KEP2 | interpretive | SSP KEP2 |
| 25,3331  | interpretive | switch branch target |
| 25,3333 PREFINAL | interpretive | BMN target |
| 25,3337  | interpretive | after RTB P67 |
| 25,3343 PREDICT3 | interpretive | SSP PREDICT3 |
| 25,3350  | basic | after EXIT |
| 25,3437  | interpretive | after TC INTPRET |
| 25,3453 HAVDNRNG | interpretive | switch branch target |
| 25,3470 GONEGLAD | interpretive | switch branch target |
| 25,3472 GOMAXL/D | interpretive | BOV target |
| 25,3476 GLIMITER | interpretive | STCALL target |
| 25,3523 GOPOSLAD | interpretive | BMN target |
| 25,3525 STOREL/D | interpretive | GOTO target |
| 25,3526 LIMITL/D | interpretive | BPL target |
| 25,3553 L353 | interpretive | BMN target |
| 25,3566 L355 | interpretive | switch branch target |
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
| 26,3074 GETETA | interpretive | switch branch target |
| 26,3117 SUBETA | interpretive | switch branch target |
| 26,3125 SUBETA2 | interpretive | BPL target |
| 26,3133 SETMIND | interpretive | BZE target |
| 26,3136 TINYTHET | interpretive | BPL target |

Destinations outside the file: 2PHSCHNG, BANKCALL, BOVB ENDT(X), CHANG1, NOVAC, PHASCHNG, POSTJUMP, PRIOCHNG, RTB KILLGRP4, RTB P64, RTB P65, RTB P66, RTB P67, RTB SIGNMPAC. Lane A has not yet registered the Comanche Executive and Waitlist routines (WAITLIST, NOVAC, FINDVAC, TASKOVER, ENDOFJOB, IBNKCALL with ISWRETRN, POSTJUMP, BANKCALL, RESUME) or the T5RUPT/T6RUPT vectors that dispatch T5LOC/T6LOC; until it does, a transfer to them stops with EXEC_UNMAPPED at the frontier.

## Domain

Canonical 2048-word image (every word random, the counters, `TIME1`-`TIME6`, the CDUs and the save registers at 010-061 included) under the routine's `EBANK=`, FB of the routine's bank, any A, L and Q unless stated, and any values of the other erasables. Each routine runs under the EBANK= the listing has in effect at its first word. Interpretive routines are entered at NEWOPS with FIXLOC a VAC area. PREDICT3 reads VREFER and the tables after it at JJ 0 to 91 (13 words per BACK2 pass). The repository copy of the file differs from the flight rope only in text that assembles to the same words: the label GETDLEWD where its one reference says GETLEWD (repaired to GETLEWD in the assembled listing), DEC .063296 (the rope listing has .063298; both assemble to 02015) and KT1's exponent spacing. LEWD (E7,1724-1725) is positive in both words, as STARTENT and LEWDOVFL leave it: negative, LWDSTORE's BMN LEWDPTR loop never ends on the AGC either.

## Evidence

Hashes: `evidence/cm-reentry/SHA256.json`; all files are in the tree.

- **Source map.** `cm-reentry-source-map.json` lists all 1172 words of the file with the routine that compiles each (or why a constant is never run as code); 2 are untranslated.
- **Independent assembly comparison.** `make cm-reentry-differential` runs 256 states per routine (26,112 cases) through the original Comanche055 rope in yaAGC (pinned yaAGC with hardware DV and without the IMU/optics device block (tools/prepare_flight_device_engine.py), its ScalerCounter held at 0 (TIME1-TIME6 and the timer interrupts do not advance during a case)) and the native routines; all are equal in Z, 2048 erasable words (A, L 16-bit) and the interrupt-enable state. The cases and the stops reached per routine are in `cm-reentry-proof.json` (`cm-reentry-differential.log` is the run). One case in 128 is frozen in `tests/cm_reentry_vectors.h`.
- **Host and sanitizer results.** `make test-cm-reentry sanitize-cm-reentry` runs the 204 frozen cases (`cm-reentry-host.log`).
- **C64 execution results.** `make vice-cm-reentry` runs 32 diagnostic programs with 172 cases (`cm-reentry-vice.log`, which starts with each PRG's SHA-256). The cases are chosen so the C64 runs take every control-flow edge (both outcomes of each branch) of every linked source that the 26,112 differential cases take: 1988 of 1988 edges and comparison outcomes (clang -O0 trace-pc-guard,trace-cmp) and 88 of 88 compiled labels. Checked again with gcov -b over gcc -O0 `--coverage` builds of the same sources: the C64 cases take 995 of the 995 branch outcomes all the cases take (`c64_gcov_branches` in the proof).
