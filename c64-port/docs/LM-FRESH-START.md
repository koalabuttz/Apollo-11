# Translate Luminary FRESH_START_AND_RESTART (bank 05) as transfer-ABI routines

Lane A2 (restart, telemetry and support), split from Lane A. Milestone `lm-fresh-start`. Interface: `docs/NATIVE_TRANSFER_ABI.md` (owned by Lane A; this milestone adds routines, an operand table and its own device record only, no ABI change).

FRESH_START_AND_RESTART is what the AGC runs after a V36 fresh start, a hardware restart (GOJAM) or a software restart. Bank 05 holds the fresh-start initialization, the restart entry GOPROG with its check of the restart-group phase tables, the restart of each active group, ENEMA, the KLEAN routines that clear the phase tables, and the STARTSUB family that resets the Executive, Waitlist, display and DAP state. GOTOPOOH sits in fixed-fixed memory. The V37 and POOH code in banks 04 and 13 is the next milestone (`lm-v37`).

## Scope

- `Luminary099/FRESH_START_AND_RESTART.agc`, every instruction of bank 05 (2447-3401) and fixed-fixed (6001-6010):
  - SLAP1, SKIPSIM, DOFSTART, DOFSTRT1, ENDRSTRT.
  - MR.KLEAN, P00KLEAN, V37KLEAN.
  - GOPROG, BUTTONS, the ERESTORE repair, DORSTART, SETINFL.
  - ENEMA, GOPROG2, GOPROG2A, GOPROG3.
  - PCLOOP, NXTRST, PACTIVE, PINACT, PTBAD.
  - LIGHTSET, NONAVKEY, STARTSUB, STARTSB1, STARTSB2, DSPOFF.
  - GOTOPOOH.
- Not translated: STARTSIM (2452-2455). It runs only if the STARTSW word is patched to `TCF STARTSIM`; in the rope it is `TCF SKIPSIM`.
- Entries (`lm_fresh_start_routines`, 11):
  - SLAP1 (V36), GOPROG (hardware restart), GOPROG2 (V96), ENEMA.
  - MR.KLEAN, P00KLEAN, V37KLEAN (BANKCALL/IBNKCALL targets), GOTOPOOH.
  - The returns after TC MMDSPLAY, TC SWCALL (RESTARTS) and TC ALARM (PTBAD).
- Each routine runs until Z reaches MMDSPLAY, SWCALL, ALARM or POSTJUMP (DUMMYJOB +2, GOP00FIX), or the fixed-fixed return of a BANKCALL'd KLEAN entry.
- Run-time targets:
  - `TC Q` (MR.KLEAN, NONAVKEY, STARTSB2) executes the word in Q as TC and leaves Q = 3. It takes only this bank's nine return points, or a fixed-fixed return.
  - PCLOOP's `INDEX A; DCA -PHASE1`, NXTRST's `INDEX A; CCS PHASE1` and DSPOFF's `INDEX MPAC; TS DSPTAB` form the instruction as the hardware does.
  - The ERESTORE repair `INDEX SKEEP7; DXCH 0000` writes the two words ERASCHK was testing. It refuses an address below 061: ERASCHK starts at 061, so nothing produces one.
- Data: the CADRs after the file's own POSTJUMPs and BANKCALLs and the fixed constants it reads are compiled from the source listing into `src/lm_fresh_start_operands.inc` (`tools/file_operands_generate.py lm-fresh-start`) and checked against the rope. The table covers the whole file, banks 04 and 13 included, for the next milestone. Constants with an offset (`SWINIT +2`) get their own name (`LM_FRESH_K_SWINIT_P2`).
- Devices: io is an `lm_fresh_io`, which starts with `p66_bias_io` so Lane A's `lm_interbank_routines` read the same record.
  - Outputs 5, 6, 12, 13 and 14, inputs 15 (MNKEYIN), 16 (NAVKEYIN) and 33, and the three HANDRUPT trap latches.
  - As in yaAGC, writing channel 13 bits 12-14 arms the traps, and those bits do not stay in the channel. Reading channel 13 masks them too (`chan_op`), so a latch `agc_ch_op` left there (LLGE's WOR BIT12 before a restart with no GOJAM) reads as 0, as on the AGC. WOR/WAND/RAND/ROR/READ follow yaAGC's OverflowCorrected/SignExtend.
  - Channel 11 (DSALMOUT) is `program->hardware`'s. A path that writes it refuses without the hardware record.
  - INHINT acts on the interrupt-enable state.

Paths with no producer: a TS that overflows refuses (no input in the domain overflows). The CCS after `MASK ENGONBIT` is never negative; its negative branches refuse. PCLOOP's and DSPOFF's counters count down from constants inside the same run, so their negative CCS branches refuse.

## Source map

| AGC | Native (`src/lm_fresh_start.c`) |
|---|---|
| SLAP1 ... DSPOFF (bank 05), GOTOPOOH (fixed-fixed) | `bank05`, labels `L<address>`; `jump` for entries and `TC Q` returns |
| POSTJUMP/BANKCALL CADRs, constants | `lm_fresh_operands` (`src/lm_fresh_start_operands.inc`) |
| channels 5, 6, 12-16, 33, traps | `lm_fresh_io` (`include/lm_fresh_start.h`) |

## Evidence

Hashes: `evidence/lm-fresh-start/SHA256.json`. Bulk files are assets of the `evidence-lm-fresh-start` release (`python3 tools/evidence.py fetch lm-fresh-start`).

- Independent assembly comparison: `evidence/lm-fresh-start/lm-fresh-start-proof.json`. `make lm-fresh-start-differential` runs the unmodified Luminary099 rope in yaAGC (host-only), 4,096 cases.
  - Case n enters entry n % 11, with the image pseudo-random within the program's domain:
    - Restart-group phase pairs agree (inactive, active or negative), with one pair corrupted in an eighth of the cases (a flipped bit, or -PHASEn equal to PHASEn, which PCLOOP's RXOR makes +0).
    - MPAC +5/+6 are the group scan's counters for the PINACT return.
    - ERESTORE is +0, -0, the address ERASCHK was testing (061-01776, with SKEEP7 equal to it) or a corrupt value.
    - A KLEAN caller's Q is SWRETURN or another fixed-fixed word.
    - ELR is sometimes held on channel 15. FB/EB, SUPERBNK, channels 5, 6, 11, 12, 13 and 14 (without the bits yaAGC acts on as a peripheral), 16 and 33, the trap latches and the interrupt-enable state are pseudo-random.
  - Each case runs to the first point neither this file nor INTER-BANK_COMMUNICATION owns, or to SWRETURN.
  - Compared: the interrupt-enable state, SUPERBNK, channels 11, 5, 6, 12, 13 and 14, the trap latches and all 2,048 erasable words. The digests are frozen into `tests/lm_fresh_start_vectors.h`.
- Host and sanitizer results: `make test-lm-fresh-start sanitize-lm-fresh-start` (`evidence/lm-fresh-start/lm-fresh-start-host.log`). Beyond the oracle digests, the test checks that:
  - all 11 entries are reached;
  - each case ends at one of seven kinds of exit (DUMMYJOB +2, GOP00FIX, RESTARTS, MMDSPLAY, ALARM, SWRETURN, another KLEAN return), and every kind is reached;
  - the C64 rows are host cases with their host digests;
  - every case run again with channel 13 bits 12-14 latched ends exactly as without them (the trap latches, channels and erasable);
  - an ERESTORE repair into a counter, a TC Q to no return point, a missing io and a fresh start without the hardware record are refused.
- C64 execution results: `make vice-lm-fresh-start` (`evidence/lm-fresh-start/lm-fresh-start-vice.log`): 259 C64 rows chosen by native branch outcome (`tools/a2_c64_rows.py`: every 16th case plus a greedy cover of the gcov branch arcs of every linked source; the freeze fails unless the rows take all 389 arcs the host cases take). Among them are the outcomes a stride missed: the ERESTORE != SKEEP7 fresh start (05,2716-2717) and the no-group-active restart, both outcomes of BZF GOTOPOOH and ENDRSTRT (05,3045-3053). The proof records the host and C64 arc counts; the host test checks every C64 row is a host case with its host digest.
