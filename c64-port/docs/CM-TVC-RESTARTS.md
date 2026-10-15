# Translate Comanche TVCRESTARTS

Lane G (CM flight). Milestone `cm-tvc-restarts`. How the routines are made and checked: `docs/INTERP-COMPILE.md` (whole-file basic code: `tools/basic_compile.py`).

`Comanche055/TVCRESTARTS.agc` (`src/cm_tvc_restarts.c`) is native C in full: REDOTVC, the DAP restart (TVCDAPHS, ENABL1/ENABL2, CMDSOUT, FINCOPY, TRIM/CMD, CHKSTRK) and the TVCEXECUTIVE restart dispatch EXRSTRT, which indexes TVCEXADR by the executive phase and jumps through it (INDEX A, TCF 0). Each routine runs the file from its entry, or from where it resumes, to the point where control leaves the file. Every original word is one call of the basic-instruction primitives (`src/agc_image_ops.c`) under a label named for its address, with TS overflow skips, CCS four-way skips and INDEX on the whole word kept; computed transfers (TC Q, indexed TCF) go through one dispatch on the target address.

This replaces the earlier hand translation, which the certificate reviewer rejected in #77: its comparison was a listing check rather than a yaAGC run, and none of its entries was a routine of the control-transfer ABI. Now every entry another file names is a registered routine, and the comparison runs the original rope in yaAGC.

## Scope

7 routines, keyed by physical address in `cm_tvc_restarts_basic`:

| Entry | Kind | Why it is a routine |
|---|---|---|
| 16,3140 REDOTVC | basic | entry REDOTVC (FRESH_START_AND_RESTART.agc 2CADR REDOTVC) |
| 16,3152 TVCDAPHS | basic | return from WAITLIST |
| 16,3174 ENABL2 | basic | GENADR ENABL2 |
| 16,3205 CMDSOUT | basic | GENADR CMDSOUT |
| 16,3232 CHKSTRK | basic | return from IBNKCALL |
| 16,3243  | basic | return from ISWCALL |
| 16,3253 EXRSTRT | basic | 2CADR EXRSTRT |

Destinations outside the file: IBNKCALL, ISWCALL, NOQRSM, POSTJUMP, RESUME, WAITLIST. Lane A has not yet registered the Comanche Executive and Waitlist routines (WAITLIST, NOVAC, FINDVAC, TASKOVER, ENDOFJOB, IBNKCALL with ISWRETRN, POSTJUMP, BANKCALL, RESUME) or the T5RUPT/T6RUPT vectors that dispatch T5LOC/T6LOC; until it does, a transfer to them stops with EXEC_UNMAPPED at the frontier.

## Domain

Canonical 2048-word image (every word random, the counters, `TIME1`-`TIME6`, the CDUs and the save registers at 010-061 included) under EB 6 (the file's `EBANK=`), FB of the routine's bank, any A, L and Q unless stated, and any values of the other erasables. TVCPHASE one of the DAP phases -2, -1, +0, 1, 2, 3 and TVCEXPHS one of the executive phases 1 to 6, as TVCINITIALIZE, the DAPs and TVCEXECUTIVE set them.

## Evidence

Hashes: `evidence/cm-tvc-restarts/SHA256.json`; all files are in the tree.

- **Source map.** `cm-tvc-restarts-source-map.json` lists all 90 words of the file with the routine that compiles each (or why a constant is never run as code); 0 are untranslated.
- **Independent assembly comparison.** `make cm-tvc-restarts-differential` runs 256 states per routine (1,792 cases) through the original Comanche055 rope in yaAGC (pinned yaAGC with hardware DV and without the IMU/optics device block (tools/prepare_flight_device_engine.py), its ScalerCounter held at 0 (TIME1-TIME6 and the timer interrupts do not advance during a case)) and the native routines; all are equal in Z, 2048 erasable words (A, L 16-bit) and the interrupt-enable state, the I/O channels 12, 14 and the 31A/31B/32 trap flags. The cases and the stops reached per routine are in `cm-tvc-restarts-proof.json` (`cm-tvc-restarts-differential.log` is the run). One case in 32 is frozen in `tests/cm_tvc_restarts_vectors.h`.
- **Host and sanitizer results.** `make test-cm-tvc-restarts sanitize-cm-tvc-restarts` runs the 56 frozen cases (`cm-tvc-restarts-host.log`).
- **C64 execution results.** `make vice-cm-tvc-restarts` runs 1 diagnostic program with 19 cases (`cm-tvc-restarts-vice.log`, which starts with each PRG's SHA-256). The cases are chosen so the C64 runs take every control-flow edge (both outcomes of each branch) of every linked source that the 1,792 differential cases take: 398 of 398 edges and comparison outcomes (clang -O0 trace-pc-guard,trace-cmp) and 73 of 73 compiled labels. Checked again with gcov -b over gcc -O0 `--coverage` builds of the same sources: the C64 cases take 235 of the 235 branch outcomes all the cases take (`c64_gcov_branches` in the proof).

## Flight build

The spec sets `trusted_image`. In flight builds (`RK_FIXED_IMAGE`), the entry checks skip their scan of the whole erasable image, which costs about 121,000 cycles a routine on the C64. Host tests and C64 diagnostics keep the scan.
