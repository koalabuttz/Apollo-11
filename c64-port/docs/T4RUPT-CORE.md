# Run the T4RUPT display and dispatch core of both missions as native routines

Lane A. Milestone `t4rupt-core`. Interface: `docs/NATIVE_TRANSFER_ABI.md`.

T4RUPT is the AGC's display and monitor interrupt. Each 120 ms phase drives one DSKY relay word to output channel 10, checks the PRO key, and then dispatches through T4JUMP to one of the monitors: RCS, rendezvous radar, IMU and DAP checks in the LM, or optics and IMU in the CM. Between phases, 20 ms quick-display interrupts flush any further relay words. This milestone covers the first part of T4RUPT_PROGRAM.agc in both missions. It runs from T4RUPT to the end of the quick-display code (bank 06,2000-2171 LM, 06,2000-2173 CM) and includes the RELTAB relay table. It is native as `lm_t4rupt_core_routines` and `cm_t4rupt_core_routines` (`src/t4rupt_core.c`). The monitors that T4JUMP reaches (IMUMON onward) remain to be done.

| AGC | Native |
|---|---|
| T4RUPT 06,2000 (TS BANKRUPT; QXCH QRUPT; CCS DSRUPTSW) | `t4rupt`: the 120 ms phase when DSRUPTSW is a count, the quick display when it is negative |
| NORMT4, CDRVE (the lamps word DSPTAB +11D first) | `t4rupt`, then `dspout` or the lamps relay word and HANG20 |
| DSPOUT, DSPOUTSB, DSPSCAN, DSPLAY | `dspout`, `dspoutsb`: scan DSPTAB down from DSPCNT for a word waiting (negative), write its relay word to OUT0, and leave through TCF Q+1. Q+1 is the fixed-fixed routine INDEX Q; TC 1, which also leaves Q. |
| NODSPOUT, HANG20, SETTIME4 | `nodspout`, `hang20`, `settime4`: TIME4 for 120 ms, or 20 ms with DSRUPTSW switched to quick display |
| PROCEEDE (RXOR CHAN32, RXOR LCHAN) | `settime4`: a PRO key change starts PROCKEY through NOVAC (LM 05072, CM 05134) |
| T4JUMP 06,2116 (INDEX RUPTREG1; TCF +1) | `t4jump`: LM TC RCSMONIT, TCF RRAUTCHK, IMUMON or DAPT4S; CM TCF OPTTEST (TC IBNKCALL; CADR OPTDRIVE), OPTMON, IMUMON or RESUME. It is also the entry for NOVAC's return. |
| LM 06,2121 and 06,2125, CM OPTTEST 06,2130 | entries: RCSMONIT's returns (TCF RRAUTCHK); OPTTEST |
| QUIKDSP, QUIKRUPT, NODSPY, SYNCT4, QUIKOFF | `quikdsp`: one more relay word, or catch TIME4 up in 20 ms steps (SYNCT4), then RESUME |

Both missions have the same code at the same addresses up to T4JUMP. Comanche's two-word OPTTEST shifts the rest by two. Only DSRUPTSW, IMODES33, the Q+1 routine and the T4JUMP targets differ, and they are per-mission constants. WRITE OUT0 goes to `program->hardware->channel10` and RXOR CHAN32 reads `channel32`. Both channels join `agc_hardware`, an ABI addition: initializers that name only the earlier members leave them zero. Three constants are executed as instructions, but only on paths that no producer reaches:

- TABLNTH runs as TC 012 for a -0 display word or a negative DSPCNT.
- 120MRUPT runs as CA 7764 for a positive DSRUPTEM inside DSPSCAN.
- OCT37737 runs as CA 7737 when SYNCT4 finds DSRUPTSW +0, which adding BIT9 to a negative value never gives.

### Input domain

- T4RUPT runs inside the interrupt with FB 06 and no overflow in A, as the T4RUPT lead-in leaves them.
- DSRUPTSW is at most 8 when it is positive, because RUPTREG1 indexes the eight-entry T4JUMP table.
- DSPCNT is 0-012 or -0.
- No display word (DSPTAB 0-10) is +0 or -0. Fresh start fills them with -BIT12, and DSPIN stores complemented codes and alarms on +0. A +0 word would send DSPSCAN round forever.
- At T4JUMP, RUPTREG1 is 0-7.

Anything else is refused (`EXEC_OUT_OF_SCOPE`) with the image and io unchanged.

## Evidence

Hashes: `evidence/t4rupt-core/SHA256.json`.

- **Source maps.** `tools/t4rupt_core_source_map.py` (`evidence/t4rupt-core/t4rupt-core-source-map-{lm,cm}.json`) lists the 134 Luminary099 and 136 Comanche055 words of the core and RELTAB. The original executed every instruction in the oracle run below, except the three unreachable constants above. The script also checks the per-mission constants, targets and RELTAB built into the native routines against the assembled words.
- **Independent assembly comparison.** `make t4rupt-core-differential` runs the unmodified ropes in yaAGC (host-only, `tools/t4rupt_core_reference.c`). It uses 2,048 cases at T4RUPT and 64 at each return entry: 2,240 LM and 2,176 CM.
  - DSRUPTSW is a phase count, +0, or a quick-display value, some of which SYNCT4 carries to -0.
  - DSPCNT is 0-012 or -0. The display words are mostly settled (positive) and some are waiting (negative). The lamps word is sometimes +0.
  - NOUT, FLAGWRD5, IMODES33 and channels 10 and 32 are random, except that from case 128 on, one case in sixteen sets FLAGWRD5 to -0 so DSPOUT's `CCS FLAGWRD5` takes its -0 branch. (Cases 0-127 stay as they were, because the resident interrupt vectors reuse them.)

  The native routine runs once. The original then runs from the same entry until it first reaches the continuation the native run left in Z, and FB in banked memory. Every case equals the original in all 2,048 erasable words, SUPERBNK, the interrupt-enable bit, InIsr and channel 10. Per-case digests are frozen into `tests/t4rupt_core_vectors.h`; see `evidence/t4rupt-core/t4rupt-core-proof.json`.
- **Host and sanitizer results.** `make test-t4rupt-core sanitize-t4rupt-core` (`evidence/t4rupt-core/t4rupt-core-host.log`) checks:
  - the digests;
  - that every continuation of both missions is reached, along with the HANG20, NODSPOUT, QUIKOFF and quick-display paths;
  - six refusals with the image unchanged.
- **C64 execution results.** `make vice-t4rupt-core` (`evidence/t4rupt-core/t4rupt-core-vice.log`) runs every sixteenth case of both missions, plus the cover rows in `tests/t4rupt_core_c64_rows.h`. `tools/lane_a_c64_cover.py t4rupt-core` picks those rows so that the C64 cases take every branch arc (`gcov -b`, all linked sources) that the 4,416 host cases take. The arc counts are in `evidence/t4rupt-core/t4rupt-core-c64-cover.json`.
