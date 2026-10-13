# Translate Comanche FRESH_START_AND_RESTART (bank 05) as transfer-ABI routines

Lane A2 (restart, telemetry and support), split from Lane A. Milestone `cm-fresh-start`. Interface: `docs/NATIVE_TRANSFER_ABI.md` (owned by Lane A; this milestone adds routines, an operand table and its own device record only, no ABI change).

FRESH_START_AND_RESTART is what the CM AGC runs after a V36 fresh start, a hardware restart (GOJAM) or a software restart. Bank 05 holds the fresh-start initialization, the restart entry GOPROG (which first saves the abort state through VAC5STOR) with its check of the restart-group phase tables, the restart of each active group, ENEMA, the KLEAN routines that clear the phase tables, the STARTSUB family that resets the Executive, Waitlist and display state, and T5IDLOC, the idle T5RUPT. GOTOPOOH sits in fixed-fixed memory. The V37/POOH code in bank 04, VAC5STOR in bank 20 and the bank 13 interpretive code are later milestones. The Luminary version is `lm-fresh-start`; the Comanche body differs (optics and coarse-alignment restarts, T5LOC selection by FLAGWRD6, the TVC Waitlist restart, the restart light test in CHAN33).

## Scope

- `Comanche055/FRESH_START_AND_RESTART.agc`, every instruction of bank 05 (2342-3217) and fixed-fixed (4106-4111):
  - SLAP1, SKIPSIM, DOFSTART, ENDRSTRT.
  - MR.KLEAN, P00KLEAN, V37KLEAN.
  - GOPROG, BUTTONS, the ERESTORE repair, ELRSKIP, NOCOARSE.
  - ENEMA (GOPROG2), GOPROG3.
  - PCLOOP, NXTRST, PACTIVE, PINACT, PTBAD.
  - LIGHTSET, NONAVKEY, STARTSUB, STARTSB2, DSPOFF.
  - T5IDLOC, GOTOPOOH.
- Not translated: STARTSIM (2345-2350). It runs only if the STARTSW word is patched to `TCF STARTSIM`; in the rope it is `TCF SKIPSIM`.
- Entries (`cm_fresh_start_routines`, 17):
  - SLAP1 (V36), GOPROG (hardware restart), ENEMA (also GOPROG2, V96), GOPROG3.
  - MR.KLEAN, P00KLEAN, V37KLEAN (BANKCALL targets), T5IDLOC (T5RUPT), GOTOPOOH.
  - The returns after TC BANKCALL (VAC5STOR), TC IBNKCALL (SETCOARS), TC WAITLIST (CA+ECE; the TVCEXEC return is GOPROG3), TC MMDSPLAY, TC STOPRATE, TC SWCALL (RESTARTS), TC ALARM (PTBAD) and TC PHASCHNG (GOTOPOOH).
- Each routine runs until Z reaches one of those external entries, POSTJUMP (DUMMYJOB +2, GOP00FIX), NOQRSM +1, or the fixed-fixed return of a BANKCALL'd KLEAN entry.
- Run-time targets:
  - `TC Q` (MR.KLEAN, NONAVKEY, STARTSUB) executes the word in Q as TC and leaves Q = 3. It takes only this bank's seven return points, or a fixed-fixed return.
  - PCLOOP's `INDEX A; DCA -PHASE1`, NXTRST's `INDEX A; CCS PHASE1` and DSPOFF's `INDEX MPAC; TS DSPTAB` form the instruction as the hardware does. ELRSKIP's `INDEX A; DCA T5IDLER` reads the four-entry T5IDLER table (`MASK SIX` keeps the index at 0, 2, 4 or 6).
  - The ERESTORE repair `INDEX SKEEP7; DXCH 0000` writes the two words ERASCHK was testing. It refuses an address below 061: ERASCHK starts at 061, so nothing produces one.
- Data: the fixed constants the file reads (with CA/CAF/CS/AD/MASK/MP, DCA) and the T5IDLER table are compiled from the source listing into `src/cm_fresh_start_operands.inc` (`tools/file_operands_generate.py cm-fresh-start`) and checked against the rope. The table covers the whole file, banks 04 and 20 included, for the next milestones. The CADRs after the file's BANKCALL, IBNKCALL and POSTJUMPs are in `cm_interbank_site_table`, which the program uses as its operand table (as `cm-update` does).
- Devices: io is a `cm_fresh_io`, which starts with `cm_native_io` so `cm_interbank_routines` read the same record.
  - Outputs 5, 6, 11 (DSALMOUT), 12, 13 and 14, inputs 15 (MNKEYIN), 16 (NAVKEYIN) and 33, and the three HANDRUPT trap latches.
  - As in yaAGC, writing channel 13 bits 12-14 arms the traps, and those bits do not stay in the channel; GOPROG's `WAND CHAN33` only resets the channel's latched bits 11-15 to 1 (yaAGC CpuWriteIO), except that the AGC warning latch (bit 14) sets again at once while the warning condition persists (`cm_fresh_io.warning`; yaAGC keeps the bit 0 while its WarningFilter is over the threshold, which the oracle driver sets for those cases). WOR/WAND/RAND/ROR/READ follow yaAGC's OverflowCorrected/SignExtend.
  - INHINT acts on the interrupt-enable state.
- SWRETURN: a BANKCALL'd KLEAN entry's `TC Q` leaves Q = 3 there, and the runs stop at SWRETURN, as `lm-fresh-start` does (Lane A refused that context until PR #90; it accepts it now, and the run still stops there: what follows is INTER-BANK_COMMUNICATION's return to the caller, outside these files).

Paths with no producer: a TS that overflows refuses (no input in the domain overflows). The CCSs after `CS DSPTAB +11D; MASK BITS4&6` and `CS FLAGWRD5; MASK ENGONBIT` are never negative; their negative branches refuse. PCLOOP's and DSPOFF's counters count down from constants inside the same run, so their negative CCS branches refuse.

## Source map

| AGC | Native (`src/cm_fresh_start.c`) |
|---|---|
| SLAP1 ... T5IDLOC (bank 05), GOTOPOOH (fixed-fixed) | `bank05`, labels `L<address>`; `jump` for entries and `TC Q` returns |
| constants, T5IDLER | `CM_FRESH_K_*`, `cm_fresh_t5idler` (`src/cm_fresh_start_operands.inc`) |
| BANKCALL/IBNKCALL/POSTJUMP CADRs | `cm_interbank_site_table` |
| channels 5, 6, 11-16, 33, traps | `cm_fresh_io` (`include/cm_fresh_start.h`) |

## Evidence

Hashes: `evidence/cm-fresh-start/SHA256.json`. Bulk files are assets of the `evidence-cm-fresh-start` release (`python3 tools/evidence.py fetch cm-fresh-start`).

- Independent assembly comparison: `evidence/cm-fresh-start/cm-fresh-start-proof.json`. `make cm-fresh-start-differential` runs the unmodified Comanche055 rope in yaAGC (host-only), 4,096 cases.
  - Case n enters entry n % 17, with the image pseudo-random within the program's domain:
    - Restart-group phase pairs agree (inactive, active or negative), with one pair disagreeing in an eighth of the cases: -PHASEn has one bit flipped, or equals PHASEn (so PCLOOP's RXOR gives +0 and the second TCF PTBAD runs).
    - MPAC +5/+6 are the group scan's counters for the PINACT return.
    - ERESTORE is +0, -0, the address ERASCHK was testing (061-01776, with SKEEP7 equal to it) or a corrupt value.
    - A KLEAN caller's Q is SWRETURN (BUF2 a bank 04 return) or another fixed-fixed word.
    - In a quarter of the cases the AGC warning persists: channel 33 bit 14 is 0 and stays 0 after GOPROG's reset, so `RAND CHAN33` finds it and the restart loop takes the fresh start (BZF NONAVKEY +1, 05,2545).
    - ELR is sometimes held on channel 15 or 16. FB/EB, SUPERBNK, channels 5, 6, 11, 12, 13 and 14 (without the bits yaAGC acts on as a peripheral) and 33, the trap latches and the interrupt-enable state are pseudo-random.
  - Each case runs to the first point neither this file nor INTER-BANK_COMMUNICATION owns, or to SWRETURN.
  - Compared: the interrupt-enable state, SUPERBNK, channels 11, 5, 6, 12, 13, 14 and 33, the trap latches and all 2,048 erasable words. The digests are frozen into `tests/cm_fresh_start_vectors.h`.
  - The oracle traces the words each case executes (`tools/case_trace.h`). Together the cases execute every instruction of bank 05 but the patch-only STARTSIM, and of GOTOPOOH; the freeze fails otherwise.
- Host and sanitizer results: `make test-cm-fresh-start sanitize-cm-fresh-start` (`evidence/cm-fresh-start/cm-fresh-start-host.log`). Beyond the oracle digests, the test checks that:
  - all 17 entries are reached;
  - each case ends at one of 13 kinds of exit (SWRETURN, DUMMYJOB +2, RESTARTS, MMDSPLAY, WAITLIST, PHASCHNG, VAC5STOR, GOP00FIX, NOQRSM +1, ALARM, STOPRATE, SETCOARS, another KLEAN return), and every kind is reached;
  - the C64 rows are host cases with their host digests;
  - an ERESTORE repair into a counter, a TC Q to no return point, a missing io and a SUPERBNK outside bits 5-7 are refused.
- C64 execution results: `make vice-cm-fresh-start` (`evidence/cm-fresh-start/cm-fresh-start-vice.log`): 259 C64 rows chosen by native branch outcome (`tools/a2_c64_rows.py`: every 16th case plus a greedy cover of the gcov branch arcs of every linked source; the freeze fails unless the rows take all 436 arcs the host cases take), listed in `tests/cm_fresh_start_c64_vectors.h`. A word cover (the earlier `tools/c64_rows.py`) missed outcomes that share words, such as CCS PHASEn = -0 (05,2726), BZF GOTOPOOH taken (05,2743) and the -0 CCS/BZF paths in `agc_image_ops.c`; the arc cover takes them. The proof records the host and C64 arc counts.
