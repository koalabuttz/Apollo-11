# Flight store and whole-program differential (phase 1)

Flight image integration, phase 1, Resident kernel lane. Two tools put every
lane's native routines together and run them as one program:

- `make flight-store` (`tools/flight_store_build.py`): every lane's routine
  tables in ABI 2 modules, packed into the REU store with one canonical
  resolver, plus a fit report (`build/flight-store/report.json`).
- `make flight-host-differential` (`tools/flight_host_differential.py`):
  both missions from Fresh Start on the host, against yaAGC, with erasable
  memory compared every centisecond. Logs, a `summary.json` and the
  unregistered points by file and lane go to `build/flight-store/differential/`.

Neither target is part of `test`. They measure the whole program while lanes
are still filling it in, and phase 1 adds no certificate.

## Store builder

`tools/flight_store_build.py` compiles every `src/*.c` on the host, reads
the symbols with `nm`, prints every routine table's keys, assigns tables to a
mission and builds one module per source file (the generic wrapper
`platform/c64/flight_table_module.c` plus the file's link closure, `-Oz`,
LTO, gc-sections). Lane and slot come from `docs/overlay-allocation.json`.
The resident's own tables (thin kernel, SPSIN/SPCOS, interrupt bodies) and
the certified Executive tables the thin kernel replaces are not packed. A
key two packed tables both hold is a collision: the resolver sends it to the
table of the file `docs/port-coverage.json` lists first, and the report
lists every collision for its lanes. `--host-only` writes only the host
table index (`build/flight-store/<m>-host.inc`) and the whole-program site
table (`build/flight-store/<m>-sites.inc`) for the differential.

Each module carries its own operand table (`FS_SITES`): the
`agc_site_operand` arrays its sources compile to for the mission (read from
`cc -E` of each source), so no whole-program site table is needed. The
builder fails unless every `src/*sites.inc` has exactly one includer. The
host table index also writes `build/flight-store/<m>-sites.inc` (one array
per source) for the harness. A module's `fixed_data` is the resident's
(boundary words in the REU) unless `FIXED_DATA` names another:
`<m>_self_check` reads the rope image through its own hook while ROPECHK
runs, and `lm_trim_gimbal` (GTS reads an indexed CA through `fixed_data`)
reads every rope word from the mission rope image the store carries
(`FS_ROPE_DATA`, `platform/c64/rope_image_reu.c`). The builder writes that
image into `<m>.reu` at the rope base (`tools/rope_image_pack.py` `BASES`:
LM 0x1c0000, CM 0x1e0000) and pads the store to 2 MB.

The 2 MB REU, by address:

| REU range | Use |
| --- | --- |
| 0x000000 up | The module store (header, index, resolver, modules); then the module-test harness's case blob, below 0x1c0000 |
| 0x1c0000-0x1d1fff | LM rope image (LM store only) |
| 0x1df000-0x1dffff | Lane C Pinball trampoline's state save (`LANEC_PIN_REU_SAVE`, `src/lanec_pin_trampoline.c`) |
| 0x1e0000-0x1f1fff | CM rope image (CM store only) |
| 0x1f8000-0x1fffff | The load check's scratch (`SCRATCH_BYTES`; the build checks it is unused) |

The compact interpreter (Interpreter lane, `platform/c64/interp_module.c`)
goes into the store as two modules linked by `tools/interp_runtime_size.py`
`link_core`: $0A01 in the job slot (LM 15,847, CM 15,856 bytes) and its cold
bodies $0A02 at $D000 (slot class 2, loaded at boot; LM 4,038, CM 4,029).
`INTERP_POINTS` gives the resolver a record to $0A01 at each of the
interpreter's fixed entry points (logical Z, canonical bank 04000 or 06000)
unless a table record or a resident table already holds it: today DANZIG is
the resident's, and SWRETURN and INTPRET are table records, so 9 of 12 are
the interpreter's on each mission. CHANG2 is left out, since that is where
the interpreter hands back to the Executive. The module returns to the step
when the CIA1 count at $6FBC moves, so an interrupt waits at most one
interpretive slot. The load check resolves the records; it does not run the
interpreter (`make vice-interp-core` does).

`CO_LOCATE` packs a table into a second module as well, when two modules
would otherwise swap all the time: the interbank routines (POSTJUMP,
BANKCALL, ...) ride in `<m>_self_check`, whose idle loop calls them, which
cut module loads from about 4,200 a second to the rates below. It only
fits once self-check and the interbank routines shrink (fit, above).

The device record `include/flight_io.h` covers every lane's io kind. Two
new kinds, `FLIGHT_IO_LM_CHANNEL` and `FLIGHT_IO_CM_CHANNEL`, carry the
mission record followed by `agc_input_channels` (channels 3, 4, 30-33);
the builder picks them for any file that uses `<m>_channel_io` (CM
`FINETIME` in IMU_CALIBRATION needs it).

On the C64 the resident hands every module its base record (`rk_io`:
`p66_bias_io` or `cm_native_io`). A module whose tables take another kind
(`module_io`; `FS_IO_KIND` in `platform/c64/flight_table_module.c`) builds
that record with `flight_io_enter` from `rk_hardware`, `rk_flight` and
`rk_device` before its run and writes it back with `flight_io_leave` after;
on the CM it also copies `cm_channels` to and from `rk_hardware`'s channels
5, 6 and 30-33. Its tables are marked `RK_TABLES_NOT_DIRECT`, so `rk_direct`
leaves them to the module main. A module may mix the base kind only with a
kind whose record starts with the base record (LM_*, CM_*), and the builder
refuses a kind with no `flight_io_enter`/`flight_io_leave` case. The
conversion adds about 330 bytes to the module main (1,076 bytes in the
pinned LM DAP set, against about 750 for a base-kind module); the set
measures 39,341 bytes. Every CM module links the resident's
`cm_channels` (`--defsym`); the builder refuses one that keeps a copy.

A pinned module is direct-only (`FS_DIRECT_ONLY`): it links no transfer
engine, its main only builds the table list and returns `EXEC_OK`, and the
resident runs its routines through `rk_direct` from the next step. That
needs the base record, so a pinned set whose tables take another kind
links the engine and conversion until the kind folds into `agc_hardware`;
`report.json` gives both sizes (`bytes`, `direct_bytes`). The LM DAP set
measures 39,341 bytes today and 38,534 direct-only.

The VICE load check also fills the free soft stack below the timings with a
pattern and reports the deepest byte touched (`soft_stack_peak_bytes` in
`vice-load.json`): 302 bytes LM and 174 bytes CM, over every module's main
and one resolver record per module run through `rk_direct`.

### Fit on 6b6ed2dc

| | Tables | Modules | Linked | Failed | Resolver records | Colliding keys | Store |
|---|---|---|---|---|---|---|---|
| LM | 113 | 63 | 16 | 47 | 213 | 650 (25 table pairs) | 297,472 bytes |
| CM | 89 | 52 | 16 | 36 | 202 | 405 (12 table pairs) | 294,912 bytes |

Every failure is a module bigger than its 16 KB slot (median overflow about
34 KB, largest 136 KB LM and 170 KB CM). LM `service_transfer` misses by 65
bytes since it carries its own 171 operand sites (684 bytes), and LM
`lm_fresh_start` by 359. `<m>_self_check` links since services 0x0202
(`apollo_reu_transfer`) but misses by about 1.1 KB on its own (its step
machine `body` is 10 KB) and by about 9 KB with the interbank tables
co-located, so the co-location below is the target layout rather than one
that fits yet. The store, with the resident's own modules (five LM, four
CM), is far below the 2 MB REU. The load path works in VICE (below); the
whole-program scenarios in VICE wait on the modules fitting.
Interpretive modules wait on the interpreter decision
(`/mnt/project-files/steward/interpreter-proposal.md`).

A basic-only module over its slot is split into chunks of consecutive
routines (`platform/c64/flight_table_module.c` `FS_CHUNK`: entries copied
field by field by constant index, so LTO links only the chunk's routines),
halving until each chunk fits. A chunk holds only its own file's sources
and operand sites; a table co-located into another module is never
chunked. Chunks that fit are kept even when others do not; a single
routine still over its slot is reported as a failed entry
(`one routine over its slot`, with its `routine_key`), and a file of more
than 64 KB is `too big to split`. Before #211-era step-machine splits and
Lane F's fix to the entry copy (a whole-struct copy kept every routine
linked), chunking gained nothing; now it does.

### Fit with chunking (this branch, after #209)

| | Tables | Modules | Linked | Failed | Store image |
|---|---|---|---|---|---|
| LM | 103 | 146 | 89 | 57 (25 one routine over its slot, 32 whole files: interpretive, every routine over, or too big to split) | 1,067,264 bytes |
| CM | 90 | 207 | 146 | 61 (34 one routine over its slot, 27 whole files) | 1,473,536 bytes |

The over-slot routines and files, by lane, are in
`/mnt/project-files/resident-kernel/over-slot-2026-10-13.md`.

### Runtime sharing

A module no longer links its own copy of the transfer ABI and the common
image ops. The resident exports them as ABI 2 services (table 0x0203,
docs/overlay-v2-services.json): `agc_transfer_run`, `agc_transfer_point`,
`agc_fixed_physical`, `memcpy` and the kernel's `k_<op>` for 17 image ops
(CA, CAF, CS, CSF, AD, ADF, MASKF, TS, XCH, LXCH, ADS, DXCH, CCS, TC, DCAF,
BZF, BZMF). `tools/flight_store_build.py` links each module with
`-Wl,--wrap=ai_<op>` and `platform/c64/flight_image_op_services.c`, whose
`__wrap_ai_<op>` calls `k_<op>` (the frame's image is always the canonical
one at `APOLLO_SHARED_ADDRESS`). Ops whose export would cost the resident
too much (DAS, INCR, QXCH, MASK, DCSF, `ai_put`, `ai_word`;
docs/RESIDENT-FLIGHT.md, Size) and ops the kernel does not wrap stay in
each module. On def640e3d, against the same tree without sharing:

| | Modules (linked, failed) | Resolver records | Mean module | Store image |
|---|---|---|---|---|
| LM before | 129 (72, 57) | 596 | 11,040 bytes | 936,192 bytes |
| LM after | 113 (66, 47) | 606 | 9,022 bytes | 721,920 bytes |
| CM before | 248 (183, 65) | 791 | 8,181 bytes | 1,636,608 bytes |
| CM after | 183 (133, 50) | 806 | 7,353 bytes | 1,106,944 bytes |

Fewer modules link because more files fit whole and need no chunks; the
resolver records (keys the store can run) go up on both missions.

`PIN` names task-slot tables that must link as one module (the steward's
DAP rule: the LM T6 hot path pinned in one task-slot module). The LM set
(DAPIDLER, P-axis, Q/R-axis, TJET law) measures 47,075 bytes against the
8,186-byte task slot, so it is reported (`report.json` `pinned`) and not
packed yet. A slot-3 set goes to slot class 3 ($C000-$CBFF, 3,072 bytes;
docs/RESIDENT-FLIGHT.md): when it fits direct-only the builder packs it into
the store, the resident loads it at boot, and its routines run from there
through `rk_direct` while the resolver still names their packed modules. Both
missions pin the background self-check idle loop there (LM
`lm_self_check_idle_routines` 1,897 bytes, CM `cm_self_check_idle_routines`
1,920), so those routines run without loading a module. The load check runs one
of the pinned set's points through the module that packs the same table and through
`rk_direct` with only the pinned module present, compares them, and then
drops the pinned module (its mailbox and timings use $C000-$CBFF).

### Direct calls into a loaded module

Header word 22 (reserved before) is `TABLES`: the address, inside the
slot, of the module's table list (`rk_module_tables`,
`platform/c64/resident_flight.h`), or 0. `overlay_v2_payload.ld` fills it
from a module's `.ov2_tables` word, so only flight modules carry one. A
module builds the list on its first run after a load (the load clears its
BSS, so a fresh copy is never trusted). `tools/overlay_v2_pack.py` checks
the word lies inside the slot; the loader does not read it.

After the trampoline and before the resolver, the resident's step calls
`rk_direct`: for each loaded slot with a ready list, it runs
`agc_transfer_run` on the resident's program with the module's tables, at
most 256 steps. `EXEC_UNMAPPED` with no step means the point is not that
module's; the resolver and an ABI 2 run follow only when no loaded module
holds the point. A module's main does the same through its own list, so
both paths run the same tables.

A module never crosses a 512 KB REU bank (`tools/overlay_v2_pack.py`
starts it on the next bank instead): one DMA loads a module, and the REU's
address counter wraps inside the bank, so a crossing module loaded the
store's first bytes into the slot's tail.

The resident and every module are compiled with
`-DAGC_TRANSFER_DANZIG_MARKER` (`tools/resident_flight_build.py` COMMON):
the pending-DANZIG marker is opt-in on the C64 since #184, and the resident
resolves pending RTB right-hand orders through it.

## VICE load path

`make flight-store-vice` (`tools/flight_vice_load.py`) boots the flight
resident on the whole-program store in VICE. The store builder also packs
the resident's own modules into the store (the boundary-word module, built
for the store's resolver records, and the interrupt-body wrappers with their
records), so `build/flight-store/<m>.reu` is a bootable flight image. It
also links `build/flight-store/<m>-load-check.prg`: the resident with
`platform/c64/flight_load_check.c` as main. After `rk_boot`, the check
resolves every resolver record through the resident's resolver (LM 245, CM
226, the wrappers' included) and checks the module it names. It then runs each
flight module twice, with interrupts off, from a fixed point no table holds:
once loading it from the REU into its slot, and once already loaded. Each run
must come back with `EXEC_UNMAPPED`. CIA2's cascaded timers count the
cycles. The timings go to `build/flight-store/vice-load.json`.

On 6b6ed2dc both missions pass (LM 278 checks, CM 259): every record
resolves, and all 16 fitting modules per mission load and return. Load cost
over a cached call, fitted over the 32 modules (5.7 to 16.2 KB, largest
residual 880 cycles):

    load = 7,459 + 1.103 cycles per byte   (PAL: 985,248 cycles a second)

With chunking (this branch) both missions pass with every linked module:
LM 740 checks (89 modules, load and call 18,367-48,231 cycles), CM 1,067
(146 modules, 18,187-60,798). Over 235 modules the fit is 10,645 + 1.227
cycles per byte with a largest residual of 22,476: some modules' first run
does more work than their cached run, so the 32-module fit above is the
cleaner transfer cost. For bisecting a crash, `flight_load_check.c` takes
`-DFL_FIRST`/`-DFL_LIMIT` (a module range) and `-DFL_NO_PROFILE`.

The check then runs one resolver record per module both ways: through the
module's main (an ABI 2 run) and through `rk_direct`, from the same image,
slot contents and channels (snapshotted in the REU's last 32 KB), with only
that module loaded. Status, image and channels must match. On def640e3d
plus this change both missions pass (LM 903 checks, 44 modules compared;
CM 1,394, 99), and a direct call saves 1,433 to 2,536 cycles over the
module main: the ABI 2 run and the copy of the program record. Most of
what is left is the step itself: the transfer point and the table search.

The step now computes the transfer point once (it was computed again to
pick the table set), which cut it from about 1,100 to 880 cycles, and the
search skips a table whose key range cannot hold the point.
`flight_store_build.py` refuses a table whose keys do not strictly ascend,
since both the binary search and the range skip rely on it. With this,
cached calls are 6,081 to 8,158 cycles (from 7,287 to 11,407), and the
check passes on both missions (LM 907 checks, CM 1,398). The resident
resolve is still 4,200 to 4,900 cycles of each step.

A cached call (no load) costs 6,100 to 8,200 cycles on its own: the ABI 2
run, the module copying its program record and tables, and one resolve. So a
swap to a 12 KB module costs about 21,000 cycles plus the call, about 30,000
in all (3% of a second), and a full 16 KB slot about 35 ms. At the LM's 40
module loads a second (DAP chain, results below), swaps alone would take
more than the CPU (about 1.2 million cycles a second), and even 5 a second
costs 15%. The task-slot DAP chain needs its modules merged or kept
resident, not just made to fit.

## Host differential method

`tools/flight_host_run.c` (GPL, host only, links yaAGC; never part of the
port) runs one mission:

1. **Cold start.** Native and yaAGC both start at GOJAM with yaAGC's
   `agc_engine_init` state (channel 30 = 037777, channels 31-33 = 077777,
   no DOWNRUPT). Both missions take PTBAD (FAILREG 01107) and then
   DOFSTART, natively and in yaAGC alike.
2. **Native run.** The thin kernel and every packed table, resolved by the
   canonical key, run through `rk_rupt_take` and the trampoline. Each call
   sees the operands and `fixed_data` its module would hold on the C64: its
   own site table, and the boundary words (the rope word at every table key,
   the interpreter fetch point and DANZIG) or self-check's rope hook.
   Interrupts take the word at Z, and at an interpretive routine Z is the
   fetch point or DANZIG rather than the key, so without those two words an
   interrupt there stays pending (LM V37E00E: 500 of 3000 samples equal).
   The resident's boundary table needs the fetch point too once it
   resolves interpretive orders.
3. **Lockstep timing.** A native routine has no clock, so each call is
   checked by running yaAGC from the same before-state until it reaches the
   routine's continuation with equal erasable memory (024-031 and Z
   excepted; a RESUME boundary with SubstituteInstruction set counts). On a
   match the native side takes yaAGC's timers 024-031, channels 3/4, channel
   13 bit 15 and the interrupt requests raised up to the match. On a
   mismatch the routine is reported and the timing comes from yaAGC's first
   arrival at the continuation.
4. **Bridges.** A point no table registers, or a routine that refuses (its
   effects undone), runs in yaAGC with its own clock and interrupts until a
   registered point or idle, and is reported with its arrivals and
   instruction count.
5. **Samples.** Each centisecond, at the first pass through ADVAN (LM
   03214, CM 03231, checked by physical address), idle, nothing pending, in
   the same SCALER1 phase, erasable memory is compared with an independent
   yaAGC run (0-017, 024-031 and RUPTREG1-4 excepted).
6. **Keys.** DSKY keys are pressed at a sample point. The yaAGC run enters
   KEYRUPT1 at the same instruction as `rk_rupt_take` does (ZRUPT =
   ADVAN+1, BRUPT = the rope word at ADVAN under the current FB and
   superbank).
7. **Module loads.** One module per slot (job, task); a routine whose
   module is not in its slot loads it. Each scenario reports loads per
   second and the points that cause them.
8. **Set-up knobs** (environment, both sides alike): `FLIGHT_CH30` and
   `FLIGHT_CH31` set input channels 30 and 31 at start (octal; e.g.
   `036377` and `057777` for IMU operate, PGNCS control and AUTO, so
   DAPIDLER starts the DAP); `FLIGHT_POKE=CS:ADDR=VALUE,...` sets erasable
   words at the first sample point at or after CS centiseconds (e.g.
   `5000:035=02000` moves CDUY). The reference keeps up to 8,191
   centiseconds.

9. **Scaled time** (`FLIGHT_SCALED=K,D,LAG[,SEED]`, the replay proof of
   docs/RESIDENT-FLIGHT.md, Scaled time). yaAGC's own clock is frozen:
   `tools/flight_host_differential.py` builds a local engine variant whose
   `AgcFrozenClock` stops the scaler loop (TIME1-TIME6, the scalers and the
   cycles their counts take) and the IMU CDU drive, which bursts on
   yaAGC's cycle counter; unset, the variant is the unchanged engine. The
   AGC clock then runs only as the flight resident runs it: a modeled C64
   wall clock advances by each span's AGC machine cycles times K, plus
   20 ms for each module swap (the VICE load cost), each optionally
   jittered from 1/4 to 4 times by SEED. A CIA1 interrupt lands every
   D × 10 ms of it, and at each native boundary at most LAG of them become
   `rk_timer_tick` periods, the rest dropped, as `rk_rupts` does; bridged
   code takes its periods between instructions. Lockstep then compares the
   clock counters too and adopts nothing from yaAGC, so every routine must
   match yaAGC run from the same state under the same periods, and the
   run as a whole is one yaAGC run under that tick schedule. No
   free-running reference runs, and no samples are compared.

`tools/flight_holes.py` names the file, label and owning lane of every
unregistered or refused point from the listing and the allocation.

## Results on 8564ca79

| Scenario | Samples equal | Major mode native / yaAGC | Routines yaAGC did not match | Module loads per second |
|---|---|---|---|---|
| LM P00 idle, 4000 cs | 4000 / 4000 | -0 / -0 | 0 | 40.3 |
| CM P00 idle, 4000 cs | 3998 / 4000 | -0 / -0 | 0 | 0.3 |
| LM V37E00E, 3000 cs | 3000 / 3000 | 00 / 00 | 0 | 43.2 |
| CM V37E00E, 3000 cs | 2998 / 3000 | 00 / 00 | 0 | 2.2 |
| LM V37E12E, 3000 cs | 3000 / 3000 | 14 / 14 | 0 | 43.1 |
| CM V37E06E, 3000 cs | 2998 / 3000 | 06 / 06 | 1 | 3.1 |
| LM V37E63E, 3000 cs | 3000 / 3000 | 77 / 77 | 0 | 43.1 |

On 0ce769c6 with this branch's store, the CM rows are unchanged; LM P00
idle stays 4000/4000, and the three LM V37 scenarios are 2900/3000 with
52.8-56.3 module loads a second. The 100 samples (4.01 to 5.00 s) differ only
in LOC (0164, 20002 native, 20042 yaAGC) after the DSKY key at 4.00 s: yaAGC
passes interpretive point 33490 after Pinball's 32831, natively the
`lanec_pin_lm` routine runs past it. The same tables give the same result
with the runner from 0ce769c6, so this is drift in the base's tables since
8564ca79, not the store builder.

Findings:

- **LM module loads.** About 40 a second, all in the task slot: the DAP
  chain `lm_dapidler` (MOREIDLE+2, SHUTDOWN), `lm_q_r_axis` (QERRCALC) and
  `lm_p_axis` (CALCPERR) each load once per DAP pass. They fit together only
  once Lane F splits the step machines.

- **CM samples at 1 and 2 cs.** Native interrupt latency: the CM fresh-start
  routine at 05608 runs about a centisecond as one native call, so a T3RUPT
  due at 2.20 cs waits for it while yaAGC takes it at once (words 01361 and
  01371 differ). Memory converges by 3 cs.
- **CM `cm_imu_calibration_basic` at 02,2554 (FINETIME).** L 37373 native,
  37374 yaAGC: the `agc_input_channels` ABI latches the scaler for the
  routine's whole call while yaAGC's scaler advances between the reads.
- **CM P11.** V37E11E is rejected (operator error) by yaAGC and natively
  alike, so P11 is not reachable by V37 from Fresh Start. CM V37E06E reaches
  P06 instead.
- **No lost BRUPT.** An interrupt never landed at a RESUME point whose
  BRUPT differs from the rope word.

### Unregistered and refused points, by lane

Every point below ran in yaAGC. Arrivals are from the 4000 cs idle runs
where the point is periodic.

| Lane | Mission | Points |
|---|---|---|
| A | LM | T4RUPT_PROGRAM: GPMATRIX 06,3132 (84), IMUMON 06,2172 (84), RRAUTCHK 06,3006 (83). SERVICE_ROUTINES: GENTRAN+1 02,3545 refused by `lm_service_routines`, because `tools/agc_sites_generate.py` takes `TC GENTRAN` sites but not the two `TC GENTRAN +1` sites in AOSTASK_AND_AOSJOB (20,3574 and 20,3615; no other `TC <callee> +n` site exists in either mission). INTERPRETER: INTPRET 03,2037 refused by `lanec_ev_lm_intpret`, Q+1 03,2737 |
| A | CM | T4RUPT_PROGRAM: IMUMON 06,2174 (84), OPTMON 06,2765 (83), OPTDRIVE 10,2000 (83). ALARM_AND_ABORT: ALARM 02,3650 (native code, no routine table). INTERPRETER: INTPRET 03,2006, Q+1 03,2706, and in V37E00E NOIBNKSW, SWSTORE, VLOAD+3, NEG4+1, TSSL+5, OPJUMP+2, VSHR2+11, VPDVL+3, NEWOPS+1, DSQSUB+15 |
| B | LM | INTEGRATION_INITIALIZATION: INTSTALL 13,3414/3415 and GOBAC+1 13,3464 refused by `lm_integration_init_*` |
| B | CM | INTEGRATION_INITIALIZATION (V37E00E): STATINT1, INTEGRV, MOVEPCSM, MOVEACSM, ATOPCSM+6, MOVEPLEM, ATOPLEM+6, MOVEALEM, ANDOUT+5/+7, GRP2PC+4, INTWAKE+11, WAKE1+4, INTSTALL, SETIFLGS, ENDINT, ENDINT+4/+11 |
| C | both | PINBALL_GAME_BUTTONS_AND_LIGHTS: CHARIN, LODSAMPT, ENTER, MMCHANG, MMCHANG+15, REQMM+10, 2BLANK, RELDSP, DSPMMJB, NVMONOPT, GOVNUPDT, UPDAT1+2, FLASHON, FLASHOFF, BLANKSUB, ENDIDLE; CM also DSPA, DSPOCTIN, OCTBACK, DSLV+1 |
| D1 | CM | KALCMANU_STEERING: STOPRATE 02,2526 refused by `cm_kalcmanu_steering_basic` |
| D2 | LM | AOSTASK_AND_AOSJOB: 1/ACCSET 20,2447, 1/ACCS+2, 1/ACCS+3, 1/ACCJOB+2, GETAOSUV+6, DOACCFUN+4. P20-P25: CLRADMOD 03,2011 |
| F | LM | RCS_FAILURE_MONITOR: RCSMON 06,3156 (83). P40-P47: ENGINOF1, ENGINOF3 (+4 to +21), ENGINOF4 refused by `lm_p40_p47_basic` |
| unallocated | CM | RTB_OP_CODES: LOADTIME 22,3510, SGNAGREE 22,3544 |

## Next (phase 2)

- Fit the store (steward decision above), then the same scenarios in VICE
  from the REU store.
- Give the flight resident the interpreter fetch point's boundary word when
  it resolves interpretive orders.
- Cut the cost of a cached module call (about 8,000 cycles) and plan the
  swap budget around the measured load cost.
- Rerun after each lane fills a point in; the target prints the remaining
  list.
