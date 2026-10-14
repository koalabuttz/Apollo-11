# Run the LM abort programs (P70-P71) as native routines

Lane A. Milestone `lm-p70-p71-file`. Interface: `docs/NATIVE_TRANSFER_ABI.md`. Lane F's earlier partial milestone is `docs/LM-P70-P71.md` (`src/lm_p70_p71.c`).

This milestone covers Luminary099 P70-P71.agc, the whole file, basic and interpretive, including the eight channel-instruction words the partial milestone left out:
- **R10,R11 (21,2006-2127, basic).** The abort-switch task flashes the landing displays (TWIDDLE, FLIP). With LETABORT set (FLAGWRD9 bit 9) it starts P71 when the abort-stage switch is in (READ CHAN30, bit 4), and it starts P70 when the abort switch is in (bit 1) unless that program already runs. Otherwise it goes on to R10 (LANDISP).
- **P70, P71 and LEGAL? (21, basic).** P70A and P71A set Q (-0 or 2), go INHINT, and jump to CONTABRT in bank 05 through DTCB.
- **CONTABRT and ABRTJASK (05,3402-3501, basic).** CONTABRT puts TCF ABRTJASK in BRUPT and runs RESUME. ABRTJASK sets the phase, MODREG (MODE70 or MODE71 by Q), DISPDEX, APSFLAG for P71, clears the DAP bits, and sets ENGONFLG. It then turns the engine on if armed (RAND/WRITE DSALMOUT), terminates R12, and goes through the ENEMA to SERVEXIT.
- **GOABORT to THROTUP (32,3540-3767, basic and interpretive).** GOABORT, P70INIT, INJTARG, BOTHPOLY (the DPS or APS coefficients), UPRATE, YOK, UPTHROT, P71RET, OLDTIME, TGOCOMP, and THROTUP. THROTUP drives the throttle (WOR CHAN14).

`tools/interp_compile.py lm-p70-p71-file` (spec in `tools/interp_specs.py`, `'basic': 'full'`, `resume_keys`) compiles the file into `src/lm_p70_p71_file.c`, 78 routines:
- `lm_p70_p71_file_basic` (32): the entries, and the returns from TWIDDLE, FLIP, CLRADMOD, FALTON, RELDSP, CHECKMM, DOWNFLAG, UPFLAG, PHASCHNG and BANKCALL.
- `lm_p70_p71_file_interpretive` (46): the words after each TC INTPRET, the CALL and STCALL returns, the BPL, BMN and switch targets, and the CHANG2 resume key after every order pair that can end in DANZIG with a job waiting.

The channel orders run through the program's `agc_hardware` (`include/agc_channel_ops.h`), as in lm-kalman: READ CHAN30 reads the latched input channel, RAND/WRITE DSALMOUT (channel 11) and WOR CHAN14 write the output channels. The cases set channels 11, 14 and 30 on both sides, and each digest covers them.

## Compiler change

`tools/basic_compile.py` now compiles `RESUME` outside an interrupt, as `tools/interp_compile.py` already did for Lane F's milestone. The next instruction is BRUPT's, run in place (yaAGC's substitute instruction): a TCF to fixed memory, which CONTABRT stores. Any other BRUPT word is refused. Every other spec regenerates byte-identical (`tools/interp_compile.py --check`).

## Spec states

- **ZRUPT holds ABRTJASK in every case (`all_states`).** yaAGC's RESUME sets Z to ZRUPT before it runs the substitute TCF, and the differential's oracle stops as soon as Z leaves the file. With ZRUPT at ABRTJASK the oracle runs the TCF, as the flight program does. ZRUPT has no other effect here: the TCF replaces Z.
- **Q at CONTABRT and ABRTJASK is -0 (P70) or 2 (P71),** the two values P70A and P71A set. INDEX Q; CAF MODE70 reads MODE70 or MODE71 (`fixed_tables` MODE70 to MODE70 +2), and any other fixed word is refused.
- **BOTHPOLY's X1 is -0 or -8 (`vac+046`),** for the DPS or APS coefficients, as in Lane F's spec.
- **At 10,11 (21,2042, the return from FLIP),** FLAGWRD9 has LETABORT set in most cases, and MODREG is 70, 71 or another program, so both P71NOW? and P70NOW? run.
- **`io_channels` names channels 11, 14 and 30.** `'auto'` leaves out channel 30, which READ CHAN30 takes as an input, and then the case's channel edits zero it, so P70NOW? never ran.

## Evidence

Hashes: `evidence/lm-p70-p71-file/SHA256.json`.
- **Source map.** `evidence/lm-p70-p71-file/lm-p70-p71-file-source-map.json` lists the file's 298 words with their native routines, none untranslated.
- **Executed-instruction closure.** `make lm-p70-p71-file-closure`: the oracle executed all 169 basic instructions and the interpreter read all 91 interpretive words, none named unreached (`evidence/lm-p70-p71-file/lm-p70-p71-file-closure.json`). The 38 data words are the ADRES, CADR, 2CADR, REMADR, DEC, 2DEC and OCT constants, plus ABRTJADR (TCF ABRTJASK), which CONTABRT reads with CAF and executes only as BRUPT's substitute instruction.
- **Independent assembly comparison.** `make lm-p70-p71-file-differential`: the unmodified Luminary099 rope in yaAGC, 512 cases at each of the 78 routines, 39,936 in all. Every case equals the native run (`evidence/lm-p70-p71-file/lm-p70-p71-file-proof.json`) in Z, all 2,048 erasable words, the interrupt-enable state, and channels 11, 14 and 30. No case waits for a channel.
- **Host and sanitizer results.** `make test-lm-p70-p71-file sanitize-lm-p70-p71-file`: 2,506 frozen cases, 0 failures.
- **C64 execution results.** `make vice-lm-p70-p71-file`: 127 cases in 3 programs, picked by edge coverage. 127 checks, 0 failures.
- **C64 branch outcomes.** The proof's `c64_gcov_branches`: the C64 cases take all 1,004 gcov branch outcomes the host cases take, and all 896 trace edges.

Lane F's partial milestone (`src/lm_p70_p71.c`, `docs/LM-P70-P71.md`) stays for its existing callers. This milestone does not change it.
