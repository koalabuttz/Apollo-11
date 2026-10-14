# Run the LM ground-track program (P21) as native routines

Lane A. Milestone `lm-ground-track`. Interface: `docs/NATIVE_TRANSFER_ABI.md`. Lane E's hand translation is `docs/LM-P21.md`.

This milestone covers Luminary099 GROUND_TRACKING_DETERMINATION_PROGRAM.agc (P21), the whole file (24,3505-3667), basic and interpretive:
- **PROG21 and P21PROG1 (basic).** PROG21 sets OPTION2 to 1 and requests GOPERF4 (option code, BIT2). P21PROG1 flashes V06N34 for the time. Each returns three ways from BANKCALL: terminate (GOTOPOOH), proceed, or a value loaded through the DSKY (TC -5, back to the request). The second proceed enters INTPRET.
- **Integration (interpretive).** DSPTEM1 into TDEC1 and CALL INTSTALL. BON P21FLAG goes to P21CONT; otherwise VINTFLAG by OPTION2 (SR1, BHIZ), DIM0FLAG and INTYPFLG cleared, INTEGRV, then GOTO P21VSAVE.
- **P21CONT.** Loads RCV, VCV and TET from the base vector, MOONFLAG by P21ORIG (BZE), and calls INTEGRVS.
- **P21VSAVE.** Stores the base vector (TAT, RATT1, VATT1), P21VEL (ABVAL SL* 0,2), and U(R).U(V) through UNIT, DOT and DDV to ASIN, then P21GAM, P21ORIG (SXA,2), and P21FLAG by OPTION2 and SURFFLAG.
- **P21DSP.** LUNAFLAG by X2, ALPHAV and TAT, LAT-LONG, ALT/100 as P21ALT, then EXIT to the V06N43 flash. Its recycle (V32E) adds 600SEC to P21TIME into DSPTEM1 and returns to P21PROG1 by RTB.

`tools/interp_compile.py lm-ground-track` (spec in `tools/interp_specs.py`, `'basic': 'full'`, `resume_keys`) compiles the file into `src/lm_ground_track.c`, 53 routines:
- `lm_ground_track_basic` (12): PROG21, P21PROG1, the three returns after each BANKCALL (24,3512-3514, 3520-3522, 3651-3653), and the word after EXIT (3646). The two loaded-value returns (`TC -5`, 3514 and 3522) are named entries, `PROG21+7` and `P21PROG1+5`.
- `lm_ground_track_interpretive` (41): the words after each TC INTPRET, the returns after STCALL and CALL, the branch targets (BHIZ, BZE, GOTO, BON, RTB), and the CHANG2 resume key after every order pair that can end in DANZIG with a job waiting.

The transfers out of the file are BANKCALL (GOPERF4, GOFLASH), GOTOPOOH, the interpreter (INTPRET, NEWOPS, and the subroutines it calls: INTSTALL, INTEGRV, INTEGRVS, ASIN, LAT-LONG), the Executive's CHANG2 (05126) with a job waiting, and ALARM1 (05732), which the interpreter reaches on an overflow or an out-of-range DDV. The run stops there, at Z.

No resume point here runs from the middle of a loop, so the spec needs no `states`. It carries `index_carry16` and `image_w` (#260), and `edge_values` (MPAC, the push-down list and the operands as edge vectors in a quarter of the cases). P21VSAVE's `DDV P21VEL` (24,3606) also starts from edge operands in half its cases (`ddv_joint`, as SMCDURES in `cm-powered-subs`, reviewer 1 #314): MPAC 0, -0 or small, and P21VEL 0, -0, small, major part +-1 with a minor of either sign, 1/2, or a major part equal to MPAC's.

`tools/interp_executed_closure.py` now counts a source-map constant word (the second word of a 2DEC, shown as `(+1)`) as data, like the first.

## Evidence

Hashes: `evidence/lm-ground-track/SHA256.json`.
- **Source map.** `evidence/lm-ground-track/lm-ground-track-source-map.json` lists the file's 115 words with their native routines: 106 instructions (18 basic, 88 interpretive) and 9 data words (the CADRs after BANKCALL, 600SEC, V06N43, V6N34 and K.01).
- **Executed-instruction closure.** `make lm-ground-track-closure` (`tools/oracle_trace` with `AGC_READ_WORDS`, then `tools/interp_executed_closure.py`). All 18 basic instructions executed, and all 88 interpretive words read by the interpreter, none named unreached (`evidence/lm-ground-track/lm-ground-track-closure.json`).
- **Independent assembly comparison.** `make lm-ground-track-differential`: the unmodified Luminary099 rope in yaAGC, 512 cases at each of the 53 routines, 27,136 in all, and every one equals the native run (`evidence/lm-ground-track/lm-ground-track-proof.json`). It compares Z, all 2,048 erasable words and the interrupt-enable state.
- **Host and sanitizer results.** `make test-lm-ground-track sanitize-lm-ground-track`: the 1,708 frozen cases, 0 failures.
- **C64 execution results.** `make vice-lm-ground-track`: 102 cases in 22 programs that the differential picks by edge coverage. They take every branch outcome the host cases take (853 gcov branches, and 817 of 817 trace edges). 102 checks, 0 failures.

Lane E's hand translation (`src/lm_p21_routines.c`, `docs/LM-P21.md`) stays for its existing callers. This milestone does not change it.
