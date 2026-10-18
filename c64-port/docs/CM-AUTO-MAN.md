# Translate Comanche AUTOMATIC_MANEUVERS

Lane G (CM flight). Milestone `cm-auto-man`. How the routines are made and checked: `docs/INTERP-COMPILE.md` (whole-file basic code: `tools/basic_compile.py`).

`Comanche055/AUTOMATIC_MANEUVERS.agc` (`src/cm_auto_man.c`) is native C in full: The RCS automatic maneuver logic (the J-series phase-plane tests, jet on-times and DOJET) and its returns from STICKCHK. Each routine runs the file from its entry, or from where it resumes, to the point where control leaves the file. Every original word is one call of the basic-instruction primitives (`src/agc_image_ops.c`) under a label named for its address, with TS overflow skips, CCS four-way skips and INDEX on the whole word kept; computed transfers (TC Q, indexed TCF) go through one dispatch on the target address.

This replaces the earlier hand translation, which the certificate reviewer rejected in #84: its comparison was a listing check rather than a yaAGC run, and none of its entries was a routine of the control-transfer ABI. Now every entry another file names is a registered routine, and the comparison runs the original rope in yaAGC.

## Scope

39 routines, keyed by physical address in `cm_auto_man_basic`:

| Entry | Kind | Why it is a routine |
|---|---|---|
| 21,3250 AHFNOROT | basic | entry AHFNOROT (RCS-CSM_DIGITAL_AUTOPILOT.agc BZMF AHFNOROT) |
| 21,3304  | basic | return from STICKCHK |
| 21,3322 ATTHOLD | basic | part key |
| 21,3363  | basic | part key |
| 21,3364  | basic | part key |
| 21,3372 HOLDFUNC | basic | part key |
| 21,3376 GRABANG | basic | part key |
| 21,3441 JETS | basic | entry JETS (RCS-CSM_DIGITAL_AUTOPILOT.agc TCF JETS) |
| 21,3445 JLOOP | basic | part key |
| 21,3463 INHOLD | basic | part key |
| 21,3472 SIGNCK1 | basic | part key |
| 21,3477 POSVEL | basic | part key |
| 21,3507 NEGVEL | basic | part key |
| 21,3571  | basic | part key |
| 21,3613  | basic | part key |
| 21,3614 SIGNCK3 | basic | part key |
| 21,3630 J18 | basic | part key |
| 21,3634 J20 | basic | part key |
| 21,3645 J21 | basic | part key |
| 21,3655 JN | basic | part key |
| 21,3664 JP | basic | part key |
| 21,3673 J22 | basic | part key |
| 21,3674  | basic | part key |
| 21,3675  | basic | part key |
| 21,3676  | basic | part key |
| 21,3677 SIGNCK5 | basic | part key |
| 21,3700  | basic | part key |
| 21,3701  | basic | part key |
| 21,3702  | basic | part key |
| 21,3703  | basic | part key |
| 21,3704 J23 | basic | part key |
| 21,3716 J24 | basic | part key |
| 21,3723  | basic | part key |
| 21,3727 JTIME | basic | part key |
| 21,3732  | basic | return from SMALLMP |
| 21,3734  | basic | return from SMALLMP |
| 21,3750  | basic | part key |
| 21,3757 T6PROG | basic | entry T6PROG (RCS-CSM_DIGITAL_AUTOPILOT.agc TCF T6PROG) |
| 21,3761  | basic | part key |

The step machine is split into part functions (spec `basic_parts`) so each routine's link closure fits its flight-store slot. A transfer across a part hands Z to the target word, whose routine is a part key, and the transfer runner runs it as the next step. Part keys are not entries: their runs come from the 6 entry routines, which draw all the cases, so the case set and the host vectors are the same as before the split.

Destinations outside the file: REINIT, RESUME, SMALLMP, STICKCHK, T6PROGM. Lane A has not yet registered the Comanche Executive and Waitlist routines (WAITLIST, NOVAC, FINDVAC, TASKOVER, ENDOFJOB, IBNKCALL with ISWRETRN, POSTJUMP, BANKCALL, RESUME) or the T5RUPT/T6RUPT vectors that dispatch T5LOC/T6LOC; until it does, a transfer to them stops with EXEC_UNMAPPED at the frontier.

## Domain

Canonical 2048-word image (every word random, the counters, `TIME1`-`TIME6`, the CDUs and the save registers at 010-061 included) under EB 6 (the file's `EBANK=`), FB of the routine's bank, any A, L and Q unless stated, and any values of the other erasables. SPNDX 0-2 and RMANNDX/PMANNDX/YMANNDX 0-3 as the RCS DAP leaves them; MINTAU (0-3), BIT13 (0-2) and OCT01400 (0-2) indexed by them.

## Evidence

Hashes: `evidence/cm-auto-man/SHA256.json`; all files are in the tree.

- **Source map.** `cm-auto-man-source-map.json` lists all 333 words of the file with the routine that compiles each (or why a constant is never run as code); 0 are untranslated.
- **Independent assembly comparison.** `make cm-auto-man-differential` runs 256 states per entry routine (1,536 cases) through the original Comanche055 rope in yaAGC (pinned yaAGC with hardware DV and without the IMU/optics device block (tools/prepare_flight_device_engine.py), its ScalerCounter held at 0 (TIME1-TIME6 and the timer interrupts do not advance during a case)) and the native routines; all are equal in Z, 2048 erasable words (A, L 16-bit) and the interrupt-enable state, the I/O channels 31, 32 and the 31A/31B/32 trap flags. The cases and the stops reached per routine are in `cm-auto-man-proof.json` (`cm-auto-man-differential.log` is the run). One case in 32 is frozen in `tests/cm_auto_man_vectors.h`.
- **Host and sanitizer results.** `make test-cm-auto-man sanitize-cm-auto-man` runs the 48 frozen cases (`cm-auto-man-host.log`).
- **C64 execution results.** `make vice-cm-auto-man` runs 1 diagnostic program with 24 cases (`cm-auto-man-vice.log`, which starts with each PRG's SHA-256). The cases are chosen so the C64 runs take every control-flow edge (both outcomes of each branch) of every linked source that the 1,536 differential cases take: 754 of 754 edges and comparison outcomes (clang -O0 trace-pc-guard,trace-cmp) and 301 of 301 compiled labels. Checked again with gcov -b over gcc -O0 `--coverage` builds of the same sources: the C64 cases take 489 of the 489 branch outcomes all the cases take (`c64_gcov_branches` in the proof).

## Flight build

The spec sets `trusted_image`. In flight builds (`RK_FIXED_IMAGE`), the entry checks skip their scan of the whole erasable image, which costs about 121,000 cycles a routine on the C64. Host tests and C64 diagnostics keep the scan.

## Hot table (docked RCS DAP)

The spec's `hot_points` name the 6 basic routines one docked RCS DAP pass (T5RUPT) reaches in the cm-p40 window after the burn (from 70660 cs, found with `FLIGHT_PATH`). `cm_auto_man_hot_routines` holds them, with the same wrappers as `cm_auto_man_basic`, for the flight store's `cm_man` (with `cm_mysubs_routines`) pack: a pass loads one small module instead of chunks of the whole table. `make test-cm-auto-man-hot` checks that each hot key calls the same function through either table.
