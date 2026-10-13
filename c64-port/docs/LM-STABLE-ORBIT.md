# Compile STABLE_ORBIT into transfer routines (Luminary099)

Lane B (orbital math). Milestone `lm-stable-orbit`. How the routines are made and checked: `docs/INTERP-COMPILE.md`.

`Luminary099/STABLE_ORBIT.agc` (`src/lm_stable_orbit.c`, 233 words in bank 34) is native C in full, as 101 routines of the control-transfer ABI compiled by `tools/interp_compile.py`. The spec is `lm-stable-orbit` in `tools/interp_specs.py`.

## Scope

- **Entries.** P38 and P78 (stable orbit rendezvous, this or the other vehicle active) and P39 and P79 (stable orbit midcourse), the major modes FRESH_START_AND_RESTART's tables start, each keyed by its address in `lm_stable_orbit_interpretive` or `lm_stable_orbit_basic`.
- **Inside.** The display sequences (V06N33, N55, V04N06, N57, N34, N58, N81), the SOI and SOR options, RECYCLE, MAINRTNE, TEST3979, MAINRTN1, P39/P79A and B, PREC/TT, INTRPVP and OTHERV, and VNDSPLY.
- **Resumes.** Every branch target, CALL/STCALL return, EXIT and TC INTPRET point is a routine. The returns of GOFLASH (terminate, proceed, recycle) and GOFLASHR (also the immediate return) are routines too (`returns`, `resume_orphans`), since the display system comes back to them from outside the file.
- **LOADTIME.** The RTB in P39/P79B calls the native `rtb_angles` LOADTIME (`src/rtb_angles.c`).
- **Leaves at.** NEWOPS with LOC outside the file (the outside subroutines: AVFLAGA, AVFLAGP, P20FLGON, SELECTMU, PRECSET, S34/35.1, S3435.25, TIMETHET, PERIAPO1, SHIFTR1, VN1645, CSMPREC, LEMPREC), the outside basic routines BANKCALL, BLANKET, TCF ENDOFJOB and TCF GOTOPOOH, and the interpreter's POODOO1/ALARM1 exits.

## States the cases start from

As in every compiled file, each case is a random 2,048-word image with the routine's legal entry state, under EBANK 7 (`EBANK= SUBEXIT`).

## Evidence

Hashes: `evidence/lm-stable-orbit/SHA256.json`. Bulk files are assets of the `evidence-lm-stable-orbit` release.

- **Source map.** `lm-stable-orbit-source-map.json` lists every word with the routine that compiles it: 233 words, none untranslated.
- **Independent assembly comparison.** `lm-stable-orbit-proof.json` and `lm-stable-orbit-differential.log`, made by `make lm-stable-orbit-differential`. 256 states per routine (25,856 cases) equal yaAGC in Z, all 2,048 erasable words and the interrupt-enable state. Every 128th case is frozen in `tests/lm_stable_orbit_vectors.h`.
- **Host and sanitizer results.** `make test-lm-stable-orbit sanitize-lm-stable-orbit` runs the 202 frozen cases (`lm-stable-orbit-host.log`).
- **C64 execution results.** `make vice-lm-stable-orbit` runs 2 diagnostics with all 202 frozen cases in VICE (`lm-stable-orbit-vice.log`). No routine is host-only. The image lives in the RAM under KERNAL at $e000, and the programs build without inlining, with a static stack and with 110 zero-page bytes.
