# Translate AOSTASK_AND_AOSJOB (Luminary)

Lane D2. Milestone `lm-aostask`. How the routines are made and checked: `docs/INTERP-COMPILE.md`.

`Luminary099/AOSTASK_AND_AOSJOB.agc` (`src/lm_aostask.c`) is native C in full. Every word of the file (723 words, bank 20) is compiled by `tools/interp_compile.py` with `'basic': 'full'` into 11 basic routines of the control-transfer ABI, at their original addresses, and 1/ACCS +2, which 1/ACCJOB reaches through BANKCALL. The spec is `lm-aostask` in `tools/interp_specs.py`.

The file holds the DAP's mass and inertia interface: 1/ACCSET, 1/ACCJOB and 1/ACCS (the jet acceleration and inertia tables from the LM and CSM masses, docked or not, and the DPS gimbal factors), and the AOSTASK/AOSJOB offset-acceleration filter (GETAOSUV, the deadband and 1/NET+ / 1/NET- functions).

## Spec keys

- **`ebank: 6`.** The file's `EBANK= AOSQ`.
- **`channels: 'all'`, `channel_inputs`.** Channel orders go through the shared channel model (`include/agc_channel_ops.h`, Lane D1's #259): outputs to the program's `agc_hardware`, inputs from the latched input channels.
- **`call_skips: {'DVOVSUB': 1}`.** DVOVSUB returns past the ADRES of its divisor (INCR Q).
- **`fixed_tables`.** The constants the file reads under INDEX: the inertia and mass tables (INERCONA through GFACTM), the jet torque and acceleration coefficients (TORQCONS through COEFF), the U/V jet masks, and DVOVSUB's divisors through the ADRES after each TC DVOVSUB (LRESC, TORQCONS +11 and +42).
- **`tcf_words` (new).** A TC to an erasable word that holds a stored TCF (ARET, set by QXCH ARET or `CA ACCHERE; TS ARET`) runs that TCF's target.
- **`states`.** DOCKTEST's CCS DOCKTEMP: DOCKTEMP is DAPBOOLS' CSMDOCKD bit, 0 or BIT13. Negative, it would fall into DVOVSUB with no call.
- **`edge_cases`, `c64_rows: 'cover'`, `index_carry16`, `image_w`.**

Every transfer from the file to a fixed address outside it (BANKCALL and the other Executive and DAP destinations) stops the run.

## Evidence

Hashes: `evidence/lm-aostask/SHA256.json` (all files in the tree).

- **Source map.** `lm-aostask-source-map.json`: every word with the routine that compiles it.
- **Independent assembly comparison.** `make lm-aostask-differential`: 2,816 cases equal yaAGC in Z, all 2,048 erasable words, the interrupt-enable state, I/O channel 12 and the 31A/31B/32 trap flags. Every 32nd case is frozen in `tests/lm_aostask_vectors.h`.
- **Host and sanitizer results.** `make test-lm-aostask sanitize-lm-aostask` runs the 88 frozen cases (`lm-aostask-host.log`).
- **C64 execution results.** `make vice-lm-aostask` runs 1 diagnostic with 33 cases (`lm-aostask-vice.log`, with the PRG's sha256).
- **C64 branch outcomes.** In `lm-aostask-proof.json`: the 33 C64 cases take all 700 gcov branch outcomes and all 1,078 compare edges the host cases take.
