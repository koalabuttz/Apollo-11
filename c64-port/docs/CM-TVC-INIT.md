# Translate Comanche TVCINITIALIZE

Lane G (CM flight). Milestone `cm-tvc-init`. How the routines are made and checked: `docs/INTERP-COMPILE.md` (whole-file basic code: `tools/basic_compile.py`).

`Comanche055/TVCINITIALIZE.agc` (`src/cm_tvc_init.c`) is native C in full: TVCDAPON, MRCLEAN, TVCINIT1 to TVCINIT4 with the filter coefficient load LOADCOEF, attitude-error initialisation, the CSM/LM V46 switch-over PRESWTCH/SWICHOVR and the gain computation S40.15. Each routine runs the file from its entry, or from where it resumes, to the point where control leaves the file. Every original word is one call of the basic-instruction primitives (`src/agc_image_ops.c`) under a label named for its address, with TS overflow skips, CCS four-way skips and INDEX on the whole word kept; computed transfers (TC Q, indexed TCF) go through one dispatch on the target address.

This replaces the earlier hand translation, which the certificate reviewer rejected in #77: its comparison was a listing check rather than a yaAGC run, and none of its entries was a routine of the control-transfer ABI. Now every entry another file names is a registered routine, and the comparison runs the original rope in yaAGC.

## Scope

11 routines, keyed by physical address in `cm_tvc_init_basic`:

| Entry | Kind | Why it is a routine |
|---|---|---|
| 17,2147 TVCDAPON | basic | entry TVCDAPON (P40-P47.agc 2CADR TVCDAPON) |
| 17,2152 MRCLEAN | basic | entry MRCLEAN (TVCRESTARTS.agc CADR MRCLEAN) |
| 17,2171 TVCINIT1 | basic | 2CADR TVCINIT1 |
| 17,2176  | basic | return from IBNKCALL |
| 17,2324 TVCINIT4 | basic | entry TVCINIT4 (TVCRESTARTS.agc CADR TVCINIT4) |
| 17,2343  | basic | return from WAITLIST |
| 17,2351 PRESWTCH | basic | entry PRESWTCH (EXTENDED_VERBS.agc CADR PRESWTCH) |
| 17,2352  | basic | TC 02352 stored in E6,1613 |
| 17,2361  | basic | entry 17,2361 (TVCRESTARTS.agc CADR SWICHOVR   +5) |
| 17,2462 S40.15 | basic | entry S40.15 (TVCEXECUTIVE.agc CADR S40.15) |
| E6,1613 (erasable) | continuation | the TC words the file stores there: 02352, 04753 |

TC RTRNLOC is the erasable continuation E6,1613: it accepts the two Q values SWICHOVR +5 saves there, the return into PRESWTCH (TC 2352, a routine of this file) and IBNKCALL's ISWRETRN (TC 4753), and refuses any other.

Destinations outside the file: IBNKCALL, POSTJUMP, RESUME, WAITLIST. Lane A has not yet registered the Comanche Executive and Waitlist routines (WAITLIST, NOVAC, FINDVAC, TASKOVER, ENDOFJOB, IBNKCALL with ISWRETRN, POSTJUMP, BANKCALL, RESUME) or the T5RUPT/T6RUPT vectors that dispatch T5LOC/T6LOC; until it does, a transfer to them stops with EXEC_UNMAPPED at the frontier.

## Domain

Canonical 2048-word image (every word random, the counters, `TIME1`-`TIME6`, the CDUs and the save registers at 010-061 included) under EB 6 (the file's `EBANK=`), FB of the routine's bank, any A, L and Q unless stated, and any values of the other erasables. CSMMASS within the CSM mass and EMDOT (SPS flow rate) up to 01400 at B+3 kg/cs, so the 10-second mass decrement never overflows TS MASSTMP; the gains S40.15 multiplies (EKTLX/I, KTLX/I, IXX, IAVG, IAVG/TLX) within +-07777 as MASSPROP and the pad load leave them, so TS 1/CONACC and TS VARK never overflow into the coefficient tables. COEFFADR holds one of the three coefficient-table addresses the file stores (CSMN10, HBN10, LBN10). SWICHOVR +5 is entered only through IBNKCALL (Q = ISWRETRN, 4753).

## Evidence

Hashes: `evidence/cm-tvc-init/SHA256.json`; all files are in the tree.

- **Source map.** `cm-tvc-init-source-map.json` lists all 259 words of the file with the routine that compiles each (or why a constant is never run as code); 0 are untranslated.
- **Independent assembly comparison.** `make cm-tvc-init-differential` runs 256 states per routine (2,560 cases) through the original Comanche055 rope in yaAGC (pinned yaAGC with hardware DV and without the IMU/optics device block (tools/prepare_flight_device_engine.py), its ScalerCounter held at 0 (TIME1-TIME6 and the timer interrupts do not advance during a case)) and the native routines; all are equal in Z, 2048 erasable words (A, L 16-bit) and the interrupt-enable state, the I/O channels 11 and the 31A/31B/32 trap flags. The cases and the stops reached per routine are in `cm-tvc-init-proof.json` (`cm-tvc-init-differential.log` is the run). One case in 32 is frozen in `tests/cm_tvc_init_vectors.h`.
- **Host and sanitizer results.** `make test-cm-tvc-init sanitize-cm-tvc-init` runs the 80 frozen cases (`cm-tvc-init-host.log`).
- **C64 execution results.** `make vice-cm-tvc-init` runs 1 diagnostic program with 14 cases (`cm-tvc-init-vice.log`, which starts with each PRG's SHA-256). The cases are chosen so the C64 runs take every control-flow edge (both outcomes of each branch) of every linked source that the 2,560 differential cases take: 612 of 612 edges and comparison outcomes (clang -O0 trace-pc-guard,trace-cmp) and 198 of 198 compiled labels. Checked again with gcov -b over gcc -O0 `--coverage` builds of the same sources: the C64 cases take 371 of the 371 branch outcomes all the cases take (`c64_gcov_branches` in the proof).
