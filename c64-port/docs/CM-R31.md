# Translate Comanche055 R31

Lane E (targeting). Milestone `cm-r31`. How the routines are made and checked: `docs/INTERP-COMPILE.md`.

`Comanche055/R31.agc` (`src/cm_r31.c`) is native C in full: all 218 words in bank 31 are compiled by `tools/interp_compile.py` (spec `cm-r31` in `tools/interp_specs.py`) into 44 routines of the control-transfer ABI. R31 is V83, the rendezvous display of range, range rate and theta. Each routine runs the file from its entry, or from where it resumes, to the point where control leaves the file.

## Scope

| Comanche055 | Source | Leaves at |
|---|---|---|
| 31,2000 R31CALL, 31,2004 DSPDELAY, 31,2007 | the display job and its one-second delay loop | FINDVAC, BANKCALL DELAYJOB |
| 31,2013 DISPN5X to 31,2026 | V16N54 or V16N53 through GOMARKF and its three answers | BANKCALL GOMARKF, B5OFF |
| 31,2027 V83, 31,2030, 31,2032 V83CALL, 31,2033 | TC INTPRET, then GOTO HAVEBASE or STATEXTP | the file's own routines |
| 31,2035 COMPDISP, 31,2056 | range (NORM, VSL*), range rate, CDUTRIG | CDUTRIG, R34LOS |
| 31,2060 R34ANG, 31,2070, 31,2072, 31,2125, 31,2126 | theta: *NBSM*, VXM REFSMMAT, VPROJ, ACOS, the EXTVBACT check | *NBSM*, ENDEXT, V83 |
| 31,2140 STATEXTP to 31,2177 | base vectors: RTB LOADTIME, GETRVN, LEMPREC, CSMPREC | LEMPREC, CSMPREC |
| 31,2171 HAVEBASE to 31,2305 | INTSTALL, INTEGRVS, OTHINT, GETRVN5, GETRVN6, GETRVN | INTSTALL, INTEGRVS, LEMPREC |
| 31,2311 R34LOS, 31,2312, 31,2326, 31,2330 | the sextant line of sight (CDUS, CDUT) | SXTNB |

RTB LOADTIME runs the native `rtb_angles(RTB_CM, RTB_LOAD_TIME)`. Destinations outside the file (FINDVAC, BANKCALL, B5OFF, ENDEXT, CDUTRIG, *NBSM*, SXTNB, LEMPREC, CSMPREC, INTSTALL, INTEGRVS) stop with `EXEC_UNMAPPED` until their files have routines.

Two shared-tool additions came with this file. `interp_compile.py` compiles VXM, MXV and VPROJ (erasable operands only; a fixed matrix operand is refused), and a CALL whose return point lies past the end of the file adds no routine (R34LOS ends with STCALL 12D R34ANG, which never returns). `interp_routine_differential.py` accepts a one-mission spec, and no longer seeds the counters and NEWJOB (045-077) as data operands: a word there is a switch bit (CLEAR MOONFLAG, 063). Neither change alters the routines or cases of the earlier specs.

## Evidence

Hashes: `evidence/cm-r31/SHA256.json`. Bulk files are assets of the `evidence-cm-r31` release.

- **Source map.** `cm-r31-source-map.json` lists all 218 words with the routine that compiles each; none is untranslated.
- **Independent assembly comparison.** `make cm-r31-differential`: 512 states per routine (22,528 cases) equal yaAGC in Z, all 2,048 erasable words and the interrupt-enable state. The stops reached per routine are in `cm-r31-proof.json`. Every 32nd case is frozen in `tests/cm_r31_vectors.h`.
- **Host and sanitizer results.** `make test-cm-r31 sanitize-cm-r31` runs the 704 frozen cases (`cm-r31-host.log`).
- **C64 execution results.** `make vice-cm-r31` runs seven diagnostics, one per group of routines, with 88 cases (`cm-r31-vice.log`).
