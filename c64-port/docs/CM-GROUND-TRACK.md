# Run the CM ground-track program (P21) as native routines

Lane A. Milestone `cm-ground-track`. Interface: `docs/NATIVE_TRANSFER_ABI.md`. The Luminary099 form is `docs/LM-GROUND-TRACK.md`; Lane E's hand translation is `docs/CM-P21.md`.

This milestone covers Comanche055 GROUND_TRACKING_DETERMINATION_PROGRAM.agc (P21), the whole file (37,2001-2154), basic and interpretive:
- **PROG21 and P21PROG1 (basic).** PROG21 sets OPTION2 to 1 and requests GOPERF4 (option code, BIT2). P21PROG1 flashes V06N34 for the time. Each returns three ways from BANKCALL: terminate (GOTOPOOH), proceed, or a value loaded through the DSKY (TC -5, back to the request). The second proceed enters INTPRET.
- **Integration (interpretive).** DSPTEM1 into TDEC1 and CALL INTSTALL. BON P21FLAG goes to P21CONT; otherwise VINTFLAG is set, and cleared again unless OPTION2 SR1 is zero (BHIZ skips the CLEAR). DIM0FLAG and INTYPFLG are cleared, INTEGRV runs, then GOTO P21VSAVE.
- **P21CONT.** Loads RCV, VCV and TET from the base vector, MOONFLAG by P21ORIG (BZE), and calls INTEGRVS.
- **P21VSAVE.** Stores the base vector (TAT, RATT1, VATT1), P21VEL (ABVAL SL* 0,2), and U(R).U(V) through UNIT, DOT and DDV to ASIN, then P21GAM, P21ORIG (SXA,2), and sets P21FLAG. (Comanche has no SURFFLAG test.)
- **P21DSP.** LUNAFLAG by X2, ALPHAV and TAT, LAT-LONG, ALT/100 as P21ALT, then EXIT to the V06N43 flash. Its recycle (V32E) adds 600SEC to P21TIME into DSPTEM1 and returns to P21PROG1 by RTB.

`tools/interp_compile.py cm-ground-track` (spec in `tools/interp_specs.py`, `'basic': 'full'`, `resume_keys`) compiles the file into `src/cm_ground_track.c`, 49 routines:
- `cm_ground_track_basic` (12): PROG21, P21PROG1, the three returns after each BANKCALL (37,2006-2010, 2014-2016, 2134-2136), and the word after EXIT (2131). The two loaded-value returns (`TC -5`, 2010 and 2016) are named entries, `PROG21+7` and `P21PROG1+5`.
- `cm_ground_track_interpretive` (37): the words after each TC INTPRET, the returns after STCALL and CALL, the branch targets (BHIZ, BZE, GOTO, BON, RTB), and the CHANG2 resume key after every order pair that can end in DANZIG with a job waiting.

The transfers out of the file are BANKCALL (04662: GOPERF4, GOFLASH), GOTOPOOH (04106), the interpreter (INTPRET, NEWOPS 06037, and the subroutines it calls: INTSTALL, INTEGRV, INTEGRVS, ASIN, LAT-LONG), the Executive's CHANG2 (05170) with a job waiting, and ALARM (05650), which the interpreter reaches on an overflow or an out-of-range DDV. The run stops there, at Z.

No resume point here runs from the middle of a loop, so the spec needs no `states`. It carries `index_carry16` and `image_w` (#260), and `edge_values` (MPAC, the push-down list and the operands as edge vectors in a quarter of the cases). P21VSAVE's `DDV P21VEL` (37,2102) also starts from edge operands in half its cases (`ddv_joint`, as SMCDURES in `cm-powered-subs`, reviewer 1 #314): MPAC 0, -0 or small, and P21VEL 0, -0, small, major part +-1 with a minor of either sign, 1/2, or a major part equal to MPAC's.

## Evidence

Hashes: `evidence/cm-ground-track/SHA256.json`.
- **Source map.** `evidence/cm-ground-track/cm-ground-track-source-map.json` lists the file's 108 words with their native routines: 97 instructions (18 basic, 79 interpretive) and 11 data words (the CADRs after BANKCALL, 600SEC, P21ONENN (OCT 1, OCT 0, which TPI_SEARCH reads), V06N43, V6N34 and K.01).
- **Executed-instruction closure.** `make cm-ground-track-closure` (`tools/oracle_trace` with `AGC_READ_WORDS`, then `tools/interp_executed_closure.py`). All 18 basic instructions executed, and all 79 interpretive words read by the interpreter, none named unreached (`evidence/cm-ground-track/cm-ground-track-closure.json`).
- **Independent assembly comparison.** `make cm-ground-track-differential`: the unmodified Comanche055 rope in yaAGC, 512 cases at each of the 49 routines, 25,088 in all, and every one equals the native run (`evidence/cm-ground-track/cm-ground-track-proof.json`). It compares Z, all 2,048 erasable words and the interrupt-enable state.
- **Host and sanitizer results.** `make test-cm-ground-track sanitize-cm-ground-track`: the 1,582 frozen cases, 0 failures.
- **C64 execution results.** `make vice-cm-ground-track`: 95 cases in 10 programs that the differential picks by edge coverage. They take every branch outcome the host cases take (760 gcov branches, and 762 of 762 trace edges). 95 checks, 0 failures.

Lane E's hand translation (`src/cm_p21_routines.c`, `docs/CM-P21.md`) stays for its existing callers. This milestone does not change it.
