# Translate Comanche TVCDAPS

Lane G (CM flight). Milestone `cm-tvc-daps`. How the routines are made and checked: `docs/INTERP-COMPILE.md` (whole-file basic code: `tools/basic_compile.py`).

`Comanche055/TVCDAPS.agc` (`src/cm_tvc_daps.c`) is native C in full: The pitch and yaw TVC autopilots (PITCHDAP, YAWDAP with their HACK returns, the copycycles PCOPY +1 and YCOPY +1) and DAPINIT. Each routine runs the file from its entry, or from where it resumes, to the point where control leaves the file. Every original word is one call of the basic-instruction primitives (`src/agc_image_ops.c`) under a label named for its address, with TS overflow skips, CCS four-way skips and INDEX on the whole word kept; computed transfers (TC Q, indexed TCF) go through one dispatch on the target address.

This replaces the earlier hand translation, which the certificate reviewer rejected in #77: its comparison was a listing check rather than a yaAGC run, and none of its entries was a routine of the control-transfer ABI. Now every entry another file names is a registered routine, and the comparison runs the original rope in yaAGC.

## Scope

9 routines, keyed by physical address in `cm_tvc_daps_basic`:

| Entry | Kind | Why it is a routine |
|---|---|---|
| 20,2530 PITCHDAP | basic | GENADR PITCHDAP |
| 20,2541  | basic | return from HACK |
| 20,2543 PCDUDOTS | basic | return from HACK |
| 20,2675  | basic | entry 20,2675 (TVCRESTARTS.agc CADR PCOPY      +1) |
| 20,2732 YAWDAP | basic | GENADR YAWDAP |
| 20,2743  | basic | return from HACK |
| 20,2745 YINTEGRL | basic | return from HACK |
| 20,3051  | basic | entry 20,3051 (TVCRESTARTS.agc CADR YCOPY      +1) |
| 20,3107 DAPINIT | basic | entry DAPINIT (TVCINITIALIZE.agc 2CADR DAPINIT) |

Destinations outside the file: HACK, NOQRSM, RESUME. Lane A has not yet registered the Comanche Executive and Waitlist routines (WAITLIST, NOVAC, FINDVAC, TASKOVER, ENDOFJOB, IBNKCALL with ISWRETRN, POSTJUMP, BANKCALL, RESUME) or the T5RUPT/T6RUPT vectors that dispatch T5LOC/T6LOC; until it does, a transfer to them stops with EXEC_UNMAPPED at the frontier.

## Domain

Canonical 2048-word image (every word random, the counters, `TIME1`-`TIME6`, the CDUs and the save registers at 010-061 included) under EB 6 (the file's `EBANK=`), FB of the routine's bank, any A, L and Q unless stated, and any values of the other erasables. Any values of the DAP erasables; PITCHDAP and YAWDAP entered as the T5RUPT task their GENADR/BBCON pairs name.

## Evidence

Hashes: `evidence/cm-tvc-daps/SHA256.json`; all files are in the tree.

- **Source map.** `cm-tvc-daps-source-map.json` lists all 489 words of the file with the routine that compiles each (or why a constant is never run as code); 0 are untranslated.
- **Independent assembly comparison.** `make cm-tvc-daps-differential` runs 256 states per routine (2,304 cases) through the original Comanche055 rope in yaAGC (pinned yaAGC with hardware DV and without the IMU/optics device block (tools/prepare_flight_device_engine.py), its ScalerCounter held at 0 (TIME1-TIME6 and the timer interrupts do not advance during a case)) and the native routines; all are equal in Z, 2048 erasable words (A, L 16-bit) and the interrupt-enable state, the I/O channels 14 and the 31A/31B/32 trap flags. The cases and the stops reached per routine are in `cm-tvc-daps-proof.json` (`cm-tvc-daps-differential.log` is the run). One case in 32 is frozen in `tests/cm_tvc_daps_vectors.h`.
- **Host and sanitizer results.** `make test-cm-tvc-daps sanitize-cm-tvc-daps` runs the 72 frozen cases (`cm-tvc-daps-host.log`).
- **C64 execution results.** `make vice-cm-tvc-daps` runs 1 diagnostic program with 18 cases (`cm-tvc-daps-vice.log`, which starts with each PRG's SHA-256). The cases are chosen so the C64 runs take every control-flow edge (both outcomes of each branch) of every linked source that the 2,304 differential cases take: 504 of 504 edges and comparison outcomes (clang -O0 trace-pc-guard,trace-cmp) and 487 of 487 compiled labels. Checked again with gcov -b over gcc -O0 `--coverage` builds of the same sources: the C64 cases take 296 of the 296 branch outcomes all the cases take (`c64_gcov_branches` in the proof).
