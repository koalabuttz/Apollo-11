# Translate MEASUREMENT_INCORPORATION (both missions)

Lane D2 (rendezvous navigation). Milestone `measurement-inc`. How the routines are made and checked: `docs/INTERP-COMPILE.md`.

`Luminary099/MEASUREMENT_INCORPORATION.agc` (`src/lm_measurement_inc.c`) and `Comanche055/MEASUREMENT_INCORPORATION.agc` (`src/cm_measurement_inc.c`) are native C in full. Every word of both files is compiled by `tools/interp_compile.py` into 160 routines per mission of the control-transfer ABI. The spec is `measurement-inc` in `tools/interp_specs.py`.

The file holds the navigation filter's update, which P20-P25 run for every rendezvous radar, VHF or sextant mark:
- INCORP1 forms the Z vectors (the W matrix times the measurement geometry BVECTOR), the measurement weighting GAMMA, the OMEGA vectors and the state correction DELTAX. NEWZCOMP rescales the Z vectors.
- INCORP2 takes the correction into the W matrix (FAZA, FAZB, in restartable phases) and the state vector (FAZC, FAZAB), rectifying the integration when the correction overflows, and saves the downlink state vector.

## Spec keys

- **`ebank: 5`.** The file's `EBANK= W`. The interpreter reaches its E3, E4 and E7 names (RCV, NORMGAM, OMEGA, DELTAQ...) through the address bits above the bank's 256 words, so EB must hold E5 for its E5 names to mean W, ZI and DELTAX.
- **`rtb`.** TPMODE is `rtb_tpmode`. The MOVE* state-vector copies (INTEGRATION_INITIALIZATION) are `lm_integration_move` and the new `cm_integration_move` (`src/cm_integration_moves.c`, the Comanche addresses), and BOVB TCDANZIG is `lm_integration_tcdanzig` / `cm_integration_tcdanzig`. DOCSM's `RTB GOTO MOVEPCSM` and DOCSM1's `RTB CALL MOVEACSM` run the copy as the pair's left order and go on to its right one in the same routine. RTB FAZA runs the file's own basic code.
- **`resume_keys`, `resume_flow` and `flow_from`.** Incorporation runs as a job, so every word after a DANZIG is a CHANG2 resume point. With `flow_from` (new, `tools/interp_routine_differential.py`), every point but the entries starts from the state the original program reaches there from INCORP1 or INCORP2, with random measurements, W matrices and state vectors, run on through the routines the file calls (`IRC_SNAP_THROUGH` in `tools/interp_routine_reference.c`). The index registers of the Z, OMEGA and DELTAX loops (X2 162, 108, 54 over W +162D,2) and the W-matrix phase indexes WIXA and ZIXA are then the values the program gives them. Random index registers would put VXM* W +162D,2 anywhere in erasable memory. The flowed states stop at FAZA's PHASCHNG, so on their own they are all the loop's first pass (WIXA 54); `flow_range_states` (new) gives the flowed cases in FAZA-FAZB2 the later passes as well (WIXA/WIXB 54 down to 6, ZIXA/ZIXB 0 down to -16, FAZB1's X1 as WIXA), so FAZB2's TIX,1 exit (9-D) and FAZB5's BHIZ exit (6-D) are compared, and gives INCORP1's DDV (DELTAQ/A) a +0 or -0 dividend in a quarter of its cases. Random-state points in FAZA-FAZB2 take the same index ranges (`range_states`).
  Each flow case runs from the same case number of every entry; the entry a hash of the case number picks gives the state, and another entry's run gives it when that one never gets there. So a point that only one entry's cases of one parity reach still gets flowed states (BZE at 23,2621 with MPAC zero). An interpretive entry returns through QPRET (FIXLOC +42D), which its STQ saves. `flow_seed` pins QPRET to the fixed CADR of a word of the entry's bank outside the file, and the reference ends the run when LOC gets there at NEWOPS. A random QPRET would take the run into random code, and an erasable CADR would make GOTO indirect through erasable without end. Flowed cases keep their edits packed (`Edits`): as tuples, all 163,840 took 8 GB.
- **`edge_cases`, `c64_rows: 'cover'`.** Edge values are drawn for the erasables, and the C64 rows are chosen over every case so that they take every branch outcome the host cases take.
- **`c64_flags`.** `-fno-inline -fstatic-stack -mlto-zp=110`, with the 4 KB image at $e000 under the KERNAL (`platform/c64/measurement_inc_diagnostic.c`: interrupts off, `$01=$35`). Without them, the groups the cover needs do not link below $c000.
- **`index_carry16`, `image_w`.** Set at this refreeze, as `docs/INTERP-COMPILE.md` asks. `src/cm_integration_moves.c` takes `W` from `include/agc_image_w.h`.

Destinations outside the file (INTSTALL, GRP2PC, RECTIFY, SVDWN1, SVDWN2, EGRESS's return, POSTJUMP to INTWAKE, PHASCHNG, UPFLAG) stop the run.

## Evidence

Hashes: `evidence/measurement-inc/SHA256.json`. Bulk files are assets of the `evidence-measurement-inc` release.

- **Source map.** `measurement-inc-source-map.json` lists every word with the routine that compiles it.
- **Independent assembly comparison.** `make measurement-inc-differential` runs 512 states per routine (163,840 cases). All equal yaAGC in Z, all 2,048 erasable words and the interrupt-enable state. Every 64th case is frozen in `tests/measurement_inc_vectors.h`.
- **Host and sanitizer results.** `make test-measurement-inc sanitize-measurement-inc` runs the frozen cases (`measurement-inc-host.log`).
- **C64 execution results.** `make vice-measurement-inc` runs 18 diagnostics with 354 cases (`measurement-inc-vice.log`).
- **C64 branch outcomes.** `measurement-inc-c64-cover.json`: 354 C64 rows take all 2,251 branch outcomes the 163,840 host cases take.
