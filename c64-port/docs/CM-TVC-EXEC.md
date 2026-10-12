# Translate Comanche TVCEXECUTIVE

Lane G (CM flight). Milestone `cm-tvc-exec`. How the routines are made and checked: `docs/INTERP-COMPILE.md` (whole-file basic code: `tools/basic_compile.py`).

`Comanche055/TVCEXECUTIVE.agc` (`src/cm_tvc_exec.c`) is native C in full: The half-second TVC executive task (TVCEXEC, ROLLPREP, NEEDLEUP, VARGAINS and the gain update), and the restart points EXECCOPY +1, 1SHOTCHK, TEMPSET, CORSETUP, CORCOPY +1 and CNTRCOPY that TVCRESTARTS reaches through its GENADR table. Each routine runs the file from its entry, or from where it resumes, to the point where control leaves the file. Every original word is one call of the basic-instruction primitives (`src/agc_image_ops.c`) under a label named for its address, with TS overflow skips, CCS four-way skips and INDEX on the whole word kept; computed transfers (TC Q, indexed TCF) go through one dispatch on the target address.

This replaces the earlier hand translation, which the certificate reviewer rejected in #77: its comparison was a listing check rather than a yaAGC run, and none of its entries was a routine of the control-transfer ABI. Now every entry another file names is a registered routine, and the comparison runs the original rope in yaAGC.

## Scope

12 routines, keyed by physical address in `cm_tvc_exec_basic`:

| Entry | Kind | Why it is a routine |
|---|---|---|
| 16,2720 TVCEXEC | basic | entry TVCEXEC (FRESH_START_AND_RESTART.agc 2CADR TVCEXEC) |
| 16,2730 ROLLPREP | basic | return from WAITLIST |
| 16,2746 NEEDLEUP | basic | return from WAITLIST |
| 16,2750 VARGAINS | basic | return from IBNKCALL |
| 16,2773  | basic | return from IBNKCALL |
| 16,2775  | basic | return from IBNKCALL |
| 16,3003  | basic | entry 16,3003 (TVCRESTARTS.agc GENADR EXECCOPY   +1) |
| 16,3011 1SHOTCHK | basic | entry 1SHOTCHK (TVCRESTARTS.agc GENADR 1SHOTCHK) |
| 16,3040 TEMPSET | basic | entry TEMPSET (TVCRESTARTS.agc GENADR TEMPSET) |
| 16,3043 CORSETUP | basic | entry CORSETUP (TVCRESTARTS.agc GENADR CORSETUP) |
| 16,3121  | basic | entry 16,3121 (TVCRESTARTS.agc GENADR CORCOPY    +1) |
| 16,3132 CNTRCOPY | basic | entry CNTRCOPY (TVCRESTARTS.agc GENADR CNTRCOPY) |

Destinations outside the file: IBNKCALL, TASKOVER, WAITLIST. Lane A has not yet registered the Comanche Executive and Waitlist routines (WAITLIST, NOVAC, FINDVAC, TASKOVER, ENDOFJOB, IBNKCALL with ISWRETRN, POSTJUMP, BANKCALL, RESUME) or the T5RUPT/T6RUPT vectors that dispatch T5LOC/T6LOC; until it does, a transfer to them stops with EXEC_UNMAPPED at the frontier.

## Domain

Canonical 2048-word image (every word random, the counters, `TIME1`-`TIME6`, the CDUs and the save registers at 010-061 included) under EB 6 (the file's `EBANK=`), FB of the routine's bank, any A, L and Q unless stated, and any values of the other erasables. VCNTR (the variable-gain pass count) from +0 to 19 or -0, as TVCINITIALIZE and the executive leave it; a negative count reaches TCF +0, where the AGC spins until its TC trap restarts it. The port refuses that word (EXEC_INVALID_ACTION) instead of spinning.

## Evidence

Hashes: `evidence/cm-tvc-exec/SHA256.json`; all files are in the tree.

- **Source map.** `cm-tvc-exec-source-map.json` lists all 144 words of the file with the routine that compiles each (or why a constant is never run as code); 0 are untranslated.
- **Independent assembly comparison.** `make cm-tvc-exec-differential` runs 256 states per routine (3,072 cases) through the original Comanche055 rope in yaAGC (pinned yaAGC with hardware DV and without the IMU/optics device block (tools/prepare_flight_device_engine.py), its ScalerCounter held at 0 (TIME1-TIME6 and the timer interrupts do not advance during a case)) and the native routines; all are equal in Z, 2048 erasable words (A, L 16-bit) and the interrupt-enable state, the I/O channels 11 and the 31A/31B/32 trap flags. The cases and the stops reached per routine are in `cm-tvc-exec-proof.json` (`cm-tvc-exec-differential.log` is the run). One case in 32 is frozen in `tests/cm_tvc_exec_vectors.h`.
- **Host and sanitizer results.** `make test-cm-tvc-exec sanitize-cm-tvc-exec` runs the 96 frozen cases (`cm-tvc-exec-host.log`).
- **C64 execution results.** `make vice-cm-tvc-exec` runs 1 diagnostic program with 25 cases (`cm-tvc-exec-vice.log`, which starts with each PRG's SHA-256). The cases are chosen so the C64 runs take every control-flow edge (both outcomes of each branch) of every linked source that the 3,072 differential cases take: 443 of 443 edges and comparison outcomes (clang -O0 trace-pc-guard,trace-cmp) and 140 of 140 compiled labels. Checked again with gcov -b over gcc -O0 `--coverage` builds of the same sources: the C64 cases take 255 of the 255 branch outcomes all the cases take (`c64_gcov_branches` in the proof).
