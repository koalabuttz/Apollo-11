# Run the LM external delta-V program (P30) as native routines

Lane A. Milestone `lm-p30-file`. Interface: `docs/NATIVE_TRANSFER_ABI.md`. Lane E's hand translation is `docs/LM-P30.md`.

This milestone covers Luminary099 P30_P37.agc, the whole file, basic and interpretive, in two banks:
- **P30 (35,2000-2026, basic).** Sets UPDATFLG and TRACKFLG (UPFLAG). Then it shows V06N33 (the ignition time) and V06N81 (the delta-V) through VNPOOH, which returns here on proceed, redisplays on recycle and goes to POOH on terminate. It clears UPDATFLG (DOWNFLAG) and enters INTPRET: CALL S30.1, SET UPDATFLG, EXIT. PARAM30 shows V06N42 (apogee, perigee, delta-V) through VNPOOH. INTPRET again: SETGO XDELVFLG to REVN1645.
- **S30.1 (34,2000-2056, interpretive).** Integrates the LM to TIG (LEMPREC). It keeps RTIG and VTIG and builds the local-vertical frame (UNIT, VCOMP, VXV). It turns DELVSLV into the inertial DELVSIN (VXM VSL1) and its magnitude DELVSAB. Then it calls PERIAPO1, SHIFTR1 and MAXCHK twice, for HPER and HAPO, and returns through QTEMP.

`tools/interp_compile.py lm-p30-file` (spec in `tools/interp_specs.py`, `'basic': 'full'`, `resume_keys`) compiles the file into `src/lm_p30_file.c`, 35 routines:
- `lm_p30_file_basic` (8): P30, the returns after UPFLAG and DOWNFLAG, the returns after each VNPOOH, and PARAM30 after the EXIT.
- `lm_p30_file_interpretive` (27): the words after each TC INTPRET, S30.1, the returns after STCALL and CALL, and the CHANG2 resume key after every order pair that can end in DANZIG with a job waiting.

The transfers out of the file are UPFLAG (05504), DOWNFLAG (05516), VNPOOH (35,3712), the interpreter (INTPRET 06037, NEWOPS 06070, and the subroutines it calls: LEMPREC, PERIAPO1, SHIFTR1, MAXCHK and REVN1645), and the Executive's CHANG2 (05126) with a job waiting. The run stops there, at Z.

No resume point here runs from the middle of a loop, so the spec needs no `states`. It carries `index_carry16`, `image_w` (#260), `superbank_bit` (#321), and `edge_values` (MPAC, the push-down list and the operands as edge vectors in a quarter of the cases).

## Evidence

Hashes: `evidence/lm-p30-file/SHA256.json`.
- **Source map.** `evidence/lm-p30-file/lm-p30-file-source-map.json` lists the file's 70 words with their native routines: 65 instructions (11 basic, 54 interpretive) and 5 data words (the three ADRES words after UPFLAG and DOWNFLAG, and V06N33 and V06N42).
- **Executed-instruction closure.** `make lm-p30-file-closure` (`tools/oracle_trace` with `AGC_READ_WORDS`, then `tools/interp_executed_closure.py`). All 11 basic instructions executed, and all 54 interpretive words read by the interpreter, none named unreached (`evidence/lm-p30-file/lm-p30-file-closure.json`).
- **Independent assembly comparison.** `make lm-p30-file-differential`: the unmodified Luminary099 rope in yaAGC, 512 cases at each of the 35 routines, 17,920 in all, and every one equals the native run (`evidence/lm-p30-file/lm-p30-file-proof.json`). It compares Z, all 2,048 erasable words and the interrupt-enable state.
- **Host and sanitizer results.** `make test-lm-p30-file sanitize-lm-p30-file`: the 1,127 frozen cases, 0 failures.
- **C64 execution results.** `make vice-lm-p30-file`: 57 cases in two programs that the differential picks by edge coverage. They take every branch outcome the host cases take (660 gcov branches, and 644 of 644 trace edges). 57 checks, 0 failures.

Lane E's hand translation (`src/lm_p30_routines.c`, `docs/LM-P30.md`) stays for its existing callers. This milestone does not change it.
