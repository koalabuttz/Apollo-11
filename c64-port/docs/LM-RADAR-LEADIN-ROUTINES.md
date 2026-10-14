# Translate RADAR_LEADIN_ROUTINES (Luminary), compiled form

Lane D2. Milestone `lm-radar-leadin-routines`. How the routines are made and checked: `docs/INTERP-COMPILE.md`.

`Luminary099/RADAR_LEADIN_ROUTINES.agc` (`src/lm_radar_leadin_routines.c`) is native C in full. Every word of the file (47 words, bank 25) is compiled by `tools/interp_compile.py` with `'basic': 'full'` into 7 basic routines of the control-transfer ABI, at their original addresses. The spec is `lm-radar-leadin-routines` in `tools/interp_specs.py`. (`src/lm_radar_leadin.c`, `docs/LM-RADAR-LEADIN.md`, is the earlier hand-written partial translation.)

The file holds RADSAMP (the radar sampling Waitlist task), DORSAMP (its job: read the radar, store the sample in RSTACK unless R77 runs, cycle RTSTLOC) and VARADAR (select the radar read RRRANGE, RRRDOT, LRVELX/Y/Z or LRALT and go there by SWCALL).

## Spec keys

- **`ebank: 4`.** The file's `EBANK= RSTACK`.
- **`fixed_tables: {'RDRLOCS': (0, 5)}`.** VARADAR's `INDEX RTSTDEX; CAF RDRLOCS`.
- **`range_states`.** The sampling words as P20-P25 and R77 set them: RSTACK holds four DP samples, so RTSTLOC and RTSTMAX step 0-6 by two; RTSTBASE is 0 (RR) or 2 (LR); RTSTDEX names one of the six radar reads; RSAMPDT is the sample period or +0 (stop). RSAMPDT -0 would run RADSAMP's 2CADR as an instruction.
- **`channels: 'all'`, `channel_inputs`, `edge_cases`, `c64_rows: 'cover'`, `index_carry16`, `image_w`.**

Destinations outside the file (WAITLIST, NOVAC, TASKOVER, BANKCALL RADSTALL, ENDOFJOB, and SWCALL to the radar reads) stop the run.

## Evidence

Hashes: `evidence/lm-radar-leadin-routines/SHA256.json` (all files in the tree).

- **Source map.** `lm-radar-leadin-routines-source-map.json`.
- **Independent assembly comparison.** `make lm-radar-leadin-routines-differential`: 1,792 cases equal yaAGC in Z, all 2,048 erasable words and the interrupt-enable state. Every 32nd case is frozen in `tests/lm_radar_leadin_routines_vectors.h`.
- **Host and sanitizer results.** `make test-lm-radar-leadin-routines sanitize-lm-radar-leadin-routines` runs the 56 frozen cases.
- **C64 execution results.** `make vice-lm-radar-leadin-routines` runs 1 diagnostic with 14 cases (PRG sha256 in the log).
- **C64 branch outcomes.** In the proof: the 14 C64 cases take all 179 gcov branch outcomes and all 315 compare edges the host cases take.
