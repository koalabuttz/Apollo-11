# The native x86_64 flight program

Lane B, for David's second product target (roadmap, 2026-10-13): a native x86_64 build of the whole flight program, beside the C64. Design note: `/mnt/project-files/lane-b/x86-build.md`.

`make x86-flight` builds one program per mission and interpretive mode:

| Binary | Interpretive code |
|---|---|
| `build/x86-flight/x86-flight-lm`, `-cm` | The compiled routines of `tools/interp_compile.py` (and the other compiled tables). The interpreter core runs only the points no compiled table holds. |
| `build/x86-flight/x86-flight-lm-core`, `-cm-core` | The native interpreter core (`src/lm_interp_core.c`, `src/cm_interp_core.c`) runs every interpretive point, as on the C64. |

`tools/x86_flight_build.py --certified-only` also builds `-certified` binaries (`X86_CERTIFIED_ONLY`), which link no table of a file that `docs/port-coverage.json` does not list as complete.

```
x86-flight-lm ROPE.bin [CS [KEYS]]
```

It runs Luminary099 from GOJAM for CS centiseconds of AGC time (default 1000), pressing the DSKY KEYS (for example `V37E12E`) one every half second from 2 s. `@CS` in KEYS moves the next key and the ones after it to that centisecond or later (`V37E63E@9000P` presses PRO at 90 s). Environment:

| Variable | Effect |
|---|---|
| `X86_SPEED` | AGC time per wall time. 1 is real time (the default), 0 runs flat out. |
| `FLIGHT_CH30` to `FLIGHT_CH33` | Input channels 30-33 at start, in octal (`FLIGHT_CH30=036377 FLIGHT_CH31=057777`: IMU operate, PGNCS control, AUTO, so the LM DAP runs). |
| `FLIGHT_POKE` | `CS:ADDR=VALUE,...`: erasable words set on AGC time, in octal. |
| `FLIGHT_LOAD` | `CS:FILE,...`: the same from files of octal `ADDR VALUE` lines, a scenario's erasable load (`tools/flight_scenarios/`). |
| `FLIGHT_CHAN` | `CS:CH=VALUE,...`: input channel 16 or 30-33 set on AGC time, in octal, in time order. |
| `FLIGHT_ENGINE` ... `FLIGHT_SIVB`, `FLIGHT_RR`, `FLIGHT_LR` | The vehicle and radar models (below). |
| `X86_START` | A header `tools/flight_host_run.c`'s `FLIGHT_SNAPSHOT` wrote: start from that state (erasable memory, I/O channels, interrupt state and AGC time) instead of GOJAM, skipping the inputs due at or before it (below). |
| `FLIGHT_GOJAM` | `CS[,CS...]`: a hardware restart (yaAGC's GOJAM: Q = Z, Z = 04000, interrupts enabled and cleared, output channels zeroed), then the program's own restart code. |
| `FLIGHT_DOWNRUPT` | `1`: a DOWNRUPT every 20 ms of AGC time, so the downtelemetry program runs. |
| `X86_REPORT` | A JSON report: the points the interpreter core ran and why (no table, or the table refused), basic points no routine holds, the tables used with their certification, and the clock. |
| `X86_SAMPLES` | Erasable memory at the first idle pass through ADVAN in each centisecond, after that point's inputs and with nothing pending (as the harness's `FLIGHT_SNAPSHOT`). |
| `X86_SAMPLES_FROM` | The first centisecond `X86_SAMPLES` writes (default 1). |
| `X86_DSKY` | `1`: a live DSKY in the terminal (below). With no CS argument the run goes on until `q`, or until a pipe's last key has gone in. |
| `X86_TYPED` | Characters sent through the live DSKY's typed-key path from the start, as if typed (below). The lockstep check takes it too. |

## What links

The program links what `tools/flight_store_build.py --host-only` compiles: every `src/*.c` on the host, and every routine table in the order the flight store resolves them (`build/flight-store/<m>-host.inc`, resident tables first, then `docs/port-coverage.json` order). Each routine sees its own source's inline operands (`<m>-sites.inc`), as its C64 module does, and reads fixed memory from the rope image (`src/rope_image_host.c`). Self-check reads through its own rope hook. The program is MIT and links no Virtual AGC code.

A table's kind (basic, keyed by Z; or interpretive, keyed by LOC under BANKSET) comes from `interpretive_table()` in `tools/flight_store_build.py`. `tools/flight_table_kinds.py` checks every table's kind against its keys' words in the listing.

## How a step runs

`platform/x86/flight_dispatch.c` runs one step:

1. **Boundary.** It runs the clock steps due (`rk_timer_phase`: TIME4 at 2.5 ms, TIME1 and TIME3 at 5 ms, TIME5 at 10 ms), with `rk_rupt_take` before each, as the flight resident does, so a request taken holds the later steps back.
2. **Point.** If Z is the interpreter's fetch point, or DANZIG with EDOP set, the point is interpretive at LOC and BANKSET. Otherwise it is basic at Z.
3. **Routine.** Two direct-mapped indexes (basic and interpretive, by physical key) give the first table in resolution order that holds the point. The routine runs with its table's io record (`flight_io_enter`, `flight_io_leave`).
4. **Fallback.** For an interpretive point no table holds, or one whose routine refuses (its effects undone), one interpreter core slot runs from that point. Each such point is counted for the report. A basic point no routine holds stops the program: there is no bridge.

`platform/x86/flight_scenario.h` holds the scenario inputs (keys, pokes and loads, restarts, DOWNRUPT, the sample point), with `tools/flight_host_run.c`'s meanings, and the program and its lockstep check share it. At the first idle pass through ADVAN at or after their time it applies, in this order: the pokes and loads, the vehicle models, the channel changes (`FLIGHT_CHAN`), then a restart or else one key. This is the order `tools/flight_host_run.c` uses.

## The AGC clock

Each routine adds its cost to a virtual AGC clock (85,333 machine cycles a second). Every 853 1/3 cycles is one clock period, run at the next boundary, with no cap and nothing dropped. At `X86_SPEED` 1 the program sleeps whenever AGC time gets ahead of `CLOCK_MONOTONIC`. When the host falls behind, AGC time runs late (scaled time, docs/RESIDENT-FLIGHT.md). Keys and pokes arrive on AGC time.

The costs come from `docs/flight-routine-mct.json`: for each routine point, the machine cycles yaAGC took for it over the host differential's scenarios and the LM DAP holding and firing (median, runs, sum, sum of squares, least, most and the table that ran; `make x86-flight-mct`, `tools/x86_flight_mct.py`, using `FLIGHT_MCT_OUT` in `tools/flight_host_run.c`). The x86 clock charges each point its median. Points no scenario reaches cost the median of their kind. An interpreter core slot costs the cycles of the interpretive words it fetched: the core is built with `INTERP_WORD_CHARGE` (include/interp_word_mct.h), and the x86 program starts its count at 0 for each slot, as `tools/flight_host_run.c`'s `core_run` does off the work clock. Since #739 the files in `FLIGHT_INTERP_STOP` hand their interpretive code to the core, so this charge matters: left running, the count's bit 15 made the core skip NEWOPS, and it never fetched a word again (478,218 slots in `lm-doi-nassp`). The C64 resident's work clock takes its per-module weights from the same file.

## Acceptance: lockstep against yaAGC

`make x86-flight-check` (`tools/x86_flight_check.py`) builds both modes and the lockstep check `tools/x86_flight_lockstep.c`. That check is GPL, host only, and links yaAGC. It runs each scenario on the program's own step. Every routine and every core slot is checked: yaAGC, with its clock frozen (`AgcFrozenClock`, as under scaled time in docs/FLIGHT-STORE.md), runs from the state the routine started in until it reaches the routine's continuation with the same 2,048 erasable words (Z aside), I/O channels and interrupt state. Q has one exception. Native routines leave Q's 16th bit clear where yaAGC sign-extends bit 15 into it (LM P63's landing displays, LLGE and integration init, CM P11 and P22). Nothing reads that bit back except as a return address, and the host differential compares no central register. So the check accepts that exact difference (native bit 16 clear, yaAGC's bit 16 equal to its bit 15) and still fails any other. The clock steps and interrupt entries are the program's own `rk_timer_phase` and `rk_rupt_take`, which `make test-resident-rupt` checks against yaAGC. So a run with no mismatch is one yaAGC run whose clock ran on the x86 program's schedule. The I/O channels compared include the downlink words (channels 34 and 35), which carry the downtelemetry list under `FLIGHT_DOWNRUPT`. The check fails on any mismatch, any basic point no routine holds, or a run that stops early.

On base fea3e3b8:

| Scenario | Routines and core slots checked (compiled / core) | yaAGC did not match | Points the core ran (compiled / core) |
|---|---|---|---|
| LM P00, 4000 cs | 258,356 / 258,356 | 0 | 0 / 0 |
| CM P00, 4000 cs | 261,326 / 261,326 | 0 | 0 / 0 |
| LM DAP hold, 3000 cs | 177,110 / 177,110 | 0 | 0 / 0 |
| CM V37E00E, 3000 cs | 193,699 / 193,696 | 0 | 0 / 108 |
| CM V37E06E, 3000 cs | 195,334 / 195,310 | 0 | 0 / 4 |
| LM P00 with DOWNRUPT, 4000 cs | 257,741 / 257,741 | 0 | 0 / 0 |
| CM P00 with DOWNRUPT, 4000 cs | 259,481 / 259,481 | 0 | 0 / 0 |
| LM P00, GOJAM at 20 s, DOWNRUPT, 4000 cs | 257,518 / 257,518 | 0 | 0 / 0 |
| CM V37E06E at 25 s, GOJAM at 35 s, DOWNRUPT, 4000 cs | 258,548 / 258,519 | 0 | 0 / 4 |

On the same base, keys sent through the live DSKY's typed-key path (`X86_TYPED`, below), in both the x86 run and the lockstep:

| Scenario | Routines and core slots checked (compiled / core) | yaAGC did not match | Points the core ran (compiled / core) |
|---|---|---|---|
| LM typed `v35e`, 3000 cs | 193,532 / 193,532 | 0 | 0 / 0 |
| LM typed `v37e00ep` (PRO held), 3000 cs | 191,447 / 191,620 | 0 | 0 / 107 |
| CM typed `v37e06epmj` (PRO, MARK, MARK REJECT), 3000 cs | 195,045 / 195,013 | 0 | 0 / 4 |

On base fea3e3b8 the same check (compiled; not in CI) runs the 42 flight scenarios below that start from GOJAM and fly to their end: 13 LM and 29 CM, from 0.4 to 5.1 million routines each. No run shows a miss. The CM's channel 12 miss of earlier bases (zero ICDU in `cm_t4rupt_imu_routines`) is gone on this base: D1 fixed the channel copy in `src/flight_io.c` (#734).

Flat out, 40 s of AGC time takes about 0.05 s of host time.

CI runs `x86-flight-check` in the host job (`ci-extra-host`) when a PR changes any source, header or x86 tool (`ci-watch-x86-flight-check`).

## Vehicle models

`platform/x86/vehicle_models.h` (MIT, header only) holds the vehicle the AGC flies: the LM descent and ascent engines with touchdown, abort stage and the P66 crew (`FLIGHT_ENGINE`, `FLIGHT_ENGINE_LAG`, `FLIGHT_TOUCHDOWN`, `FLIGHT_ASCENT`, `FLIGHT_STAGE`, `FLIGHT_PILOT`), the LM RCS jets (`FLIGHT_RCS`), the CSM's SPS with its gimbal and the attitude the thrust turns (`FLIGHT_SPS=KG[,PITCH,YAW]`, the TVC DAP's closed loop of #786), entry drag and lift (`FLIGHT_AERO`), the IMU gimbals for P01 (`FLIGHT_IMU`), the Saturn V boost (`FLIGHT_SATURN`) and TLI (`FLIGHT_SIVB`). They were written for this port's flight scenarios and contain no Virtual AGC code. `platform/x86/test_vehicle_models.c` checks each one (`make x86-flight-check` builds and runs it first).

`platform/x86/radar_models.h` (MIT, header only) holds the rendezvous radar the same way (`FLIGHT_RR`, `FLIGHT_RR_DROP`): on the LM the rendezvous radar locked on the CSM, on the CM the VHF ranging link and the sextant. Both vehicles fly Kepler conics about the moon from the states `FLIGHT_RR` gives. The model points the antenna (or the sextant) along the true line of sight through REFSMMAT and the CDUs, sets the data-good bits, and answers each read with range or range rate in RNRAD. It is `tools/flight_radar.h`'s model, written for this port's rendezvous scenarios, with the same arithmetic. `platform/x86/test_radar_models.c` checks it, built once for each mission. Reads start at any routine boundary and the resident timer ends them, often before the next sample point, so the radar also answers a read after any routine where channel 13 bit 4 has come on. `FLIGHT_BUSY_SAMPLE` adds the harness's extra sample points for jobs that busy-wait for the crew (LM R22's V06N49 loop).

`platform/x86/landing_radar.h` (MIT, header only) holds the LM's landing radar (`FLIGHT_LR=CS[:DH]`, `FLIGHT_LR_POS2`, `FLIGHT_LR_DROP`): from `FLIGHT_LR`'s time, once descent navigation has an altitude (HCALC), it takes the LM's true state from the AGC's own state vector and flies it on with lunar gravity and the engines' and jets' velocity changes. Each read of the four beams answers with the slant range to the surface (`DH` metres above the landing site's radius) or a beam velocity, through REFSMMAT and the CDUs, high or low scale, with data good off in a dropout and after touchdown. `FLIGHT_LR_POS2` moves the antenna to position 2 at that time after the AGC commands it. It is `tools/flight_lr.h`'s model, written for this port's landing scenarios, with the same arithmetic. `platform/x86/test_landing_radar.c` checks it.

A scenario that starts from the NASSP P63 core dump (`FLIGHT_CORE`) needs yaAGC to load the dump, and the x86 program never reads it. `make x86-flight-scenarios` runs the harness with `FLIGHT_SNAPSHOT=0`, which writes the harness's state at its first sample point as a C header. The x86 program then starts from that header (`X86_START`), with AGC time counting from the dump's time as the harness counts it. The header is made at test time and is never kept or shipped. Inputs due at or before the start are skipped, since the header already holds them. The vehicle models start fresh at that point, which is before any of them run in these scenarios. The lockstep check starts only from GOJAM.

The x86 program runs them with the same settings and at the same points as the host differential (`tools/flight_host_run.c`, which still has its own copy): at each sample point after the pokes (the radar after the SPS), then the channel changes (`FLIGHT_CHAN`), then a restart or a key; with `FLIGHT_ENGINE_LAG` the engine, and always the RCS jets, also after every routine. Keys include the mark buttons (`M`, `J`; LM `x`, `y`, `j`: channel 16 and KEYRUPT2). Once each clock period, at the TIME5 step, the x86 program runs the output drives as the C64 resident and the harness do (`rk_cdu_drive`, `include/resident_cdu.h`): the IMU CDU drive and, in the CM, the optics drive, whose pulses the SPS gimbal follows in TVC mode. `X86_START` takes the drives' state from the header (`SNAP_CDU`).

`make x86-flight-scenarios` (`tools/x86_flight_scenarios.py`, not in CI) flies every host differential flight scenario the x86 program can run on its own and compares it with the harness on the work clock (`FLIGHT_SCALED=W`) at the first sample point at or after 1,000 cs before its end. The two programs charge routine costs differently: the x86 program charges each point's median, and the work clock charges each table's weight. So their states are close, not equal. The yardstick is the harness against itself, the same scenario on its other clock (`FLIGHT_SCALED=1,1,2`, yaAGC's own cycles). A scenario passes when all of these hold:

- the major mode is the same;
- RN and VN are within 4 times the harness's own spread, plus 50 m and 0.15 m/s for the per-routine charges the spread cannot see (the largest such case is CM P40: VN 0.13 m/s against a spread of 0.004);
- the erasable image (061-3777) differs in at most 1.5 times as many words as the harness's two clocks differ in, plus 10;
- the alarm codes (FAILREG and the two words after it) are the harness's, wherever its two clocks agree on them.

The alarm check is what sees the LM's RR dropout (`lm-rendezvous-rr-dropout`, D2's #748): with the dropout both runs raise R22's alarm 521, and with it left out of the x86 run only, that run raises none and fails (0376: 00521 against 00000). The state checks below don't see it.

The landing radar rows (`FLIGHT_LR`) also compare what the AGC takes from the radar. Both programs watch HMEAS (each altitude reading), VMEAS (each velocity reading), DELTAH (the altitude update a reading gives) and LRALTFLG (FLGWRD12 bit 5, no LR altitude data, which follows the range data-good bit): `X86_WATCH` in the x86 program, `FLIGHT_WATCH` with `FLIGHT_WATCH_STEP` in the harness. Both print the model's altitude readings with the true height and HCALC (`FLIGHT_LR_TRACE`). Readings are matched by time, within 100 cs. The x86 run may leave unmatched no more than the harness's two clocks do, plus 4 (a reading one cycle early or late at each data-good change). The median and 90th percentile of the differences must be within 4 times the harness's own, plus 0.2% and 0.5% (HMEAS, VMEAS) or 10 and 20 counts (DELTAH). LRALTFLG must change as often as on the harness, each time within twice the harness's own spread plus 250 cs. HCALC's error from the true height at each altitude reading must be within 4 times the harness's own difference plus 10 m. Each of these model changes, made in the x86 run only, fails every row that can see it (`lm-p63-lr-v58` stops before the velocity reads, and only `lm-p63-lr-dropout` has a dropout):

| Change to the x86 landing radar | `lm-p63-lr-dropout` | `lm-p63-lr-update` | `lm-p63-lr-v58` |
|---|---|---|---|
| Radar off | fails | fails | fails |
| Velocity readings 0 | fails | fails | passes (no velocity reads) |
| Velocity times 1.2 | fails | fails | passes (no velocity reads) |
| Range times 1.02 | fails | fails | fails |
| Data good stuck on | fails | fails | fails |
| Data good stuck off | fails | fails | fails |
| Dropout left out | fails | passes (no dropout) | passes (no dropout) |

With the rendezvous radar, the own state vector and its time (PIPTIME) move by amounts that depend on when each mark falls. Two good runs can then hold state vectors hundreds of kilometres apart, each correct for its own time. On LM P78 the harness's is at PIPTIME 19,234 cs and the x86 program's at 41,786, 369 km apart, and each is within 90 m of the true orbit. So when the harness's two clocks leave the state vector at different times, each run's state vector is compared with the true orbit at its own PIPTIME. The x86 program's error must be within 4 times the larger of the harness's two errors, plus 50 m and 0.15 m/s.

That check alone doesn't see the radar: with the radar off the own state stays as close to the true orbit. So every `FLIGHT_RR` scenario also checks both vehicles' permanent state blocks, the states the marks update (the CSM's at 01554 and the LM's at 01626 on both missions: the conic RCV/VCV plus the deviation DELTA/NUV at the block's time TET). Which block a mark updates is VEHUPFLG's choice: on the LM by default its own, on the CM the LM's. Each block is compared with its vehicle's true orbit at its TET. Its error may exceed the larger of the harness's two by at most 50 m and 0.15 m/s; the factor of 4 is left out here, because a target 300 m off moves the LM's own block by 270 to 300 m, inside 4 times the harness's error. Each TET must be no more than 1,000 cs behind the earlier of the harness's two. It may be ahead, because the x86 program marks at other times (LM P78: 41,786 against 19,234). Each of these breaks every rendezvous row on that check (x86 run only, harness unchanged):

| Change to the x86 run | The five LM rows (`lm-rendezvous`, `-rr-dropout`, `-tpm`, `lm-stable-orbit`, `-lm`) | The seven CM rows (`cm-p20`, `-vhf-dropout`, `-crew-verbs`, `cm-tpi-search`, `cm-stable-orbit`, `-lm`, `cm-rendezvous-tpm`) |
|---|---|---|
| Radar off | Both blocks' TET stays at 4,704 cs | The LM block 25.9 km off, or the run stops |
| Target moved 5 km | Both TETs stay at 4,704 cs, or the run stops | The LM block 3.2 to 5.1 km off |
| Target moved 300 m | The LM block 269 to 297 m off (limits 124 to 158) | The LM block 203 to 299 m off (limits 83 to 164) |
| Sextant marks (`M`) left out | (no `M` on the LM) | The LM block 0.5 to 3.6 km off, or its TET 5,800 cs too far behind (`-vhf-dropout`) |
| Every `V33E` left out | Both TETs stay at 4,704 cs | The LM block 1.8 km off, or the run stops |

The CM's VHF dropout (`cm-p20-vhf-dropout`) moves the marks and so both blocks.

The word count is what sees programs that don't move the state vector. With the mark buttons left out of the x86 run (`--drop-keys MJxyj`), P52 differs in 210 words against a limit of 19 and P57 in 188 against 37, so both fail. P22 and P23 fail on RN as well.

On base fea3e3b8 the tool flies 64 scenarios, and all pass. 57 reach their compare point, six of them as known with an envelope of their own (marked "known"; Gaps gives each cause). The other seven stop at a Pinball point not yet certified, exactly where expected (Gaps).

The NASSP rows start from the harness's state at the core dump's time (`X86_START`), and the `lm-p63-lr-*` rows fly the landing radar. So the LM RCS, the throttle lag, touchdown, the P66 crew, the ascent and the abort stage are now compared with the harness (`lm-p63-nassp`, `lm-p67-nassp`, `lm-landing-ascent-nassp`, `lm-abort-p70-nassp`, `lm-abort-p71-nassp`), as well as checked by `test_vehicle_models.c`.

Compared directly with the harness:

| Scenario | Major mode | RN apart (limit; harness spread) | VN apart (limit; spread) | Words differing (limit; spread) |
|---|---|---|---|---|
| `lm-p63-descent` | 77 / 77 | 23 m (646; 149) | 0.02 m/s (0.73; 0.14) | 150 (281; 181) |
| `lm-p63-descent-thrust` | 77 / 77 | 7 m (744; 174) | 0.16 m/s (1.43; 0.32) | 232 (404; 263) |
| `cm-entry` | 103 / 103 | 97 m (690; 160) | 2.05 m/s (11.12; 2.74) | 333 (448; 292) |
| `cm-p52` | 00 / 00 | 0 (50; 0) | 0 (0.15; 0.00) | 7 (23; 9) |
| `cm-p23` | 00 / 00 | 0 (50; 0) | 0 (0.15; 0.00) | 24 (47; 25) |
| `cm-p40` | 00 / 00 | 19 m (99; 12) | 0.13 m/s (0.16; 0.00) | 93 (121; 74) |
| `cm-p11` (known) | 00 / 00 | 437 m (447; 99) | 0.71 m/s (0.43; 0.07) | 174 (206; 131) |
| `cm-tli` | 00 / 00 | 148 m (506; 114) | 0.08 m/s (0.47; 0.08) | 113 (176; 111) |
| `cm-p22` | 00 / 00 | 20 m (130; 20) | 0.02 m/s (0.22; 0.02) | 225 (356; 231) |
| `cm-tei` | 00 / 00 | 14 m (61; 3) | 0.00 m/s (0.15; 0.00) | 132 (217; 138) |
| `cm-p37` | 00 / 00 | 0 (50; 0) | 0 (0.15; 0.00) | 39 (68; 39) |
| `lm-p57` | 00 / 00 | 0 (50; 0) | 0 (0.15; 0.00) | 16 (41; 21) |
| `lm-csi` | 110 / 110 | 0 (50; 0) | 0 (0.15; 0.00) | 11 (37; 18) |
| `cm-csi` | 40 / 40 | 0 (50; 0) | 0 (0.15; 0.00) | 3 (17; 5) |
| `cm-cdh` | 111 / 111 | 0 (50; 0) | 0 (0.15; 0.00) | 4 (19; 6) |
| `cm-cdh-csm` | 41 / 41 | 0 (50; 0) | 0 (0.15; 0.00) | 5 (19; 6) |
| `lm-p63-lr-dropout` | 100 / 100 | 140 m (386; 84) | 1.27 m/s (34.41; 8.56) | 325 (524; 343) |
| `lm-p63-lr-update` | 77 / 77 | 220 m (774; 181) | 1.36 m/s (3.75; 0.90) | 312 (464; 303) |
| `lm-p63-lr-v58` | 77 / 77 | 135 m (474; 106) | 0.48 m/s (2.25; 0.52) | 232 (425; 277) |
| `lm-p63-nassp` | 100 / 100 | 83 m (359; 77) | 2.01 m/s (41.69; 10.38) | 325 (502; 328) |
| `lm-p52-realign` | 64 / 64 | 0 (50; 0) | 0 (0.15; 0.00) | 13 (19; 6) |
| `lm-p52-imu-fail` | 64 / 64 | 0 (50; 0) | 0 (0.15; 0.00) | 13 (19; 6) |
| `lm-p52-imu-cage` (known) | 64 / 64 | 0 (50; 0) | 0 (0.15; 0.00) | 27 (22; 8) |
| `cm-p52-imu-fail` | 00 / 00 | 0 (50; 0) | 0 (0.15; 0.00) | 7 (23; 9) |
| `cm-p52-imu-cage` | 00 / 00 | 0 (50; 0) | 0 (0.15; 0.00) | 16 (34; 16) |
| `cm-p52-gimbal-lock` | 00 / 00 | 0 (50; 0) | 0 (0.15; 0.00) | 7 (20; 7) |
| `cm-p23-attached` | 00 / 00 | 0 (50; 0) | 0 (0.15; 0.00) | 29 (55; 30) |
| `cm-v49-maneuver` | 77777 / 77777 | 0 (50; 0) | 0 (0.15; 0.00) | 4 (14; 3) |
| `cm-p21` | 25 / 25 | 0 (50; 0) | 0 (0.15; 0.00) | 4 (16; 4) |
| `cm-p52-coarse` | 64 / 64 | 0 (50; 0) | 0 (0.15; 0.00) | 95 (143; 89) |
| `cm-p41` | 00 / 00 | 0 (50; 0) | 0 (0.15; 0.00) | 14 (43; 22) |
| `cm-v48-dap` | 77777 / 77777 | 0 (50; 0) | 0 (0.15; 0.00) | 0 (10; 0) |
| `cm-p40-sps-fail` | 00 / 00 | 0 (50; 0) | 0 (0.15; 0.00) | 11 (31; 14) |
| `lm-landing-ascent-nassp` | 14 / 14 | 182 m (330; 70) | 0.20 m/s (0.46; 0.08) | 353 (611; 401) |
| `lm-p67-nassp` | 104 / 104 | 19 m (171; 30) | 0.01 m/s (0.20; 0.01) | 316 (652; 428) |
| `lm-doi-nassp` | 00 / 00 | 2 m (58; 2) | 0.01 m/s (0.20; 0.01) | 189 (268; 172) |
| `lm-p41-nassp` | 00 / 00 | 16 m (111; 15) | 0.00 m/s (0.15; 0.00) | 169 (239; 153) |
| `lm-p42-nassp` | 00 / 00 | 13 m (116; 16) | 0.26 m/s (1.45; 0.33) | 137 (223; 142) |
| `lm-abort-p70-nassp` | 106 / 106 | 401 m (1467; 354) | 0.42 m/s (1.51; 0.34) | 264 (418; 272) |
| `lm-abort-p71-nassp` | 107 / 107 | 957 m (3011; 740) | 0.87 m/s (3.02; 0.72) | 342 (436; 284) |
| `lm-p63-descent-restart` | 77 / 77 | 7 m (796; 187) | 0.16 m/s (1.25; 0.28) | 212 (422; 275) |
| `lm-coast-verbs` | 77777 / 77777 | 0 (50; 0) | 0 (0.15; 0.00) | 49 (85; 50) |
| `lm-coast-programs` | 06 / 06 | 0 (50; 0) | 0 (0.15; 0.00) | 118 (175; 110) |
| `lm-p51` | 63 / 63 | 0 (50; 0) | 0 (0.15; 0.00) | 10 (17; 5) |
| `lm-p22-surface` | 26 / 26 | 0 (50; 0) | 0 (0.15; 0.00) | 8 (20; 7) |

CM P11's envelope is RN within 600 m and VN within 1 m/s, with the normal word limit. With the vehicle models removed, P11 ends 6,563 km off and fails (base f499d7ee).

The rendezvous scenarios whose state vector the marks move, compared with the true orbit. Each one's two state blocks are within their limits too (`build/x86-flight/scenarios/summary.json` lists them):

| Scenario | Major mode | RN from the true orbit (limit; harness's larger error) | VN (limit; harness) | Words differing (limit; harness's clocks) | Alarm codes (harness / x86) |
|---|---|---|---|---|---|
| `cm-p20` | 24 / 24 | 13 m (98; 12) | 0.07 m/s (0.41; 0.07) | 457 (602; 395) | same |
| `lm-rendezvous` | 42 / 42 | 16 m (456; 102) | 0.20 m/s (1.07; 0.23) | 674 (1004; 663) | same |
| `lm-rendezvous-rr-dropout` | 42 / 42 | 18 m (520; 118) | 0.19 m/s (0.95; 0.20) | 604 (973; 642) | same |
| `lm-rendezvous-tpm` (known) | 114 / 114 | 62 m (524; 119) | 0.25 m/s (1.16; 0.25) | 895 (1024; 676) | 0376 00000 / 00611 |
| `lm-stable-orbit` (known) | 117 / 117 | 85 m (377; 82) | 0.22 m/s (1.12; 0.24) | 835 (878; 579) | 0376 00000 / 00430 |
| `lm-stable-orbit-lm` (known) | 47 / 47 | 108 m (335; 71) | 0.28 m/s (1.07; 0.23) | 848 (928; 612) | 0376 00000 / 00430 |
| `cm-rendezvous-tpm` | 114 / 114 | 13 m (142; 23) | 0.09 m/s (0.51; 0.09) | 646 (938; 619) | same |
| `cm-stable-orbit` | 47 / 47 | 13 m (98; 12) | 0.07 m/s (0.41; 0.07) | 634 (964; 636) | same |
| `cm-stable-orbit-lm` | 117 / 117 | 13 m (98; 12) | 0.07 m/s (0.41; 0.07) | 648 (940; 620) | same |
| `cm-tpi-search` | 115 / 115 | 13 m (98; 12) | 0.07 m/s (0.41; 0.07) | 607 (950; 627) | same |
| `cm-p20-vhf-dropout` (known) | 24 / 24 | 28 m (129; 20) | 0.11 m/s (0.60; 0.11) | 636 (616; 404) | same |
| `cm-p20-crew-verbs` | 24 / 24 | 12 m (142; 23) | 0.09 m/s (0.50; 0.09) | 483 (692; 455) | 0377 40407 / 00407 |

## Live DSKY

`X86_DSKY=1 build/x86-flight/x86-flight-lm ROPE.bin` flies the LM in real time with a DSKY in the terminal (`platform/x86/terminal_dsky.h`, MIT). The display is the port's own DSKY device adapter (`src/dsky_display.c`, the one the C64 draws with): the relay words the program writes to channel 10 and the lamps of channel 11, drawn with ANSI escapes. Lit lamps show in bold yellow. Channel 13 bit 10 (TEST DSKY, set by the `V35E` lamp test) also lights RESTART and STBY. The LM's PRIO DISP and NO DAP (relay row 12, bits 1 and 2) are on the status line under the DSKY, because the adapter's 40x25 screen has no place for them. RESTART and STBY light only in the lamp test (Gaps).

| Key | DSKY |
|---|---|
| `0`-`9`, `+`, `-` | digits and signs |
| `v`, `n`, `e` or Enter | VERB, NOUN, ENTER |
| `c`, `r`, `k` | CLR, RSET, KEY REL |
| `p` | PRO (channel 32 bit 14 held low for half a second) |
| `m`, `j` | the CM's MARK and MARK REJECT (nothing on the LM) |
| `q` or Ctrl-C | quit |

Typed keys go in at idle sample points at least 20 cs of AGC time apart (scenario keys are 50 cs apart). Keys are read without waiting, from a terminal or a pipe, so the AGC clock never stops for input. Keys piped in (`printf v35e | X86_DSKY=1 ...`) work too; with no CS argument, a run whose pipe has ended stops once its last key has gone in. `X86_TYPED=chars` sends the same characters through the same path from the start without a terminal, and the lockstep check takes it too (the typed rows above). KEYS and the `FLIGHT_*` inputs still apply. The lamp test (`V35E`), `V37E` program changes and octal displays (`V01N01E`, `V05N09E`) run on both programs. Decimal noun displays don't yet (Gaps).

## Gaps

On base fea3e3b8:

- **CM P11 boost:** at 89,001 cs the x86 run is 437 m and 0.72 m/s from the harness, against a spread of 99 m and 0.07 m/s between the harness's own two clocks. The cause is when the AGC sees liftoff. The gyrocompass loop polls the liftoff discrete (channel 30 bit 5), and P11 zeroes its clock and navigates from the time it saw it. With `FLIGHT_SATURN` lifting off at 10,000 cs, TLIFTOFF is 10,005 on the harness's work clock, 10,022 on its other clock, and 10,038 on the x86 program's clock. So the state vector carries that offset from liftoff on, and it grows to orbit. This comes from the clock schedule, not from a routine computing differently. Comparing at equal time since liftoff, or making the x86 schedule closer to yaAGC's, would bring P11 inside the limits.
- **Inputs at fixed times on another schedule:** a scenario's keys and channel changes go in at fixed centiseconds, and the x86 program's clock runs its jobs at slightly other times than the harness's. Where that moves an input across a display or a DAP pass, the run takes another path. The lockstep check shows no miss on any of these, and the tool lists them as known with the cause:
  - `lm-p52-imu-cage`: the cage (channel 30 at 3,000 cs) reaches the DAP at the x86 program's sample point 3,000 and the harness's 3,002, so the DAP counts one trap fewer and its jet words differ (27 words against a limit of 22). The state vector is the same.
  - `cm-p20-vhf-dropout`: the marks fall at other times (both state blocks at 30,348 cs against the harness's 30,669 and 30,678, each within its limits of the true orbit), so 636 words differ against a limit of 616.
  - `lm-rendezvous-tpm`, `lm-stable-orbit`, `lm-stable-orbit-lm`: after a V25's last ENTER the x86 program stays in P20/R22 for 13 s or more before the program redisplays its noun, where both harness clocks redisplay within 0.75 s. So the next V33 lands on a blank display, and from there each key answers a display other than the harness's. The x86 keys go in on time (19 to 50 cs after the harness's); the harness is the one late on `lm-stable-orbit`'s V37E78E (143 cs on the work clock, 112 on the other, busy at 8,000). In `lm-rendezvous-tpm` the V33 at 47,500 cs comes before P74's N37 is back (48,692 on x86), so the V25 at 50,000 loads N37 instead of N55, with no elevation angle, and P74 raises alarm 611. In `lm-stable-orbit` and `-lm` the x86 run keeps marking on interim passes to TET 41,786 cs (the harness reaches its final pass at 23,100), and P79's integration raises alarm 430. Each is allowed exactly that alarm. The radar turned off, the target moved 300 m, the `V33E`s left out or any other alarm still fail them.

  Keys sent when a display asks for them, as a crew would (in the harness and the scenario inputs), should fly these as the harness does; moving the x86 keys would not. (Reviewer 1 measured these times on #768: `X86_SAMPLES` times are labels of the first idle point at or after a centisecond, so a busy stretch makes them earlier than the moment the state was taken.)
- **Pinball points without a certified routine:** these stop in the x86 program where the harness runs yaAGC. The tool passes each scenario only when it stops at exactly that point, so any other outcome, a full run included, fails until its entry is removed. Lane C's Pinball LM and CM certificate (#674) keys or certifies all of them:
  - `lm-cdh`, `lm-cdh-csm`: P33's decimal display (DECDSP, 41,2523).
  - `lm-p47-nassp`: P47's display (41,3321).
  - `lm-coast-v55`: V55's display (42,3452).
  - `lm-p65-autoland-nassp`: P65's display (40,2732).
  - `lm-coast-r04`, `lm-p63-v60-nassp`: Pinball's EV TC Q (Z 00002, FB 66000: 50,2002), which no table keys.
  - In the live DSKY, decimal noun displays (`V06N36E`, `V16N36E`, `V16N65E`) stop at 42,3230 (LM) or 42,3143 (CM).
- **The TC trap:** V69 (VERB69, a TC to itself) restarts the AGC through the hardware's TC trap, which the harness and the C64 now model (#752). The x86 program has no TC trap yet, and no scenario here keys V69 (`lm-v69`, `cm-v69` are restart-check rows).
- **RESTART and STBY lamps** light only in the lamp test. The x86 program models no hardware restart lamp (set by a GOJAM, cleared by RSET) and no standby.
- **Missing models:** uplink (`FLIGHT_UPLINK`) and stolen time (`FLIGHT_STEAL`); the tool leaves those scenarios out.
- The uncompiled interpretive code by file is listed in the design note.
