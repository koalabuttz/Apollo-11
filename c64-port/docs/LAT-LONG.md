# Translate LATITUDE_LONGITUDE_SUBROUTINES (Luminary099 and Comanche055)

Lane A2 (restart and telemetry; file moved from Lane B). Milestone `lat-long`. How the routines are made and checked: `docs/INTERP-COMPILE.md`.

`Luminary099/LATITUDE_LONGITUDE_SUBROUTINES.agc` (`src/lm_lat_long.c`) and `Comanche055/LATITUDE_LONGITUDE_SUBROUTINES.agc` (`src/cm_lat_long.c`) are native C in full. Both are compiled by `tools/interp_compile.py` (spec `lat-long` in `tools/interp_specs.py`, with `resume_keys`), as 75 routines of the control-transfer ABI each. The file converts a position vector to latitude, longitude and altitude and back, over the Earth's oblate surface or the Moon's landing-site radius.

## Scope

Both missions, with the same labels:

| Routines | Source | Leaves at |
|---|---|---|
| LAT-LONG, CALLRTRP, SETGAMMA, SETGMEX | position to LAT, LONG, ALT: frame change, ARCTAN for latitude and longitude | R-TO-RP (another file), the caller through INCORPEX |
| LALOTORV, CALLRPRT | LAT, LONG, ALT to a position vector | RP-TO-R (another file), the caller |
| GETERAD | the Earth radius at a latitude (B2XSC, EE) | the caller |
| ARCTAN, CLROVFLW, NEGCOS, ARCTANXX, NEGOUT | the arctangent of SINTH/COSTH with its quadrant | the caller |
| SETRE, SETREX, TSTRLSRM, SETRXX | ERADM: the pad radius, the landing-site radius RLS, or GETERAD | the caller |

Every word is compiled, including the constants. R-TO-RP and RP-TO-R belong to PLANETARY_INERTIAL_ORIENTATION (`docs/PLANETARY.md`): a run stops at NEWOPS with LOC there.

The spec's entries are the labels other files CALL or STCALL: LAT-LONG, LALOTORV and ARCTAN in Luminary, LAT-LONG, LALOTORV, GETERAD and SETRE in Comanche. Each point a run reaches is entered with PUSHLOC and MODE as the entries' SETPDs and loads leave them.

## Evidence

Hashes: `evidence/lat-long/SHA256.json`. Bulk files are assets of the `evidence-lat-long` release.

- **Source map.** `lat-long-source-map.json` lists every word with the routine that compiles it: 155 Luminary words and 159 Comanche words, none untranslated.
- **Independent assembly comparison.** `make lat-long-differential`: 256 states per routine (38,400 cases over 150 routines) equal yaAGC in Z, all 2,048 erasable words and the interrupt-enable state. Every 64th case is frozen in `tests/lat_long_vectors.h`.
- **Host and sanitizer results.** `make test-lat-long sanitize-lat-long` runs the 600 frozen cases (`lat-long-host.log`).
- **C64 execution results.** `make vice-lat-long` runs eight diagnostics with 179 cases (`lat-long-vice.log`). They take every branch arc of the linked sources that the 38,400 host cases take (1,160 arcs, `tools/c64_row_cover.py`).
