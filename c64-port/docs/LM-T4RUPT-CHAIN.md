# Run whole LM T4RUPT interrupts through the trampoline

Lane A. Milestone `lm-t4rupt-chain`. Interface: `docs/NATIVE_TRANSFER_ABI.md`.

Luminary099 T4RUPT_PROGRAM.agc is native in three routine tables: `lm_t4rupt_core_routines` (`docs/T4RUPT-CORE.md`), `lm_t4rupt_imu_routines` (`docs/LM-T4RUPT-IMU.md`) and `lm_t4rupt_rr_routines` (`docs/LM-T4RUPT-RR.md`). Each milestone compares its routines entry by entry. This one runs whole interrupts: a program whose basic tables are the three, entered at T4RUPT (06,2000), run by `agc_transfer_run` until the first continuation outside them. That continuation is RESUME, NOVAC (the PRO key), RCSMONIT (the next file), or a service the monitors call: SPSIN, ALARM, VARALARM, WAITLIST, IBNKCALL, SETTRKF or RRLIMCHK. T4JUMP's continuation into IMUMON, RRAUTCHK or GPMATRIX resolves through the trampoline to the other tables, so the run checks that the tables compose as the file does.

It also holds the whole-file source map of T4RUPT_PROGRAM.agc (`tools/lm_t4rupt_closure_source_map.py`). Every assembled word is listed with the milestones whose oracle run executed it. Every instruction word is executed by at least one. Three constants (TABLNTH, 120MRUPT and OCT37737) would run as instructions only on paths that no producer reaches; `docs/T4RUPT-CORE.md` gives the reasons.

### Input domain

The interrupt is entered as its vector leaves it: FB 06, EB 6 in most cases (T4RPTBB), inside the interrupt, with no IMU drive in progress on channel 14 (`docs/LM-T4RUPT-IMU.md`). The domains of the three tables apply at their entries; a chain that met a refusal would report it, and none does.

## Evidence

Hashes: `evidence/lm-t4rupt-chain/SHA256.json`.

- **Source map.** `evidence/lm-t4rupt-chain/lm-t4rupt-closure-source-map.json`: the whole file, 634 words (622 in bank 06 and the 12 RELTAB words in fixed-fixed memory; 571 instructions and 63 constants), every instruction executed by a native routine's oracle run.
- **Independent assembly comparison.** `make lm-t4rupt-chain-differential` runs 4,096 T4RUPT interrupts in yaAGC (host-only, `tools/lm_t4rupt_chain_reference.c`) from T4RUPT until the original first reaches the continuation the native run stopped at, and FB in banked memory. The inputs are T4RUPT's (DSRUPTSW selects each T4JUMP target, the quick display, or the PRO key path), with IMODES30, IMODES33, RADMODES, the flag words and channels 30 and 33 near flight values in half the cases. Every case equals the original in all 2,048 erasable words, SUPERBNK, the interrupt-enable bit, InIsr and channels 10, 11, 12, 14, 30 and 33. Digests are frozen into `tests/lm_t4rupt_chain_vectors.h`. `evidence/lm-t4rupt-chain/lm-t4rupt-closure-proof.json` lists this comparison and the three per-table comparisons, with their case counts and snapshot hashes.
- **Host and sanitizer results.** `evidence/lm-t4rupt-chain/lm-t4rupt-closure-host.log`: `make test-t4rupt-core sanitize-t4rupt-core test-lm-t4rupt-imu sanitize-lm-t4rupt-imu test-lm-t4rupt-rr sanitize-lm-t4rupt-rr test-lm-t4rupt-chain sanitize-lm-t4rupt-chain`. The chain test checks every digest, that every run ends at one of the exits above, and that each exit and both chain lengths (the core alone, the core then a monitor) are reached.
- **C64 execution results.** `evidence/lm-t4rupt-chain/lm-t4rupt-closure-vice.log`: the four C64 diagnostics in VICE. The chain runs 265 interrupts: every sixteenth, plus the cases that `tools/c64_case_cover.py` adds so that the rows run every branch outcome (control-flow edge) of the three native files that the 4,096 host interrupts run.
