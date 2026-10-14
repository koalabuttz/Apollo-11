# Run the LM target delta-V program (P76) as native routines

Lane A. Milestone `lm-p76-file`. Interface: `docs/NATIVE_TRANSFER_ABI.md`. Lane E's hand translation is `docs/LM-P76.md`.

This milestone covers Luminary099 P76.agc, the whole file (13,2207-2350), basic and interpretive:
- **P76 (basic).** Sets TRACKFLG (UPFLAG), copies DELVLVC to DELVOV, then flashes V06N84 (the delta-V) and V06N33 (the time). Each display returns three ways from BANKCALL GOFLASH: terminate (TCF ENDP76), proceed, or a value loaded through the DSKY (TC -5, back to the display). The second proceed enters INTPRET.
- **COMPMAT.** Integrates the other vehicle to TIG (OTHPREC). It then builds the local-vertical frame from RATT and VATT (UNIT, VCOMP, VXV) and adds the delta-V to VATT (VXM VSL1, VAD). INTSTALL and P76SUB1 follow (MOONFLAG and X2 by CMOONFLG; its BON returns through QPRET).
- **INTOTHIS.** Loads RCV, VCV and TET, then integrates to TETTHIS (INTEGRVS) and rectifies (MINIRECT). It sets phase 4.24 (PHASCHNG) and REINTFLG, then calls ATOPOTH. SSP QPRET OUT, then BANKCALL INTWAKE1, which wakes the integration's sleepers and returns through GOBAC's RVQ to OUT, an interpretive EXIT.
- **ENDP76.** Clears MARKCTR and goes to GOTOPOOH.

`tools/interp_compile.py lm-p76-file` (spec in `tools/interp_specs.py`, `'basic': 'full'`, `resume_keys`) compiles the file into `src/lm_p76_file.c`, 51 routines:
- `lm_p76_file_basic` (14): P76, the returns after UPFLAG, PHASCHNG and each BANKCALL, the GOFLASH proceed and loaded-value returns after `TCF ENDP76` (13,2222, 2223, 2230, 2231; named entries `P76+11`, `P76+12`, `P76+17`, `P76+18`), the words after each EXIT, and ENDP76.
- `lm_p76_file_interpretive` (37): the words after each TC INTPRET, COMPMAT, OUT, P76SUB1, the returns after STCALL and CALL, and the CHANG2 resume key after every order pair that can end in DANZIG with a job waiting.

`BANKCALL INTWAKE1` never returns to the word after its CADR. The spec says so (`'noreturn': ['INTWAKE1']`), and `tools/basic_compile.py` now matches a `noreturn` name against a BANKCALL's CADR as well as the TC's target. Without that, OUT would be both a basic return and an interpretive point. No other spec's output changes (`tools/interp_compile.py --check`).

The transfers out of the file are UPFLAG (05504), PHASCHNG (05353), BANKCALL (04616: GOFLASH, INTWAKE1), GOTOPOOH (06001), the interpreter (INTPRET, NEWOPS 06070, and the subroutines it calls: OTHPREC, INTSTALL, INTEGRVS, MINIRECT, ATOPOTH), and the Executive's CHANG2 (05126) with a job waiting. The run stops there, at Z.

No resume point here runs from the middle of a loop, so the spec needs no `states`. It carries `index_carry16` and `image_w` (#260), and `edge_values` (MPAC, the push-down list and the operands as edge vectors in a quarter of the cases).

## Evidence

Hashes: `evidence/lm-p76-file/SHA256.json`.
- **Source map.** `evidence/lm-p76-file/lm-p76-file-source-map.json` lists the file's 98 words with their native routines: 90 instructions (20 basic, 70 interpretive) and 8 data words (ADRES TRACKFLG and REINTFLG, the CADRs after BANKCALL, OCT 04024 after PHASCHNG, and V06N84's two NV words).
- **Executed-instruction closure.** `make lm-p76-file-closure` (`tools/oracle_trace` with `AGC_READ_WORDS`, then `tools/interp_executed_closure.py`). All 20 basic instructions executed, and all 70 interpretive words read by the interpreter, none named unreached (`evidence/lm-p76-file/lm-p76-file-closure.json`).
- **Independent assembly comparison.** `make lm-p76-file-differential`: the unmodified Luminary099 rope in yaAGC, 512 cases at each of the 51 routines, 26,112 in all, and every one equals the native run (`evidence/lm-p76-file/lm-p76-file-proof.json`). It compares Z, all 2,048 erasable words and the interrupt-enable state.
- **Host and sanitizer results.** `make test-lm-p76-file sanitize-lm-p76-file`: the 1,638 frozen cases, 0 failures.
- **C64 execution results.** `make vice-lm-p76-file`: 77 cases in two programs that the differential picks by edge coverage. They take every branch outcome the host cases take (727 gcov branches, and 717 of 717 trace edges). 77 checks, 0 failures.

Lane E's hand translation (`src/lm_p76_routines.c`, `docs/LM-P76.md`) stays for its existing callers. This milestone does not change it.
