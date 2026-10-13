# Translate PLANETARY_INERTIAL_ORIENTATION (Luminary099 and Comanche055)

Lane A2 (restart and telemetry; file moved from Lane B). Milestone `planetary`. How the routines are made and checked: `docs/INTERP-COMPILE.md`.

`Luminary099/PLANETARY_INERTIAL_ORIENTATION.agc` (`src/lm_planetary.c`, banks 06, 24, 25 and 26) and `Comanche055/PLANETARY_INERTIAL_ORIENTATION.agc` (`src/cm_planetary.c`, bank 26) are native C in full. Both are compiled by `tools/interp_compile.py` (spec `planetary` in `tools/interp_specs.py`, with `resume_keys`), as 95 and 94 routines of the control-transfer ABI. The file converts between the basic reference frame and the planetary (Earth or Moon fixed) frame: R-TO-RP and RP-TO-R, with the Moon and Earth rotation matrices.

## Scope

Both missions, with the same labels (MOONMXA is Luminary's bank-25 continuation of MOONMX):

| Routines | Source | Leaves at |
|---|---|---|
| RP-TO-R, RPTORB, RPTORA | planetary to basic frame: R = MT(T)(RP + L x RP), with VXV/VAD and VXM through MMATRIX | the caller through RPREXIT (S1) |
| R-TO-RP, RTORPB, RPRPXXXX, RTORPA | basic to planetary frame: RP = M(T)(R - L x R), with VXV/BVSU and MXV through MMATRIX | the caller through RPREXIT (S1) |
| MOONMX, MOONMXA, NEWANGLE | the Moon matrix M from the librations (NODIO, BSUBO, FSUBO and their rates), SIN/COS | the caller through EARTHMXX (S2), or RVQ from NEWANGLE |
| EARTHMX, EARTHL | the Earth matrix from AZO and WEARTH, and the Earth L vector (-AX, -AY, 0) | the caller through EARTHMXX (S2), or RVQ from EARTHL |

Every word is compiled, including the constants (COSI, SINI, the libration and rate coefficients) as `K` arrays. The file calls no routine of another file: SIN, COS, MXV, VXM and the other orders are the shared interpreter bodies.

The spec states the facts the cases need. Each point a run reaches past R-TO-RP or RP-TO-R is entered with PUSHLOC and MODE as their SETPDs and loads leave them: a resume point entered with a deeper pushdown list overwrites S1/S2 (VAC+050/051), and the return through them then loops on the AGC as well.

## Evidence

Hashes: `evidence/planetary/SHA256.json`. Bulk files are assets of the `evidence-planetary` release.

- **Source map.** `planetary-source-map.json` lists every word with the routine that compiles it: 188 Luminary words and 204 Comanche words, none untranslated.
- **Independent assembly comparison.** `make planetary-differential`: 256 states per routine (48,384 cases over 189 routines) equal yaAGC in Z, all 2,048 erasable words and the interrupt-enable state. At RP-TO-R and R-TO-RP, a quarter of the cases each start with MPAC (the body flag) at +0, -0 and a nonzero value, so the Earth branch of `BHIZ` (MPAC ±0, what LAT-LONG, LALOTORV and Comanche P20-P25 pass for Earth) is taken as well as the Moon branch; edge values are on. Every 64th case is frozen in `tests/planetary_vectors.h`.
- **Host and sanitizer results.** `make test-planetary sanitize-planetary` runs the 756 frozen cases (`planetary-host.log`).
- **C64 execution results.** `make vice-planetary` runs nine diagnostics with 204 cases (`planetary-vice.log`). They take every branch arc of the linked sources that the 48,384 host cases take (1,165 arcs, `tools/c64_row_cover.py`), including the Earth outcome of the `BHIZ` at both entries.
