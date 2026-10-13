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
table index (`build/flight-store/<m>-host.inc`) for the differential.

The device record `include/flight_io.h` covers every lane's io kind. Two
new kinds, `FLIGHT_IO_LM_CHANNEL` and `FLIGHT_IO_CM_CHANNEL`, carry the
mission record followed by `agc_input_channels` (channels 3, 4, 30-33);
the builder picks them for any file that uses `<m>_channel_io` (CM
`FINETIME` in IMU_CALIBRATION needs it).

### Fit on port/c64-native-c a2aabc1f

| | Tables | Modules | Linked | Failed | Resolver records | Colliding keys | Store |
|---|---|---|---|---|---|---|---|
| LM | 112 | 62 | 17 | 45 | 223 | 650 (25 table pairs) | 277,504 bytes |
| CM | 88 | 51 | 15 | 36 | 184 | 405 (12 table pairs) | 251,904 bytes |

All but one failure per mission is a module bigger than its 16 KB slot
(median overflow about 34 KB, largest 136 KB LM and 170 KB CM; only 1 LM
and 3 CM are within 4 KB of fitting). The other is `<m>_self_check`, which
reads the rope and does not link outside the self-check build. The store
itself is far below the 2 MB REU. So the VICE whole-program run waits on the
modules fitting: per-file modules carry their whole link closure and their
compiled interpretive step machines, and that needs a steward decision
(chunking modules by routine, a shared runtime interpreter, or both).

## Host differential method

`tools/flight_host_run.c` (GPL, host only, links yaAGC; never part of the
port) runs one mission:

1. **Cold start.** Native and yaAGC both start at GOJAM with yaAGC's
   `agc_engine_init` state (channel 30 = 037777, channels 31-33 = 077777,
   no DOWNRUPT). Both missions take PTBAD (FAILREG 01107) and then
   DOFSTART, natively and in yaAGC alike.
2. **Native run.** The thin kernel and every packed table, resolved by the
   canonical key, run through `rk_rupt_take` and the trampoline.
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

`tools/flight_holes.py` names the file, label and owning lane of every
unregistered or refused point from the listing and the allocation.

## Results on a2aabc1f

| Scenario | Samples equal | Major mode native / yaAGC | Routines yaAGC did not match |
|---|---|---|---|
| LM P00 idle, 4000 cs | 4000 / 4000 | -0 / -0 | 0 |
| CM P00 idle, 4000 cs | 3998 / 4000 | -0 / -0 | 0 |
| LM V37E00E, 3000 cs | 3000 / 3000 | 00 / 00 | 0 |
| CM V37E00E, 3000 cs | 2998 / 3000 | 00 / 00 | 0 |
| LM V37E12E, 3000 cs | 3000 / 3000 | 14 / 14 | 0 |
| CM V37E06E, 3000 cs | 2998 / 3000 | 06 / 06 | 1 |

Findings:

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
| A | LM | T4RUPT_PROGRAM: GPMATRIX 06,3132 (84), IMUMON 06,2172 (84), RRAUTCHK 06,3006 (83). INTERPRETER: INTPRET 03,2037 refused by `lanec_ev_lm_intpret`, Q+1 03,2737. FRESH_START_AND_RESTART: R00+1 04,2144 and DUMMYAD 04,2146 refused by `lm_stateup_interpretive` |
| A | CM | T4RUPT_PROGRAM: IMUMON 06,2174 (84), OPTMON 06,2765 (83), OPTDRIVE 10,2000 (83). ALARM_AND_ABORT: ALARM 02,3650 (native code, no routine table). INTERPRETER: INTPRET 03,2006, Q+1 03,2706, and in V37E00E NOIBNKSW, SWSTORE, VLOAD+3, NEG4+1, TSSL+5, OPJUMP+2, VSHR2+11, VPDVL+3, NEWOPS+1, DSQSUB+15. FRESH_START_AND_RESTART: ROC+1 04,2131 and DUMMYAD 04,2133 refused by `cm_stateup_interpretive` |
| B | LM | INTEGRATION_INITIALIZATION: INTSTALL 13,3414/3415 and GOBAC+1 13,3464 refused by `lm_integration_init_*` |
| B | CM | INTEGRATION_INITIALIZATION (V37E00E): STATINT1, INTEGRV, MOVEPCSM, MOVEACSM, ATOPCSM+6, MOVEPLEM, ATOPLEM+6, MOVEALEM, ANDOUT+5/+7, GRP2PC+4, INTWAKE+11, WAKE1+4, INTSTALL, SETIFLGS, ENDINT, ENDINT+4/+11 |
| C | both | PINBALL_GAME_BUTTONS_AND_LIGHTS: CHARIN, LODSAMPT, ENTER, MMCHANG, MMCHANG+15, REQMM+10, 2BLANK, RELDSP, DSPMMJB, NVMONOPT, GOVNUPDT, UPDAT1+2, FLASHON, FLASHOFF, BLANKSUB, ENDIDLE; CM also DSPA, DSPOCTIN, OCTBACK, DSLV+1 |
| D1 | CM | KALCMANU_STEERING: STOPRATE 02,2526 refused by `cm_kalcmanu_steering_basic` |
| D2 | LM | AOSTASK_AND_AOSJOB: 1/ACCSET 20,2447, 1/ACCS+2, 1/ACCS+3, 1/ACCJOB+2, GETAOSUV+6, DBFUN+21, DOACCFUN+4, STORV+6. P20-P25: CLRADMOD 03,2011 |
| F | LM | RCS_FAILURE_MONITOR: RCSMON 06,3156 (83). P40-P47: ENGINOF1, ENGINOF3 (+4 to +21), ENGINOF4 refused by `lm_p40_p47_basic` |
| unallocated | CM | RTB_OP_CODES: LOADTIME 22,3510, SGNAGREE 22,3544 |

## Next (phase 2)

- Fit the store (steward decision above), then the same scenarios in VICE
  from the REU store.
- Rerun after each lane fills a point in; the target prints the remaining
  list.
