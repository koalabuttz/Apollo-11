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
runs.

`CO_LOCATE` packs a table into a second module as well, when two modules
would otherwise swap all the time: the interbank routines (POSTJUMP,
BANKCALL, ...) ride in `<m>_self_check`, whose idle loop calls them, which
cut module loads from about 4,200 a second to the rates below.

The device record `include/flight_io.h` covers every lane's io kind. Two
new kinds, `FLIGHT_IO_LM_CHANNEL` and `FLIGHT_IO_CM_CHANNEL`, carry the
mission record followed by `agc_input_channels` (channels 3, 4, 30-33);
the builder picks them for any file that uses `<m>_channel_io` (CM
`FINETIME` in IMU_CALIBRATION needs it).

### Fit on facddf23

| | Tables | Modules | Linked | Failed | Resolver records | Colliding keys | Store |
|---|---|---|---|---|---|---|---|
| LM | 128 | 62 | 16 | 46 | 213 | 650 (25 table pairs) | 270,592 bytes |
| CM | 113 | 51 | 15 | 36 | 185 | 405 (12 table pairs) | 257,536 bytes |

All but one failure per mission is a module bigger than its 16 KB slot
(median overflow about 34 KB, largest 136 KB LM and 170 KB CM; only 2 LM
and 3 CM are within 4 KB of fitting). LM `service_transfer` misses by 65
bytes since it carries its own 171 operand sites (684 bytes). The other
failure is `<m>_self_check`, which needs the resident's
`apollo_reu_transfer` through the service table (services 0x0202, Kernel
lane). The store itself is far below the 2 MB REU. So the VICE
whole-program run waits on the modules fitting. Interpretive modules wait on
the interpreter decision (`/mnt/project-files/steward/interpreter-proposal.md`).

A basic-only module over its slot is split into chunks of consecutive
routines (`platform/c64/flight_table_module.c` `FS_CHUNK`: entries copied by
constant index, so LTO links only the chunk's routines), halving until each
chunk fits. The builder keeps a split only when every chunk fits and stops
early when halving leaves the overflow nearly unchanged; the report marks
such a module `routines share one body`. On 4e5cdfbb that is every oversized
basic module (9 LM, 10 CM): each file's routines run one file-wide step
machine (LM P-AXIS `flow` alone is 9.4 KB), so a one-routine chunk still
overflows. Those files fit only once their lane splits the step machine.

The resident and every module are compiled with
`-DAGC_TRANSFER_DANZIG_MARKER` (`tools/resident_flight_build.py` COMMON):
the pending-DANZIG marker is opt-in on the C64 since #184, and the resident
resolves pending RTB right-hand orders through it.

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

`tools/flight_holes.py` names the file, label and owning lane of every
unregistered or refused point from the listing and the allocation.

## Results on facddf23

| Scenario | Samples equal | Major mode native / yaAGC | Routines yaAGC did not match | Module loads per second |
|---|---|---|---|---|
| LM P00 idle, 4000 cs | 4000 / 4000 | -0 / -0 | 0 | 40.3 |
| CM P00 idle, 4000 cs | 3998 / 4000 | -0 / -0 | 0 | 0.3 |
| LM V37E00E, 3000 cs | 3000 / 3000 | 00 / 00 | 0 | 43.2 |
| CM V37E00E, 3000 cs | 2998 / 3000 | 00 / 00 | 0 | 2.2 |
| LM V37E12E, 3000 cs | 3000 / 3000 | 14 / 14 | 0 | 43.1 |
| CM V37E06E, 3000 cs | 2998 / 3000 | 06 / 06 | 1 | 3.1 |
| LM V37E63E, 3000 cs | 3000 / 3000 | 77 / 77 | 0 | 43.1 |

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
| A | LM | T4RUPT_PROGRAM: GPMATRIX 06,3132 (84), IMUMON 06,2172 (84), RRAUTCHK 06,3006 (83). SERVICE_ROUTINES: GENTRAN+1 02,3545 refused by `lm_service_routines`, because `tools/agc_sites_generate.py` takes `TC GENTRAN` sites but not the two `TC GENTRAN +1` sites in AOSTASK_AND_AOSJOB (20,3574 and 20,3615; no other `TC <callee> +n` site exists in either mission). INTERPRETER: INTPRET 03,2037 refused by `lanec_ev_lm_intpret`, Q+1 03,2737. FRESH_START_AND_RESTART: R00+1 04,2144 and DUMMYAD 04,2146 refused by `lm_stateup_interpretive` |
| A | CM | T4RUPT_PROGRAM: IMUMON 06,2174 (84), OPTMON 06,2765 (83), OPTDRIVE 10,2000 (83). ALARM_AND_ABORT: ALARM 02,3650 (native code, no routine table). INTERPRETER: INTPRET 03,2006, Q+1 03,2706, and in V37E00E NOIBNKSW, SWSTORE, VLOAD+3, NEG4+1, TSSL+5, OPJUMP+2, VSHR2+11, VPDVL+3, NEWOPS+1, DSQSUB+15. FRESH_START_AND_RESTART: ROC+1 04,2131 and DUMMYAD 04,2133 refused by `cm_stateup_interpretive` |
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
- Rerun after each lane fills a point in; the target prints the remaining
  list.
