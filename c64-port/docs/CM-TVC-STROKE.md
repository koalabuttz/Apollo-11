# Translate Comanche TVCSTROKETEST

Lane G (CM flight). Milestone `cm-tvc-stroke`. How the routines are made and checked: `docs/INTERP-COMPILE.md` (whole-file basic code: `tools/basic_compile.py`).

`Comanche055/TVCSTROKETEST.agc` (`src/cm_tvc_stroke.c`) is native C in full: The V68 stroke test: STRKTSTI, its waveform task and HACK, which the pitch and yaw DAPs call by TC HACK and which returns to them through BUNKER. Each routine runs the file from its entry, or from where it resumes, to the point where control leaves the file. Every original word is one call of the basic-instruction primitives (`src/agc_image_ops.c`) under a label named for its address, with TS overflow skips, CCS four-way skips and INDEX on the whole word kept; computed transfers (TC Q, indexed TCF) go through one dispatch on the target address.

This replaces the earlier hand translation, which the certificate reviewer rejected in #77: its comparison was a listing check rather than a yaAGC run, and none of its entries was a routine of the control-transfer ABI. Now every entry another file names is a registered routine, and the comparison runs the original rope in yaAGC.

## Scope

5 routines, keyed by physical address in `cm_tvc_stroke_basic`:

| Entry | Kind | Why it is a routine |
|---|---|---|
| 20,3501 STRKTSTI | basic | entry STRKTSTI (EXTENDED_VERBS.agc 2CADR STRKTSTI) |
| 20,3533 HACK | basic | entry HACK (TVCDAPS.agc TC HACK) |
| 20,3541  | basic | return from WAITLIST |
| 20,3542 HACKWLST | basic | 2CADR HACKWLST |
| E6,1657 (erasable) | continuation | the TC words the file stores there: 02541, 02543, 02743, 02745, 05324 |

TC BUNKER is the erasable continuation E6,1657 (key AGC_TRANSFER_ERASABLE_BASE + agc_erasable_point(01657, E6)), owned by this file: it accepts exactly the TC words 2541, 2543, 2743, 2745 and TCTSKOVR (5324) and refuses any other with EXEC_OUT_OF_SCOPE.

Destinations outside the file: ENDOFJOB, WAITLIST. Lane A has not yet registered the Comanche Executive and Waitlist routines (WAITLIST, NOVAC, FINDVAC, TASKOVER, ENDOFJOB, IBNKCALL with ISWRETRN, POSTJUMP, BANKCALL, RESUME) or the T5RUPT/T6RUPT vectors that dispatch T5LOC/T6LOC; until it does, a transfer to them stops with EXEC_UNMAPPED at the frontier.

## Domain

Canonical 2048-word image (every word random, the counters, `TIME1`-`TIME6`, the CDUs and the save registers at 010-061 included) under EB 6 (the file's `EBANK=`), FB of the routine's bank, any A, L and Q unless stated, and any values of the other erasables. HACK entered with Q one of the DAP returns 2541, 2543, 2743, 2745 (bank 20). BUNKER (E6,1657) holds what the file stores there: one of those returns or TCTSKOVR. CARD counts -4 to -1 between waveform sets (INDEX CARD reads FCARD +1 to +6).

## Evidence

Hashes: `evidence/cm-tvc-stroke/SHA256.json`; all files are in the tree.

- **Source map.** `cm-tvc-stroke-source-map.json` lists all 73 words of the file with the routine that compiles each (or why a constant is never run as code); 0 are untranslated.
- **Independent assembly comparison.** `make cm-tvc-stroke-differential` runs 256 states per routine (1,024 cases) through the original Comanche055 rope in yaAGC (pinned yaAGC with hardware DV and without the IMU/optics device block (tools/prepare_flight_device_engine.py), its ScalerCounter held at 0 (TIME1-TIME6 and the timer interrupts do not advance during a case)) and the native routines; all are equal in Z, 2048 erasable words (A, L 16-bit) and the interrupt-enable state, the I/O channels 14 and the 31A/31B/32 trap flags. The cases and the stops reached per routine are in `cm-tvc-stroke-proof.json` (`cm-tvc-stroke-differential.log` is the run). One case in 32 is frozen in `tests/cm_tvc_stroke_vectors.h`.
- **Host and sanitizer results.** `make test-cm-tvc-stroke sanitize-cm-tvc-stroke` runs the 32 frozen cases (`cm-tvc-stroke-host.log`).
- **C64 execution results.** `make vice-cm-tvc-stroke` runs 1 diagnostic program with 12 cases (`cm-tvc-stroke-vice.log`, which starts with each PRG's SHA-256). The cases are chosen so the C64 runs take every control-flow edge (both outcomes of each branch) of every linked source that the 1,024 differential cases take: 340 of 340 edges and comparison outcomes (clang -O0 trace-pc-guard,trace-cmp) and 62 of 62 compiled labels. Checked again with gcov -b over gcc -O0 `--coverage` builds of the same sources: the C64 cases take 190 of the 190 branch outcomes all the cases take (`c64_gcov_branches` in the proof).
