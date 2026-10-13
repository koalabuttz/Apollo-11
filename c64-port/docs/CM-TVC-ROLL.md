# Translate Comanche TVCROLLDAP

Lane G (CM flight). Milestone `cm-tvc-roll`. How the routines are made and checked: `docs/INTERP-COMPILE.md` (whole-file basic code: `tools/basic_compile.py`).

`Comanche055/TVCROLLDAP.agc` (`src/cm_tvc_roll.c`) is native C in full: The TVC roll autopilot ROLLDAP (switching logic, jet timing and the OGA rate filter) and NOROLL1. Each routine runs the file from its entry, or from where it resumes, to the point where control leaves the file. Every original word is one call of the basic-instruction primitives (`src/agc_image_ops.c`) under a label named for its address, with TS overflow skips, CCS four-way skips and INDEX on the whole word kept; computed transfers (TC Q, indexed TCF) go through one dispatch on the target address.

This replaces the earlier hand translation, which the certificate reviewer rejected in #77: its comparison was a listing check rather than a yaAGC run, and none of its entries was a routine of the control-transfer ABI. Now every entry another file names is a registered routine, and the comparison runs the original rope in yaAGC.

## Scope

2 routines, keyed by physical address in `cm_tvc_roll_basic`:

| Entry | Kind | Why it is a routine |
|---|---|---|
| 16,3272 ROLLDAP | basic | entry ROLLDAP (TVCEXECUTIVE.agc 2CADR ROLLDAP) |
| 16,3676 NOROLL1 | basic | 2CADR NOROLL1 |

Destinations outside the file: NOQRSM, TASKOVER. Lane A has not yet registered the Comanche Executive and Waitlist routines (WAITLIST, NOVAC, FINDVAC, TASKOVER, ENDOFJOB, IBNKCALL with ISWRETRN, POSTJUMP, BANKCALL, RESUME) or the T5RUPT/T6RUPT vectors that dispatch T5LOC/T6LOC; until it does, a transfer to them stops with EXEC_UNMAPPED at the frontier.

## Domain

Canonical 2048-word image (every word random, the counters, `TIME1`-`TIME6`, the CDUs and the save registers at 010-061 included) under EB 6 (the file's `EBANK=`), FB of the routine's bank, any A, L and Q unless stated, and any values of the other erasables. I (E6,1670) holds PRIO30 or BIT15, the CA and CS order codes the file stores there: each INDEX I, 0 X word is compiled as CA X and CS X. TS TEMREG overflow protection reads LIMITS -1 to +1.

## Evidence

Hashes: `evidence/cm-tvc-roll/SHA256.json`; all files are in the tree.

- **Source map.** `cm-tvc-roll-source-map.json` lists all 278 words of the file with the routine that compiles each (or why a constant is never run as code); 0 are untranslated.
- **Independent assembly comparison.** `make cm-tvc-roll-differential` runs 256 states per routine (512 cases) through the original Comanche055 rope in yaAGC (pinned yaAGC with hardware DV and without the IMU/optics device block (tools/prepare_flight_device_engine.py), its ScalerCounter held at 0 (TIME1-TIME6 and the timer interrupts do not advance during a case)) and the native routines; all are equal in Z, 2048 erasable words (A, L 16-bit) and the interrupt-enable state, the I/O channels 06, 13 and the 31A/31B/32 trap flags. The cases and the stops reached per routine are in `cm-tvc-roll-proof.json` (`cm-tvc-roll-differential.log` is the run). One case in 32 is frozen in `tests/cm_tvc_roll_vectors.h`.
- **Host and sanitizer results.** `make test-cm-tvc-roll sanitize-cm-tvc-roll` runs the 16 frozen cases (`cm-tvc-roll-host.log`).
- **C64 execution results.** `make vice-cm-tvc-roll` runs 1 diagnostic program with 16 cases (`cm-tvc-roll-vice.log`, which starts with each PRG's SHA-256). The cases are chosen so the C64 runs take every control-flow edge (both outcomes of each branch) of every linked source that the 512 differential cases take: 578 of 578 edges and comparison outcomes (clang -O0 trace-pc-guard,trace-cmp) and 262 of 262 compiled labels. Checked again with gcov -b over gcc -O0 `--coverage` builds of the same sources: the C64 cases take 316 of the 316 branch outcomes all the cases take (`c64_gcov_branches` in the proof).
