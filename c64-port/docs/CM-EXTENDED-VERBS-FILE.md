# Run the CM extended verbs as native routines

Lane A. Milestone `cm-extended-verbs-file`. Interface: `docs/NATIVE_TRANSFER_ABI.md`. Lane C's hand translations are `docs/LANEC-EV-*.md`.

This milestone covers Comanche055 EXTENDED_VERBS.agc, the whole file, basic and interpretive, in two banks:
- **Bank 43.** The extended-verb front end: GOEXTVB with its fan LST2FAN (verbs 40 to 99), TESTXACT, SETXTACT and the NVSUB return, XACTALM, XACT0, ALM/END and GOPIN. Then the verb bodies: V40 (VBZERO, IMUZEROK, OP/INERT), V41 (VBCOARK, IMUCOARK, OPTCOARK), V42 (IMUFINEK), CKMODCAD, GOLOADLV, V60 to V63, V55 (ALINTIME, UPDATIME), V92 (SYSTEST), V65 (CKOPTVB), V78 (CHAZFOGC), V43 (IMUATTCK, TRMATTCK and the ATTCK1 task), CKLFTBTS, V64, V58 (ENATMA), V68 (STROKON), V46 (STABLISH), V49 (CREWMANU), V48 (DAPDISP), V82, V83, V85, V54 and V57, V86, V56 (TRACKTRM, CLEANOUT), V80 and V81, V74 (DNEDUMP), V75, CHKPOOH, EXDAPOFF, V89, V93, V91 (SHOWSUM, ENDSUMS), V76 and V77, V87 and V88 (SETVHFLG, RESETVHF), V66 (ATTACHED, ATTACHIT, TACHEXIT, TCPIN), V47 (LMTOCMSV, LMTOCM), V94, V90, V96, V52, V67, V44 and V45.
- **Bank 42.** DAPDISP1 and the noun 46, 47 and 48 loads of V48 (DONOUN46, MAXOUT, DONOUN47, DONOUN48, MAXIN).

`tools/interp_compile.py cm-extended-verbs-file` (spec in `tools/interp_specs.py`, `'basic': 'full'`, `resume_keys`) compiles the file into `src/cm_extended_verbs_file.c`, 177 routines:
- `cm_extended_verbs_file_basic` (160): every label and every return point of the basic code.
- `cm_extended_verbs_file_interpretive` (17): the words after each TC INTPRET (in TRACKTRM, SETVHFLG, RESETVHF, ATTACHIT, TACHEXIT and LMTOCM), the CALL returns, two CHANG2 resume keys, two switch-branch targets, and TCPIN (reached through CADR TCPIN).

The run stops where the file transfers out, at Z: the verb bodies in other files that the fan names, and the services the bodies call (BANKCALL, IBNKCALL, POSTJUMP, NVSUB, FINDVAC, NOVAC, WAITLIST, TASKOVER, ENDOFJOB, PHASCHNG, 2PHSCHNG, PRIOCHNG, UPFLAG, DOWNFLAG, CHECKMM, FALTON, FLASHOFF, ENDMARK, ALARM, POODOO, STOPRATE, TPAGREE, GENTRAN, NXTBNK, SELFCHK, STSHOSUM, TRFAILOF, the V70 to V73 update programs, and TCF Q+1), the interpreter (INTPRET 06006, NEWOPS 06037, and the routines it calls), and the Executive's CHANG2 with a job waiting.

## Compiler changes

- **Idle points in basic code.** `tools/basic_compile.py` now honours the spec's `idle` key, which before only the interpretive compiler did. Fan slot 69 is `VERB69 TC VERB69`, a TC to itself that spins until the TC trap restarts the computer. A transfer into an idle point returns `EXEC_IDLE` with Z there, where the oracle stops too. An entry at the idle point runs its word once first.
- **Closure.** `tools/interp_executed_closure.py` counts the second word of a two-word constant (2CADR, 2DEC; `(+1)` in the source map) as data, as it does the first.
- **SUPERBNK in CM files.** CM files with `channels: 'all'` may now write channel 7 (SUPERBNK, bits 5-7), as Luminary files already could.

## Spec

- `idle: ['VERB69']`.
- `index_targets: {'43,2001': 59}`: GOEXTVB's INDEX MPAC, TC LST2FAN reaches all 60 fan words.
- `states`. At VBZERO and VBCOARK, NOUNREG is 20, 91 or another noun (OP/INERT's three outcomes). At STABLISH and DAPDISP, FLAGWRD6 takes values that pass and fail the TVC test, and at STABLISH DAPDATR1 (E6 03066) takes both CSM/LM settings. At 43,2000, MPAC is 0 to 59 (the verb number less 40, the index into the fan). At 43,2112 (NVSUB's return), MPAC +1 is the Q SETXTACT saved: the word after one of the file's 16 TC TESTXACT calls, or ALM/END after XACT0's TC SETXTACT.
- `entries` adds the labels nobody calls (XACTALM, XACT0), the fan targets the `states` above key (VBZERO, VBCOARK, STABLISH, DAPDISP), and the returns the compiler cannot find on its own: the word after each in-file check that passes by TC Q or TCF Q+1 into another file (CHKPOOH, CKMODCAD, and CKLFTBTS's before-liftoff return at TRMATTCK +2), CHECKMM's second return (the word after its inline mode and TC ALM/END), GOXDSPF's proceed and recycle returns (IMUATTCK, DONOUN46, DONOUN47, NOKILL), and the BANKCALL NEEDLE11 and NEEDLER2 returns.
- `index_carry16`, `image_w` (#260), `superbank_bit` (#321), `edge_values`, `seed_counters`, `channels: 'all'`, `io_channels: 'auto'` and `engine: 'flight-device'`.

## Evidence

Hashes: `evidence/cm-extended-verbs-file/SHA256.json`.
- **Source map.** `evidence/cm-extended-verbs-file/cm-extended-verbs-file-source-map.json` lists the file's 703 words with their native routines: 575 instructions and 128 data words (the CADR, 2CADR, ADRES, ECADR, REMADR, VN, MM, OCT and DEC words).
- **Executed-instruction closure.** `make cm-extended-verbs-file-closure` (`tools/oracle_trace` with `AGC_READ_WORDS`, then `tools/interp_executed_closure.py`). The oracle executed 540 of the 575 instructions and the interpreter read the other 35, none named unreached (`evidence/cm-extended-verbs-file/cm-extended-verbs-file-closure.json`).
- **Independent assembly comparison.** `make cm-extended-verbs-file-differential`: the unmodified Comanche055 rope in yaAGC, 512 cases at each of the 177 routines, 90,624 in all, and every one equals the native run (`evidence/cm-extended-verbs-file/cm-extended-verbs-file-proof.json`). It compares Z, all 2,048 erasable words, the I/O channels and the interrupt-enable state.
- **Host and sanitizer results.** `make test-cm-extended-verbs-file sanitize-cm-extended-verbs-file`: the 5,712 frozen cases, 0 failures.
- **C64 execution results.** `make vice-cm-extended-verbs-file`: 288 cases in seven programs that the differential picks by edge coverage. They take every branch outcome the host cases take (1,116 gcov branches, and 952 of 952 trace edges). 288 checks, 0 failures.

Lane C's hand translations (`src/lanec_ev_*.c`) stay for their existing callers. This milestone does not change them.
