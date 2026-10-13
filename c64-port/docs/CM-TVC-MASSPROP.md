# Translate Comanche TVCMASSPROP

Lane G (CM flight). Milestone `cm-tvc-massprop`. How the routines are made and checked: `docs/INTERP-COMPILE.md` (whole-file basic code: `tools/basic_compile.py`).

`Comanche055/TVCMASSPROP.agc` (`src/cm_tvc_massprop.c`) is native C in full: MASSPROP (the LEM-off and LEM-on breakpoints and slopes of the TVC gains) and FIXCW (the CSM-weight gains IXX, IAVG, IAVG/TLX and WEIGHT/G), ending at ENDMASSP TC Q. Each routine runs the file from its entry, or from where it resumes, to the point where control leaves the file. Every original word is one call of the basic-instruction primitives (`src/agc_image_ops.c`) under a label named for its address, with TS overflow skips, CCS four-way skips and INDEX on the whole word kept; computed transfers (TC Q, indexed TCF) go through one dispatch on the target address.

This replaces the earlier hand translation, which the certificate reviewer rejected in #77: its comparison was a listing check rather than a yaAGC run, and none of its entries was a routine of the control-transfer ABI. Now every entry another file names is a registered routine, and the comparison runs the original rope in yaAGC.

## Scope

2 routines, keyed by physical address in `cm_tvc_massprop_basic`:

| Entry | Kind | Why it is a routine |
|---|---|---|
| 05,3366 MASSPROP | basic | entry MASSPROP (FRESH_START_AND_RESTART.agc CADR MASSPROP) |
| 05,3422 FIXCW | basic | entry FIXCW (TVCEXECUTIVE.agc CADR FIXCW) |

Destinations outside the file: only the caller's return through Q. Lane A has not yet registered the Comanche Executive and Waitlist routines (WAITLIST, NOVAC, FINDVAC, TASKOVER, ENDOFJOB, IBNKCALL with ISWRETRN, POSTJUMP, BANKCALL, RESUME) or the T5RUPT/T6RUPT vectors that dispatch T5LOC/T6LOC; until it does, a transfer to them stops with EXEC_UNMAPPED at the frontier.

## Domain

Canonical 2048-word image (every word random, the counters, `TIME1`-`TIME6`, the CDUs and the save registers at 010-061 included) under EB 6 (the file's `EBANK=`), FB of the routine's bank, any A, L and Q unless stated, and any values of the other erasables. LEMMASS within the LM mass (0 to 15100 kg at B+16), CSMMASS within the CSM mass (0 to 30000 kg) either side of the NEGBPW breakpoint, and VARST0-VARST8 within +-07777 as MASSPROP leaves them. Outside that range the original overflows TS VARST0 or TS IXX and loops for ever (TCF MASSPROP +1, TCF BOKKEP2 with A = 1); those states are not admitted. MASSPROP indexes the constant tables NOLEMVAL, SLOPEVAL and INTVALUE by PHI333 (9 down to 0).

## Evidence

Hashes: `evidence/cm-tvc-massprop/SHA256.json`; all files are in the tree.

- **Source map.** `cm-tvc-massprop-source-map.json` lists all 91 words of the file with the routine that compiles each (or why a constant is never run as code); 0 are untranslated.
- **Independent assembly comparison.** `make cm-tvc-massprop-differential` runs 256 states per routine (512 cases) through the original Comanche055 rope in yaAGC (pinned yaAGC with hardware DV and without the IMU/optics device block (tools/prepare_flight_device_engine.py), its ScalerCounter held at 0 (TIME1-TIME6 and the timer interrupts do not advance during a case)) and the native routines; all are equal in Z, 2048 erasable words (A, L 16-bit) and the interrupt-enable state. The cases and the stops reached per routine are in `cm-tvc-massprop-proof.json` (`cm-tvc-massprop-differential.log` is the run). One case in 32 is frozen in `tests/cm_tvc_massprop_vectors.h`.
- **Host and sanitizer results.** `make test-cm-tvc-massprop sanitize-cm-tvc-massprop` runs the 16 frozen cases (`cm-tvc-massprop-host.log`).
- **C64 execution results.** `make vice-cm-tvc-massprop` runs 1 diagnostic program with 7 cases (`cm-tvc-massprop-vice.log`, which starts with each PRG's SHA-256). The cases are chosen so the C64 runs take every control-flow edge (both outcomes of each branch) of every linked source that the 512 differential cases take: 399 of 399 edges and comparison outcomes (clang -O0 trace-pc-guard,trace-cmp) and 57 of 57 compiled labels. Checked again with gcov -b over gcc -O0 `--coverage` builds of the same sources: the C64 cases take 234 of the 234 branch outcomes all the cases take (`c64_gcov_branches` in the proof).
