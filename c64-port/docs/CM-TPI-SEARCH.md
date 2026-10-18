# Translate Comanche055 TPI_SEARCH

Lane E (targeting). Milestone `cm-tpi-search`. How the routines are made and checked: `docs/INTERP-COMPILE.md`.

`Comanche055/TPI_SEARCH.agc` (`src/cm_tpi_search.c`) is native C in full: all 443 words (banks 35, 36 and 07) are compiled by `tools/interp_compile.py` (spec `cm-tpi-search` in `tools/interp_specs.py`) into 65 routines of the control-transfer ABI. TPI_SEARCH is P17 and P77, the TPI search programs, and TRANSANG, the transfer-angle subroutine. Each routine runs the file from its entry, or from where it resumes, to the point where control leaves the file.

## Scope

| Comanche055 | Source | Leaves at |
|---|---|---|
| 35 P17, P77, VNCOMP17, ALARUMS, V06N72 | the program entries and their displays | BANKCALL GOFLASH and GOFLASHR (all three answers), AVFLAGA, AVFLAGP, P20FLGON, VNPOOH, BLANKET, ALARM, GOTOPOOH |
| 36 P17S, BIS, CONCAUL, ADVCSM, JUNCT1-3, INTERP | the search: advance the CSM, iterate on the central angle | the file's own routines, ENDOFJOB |
| 36 HALFSAFE, LRGRDVO, RNGETEST, ENDEN, USEKAY, NEXUS | the safe-perigee and range tests, the last iteration | the file's own routines |
| 07 TRANSANG | the transfer angle | its caller |

Destinations outside the file stop with `EXEC_UNMAPPED` until their files have routines.

Two parts of the file needed spec keys:

- **MUTABLE.** `STODL* MUTABLE +2,1` and its neighbours index a fixed table with X1 = -2 or -10. `fixed_index` compiles each reachable K and pins X1 to those values.
- **Switched erasable.** The search keeps its state in EB 7 (`ebank: 7`). The states pin the counters at 03745 and 03746 and the saved Q in VNCOMP17 so the cases reach every loop exit.

## Evidence

Hashes: `evidence/cm-tpi-search/SHA256.json`. Bulk files are assets of the `evidence-cm-tpi-search` release.

- **Source map.** `cm-tpi-search-source-map.json` lists all 443 words with the routine that compiles each; none is untranslated.
- **Independent assembly comparison.** `make cm-tpi-search-differential`: 512 states per routine (33,280 cases) equal yaAGC in Z, all 2,048 erasable words and the interrupt-enable state. Every 128th case is frozen in `tests/cm_tpi_search_vectors.h`.
- **Host and sanitizer results.** `make test-cm-tpi-search sanitize-cm-tpi-search` runs the 260 frozen cases (`cm-tpi-search-host.log`).
- **C64 execution results.** `make vice-cm-tpi-search` runs 16 diagnostics with 248 cases (`cm-tpi-search-vice.log`). No routine is host-only.

The spec sets `trusted_image`. In flight builds (`RK_FIXED_IMAGE`), the entry checks skip their scan of the whole erasable image, which costs about 121,000 cycles a routine on the C64. Host tests and C64 diagnostics keep the scan.
