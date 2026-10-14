# Translate LEM_GEOMETRY and CSM_GEOMETRY

Lane D1b (alignment and maneuvers). Milestone `geometry`. How the routines are made and checked: `docs/INTERP-COMPILE.md`.

`Luminary099/LEM_GEOMETRY.agc` is native C (`src/lm_geometry.c`) in full. `Comanche055/CSM_GEOMETRY.agc` is native C (`src/cm_geometry.c`) in full too, with SXTLOGIC and its SHORTMP call hand-written in `src/geometry_rtb.c`. Each routine runs the file from its entry, or from where it resumes, to the point where control leaves the file.

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
| 23,2215 EARROT1, 23,2225 EARROT2, 23,2245 OVERADAY | earth rotation of the landmark vector | the caller (EARROT2's BOVB TCDANZIG runs natively, src/cm_integration_moves.c; cases start from DTEAROT 0, see tools/interp_specs.py) |
| 23,2000 SXTNB | sextant line of sight in navigation-base axes: SLOAD* the CDUs, RTB CDULOGIC, RTB PUSH SXTLOGIC, SIN, COS, MXV NB1NB2 | the caller |
| 23,2027 SXTLOGIC (basic) | adds the 19.775° offset (CAF 10DEGS-, ADS MPAC), then CAF QUARTER and TC SHORTMP: native in `geometry_rtb`, with SHORTMP as `geometry_shortmp` | DANZIG |
| 23,2034 CALCSXA, 23,2053 SXTANG, 23,2076 SXTANG1 | sextant shaft and trunnion angles for a star: CALL *SMNB*, MXV NB2NB1, RTB TRANSP1/TRANSP2, VXV, UNIT, CALL ARCTRIG, ACOS, RTB 1STO2S | GOTO 28D (the caller), *SMNB*, ARCTRIG |

SXTLOGIC is basic code entered by `RTB PUSH`, and it calls SHORTMP, a basic multiply outside the file. The spec's `native_ranges` maps SXTLOGIC to `geometry_rtb`, and `native_calls` compiles LOG's `TC SHORTMP` to `geometry_shortmp`, so the oracle still runs through SHORTMP's own instructions. TRANSP1 and TRANSP2 come from `src/rtb_agreement.c`.

Destinations outside the file stop with `EXEC_UNMAPPED` until their files have routines.

## Evidence

Hashes: `evidence/geometry/SHA256.json`. Bulk files are assets of the `evidence-geometry` release.

- **Source map.** `geometry-source-map.json` lists every emitted word with the routine that compiles it. Luminary has 113 words, all compiled. Comanche has 253 words: 204 compiled and 49 constants (the NB2NB1/NB1NB2 matrices, the LOG coefficients and the angle constants), none untranslated.
- **Independent assembly comparison.** `make geometry-differential`: 512 states per routine and program (76,288 cases, 149 routines) equal yaAGC in Z, all 2,048 erasable words and the interrupt-enable state. A quarter of the cases put edge values into MPAC and every operand the file names: +0 and -0, one LSB, 1/2, 1/4, just below 1, half-unit axis vectors (with a matrix's rows as a signed permutation, so identity and 90° rotations occur), unit vectors in a coordinate plane, and tiny vectors. In those cases the CDUs sit at 0, 45, 90, 180 or 270°. TIME1, TIME2 and the CDUs vary in every case. The stops reached per routine are in `geometry-proof.json`. Every 128th case is frozen in `tests/geometry_vectors.h`.
- **Resume points.** Resume points: a DANZIG that finds NEWJOB positive goes to CHANG2 and the job resumes later at the next word, so with `resume_keys` every word after a DANZIG is a routine of its own (the routine counts include them). A quarter of the cases at those routines have NEWJOB positive, and the run stops at CHANG2 as the original does. Each resume point starts from the MODE, PUSHLOC and index registers the code before it leaves there (per-point `states`; `index_states` derives X1 and X2, and the SXTNB and SXTANG resume points are pinned to scalar MODE, since a vector-mode PUSH there would run over X1). SXTANG's return CADR at VAC+034 (GOTO 28D) is outside the file. Runs may take up to 1,000 routines (`max_steps`).
- **Host and sanitizer results.** `make test-geometry sanitize-geometry` runs the 658 frozen cases (every 128th plus a greedy cover of every branch outcome any case reaches) (`geometry-host.log`).
- **C64 execution results.** `make vice-geometry` runs eight diagnostics with 197 cases, chosen so all 1,671 branch outcomes the host vectors reach are reached on the C64 too (`geometry-vice.log`, with each PRG's sha256).
