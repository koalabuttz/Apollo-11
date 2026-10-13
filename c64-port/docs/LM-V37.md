# Translate Luminary FRESH_START_AND_RESTART (bank 04: verb 37 and POOH) as transfer-ABI routines

Lane A2 (restart, telemetry and support), split from Lane A. Milestone `lm-v37`. Interface: `docs/NATIVE_TRANSFER_ABI.md` (owned by Lane A; this milestone adds routines and an operand table only, no ABI change).

Bank 04 of FRESH_START_AND_RESTART is verb 37 (change major mode) and P00. V37 checks the request against the major-mode table, waits for the servicer when Average-G is on, then either goes to P00 (POOH: integration via R00, clean-up of flags and restart groups, engine off, ALLCOAST, V37KLEAN, GOPROG2) or starts the new program (NOUVEAU, the rendezvous checks, V37XEQ: priority, EBANK and FCADR from the tables, SPVAC). P70 and P71 go straight to their bank. Bank 05 was milestone `lm-fresh-start`. STATEUP (bank 13) and R00's two interpretive pairs are milestone `lm-stateup`.

## Scope

- `Luminary099/FRESH_START_AND_RESTART.agc`, every instruction of bank 04 (2024-2377):
  - GOP00FIX; V37, CANTROD, V37BAD, CHECKTAB/AGAINMM, ISSERVON, V37RET (2.0SPT, 2.11SPT, 2.7SPT), CANV37.
  - R00 (its basic TC INTPRET; the interpretive CALL INTSTALL; EXIT after it is `lm-stateup`'s), POOH, SEUDOP00, GOMOD, GOGOPROG.
  - RENDV00, RESET22, KILL2, P00FIZZ, RENDN00, STATQUO, NOUVEAU, V37NONO.
  - V37XEQ, V37XEQC, SETUP70, SETUP71.
- Entries (`lm_v37_routines`, 32): GOP00FIX, V37, V37BAD, V37RET, R00, V37XEQ, V37XEQC (V37QCAD's restart point), and the return after every TC out of the bank (DOWNFLAG, CLEARMRK, GOFLASH's three returns, ALARM, RELDSP, POSTJUMP, PHSCHNGA, PHASCHNG, INTPRET's EXIT, CLRADMOD, IBNKCALL, FALTON, SPVAC, NEWMODEA).
- Each routine runs until Z reaches the routine it calls; SETUP70/71's DTCB leaves Z and BB at P70 (or P71, three words on).
- Run-time indexing: `NDX MPAC +1; CA PREMM1` and the three `INDEX MINDEX` reads of PREMM1, FCADRMM1 and DNLADMM1 take an index inside the 30-word tables; MINDEX is only ever set by CHECKTAB from that range, so any other index refuses.
- Data: the CADRs after the bank's TC BANKCALL, POSTJUMP and IBNKCALL, its fixed constants and the PREMM1, FCADRMM1 and DNLADMM1 tables are compiled from the source listing into `src/lm_v37_operands.inc` (`tools/file_operands_generate.py lm-v37`, which gains IBNKCALL sites and INDEX-read arrays) and checked against the rope. Like `lm_fresh_operands`, the table covers the whole file, so it holds every word bank 05 reads plus the four IBNKCALL CADRs; a program running banks 04 and 05 uses `lm_v37_operands`. The test checks that and that the seven bank 04 CADRs equal Lane A's `lm_interbank_site_table`.
- Devices: io is a `p66_bias_io` (an `lm_fresh_io` when run with bank 05). `WRITE SUPERBNK` sets SUPERBNK; INHINT acts on the interrupt-enable state.

Paths with no producer: MMNUMBER is pinball's verb 37 mode and never negative, so the negative branches of `CCS MMNUMBER` refuse. The CCS after a MASK of one flag bit is never negative. A TS that overflows refuses.

## Source map

| AGC | Native (`src/lm_v37.c`) |
|---|---|
| GOP00FIX ... SETUP70 (bank 04) | `bank04`, labels `L<address>` |
| BANKCALL/POSTJUMP/IBNKCALL CADRs, constants, PREMM1/FCADRMM1/DNLADMM1 | `lm_v37_operands`, `lm_v37_premm1`, `lm_v37_fcadrmm1`, `lm_v37_dnladmm1` (`src/lm_v37_operands.inc`) |

## Evidence

Hashes: `evidence/lm-v37/SHA256.json`. Bulk files are assets of the `evidence-lm-v37` release (`python3 tools/evidence.py fetch lm-v37`).

- Independent assembly comparison: `evidence/lm-v37/lm-v37-proof.json`. `make lm-v37-differential` runs the unmodified Luminary099 rope in yaAGC (host-only), 4,096 cases.
  - Case n enters entry n % 32 and runs with bank 05 (`lm_fresh_start_routines`) and INTER-BANK_COMMUNICATION, so V37KLEAN, P00KLEAN and GOPROG2 continue into bank 05.
  - The image is pseudo-random within the program's domain: the request (A at V37) and MMNUMBER are a mode from PREMM1, P00, P70, P71 or any of 0-0177, at V37 mostly with IMODES30 bit 6 and NODOFLAG clear; MINDEX 0-29; in a quarter of the cases P20 with only the rendezvous bit of OCT500 in FLAGWRD0, or P25 with only the P25 bit (so RENDN00 reaches STATQUO); MODREG P20, P22, P25, a neighbour, P00, a mode or random, and P20 in a third of the P20 or P25 requests (RENDN00's BZF KILL2, P20 or P25 on top of P20: 04,2264); flag words, restart groups, FB/EB, SUPERBNK, channels and the interrupt-enable state as in `lm-fresh-start`.
  - Each case runs to the first point neither FRESH_START_AND_RESTART (banks 04, 05, fixed-fixed) nor INTER-BANK_COMMUNICATION owns, or to SWRETURN or ISWRETRN. A KLEAN entry under IBNKCALL returns to ISWRETRN with Q = 3, and runs end there (Lane A refused that context until PR #90; it accepts it now, and the run still stops there: what follows is INTER-BANK_COMMUNICATION's return to the caller, outside these files).
  - Compared: the interrupt-enable state, SUPERBNK, channels 11, 5, 6, 12, 13 and 14, the trap latches and all 2,048 erasable words. The digests are frozen into `tests/lm_v37_vectors.h`.
- Host and sanitizer results: `make test-lm-v37 sanitize-lm-v37` (`evidence/lm-v37/lm-v37-host.log`). Beyond the oracle digests, the test checks that:
  - all 32 entries are reached;
  - each case ends at one of 21 exits (DOWNFLAG, GOFLASH, RELDSP, ENGINOF1, MMDSPLAY, ENDOFJOB, CLRADMOD, PHASCHNG, ISWRETRN, INTPRET, CLEARMRK +2, PHSCHNGA, NEWMODEA, SPVAC, ALLCOAST, PINBRNCH, DUMMYJOB +2, P71, P70, FALTON, ALARM), and every one is reached;
  - the seven bank 04 CADRs equal the interbank site table, and every `lm_fresh_operands` word is in `lm_v37_operands`;
  - the C64 rows are host cases with their host digests;
  - a MINDEX past the tables or negative, a negative MMNUMBER, an overflowed request and a missing io are refused.
- C64 execution results: `make vice-lm-v37` (`evidence/lm-v37/lm-v37-vice.log`): 136 C64 rows chosen by native branch outcome (`tools/a2_c64_rows.py`: every 48th case plus a greedy cover of the gcov branch arcs of every linked source; the freeze fails unless the rows take all 489 arcs the host cases take). They include the SETUP70/71 and P70/71 paths, POOH, GOMOD, RESET22/RENDN00, KILL2/STATQUO, V37NONO/FALTON and the DOFSTART/MR.KLEAN/STARTSUB path in bank 05 that a stride missed. Every 48th rather than 16th or 32nd: with either the BSS runs past $c000 (with every 32nd once PR #90 grew lm_interbank). Built at -Oz: at -Os, banks 04 and 05 run past $c000, where the mailbox is.
