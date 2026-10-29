# Translate Comanche FRESH_START_AND_RESTART (bank 04 V37/P00, VAC5STOR) as transfer-ABI routines

Lane A2 (restart, telemetry and support), split from Lane A. Milestone `cm-v37`. Interface: `docs/NATIVE_TRANSFER_ABI.md` (owned by Lane A; this milestone adds routines and an operand table only, no ABI change). It follows `cm-fresh-start` (bank 05) and reuses its device record `cm_fresh_io`.

Bank 04 holds verb 37, which changes the major mode. It checks the IMU and the R00 state, looks the requested mode up in PREMM1, and then waits for the servicer or goes on to CANV37. CANV37 runs R00 (an interpretive CALL INTSTALL) and POOH. The bank also holds SEUDOP00 and the GROUPKIL cleanup through V37KLEAN/P00KLEAN, the rendezvous checks that decide which restart groups survive, NOUVEAU, V37XEQ (the new program's job), GOP00FIX (the P00 entry after a fresh start) and INITSUB/RAKE (stop rate, deadband and the flag-word reset table FLAGTABL). VAC5STOR in bank 20 saves the Executive's core sets, the phase table and the newest job to VAC5 for the abort display. The Luminary version of bank 04 is `lm-v37`. The Comanche body differs: the IMU and DSALMOUT checks in V37, R00TOP00's IBNKCALLs (SPSOFF, MASSPROP, RCSDAPON, TVCZAP) and DELAYJOB, ISITP00, KILL20, and the INITSUB/RAKE subroutine.

## Scope

- `Comanche055/FRESH_START_AND_RESTART.agc`, every instruction of:
  - Bank 04 (2000-2366): GOP00FIX, V37, R00TOP00, ISITP00, CANTR00, V37BAD, CHECKTAB/AGAINMM, ISSERVON, CANV37, ROC, the basic code after R00's EXIT, POOH, SEUDOP00, GROUPKIL, GOMOD, GOGOPROG, RENDV00, RENDN00, P00FIZZ, KILL20, REV37, STATQUO, NOUVEAU, V37NONO, V37XEQ, V37XEQC, INITSUB, SETMAXER and RAKE.
  - Bank 20 (2000-2063): VAC5STOR, V5LOOP1, V5OUT1, V5LOOP2, V5OUT2.
- Not translated here:
  - R00's two interpretive words (CALL INTSTALL; DUMMYAD EXIT, 2131-2133) run in the interpreter. The run ends at INTPRET, and the basic code after the EXIT (2134) is an entry.
  - Bank 13 and R00's interpretive words are `cm-stateup`.
- Entries (`cm_v37_routines`, 36):
  - GOP00FIX (POSTJUMP from FRESH_START), V37 (pinball) and CANV37 (POSTJUMP from SERVICER207's SERVEXIT).
  - ROC and V37XEQ, which INITDSP reaches as TEMPFLSH -3 (R00AD, V37QCAD).
  - The basic code after R00's EXIT (2134).
  - VAC5STOR (BANKCALL from GOPROG).
  - The return point after every TC out of the file: CLEARMRK +2, BANKCALL GOFLASH with its three returns, IBNKCALL SPSOFF/MASSPROP/RCSDAPON +1/TVCZAP, BANKCALL DELAYJOB, ALARM, RELDSP (three), CLEARMRK, DOWNFLAG (four), BANKCALL UPACTOFF, IBNKCALL V37KLEAN/P00KLEAN, FALTON, SPVAC, NEWMODEA, STOPRATE, BANKCALL SETMINDB/SETMAXDB, UPFLAG and GENTRAN.
- Each routine runs until Z reaches one of those external routines, POSTJUMP (PINBRNCH, GOPROG2), ENDOFJOB, INTPRET or SWRETURN.
- Run-time targets:
  - INITSUB saves its return with `QXCH MPAC +1`, and RAKE returns with `TC MPAC +1`, which executes that word as TC. It takes only INITSUB's two return points (2001 and 2146).
  - `INDEX MPAC +1; CA PREMM1` and `INDEX MINDEX` reads of PREMM1, FCADRMM1 and DNLADMM1 take only indices inside the 33-word tables.
  - RAKE's `INDEX MPAC` reads FLAGTABL and masks and stores FLAGWRD0-11. MPAC counts down from ELEVEN within the same run.
  - VAC5STOR's `INDEX ITEMP1/ITEMP2` loads and stores use the indices VAC5STOR itself sets (0, 12, ... 72 and 0, 3, ... 18; 0, 2, ... 10 and 0-5).
- Data: the fixed constants the file reads (with CA/CAF/CS/AD/MASK/MP, DCA) and the PREMM1, FCADRMM1, DNLADMM1 and FLAGTABL tables are compiled from the source listing into `src/cm_v37_operands.inc` (`tools/file_operands_generate.py cm-v37`) and checked against the rope. The CADRs after the file's BANKCALL, IBNKCALL and POSTJUMPs are in `cm_interbank_site_table`, which the program uses as its operand table.
- Devices: io is a `cm_fresh_io`.
  - V37's `RAND DSALMOUT` reads channel 11. CANV37's `WAND` acts on channels 11, 12 and 13, and as in yaAGC, channel 13 bits 12-14 arm the HANDRUPT traps and do not stay in the channel. R00TOP00's `WRITE` acts on channels 5 and 6.
  - INHINT/RELINT act on the interrupt-enable state.
- Runs reach bank 05 through IBNKCALL V37KLEAN/P00KLEAN and POSTJUMP GOPROG2, so they also take `cm_fresh_start_routines`. An IBNKCALL'd KLEAN entry's `TC Q` returns to ISWRETRN with Q = 3; the run leaves ISWRETRN out of the interbank table and ends there (Lane A refused that context until PR #90; it accepts it now, and the run still stops there: what follows is INTER-BANK_COMMUNICATION's return to the caller, outside these files). VAC5STOR's `TC SWRETURN` ends the run at SWRETURN.

Paths with no producer: a TS that overflows refuses (no input in the domain overflows). The CCSs after a one-bit MASK are never negative, and MMNUMBER (CCS at 2157 and 2207) is pinball's mode, never negative; their negative branches refuse. RAKE's counter is never negative.

## Source map

| AGC | Native (`src/cm_v37.c`) |
|---|---|
| GOP00FIX ... RAKE (bank 04) | `bank04`, labels `L<address>`; `jump` for entries and RAKE's `TC MPAC +1` |
| VAC5STOR (bank 20) | `bank20` |
| constants, PREMM1, FCADRMM1, DNLADMM1, FLAGTABL | `CM_V37_K_*`, `cm_v37_premm1`, ... (`src/cm_v37_operands.inc`) |
| BANKCALL/IBNKCALL/POSTJUMP CADRs | `cm_interbank_site_table` |
| channels 5, 6, 11-13, traps | `cm_fresh_io` (`include/cm_fresh_start.h`) |

## Evidence

Hashes: `evidence/cm-v37/SHA256.json`. Bulk files are assets of the `evidence-cm-v37` release (`python3 tools/evidence.py fetch cm-v37`).

- Independent assembly comparison: `evidence/cm-v37/cm-v37-proof.json`. `make cm-v37-differential` runs the unmodified Comanche055 rope in yaAGC (host-only), 4,096 cases.
  - Case n enters entry n % 36, with the image pseudo-random within the program's domain:
    - V37's request (A) and MMNUMBER are a mode PREMM1 lists, P00, P20 or any of 0-0177.
    - MINDEX is an index of the 33-word tables. MODREG is +0, P20 or a mode.
    - The IMU-busy and NODO bits are mostly clear, so V37 gets past CANTR00.
    - RAKE's return word (MPAC +1) is 2001 or 2146.
    - Restart-group phase pairs agree, for the bank 05 code the runs reach.
    - FB is the entry's bank. EB, SUPERBNK, channels 5, 6, 11-14 (without the bits yaAGC acts on as a peripheral), 15, 16 and 33, the trap latches and the interrupt-enable state are pseudo-random.
  - Each case runs to the first point this file's banks 04, 05 and 20 and INTER-BANK_COMMUNICATION do not own, or to SWRETURN or ISWRETRN.
  - Compared: the interrupt-enable state, SUPERBNK, channels 11, 5, 6, 12, 13, 14 and 33, the trap latches and all 2,048 erasable words. The digests are frozen into `tests/cm_v37_vectors.h`.
  - The oracle also traces the words each case executes (`tools/case_trace.h`). The freeze fails if any instruction of bank 04 2000-2366 or VAC5STOR is never executed. The only words left unexecuted are constants, CADRs, R00's interpretive words and the instruction after an EXTEND or INDEX, which runs in the same step as its prefix. The list is in the proof.
- Host and sanitizer results: `make test-cm-v37 sanitize-cm-v37` (`evidence/cm-v37/cm-v37-host.log`). Beyond the oracle digests, the test checks that:
  - all 36 entries are reached;
  - each case ends at one of the 29 points outside the milestone it can reach, and every one of them is reached;
  - VAC5STOR returns through SWRETURN, and a KLEAN entry returns to ISWRETRN with Q = 3;
  - the C64 rows are host cases with their host digests;
  - a negative MMNUMBER, an index outside the tables, a TC MPAC +1 word INITSUB never stores and a missing io are refused.
- C64 execution results: `make vice-cm-v37` (`evidence/cm-v37/cm-v37-vice.log`): 169 C64 rows chosen by native branch outcome (`tools/a2_c64_rows.py`: every 32nd case plus a greedy cover of the gcov branch arcs of every linked source; the freeze fails unless the rows take all 510 arcs the host cases take). The arc cover takes the outcomes a word cover missed: R00TOP00, BZF ISSERVON, CANTR00, V37NONO, BZF KILL20, LIGHTSET's BZF NONAVKEY and negative MP operands. The stride is 32 rather than 16 so that the image fits below $c000.
