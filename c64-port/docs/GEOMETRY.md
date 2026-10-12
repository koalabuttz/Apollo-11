# Translate LEM_GEOMETRY and CSM_GEOMETRY

Lane D1b (alignment and maneuvers). Milestone `geometry`. How the routines are made and checked: `docs/INTERP-COMPILE.md`.

`Luminary099/LEM_GEOMETRY.agc` is native C (`src/lm_geometry.c`) in full. `Comanche055/CSM_GEOMETRY.agc` is native C (`src/cm_geometry.c`) apart from its sextant routines. Each routine runs the file from its entry, or from where it resumes, to the point where control leaves the file.

## Scope

| Luminary099 | Source | Leaves at |
|---|---|---|
| 13,2070 SVDWN2, 13,2073 | state vector to downlink (BOF AVEMIDSW, RVQ) | the caller (RVQ) |
| 13,2114 SVDWN1 | other vehicle's state vector to downlink | the caller |
| 13,2133 RRANGLES, 13,2161 | rendezvous radar trunnion and shaft angles: ASIN, UNIT (BOVB LUNDESCH), ARCTRIG, RTB 2V1STO2S | LUNDESCH, ALARM1, or EXIT |
| 13,2175 (basic) | RADMODES antenna bit: swaps MODEA and MODEB | TC INTPRET |
| 13,2205 | GOTO S2 | the caller |
| 23,2041 RRNB, 23,2054 RRNB1, 23,2065 RRNBMPAC | radar line of sight in navigation-base axes (RTB CDULOGIC, SIN, COS) | the caller |

| Comanche055 | Source | Leaves at |
|---|---|---|
| 10,2237 SVDWN1, 10,2242, 10,2263 SVDWN2 | state vectors to downlink | the caller |
| 23,2155 LOG, 23,2161, 23,2173, 23,2203, 23,2210 | logarithm: NORM, EXIT, TC POLY with its table, then SHORTMP, DAS and TC INTPRET | POLY, SHORTMP, the caller |
| 23,2215 EARROT1, 23,2225 EARROT2, 23,2245 OVERADAY | earth rotation of the landmark vector | TCDANZIG (BOVB), the caller |

Not yet translated (109 Comanche words, recorded in the source map): SXTNB, SXTLOGIC, CALCSXA and SXTANG. They wait for native TRANSP1/TRANSP2 (the Comanche MXV body came with #48).

Destinations outside the file stop with `EXEC_UNMAPPED` until their files have routines.

## Evidence

Hashes: `evidence/geometry/SHA256.json`. Bulk files are assets of the `evidence-geometry` release.

- **Source map.** `geometry-source-map.json` lists every emitted word with the routine that compiles it. Luminary has 113 words, all compiled. Comanche has 253 words: 105 compiled, 39 data words no instruction reads (the NB2NB1/NB1NB2 matrices and other constants), and 109 untranslated as above.
- **Independent assembly comparison.** `make geometry-differential`: 512 states per routine and program (10,752 cases) equal yaAGC in Z, all 2,048 erasable words and the interrupt-enable state. A quarter of the cases put edge values into MPAC and every operand the file names: +0 and -0, one LSB, 1/2, 1/4, just below 1, half-unit axis vectors (with a matrix's rows as a signed permutation, so identity and 90° rotations occur), unit vectors in a coordinate plane, and tiny vectors. In those cases the CDUs sit at 0, 45, 90, 180 or 270°. TIME1, TIME2 and the CDUs vary in every case. The stops reached per routine are in `geometry-proof.json`. Every 32nd case is frozen in `tests/geometry_vectors.h`.
- **Host and sanitizer results.** `make test-geometry sanitize-geometry` runs the 357 frozen cases (every 32nd plus a greedy cover of every branch outcome any case reaches) (`geometry-host.log`).
- **C64 execution results.** `make vice-geometry` runs seven diagnostics with 57 cases, chosen so all 1,018 branch outcomes the host vectors reach are reached on the C64 too (`geometry-vice.log`, with each PRG's sha256).
