# Translate Comanche AUTOMATIC_MANEUVERS

Lane G (CM flight). Milestone `cm-auto-man`. How the routines are made and checked: `docs/INTERP-COMPILE.md` (whole-file basic code: `tools/basic_compile.py`).

`Comanche055/AUTOMATIC_MANEUVERS.agc` (`src/cm_auto_man.c`) is native C in full: The RCS automatic maneuver logic (the J-series phase-plane tests, jet on-times and DOJET) and its returns from STICKCHK. Each routine runs the file from its entry, or from where it resumes, to the point where control leaves the file. Every original word is one call of the basic-instruction primitives (`src/agc_image_ops.c`) under a label named for its address, with TS overflow skips, CCS four-way skips and INDEX on the whole word kept; computed transfers (TC Q, indexed TCF) go through one dispatch on the target address.

This replaces the earlier hand translation, which the certificate reviewer rejected in #84: its comparison was a listing check rather than a yaAGC run, and none of its entries was a routine of the control-transfer ABI. Now every entry another file names is a registered routine, and the comparison runs the original rope in yaAGC.

## Scope

6 routines, keyed by physical address in `cm_auto_man_basic`:

| Entry | Kind | Why it is a routine |
|---|---|---|
| 21,3250 AHFNOROT | basic | entry AHFNOROT (RCS-CSM_DIGITAL_AUTOPILOT.agc BZMF AHFNOROT) |
| 21,3304  | basic | return from STICKCHK |
| 21,3441 JETS | basic | entry JETS (RCS-CSM_DIGITAL_AUTOPILOT.agc TCF JETS) |
| 21,3732  | basic | return from SMALLMP |
| 21,3734  | basic | return from SMALLMP |
| 21,3757 T6PROG | basic | entry T6PROG (RCS-CSM_DIGITAL_AUTOPILOT.agc TCF T6PROG) |

Destinations outside the file: REINIT, RESUME, SMALLMP, STICKCHK, T6PROGM. Lane A has not yet registered the Comanche Executive and Waitlist routines (WAITLIST, NOVAC, FINDVAC, TASKOVER, ENDOFJOB, IBNKCALL with ISWRETRN, POSTJUMP, BANKCALL, RESUME) or the T5RUPT/T6RUPT vectors that dispatch T5LOC/T6LOC; until it does, a transfer to them stops with EXEC_UNMAPPED at the frontier.

## Domain

Canonical 2048-word image (every word random, the counters, `TIME1`-`TIME6`, the CDUs and the save registers at 010-061 included) under EB 6 (the file's `EBANK=`), FB of the routine's bank, any A, L and Q unless stated, and any values of the other erasables. SPNDX 0-2 and RMANNDX/PMANNDX/YMANNDX 0-3 as the RCS DAP leaves them; MINTAU (0-3), BIT13 (0-2) and OCT01400 (0-2) indexed by them.

## Evidence

Hashes: `evidence/cm-auto-man/SHA256.json`; all files are in the tree.

- **Source map.** `cm-auto-man-source-map.json` lists all 333 words of the file with the routine that compiles each (or why a constant is never run as code); 0 are untranslated.
- **Independent assembly comparison.** `make cm-auto-man-differential` runs 256 states per routine (1,536 cases) through the original Comanche055 rope in yaAGC (pinned yaAGC with hardware DV and without the IMU/optics device block (tools/prepare_flight_device_engine.py), its ScalerCounter held at 0 (TIME1-TIME6 and the timer interrupts do not advance during a case)) and the native routines; all are equal in Z, 2048 erasable words (A, L 16-bit) and the interrupt-enable state, the I/O channels 31, 32 and the 31A/31B/32 trap flags. The cases and the stops reached per routine are in `cm-auto-man-proof.json` (`cm-auto-man-differential.log` is the run). One case in 32 is frozen in `tests/cm_auto_man_vectors.h`.
- **Host and sanitizer results.** `make test-cm-auto-man sanitize-cm-auto-man` runs the 48 frozen cases (`cm-auto-man-host.log`).
- **C64 execution results.** `make vice-cm-auto-man` runs 1 diagnostic program with 24 cases (`cm-auto-man-vice.log`, which starts with each PRG's SHA-256). The cases are chosen so the C64 runs take every control-flow edge (both outcomes of each branch) of every linked source that the 1,536 differential cases take: 638 of 638 edges and comparison outcomes (clang -O0 trace-pc-guard,trace-cmp) and 293 of 293 compiled labels. Checked again with gcov -b over gcc -O0 `--coverage` builds of the same sources: the C64 cases take 364 of the 364 branch outcomes all the cases take (`c64_gcov_branches` in the proof).
