# Run whole CM T4RUPT interrupts through the trampoline

Lane A. Milestone `cm-t4rupt-chain`. Interface: `docs/NATIVE_TRANSFER_ABI.md`.

Comanche055 T4RUPT_PROGRAM.agc is native in three routine tables:
- `cm_t4rupt_core_routines` (`docs/T4RUPT-CORE.md`);
- `cm_t4rupt_imu_routines` (`docs/CM-T4RUPT-IMU.md`);
- `cm_t4rupt_optics_basic` (`docs/CM-T4RUPT-OPTICS.md`).

Each of those milestones compares its routines entry by entry. This one runs whole interrupts. Its program uses those three tables as its basic tables and enters at T4RUPT (06,2000). `agc_transfer_run` runs it until the first continuation outside the tables. That continuation is one of:
- RESUME;
- NOVAC (the PRO key);
- IBNKCALL (OPTTEST's OPTDRIVE, and SETCOARS);
- a service the monitors call: ALARM, VARALARM or WAITLIST.

T4JUMP's continuation into IMUMON or OPTMON resolves through the trampoline to the other tables, so the run checks that the tables compose as the file does.

It also holds the whole-file source map of T4RUPT_PROGRAM.agc (`tools/cm_t4rupt_closure_source_map.py`). Every assembled word is listed with the milestones whose oracle run executed it. Every instruction word is executed by at least one of them, except one:
- At 10,2110, OPTDRIVE's shaft stop check has `CCS A` after `MASK BIT15` and `RXOR LCHAN` of two `MASK BIT15` values. A is then +0 or 140000 (negative), never positive. So the `TCF CMDSETUP` in CCS's positive slot never runs.

The checked-in scan spells ZOPFINI as ZOPFIN1: three times in `TC ZOPFIN1` and once in the label. The source marks each of these lines. The map reads them as the listing's word.

The optics routines' oracle trace comes from rerunning the cm-t4rupt-optics differential under `tools/oracle_trace` (`make cm-t4rupt-closure-trace`).

### Channels

The core and IMU tables keep channels 10-14, 30 and 33 in `agc_hardware`. The optics routines keep 11-14 in `cm_native_io` and 30-33 in Lane G's `cm_channels`, until the shared channel model reaches them. The cases start both copies equal. T4JUMP runs at most one monitor per interrupt, so at most one copy changes; the run reports it if both do, and none does. `cch_channel()` reads the copy that changed. On a flight build the two must be one record (the shared channel model, Lane D1).

### Input domain

The interrupt is entered as its vector leaves it:
- FB 06, and EB 6 (T4RPTBB) in most cases;
- inside the interrupt;
- no IMU or optics drive in progress on channel 14, and no trap bits on channel 13.

The domains of the three tables apply at their entries. A chain that met a refusal would report it, and none does. The channel 13 and 14 conditions are not refused: the native code runs with those bits set, but the comparisons keep them clear, since yaAGC's hardware model acts on them (channel 13's trap bits arm hardware interrupts, and a channel 14 drive empties the command counters), which is the outside world rather than the program.

## Evidence

Hashes: `evidence/cm-t4rupt-chain/SHA256.json`.

- **Source map.** `evidence/cm-t4rupt-chain/cm-t4rupt-closure-source-map.json` covers the whole file: 833 words, of which 773 lines are instructions. Every instruction but the one above was executed by a native routine's oracle run.
- **Independent assembly comparison.** `make cm-t4rupt-chain-differential` runs 4,096 T4RUPT interrupts in yaAGC (host-only, `tools/cm_t4rupt_chain_reference.c`).
  - Each run goes from T4RUPT until the original first reaches the continuation the native run stopped at (and FB, in banked memory).
  - The inputs are T4RUPT's: DSRUPTSW selects each T4JUMP target, the quick display or the PRO key path. In half the cases, IMODES30, IMODES33, the flag words and channels 30 and 33 are near flight values.
  - Every case equals the original in all 2,048 erasable words, SUPERBNK, the interrupt-enable bit, InIsr and channels 10-14 and 30-33.
  - Digests are frozen into `tests/cm_t4rupt_chain_vectors.h`.
  - `evidence/cm-t4rupt-chain/cm-t4rupt-closure-proof.json` lists this comparison and the three per-table comparisons, with their case counts and snapshot or oracle hashes.
- **Host and sanitizer results.** `evidence/cm-t4rupt-chain/cm-t4rupt-closure-host.log` records `make test-t4rupt-core sanitize-t4rupt-core test-cm-t4rupt-imu sanitize-cm-t4rupt-imu test-cm-t4rupt-optics sanitize-cm-t4rupt-optics test-cm-t4rupt-chain sanitize-cm-t4rupt-chain`. The chain test checks:
  - every digest;
  - that every run ends at one of the exits above;
  - that each exit and both chain lengths (the core alone, the core then a monitor) are reached.
- **C64 execution results.** `evidence/cm-t4rupt-chain/cm-t4rupt-closure-vice.log` holds the four C64 diagnostics in VICE.
  - The chain runs 269 interrupts: every sixteenth, plus the 13 cover rows of `tests/cm_t4rupt_chain_c64_rows.h`.
  - With the cover rows, the C64 cases take every branch arc (`gcov -b`, all linked sources) that the 4,096 host interrupts take (`evidence/cm-t4rupt-chain/cm-t4rupt-chain-c64-cover.json`).
  - The 4 KB image sits under the KERNAL ROM, as in the optics diagnostic.
