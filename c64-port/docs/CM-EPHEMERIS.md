# Translate Comanche LUNAR_AND_SOLAR_EPHEMERIDES_SUBROUTINES

Lane G (CM flight). Milestone `cm-ephemeris`. How the routines are made and checked: `docs/INTERP-COMPILE.md`.

`Comanche055/LUNAR_AND_SOLAR_EPHEMERIDES_SUBROUTINES.agc` (`src/cm_ephemeris.c`) is native C in full: The ephemeris routines LSPOS, LUNPOS, LUNVEL and SOLPOS (SOLPOS has no caller in Comanche055 and is a public entry), LSTIME (the time since the ephemeris epoch) and the evaluators RES/RESA (the Sun), REM/REMA (the Moon's position polynomial) and VEM/VEMA (its velocity), all interpretive. The legacy packets (src/cm_ephemeris_*.c) are superseded and no longer cited. Each routine runs the file from its entry, or from where it resumes, to the point where control leaves the file. Every interpretive order is a call of the shared interpreter's order code (`src/cm_interp_orders.c`) at NEWOPS, with LOC, the pushdown list, MODE and the index registers as the AGC keeps them; a computed GOTO through an index register goes to the routine keyed at the address it holds.

This replaces the earlier hand translation, which the certificate reviewer rejected in #58: its comparison was a listing check rather than a yaAGC run, and none of its entries was a routine of the control-transfer ABI. Now every entry another file names is a registered routine, and the comparison runs the original rope in yaAGC.

## Scope

32 routines, keyed by physical address in `cm_ephemeris_interpretive`:

| Entry | Kind | Why it is a routine |
|---|---|---|
| 26,2110 LSPOS | interpretive | entry LSPOS (P51-P53.agc  LSPOS) |
| 26,2112  | interpretive | CHANG2 resume |
| 26,2115 LUNPOS | interpretive | entry LUNPOS (P20-P25.agc  LUNPOS) |
| 26,2120 LUNVEL | interpretive | entry LUNVEL (ORBITAL_INTEGRATION.agc  LUNVEL) |
| 26,2123 SOLPOS | interpretive | entry SOLPOS |
| 26,2126 LSTIME | interpretive | CHANG2 resume |
| 26,2131  | interpretive | CHANG2 resume |
| 26,2133  | interpretive | CHANG2 resume |
| 26,2135  | interpretive | CHANG2 resume |
| 26,2141  | interpretive | CHANG2 resume |
| 26,2143 RES | interpretive | entry RES |
| 26,2145  | interpretive | CHANG2 resume |
| 26,2146  | interpretive | CHANG2 resume |
| 26,2150  | interpretive | CHANG2 resume |
| 26,2152  | interpretive | CHANG2 resume |
| 26,2153  | interpretive | CHANG2 resume |
| 26,2155  | interpretive | CHANG2 resume |
| 26,2156  | interpretive | CHANG2 resume |
| 26,2157  | interpretive | CHANG2 resume |
| 26,2161 RESA | interpretive | entry RESA |
| 26,2162 REM | interpretive | entry REM |
| 26,2165 REMA | interpretive | CHANG2 resume |
| 26,2170  | interpretive | CHANG2 resume |
| 26,2172  | interpretive | CHANG2 resume |
| 26,2173 VEM | interpretive | entry VEM |
| 26,2176  | interpretive | CHANG2 resume |
| 26,2200 VEMA | interpretive | CHANG2 resume |
| 26,2202  | interpretive | CHANG2 resume |
| 26,2203  | interpretive | CHANG2 resume |
| 26,2205  | interpretive | CHANG2 resume |
| 26,2210  | interpretive | CHANG2 resume |
| 26,2212  | interpretive | CHANG2 resume |

Destinations outside the file: CHANG2. Lane A has not yet registered the Comanche Executive and Waitlist routines (WAITLIST, NOVAC, FINDVAC, TASKOVER, ENDOFJOB, IBNKCALL with ISWRETRN, POSTJUMP, BANKCALL, RESUME) or the T5RUPT/T6RUPT vectors that dispatch T5LOC/T6LOC; until it does, a transfer to them stops with EXEC_UNMAPPED at the frontier.

## Domain

Canonical 2048-word image (every word random, the counters, `TIME1`-`TIME6`, the CDUs and the save registers at 010-061 included) under the routine's `EBANK=`, FB of the routine's bank, any A, L and Q unless stated, and any values of the other erasables. `NEWJOB` is +0 or, in a quarter of the cases, positive: a job waiting, so any `DANZIG` goes to `CHANG2`, where the routine stops; every interpretive word that can follow a `DANZIG` is a `CHANG2` resume key of its own. EBANK= as the listing has it (E7: VECOEM, TEPHEM, the Moon and Sun coefficients); S1 any value up to LSTIME's SSP S1 6D and 6 (the TIX step it sets) from its GOTO X1 at 26,2141 on; PUSHLOC 0D to 10D of the VAC area (LSTIME's SETPD 0D), as the entries leave them. The yaAGC reference runs with interrupts inhibited.

## Evidence

Hashes: `evidence/cm-ephemeris/SHA256.json`; all files are in the tree.

- **Source map.** `cm-ephemeris-source-map.json` lists all 71 words of the file with the routine that compiles each (or why a constant is never run as code); 0 are untranslated.
- **Independent assembly comparison.** `make cm-ephemeris-differential` runs 256 states per routine (8,192 cases) through the original Comanche055 rope in yaAGC (pinned yaAGC) and the native routines; all are equal in Z, 2048 erasable words (A, L 16-bit) and the interrupt-enable state. The cases and the stops reached per routine are in `cm-ephemeris-proof.json` (`cm-ephemeris-differential.log` is the run). One case in 32 is frozen in `tests/cm_ephemeris_vectors.h`.
- **Host and sanitizer results.** `make test-cm-ephemeris sanitize-cm-ephemeris` runs the 256 frozen cases (`cm-ephemeris-host.log`).
- **C64 execution results.** `make vice-cm-ephemeris` runs 2 diagnostic programs with 38 cases (`cm-ephemeris-vice.log`, which starts with each PRG's SHA-256). The C64 rows are chosen (tools/c64_row_cover.py) so the C64 runs take every branch outcome the 8,192 differential cases take, checked with gcov -b over gcc -O0 `--coverage` builds of every linked source: 569 of 569 (`c64_gcov_branches` in the proof).

## Flight build

The spec sets `trusted_image`. In flight builds (`RK_FIXED_IMAGE`), the entry checks skip their scan of the whole erasable image, which costs about 121,000 cycles a routine on the C64. Host tests and C64 diagnostics keep the scan.
