# Translate Comanche JET_SELECTION_LOGIC

Lane G (CM flight). Milestone `cm-jet-select`. How the routines are made and checked: `docs/INTERP-COMPILE.md` (whole-file basic code: `tools/basic_compile.py`).

`Comanche055/JET_SELECTION_LOGIC.agc` (`src/cm_jet_select.c`) is native C in full: JETSLECT (translation and rotation jet tables, jet on-times, the T6 sort) and the T6 program T6START with REPLACE/REPLACER. Each routine runs the file from its entry, or from where it resumes, to the point where control leaves the file. Every original word is one call of the basic-instruction primitives (`src/agc_image_ops.c`) under a label named for its address, with TS overflow skips, CCS four-way skips and INDEX on the whole word kept; computed transfers (TC Q, indexed TCF) go through one dispatch on the target address.

This replaces the earlier hand translation, which the certificate reviewer rejected in #84: its comparison was a listing check rather than a yaAGC run, and none of its entries was a routine of the control-transfer ABI. Now every entry another file names is a registered routine, and the comparison runs the original rope in yaAGC.

## Scope

76 routines, keyed by physical address in `cm_jet_select_basic`:

| Entry | Kind | Why it is a routine |
|---|---|---|
| 17,2552 JETSLECT | basic | entry JETSLECT (AUTOMATIC_MANEUVERS.agc 2CADR JETSLECT) |
| 17,2671  | basic | part key |
| 17,3000  | basic | part key |
| 17,3013  | basic | part key |
| 17,3015  | basic | part key |
| 17,3040 NOBDZ | basic | part key |
| 17,3046 BDROLL | basic | part key |
| 17,3102  | basic | part key |
| 17,3106  | basic | part key |
| 17,3112 BDRACZ | basic | part key |
| 17,3114  | basic | part key |
| 17,3115 TAUCHCK | basic | part key |
| 17,3122 NOACY | basic | part key |
| 17,3164 PITCHTIM | basic | part key |
| 17,3230 YAWTIME | basic | part key |
| 17,3231  | basic | part key |
| 17,3232  | basic | part key |
| 17,3233  | basic | part key |
| 17,3234  | basic | part key |
| 17,3240 YTAUPOS | basic | part key |
| 17,3250  | basic | part key |
| 17,3260 AD14MSY | basic | part key |
| 17,3274 ROLLTIME | basic | part key |
| 17,3331  | basic | part key |
| 17,3334 AD14MSR | basic | part key |
| 17,3342 RBLASTOK | basic | part key |
| 17,3357 ASMBLWP | basic | part key |
| 17,3366 FPX2 | basic | part key |
| 17,3367  | basic | part key |
| 17,3372 PBYPASS | basic | part key |
| 17,3377 ASMBLWR | basic | part key |
| 17,3402  | basic | part key |
| 17,3403  | basic | part key |
| 17,3405 NO2Z | basic | part key |
| 17,3406  | basic | part key |
| 17,3422  | basic | part key |
| 17,3424  | basic | part key |
| 17,3426 AC2Y | basic | part key |
| 17,3432  | basic | part key |
| 17,3436  | basic | part key |
| 17,3453 ACBD2Z | basic | part key |
| 17,3463  | basic | part key |
| 17,3501  | basic | part key |
| 17,3504 BDF2Z | basic | part key |
| 17,3510  | basic | part key |
| 17,3516 RBYPASS | basic | part key |
| 17,3520  | basic | part key |
| 17,3523 ASMBLWY | basic | part key |
| 17,3527  | basic | part key |
| 17,3532 FYX2 | basic | part key |
| 17,3536 YBYPASS | basic | part key |
| 17,3554 CHECKT23 | basic | part key |
| 17,3560 CALCDT6 | basic | part key |
| 17,3561  | basic | part key |
| 17,3563  | basic | part key |
| 17,3577 DXCHT12 | basic | part key |
| 17,3600  | basic | part key |
| 17,3603 DXCHT23 | basic | part key |
| 17,3617 T6START | basic | entry T6START (RCS-CSM_DIGITAL_AUTOPILOT.agc 2CADR T6START) |
| 17,3620  | basic | part key |
| 17,3637  | basic | part key |
| 17,3642 T6RUPTOR | basic | part key |
| 17,3654 T6L2 | basic | part key |
| 17,3663  | basic | part key |
| 17,3666 REPLACE1 | basic | part key |
| 17,3670  | basic | part key |
| 17,3673 REPLACE2 | basic | part key |
| 17,3675  | basic | part key |
| 17,3700 REPLACER | basic | part key |
| 17,3704 REPLACEP | basic | part key |
| 17,3712  | basic | part key |
| 17,3713 REPLACEY | basic | part key |
| 17,3722 ZBLAST | basic | part key |
| 17,3723  | basic | part key |
| 17,3725 ZBLAST1 | basic | part key |
| 17,3730 ZBLAST2 | basic | part key |

The step machine is split into part functions (spec `basic_parts`) so each routine's link closure fits its flight-store slot. A transfer across a part hands Z to the target word, whose routine is a part key, and the transfer runner runs it as the next step. Part keys are not entries: their runs come from the 2 entry routines, which draw all the cases, so the case set and the host vectors are the same as before the split.

Destinations outside the file: RESUME. Lane A has not yet registered the Comanche Executive and Waitlist routines (WAITLIST, NOVAC, FINDVAC, TASKOVER, ENDOFJOB, IBNKCALL with ISWRETRN, POSTJUMP, BANKCALL, RESUME) or the T5RUPT/T6RUPT vectors that dispatch T5LOC/T6LOC; until it does, a transfer to them stops with EXEC_UNMAPPED at the frontier.

## Domain

Canonical 2048-word image (every word random, the counters, `TIME1`-`TIME6`, the CDUs and the save registers at 010-061 included) under EB 6 (the file's `EBANK=`), FB of the routine's bank, any A, L and Q unless stated, and any values of the other erasables. XTRANS +0, +1 or -1 as P40 leaves it; BLAST, BLAST1, BLAST2 +1 holding T6SETUP's branch indices 0, 4, 11 (REPLACER, REPLACEP, REPLACEY), compiled as the three TC targets. Tables XLNNDX, XLN1NDX (0-3), PYTABLE, RTABLE (0-14), YZTABLE (0-8), NJET and DFTMAX (-4 to +4, negative counts after PTAUNEG/YTAUNEG).

## Evidence

Hashes: `evidence/cm-jet-select/SHA256.json`; all files are in the tree.

- **Source map.** `cm-jet-select-source-map.json` lists all 629 words of the file with the routine that compiles each (or why a constant is never run as code); 0 are untranslated.
- **Independent assembly comparison.** `make cm-jet-select-differential` runs 256 states per entry routine (512 cases) through the original Comanche055 rope in yaAGC (pinned yaAGC with hardware DV and without the IMU/optics device block (tools/prepare_flight_device_engine.py), its ScalerCounter held at 0 (TIME1-TIME6 and the timer interrupts do not advance during a case)) and the native routines; all are equal in Z, 2048 erasable words (A, L 16-bit) and the interrupt-enable state, the I/O channels 05, 06, 13, 31 and the 31A/31B/32 trap flags. The cases and the stops reached per routine are in `cm-jet-select-proof.json` (`cm-jet-select-differential.log` is the run). One case in 32 is frozen in `tests/cm_jet_select_vectors.h`.
- **Host and sanitizer results.** `make test-cm-jet-select sanitize-cm-jet-select` runs the 16 frozen cases (`cm-jet-select-host.log`).
- **C64 execution results.** `make vice-cm-jet-select` runs 1 diagnostic program with 27 cases (`cm-jet-select-vice.log`, which starts with each PRG's SHA-256). The cases are chosen so the C64 runs take every control-flow edge (both outcomes of each branch) of every linked source that the 512 differential cases take: 986 of 986 edges and comparison outcomes (clang -O0 trace-pc-guard,trace-cmp) and 555 of 555 compiled labels. Checked again with gcov -b over gcc -O0 `--coverage` builds of the same sources: the C64 cases take 703 of the 703 branch outcomes all the cases take (`c64_gcov_branches` in the proof).
