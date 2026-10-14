# Run the LM general Lambert aimpoint guidance (P31) as native routines

Lane A. Milestone `lm-lambert-aim`. Interface: `docs/NATIVE_TRANSFER_ABI.md`. Lane E's hand translation is `docs/LM-P31.md`.

This milestone covers Luminary099 GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc (P31), the whole file (35,2450-2544), basic and interpretive:
- **P31 (basic).** P20FLGON, then V06N33 (the ignition time) through VNPOOH, then INTPRET.
- **Targeting (interpretive).** Clears UPDATFLG, integrates the LM to TIG (LEMPREC), and saves RTIG/RINIT and VTIG/VINIT. TPASS4 is TIG + DELLT4. INITVEL runs with RTX1/RTX2 saved, DELVSIN takes DELVEET3, and XDELVFLG is cleared. VGDISP runs through GET.LVC, then PERIAPO1, SHIFTR1 and MAXCHK give HPER and HAPO.
- **Displays.** It EXITs to V06N81 and V06N42 through VNPOOH, then enters INTPRET at REVN1645, which sets FINALFLG, calls VN1645 and loops.

`tools/interp_compile.py lm-lambert-aim` (spec in `tools/interp_specs.py`, `'basic': 'full'`, `resume_keys`) compiles the file into `src/lm_lambert_aim.c`, 32 routines:
- `lm_lambert_aim_basic`: P31, the returns after P20FLGON and each VNPOOH, and the word after EXIT.
- `lm_lambert_aim_interpretive`: the words after each TC INTPRET, REVN1645 (which P30,P37 enter), the returns after STCALL and CALL, and the CHANG2 resume key after every order pair that can end in DANZIG with a job waiting.

The transfers out of the file are P20FLGON (35,2361) and VNPOOH (35,3712) in the same bank, the interpreter (INTPRET, NEWOPS 06070, and the subroutines it calls: LEMPREC, INITVEL, VGDISP, GET.LVC, PERIAPO1, SHIFTR1, MAXCHK, VN1645), and the Executive's CHANG2 (05126) with a job waiting. The run stops there, at Z. The file has no data words: V06N33, V06N81, V06N42 and P30ZERO are assembled elsewhere.

The spec carries `index_carry16` and `image_w` (#260), and `edge_values` (MPAC, the push-down list and the operands as edge vectors in a quarter of the cases). No resume point runs from the middle of a loop, so it needs no `states`.

## Evidence

Hashes: `evidence/lm-lambert-aim/SHA256.json`.
- **Source map.** `evidence/lm-lambert-aim/lm-lambert-aim-source-map.json` lists the file's 61 words with their native routines: 9 basic instructions and 52 interpretive words.
- **Executed-instruction closure.** `make lm-lambert-aim-closure` (`tools/oracle_trace` with `AGC_READ_WORDS`, then `tools/interp_executed_closure.py`). All 9 basic instructions executed, and all 52 interpretive words read by the interpreter, none named unreached (`evidence/lm-lambert-aim/lm-lambert-aim-closure.json`).
- **Independent assembly comparison.** `make lm-lambert-aim-differential`: the unmodified Luminary099 rope in yaAGC, 512 cases at each of the 32 routines, 16,384 in all, and every one equals the native run (`evidence/lm-lambert-aim/lm-lambert-aim-proof.json`). It compares Z, all 2,048 erasable words and the interrupt-enable state.
- **Host and sanitizer results.** `make test-lm-lambert-aim sanitize-lm-lambert-aim`: the 1,028 frozen cases, 0 failures.
- **C64 execution results.** `make vice-lm-lambert-aim`: 55 cases in one program that the differential picks by edge coverage. They take every branch outcome the host cases take (525 gcov branches, and 507 of 507 trace edges). 55 checks, 0 failures.

Lane E's hand translation (`src/lm_p31_routines.c`, `docs/LM-P31.md`) stays for its existing callers. This milestone does not change it.
