# Translate Comanche JET_SELECTION_LOGIC

Lane G (CM flight). Milestone `cm-jet-select`. How the routines are made and checked: `docs/INTERP-COMPILE.md` (whole-file basic code: `tools/basic_compile.py`).

`Comanche055/JET_SELECTION_LOGIC.agc` (`src/cm_jet_select.c`) is native C in full: JETSLECT (translation and rotation jet tables, jet on-times, the T6 sort) and the T6 program T6START with REPLACE/REPLACER. Each routine runs the file from its entry, or from where it resumes, to the point where control leaves the file. Every original word is one call of the basic-instruction primitives (`src/agc_image_ops.c`) under a label named for its address, with TS overflow skips, CCS four-way skips and INDEX on the whole word kept; computed transfers (TC Q, indexed TCF) go through one dispatch on the target address.

This replaces the earlier hand translation, which the certificate reviewer rejected in #84: its comparison was a listing check rather than a yaAGC run, and none of its entries was a routine of the control-transfer ABI. Now every entry another file names is a registered routine, and the comparison runs the original rope in yaAGC.

## Scope

2 routines, keyed by physical address in `cm_jet_select_basic`:

| Entry | Kind | Why it is a routine |
|---|---|---|
| 17,2552 JETSLECT | basic | entry JETSLECT (AUTOMATIC_MANEUVERS.agc 2CADR JETSLECT) |
| 17,3617 T6START | basic | entry T6START (RCS-CSM_DIGITAL_AUTOPILOT.agc 2CADR T6START) |

Destinations outside the file: RESUME. Lane A has not yet registered the Comanche Executive and Waitlist routines (WAITLIST, NOVAC, FINDVAC, TASKOVER, ENDOFJOB, IBNKCALL with ISWRETRN, POSTJUMP, BANKCALL, RESUME) or the T5RUPT/T6RUPT vectors that dispatch T5LOC/T6LOC; until it does, a transfer to them stops with EXEC_UNMAPPED at the frontier.

## Domain

Canonical 2048-word image (every word random, the counters, `TIME1`-`TIME6`, the CDUs and the save registers at 010-061 included) under EB 6 (the file's `EBANK=`), FB of the routine's bank, any A, L and Q unless stated, and any values of the other erasables. XTRANS +0, +1 or -1 as P40 leaves it; BLAST, BLAST1, BLAST2 +1 holding T6SETUP's branch indices 0, 4, 11 (REPLACER, REPLACEP, REPLACEY), compiled as the three TC targets. Tables XLNNDX, XLN1NDX (0-3), PYTABLE, RTABLE (0-14), YZTABLE (0-8), NJET and DFTMAX (-4 to +4, negative counts after PTAUNEG/YTAUNEG).

## Evidence

Hashes: `evidence/cm-jet-select/SHA256.json`; all files are in the tree.

- **Source map.** `cm-jet-select-source-map.json` lists all 629 words of the file with the routine that compiles each (or why a constant is never run as code); 0 are untranslated.
- **Independent assembly comparison.** `make cm-jet-select-differential` runs 256 states per routine (512 cases) through the original Comanche055 rope in yaAGC (pinned yaAGC with hardware DV and without the IMU/optics device block (tools/prepare_flight_device_engine.py), its ScalerCounter held at 0 (TIME1-TIME6 and the timer interrupts do not advance during a case)) and the native routines; all are equal in Z, 2048 erasable words (A, L 16-bit) and the interrupt-enable state, the I/O channels 05, 06, 13, 31 and the 31A/31B/32 trap flags. The cases and the stops reached per routine are in `cm-jet-select-proof.json` (`cm-jet-select-differential.log` is the run). One case in 32 is frozen in `tests/cm_jet_select_vectors.h`.
- **Host and sanitizer results.** `make test-cm-jet-select sanitize-cm-jet-select` runs the 16 frozen cases (`cm-jet-select-host.log`).
- **C64 execution results.** `make vice-cm-jet-select` runs 1 diagnostic program with 28 cases (`cm-jet-select-vice.log`, which starts with each PRG's SHA-256). The cases are chosen so the C64 runs take every control-flow edge (both outcomes of each branch) of every linked source that the 512 differential cases take: 795 of 795 edges and comparison outcomes (clang -O0 trace-pc-guard,trace-cmp) and 532 of 532 compiled labels. Checked again with gcov -b over gcc -O0 `--coverage` builds of the same sources: the C64 cases take 531 of the 531 branch outcomes all the cases take (`c64_gcov_branches` in the proof).
