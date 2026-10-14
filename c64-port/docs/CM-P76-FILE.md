# Run the CM target delta-V program (P76) as native routines

Lane A. Milestone `cm-p76-file`. Interface: `docs/NATIVE_TRANSFER_ABI.md`. The Luminary099 form is `docs/LM-P76-FILE.md`. Lane E's hand translation is `docs/CM-P76.md`.

This milestone covers Comanche055 P76.agc, the whole file (13,2071-2230), basic and interpretive:
- **P76 (basic).** Sets TRACKFLG (UPFLAG), then flashes V06N84 (the delta-V) and V06N33 (the time). Unlike Luminary, it does not copy DELVLVC to DELVOV first; COMPMAT reads DELVOV directly. Each display returns three ways from BANKCALL GOFLASH: terminate (TCF ENDP76), proceed, or a value loaded through the DSKY (TC -5, back to the display). The second proceed enters INTPRET.
- **COMPMAT.** Integrates the other vehicle to TIG (OTHPREC). It then builds the local-vertical frame from RATT and VATT (UNIT, VCOMP, VXV) and adds the delta-V to VATT (VXM VSL1, VAD). INTSTALL and P76SUB1 follow.
- **P76SUB1.** CLEAR MOONFLAG, SLOAD X2, BHIZ, SET MOONFLAG, RVQ: MOONFLAG follows X2.
- **INTOTHIS.** Integrates to TETTHIS (INTEGRVS) and rectifies (MINIRECT). It sets phase 4.24 (PHASCHNG) and REINTFLG, then calls ATOPOTH. SSP QPRET OUT, then BANKCALL INTWAKE1, which wakes the integration's sleepers and returns through GOBAC's RVQ to OUT, an interpretive EXIT.
- **ENDP76.** Clears MARKCTR and VHFCNT, sets MRKBUF2 to -1 (NEGONE, the mark buffer is invalid) and goes to GOTOPOOH.

`tools/interp_compile.py cm-p76-file` (spec in `tools/interp_specs.py`, `'basic': 'full'`, `resume_keys`) compiles the file into `src/cm_p76_file.c`, 48 routines:
- `cm_p76_file_basic` (13): P76, the returns after UPFLAG, PHASCHNG and each BANKCALL, the GOFLASH proceed and loaded-value returns after `TCF ENDP76` (named entries `P76+6`, `P76+7`, `P76+12`, `P76+13`), the words after each EXIT, and ENDP76.
- `cm_p76_file_interpretive` (35): the words after each TC INTPRET, COMPMAT, OUT, P76SUB1, the returns after STCALL and CALL, and the CHANG2 resume key after every order pair that can end in DANZIG with a job waiting.

As in the LM form, `BANKCALL INTWAKE1` never returns to the word after its CADR (`'noreturn': ['INTWAKE1']`).

The transfers out of the file are UPFLAG (05546), PHASCHNG (05412), BANKCALL (04662: GOFLASH, INTWAKE1), GOTOPOOH (04106), the interpreter (INTPRET 06006, NEWOPS 06037, and the subroutines it calls: OTHPREC, INTSTALL, INTEGRVS, MINIRECT, ATOPOTH), and the Executive's CHANG2 (05170) with a job waiting. The run stops there, at Z.

No resume point here runs from the middle of a loop, so the spec needs no `states`. As a Comanche spec it runs on the flight-device engine with every channel (`channels`, `io_channels`). It carries `index_carry16` and `image_w` (#260), and `edge_values` (MPAC, the push-down list and the operands as edge vectors in a quarter of the cases).

## Evidence

Hashes: `evidence/cm-p76-file/SHA256.json`.
- **Source map.** `evidence/cm-p76-file/cm-p76-file-source-map.json` lists the file's 96 words with their native routines: 88 instructions (22 basic, 66 interpretive) and 8 data words (ADRES TRACKFLG and REINTFLG, the CADRs after BANKCALL, OCT 04024 after PHASCHNG, and V06N84's two NV words).
- **Executed-instruction closure.** `make cm-p76-file-closure` (`tools/oracle_trace` with `AGC_READ_WORDS`, then `tools/interp_executed_closure.py`). All 22 basic instructions executed, and all 66 interpretive words read by the interpreter, none named unreached (`evidence/cm-p76-file/cm-p76-file-closure.json`).
- **Independent assembly comparison.** `make cm-p76-file-differential`: the unmodified Comanche055 rope in yaAGC, 512 cases at each of the 48 routines, 24,576 in all, and every one equals the native run (`evidence/cm-p76-file/cm-p76-file-proof.json`). It compares Z, all 2,048 erasable words and the interrupt-enable state.
- **Host and sanitizer results.** `make test-cm-p76-file sanitize-cm-p76-file`: the 1,543 frozen cases, 0 failures.
- **C64 execution results.** `make vice-cm-p76-file`: 76 cases in two programs that the differential picks by edge coverage. They take every branch outcome the host cases take (637 gcov branches, and 670 of 670 trace edges). 76 checks, 0 failures.

Lane E's hand translation (`src/cm_p76_routines.c`, `docs/CM-P76.md`) stays for its existing callers. This milestone does not change it.
