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

It runs Luminary099 from GOJAM for CS centiseconds of AGC time (default 1000), pressing the DSKY KEYS (for example `V37E12E`) one every half second from 2 s. `@CS` in KEYS moves the next key and the ones after it to that centisecond or later (`V37E63E@9000R` presses RSET at 90 s; `P` is RSET too, as in the harness). `!` before a key makes it wait past its time until the verb and noun flash (channel 11 bit 6, the crew's cue), and moves the keys after it later by as long; the harness takes `!` the same way. Environment:

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

Each routine adds its cost to a virtual AGC clock (85,333 machine cycles a second), and the clock steps (TIME4, TIME1/TIME3, TIME5, three to a 10 ms period) run at the next boundary, with no cap and nothing dropped. At `X86_SPEED` 1 the program sleeps whenever AGC time gets ahead of `CLOCK_MONOTONIC`. When the host falls behind, AGC time runs late (scaled time, docs/RESIDENT-FLIGHT.md). Keys and pokes arrive on AGC time.

The program charges time as the C64 resident's work clock does, which is also the harness's `FLIGHT_SCALED=W` (`fd_work_clock`, `tools/flight_host_run.c`):

- each routine costs its table's weight (host.inc's work clock MCT, from `docs/flight-routine-mct.json`); the idle loop's tables (weight 0) run to the end of the clock period, which the next boundary drops if it takes a request first;
- a table left to the interpreter module (weight `FD_PER_WORD`) runs through the interpreter core, slot after slot, until the words it charged reach the next clock step (at most 64 slots, the first always), as the C64's interpreter module runs on its step budget; so does an interpretive point no table holds;
- the clock steps end at whole cycles (214, 427, 854, ... of each 2,560), as the harness's;
- a slot 5 table (a hot copy inside a module) holds its points only while its module is in slot 0, and the program follows the module loads to know which is, as the harness's `holder` and `module_load`;
- a sample point's inputs go in before the boundary's clock steps, as the harness's, so a key that ends the idle loop's period early ends it at the same cycle.

The CM, and most LM scenarios, then run the same routines at the same cycles as the harness: CM entry was checked routine by routine to 27 s with `X86_PATH` against the harness's `FLIGHT_PATH`. A `FLIGHT_INTERP_STOP` file's basic table (`FD_PLUS_WORDS`, five LM tables) costs only its weight here, where the harness adds the interpretive words yaAGC ran inside it, which this program cannot count (Gaps).

The lockstep check below keeps the measured clock: each routine point costs the median machine cycles yaAGC took for it over the host differential's scenarios (`docs/flight-routine-mct.json`, `make x86-flight-mct`, `tools/x86_flight_mct.py`, using `FLIGHT_MCT_OUT` in `tools/flight_host_run.c`), points no scenario reaches the median of their kind, and an interpreter core slot the cycles of the words it fetched, its count started at 0 for each slot as `core_run` does off the work clock (left running, the count's bit 15 made the core skip NEWOPS, and it never fetched a word again: 478,218 slots in `lm-doi-nassp`).

## Acceptance: lockstep against yaAGC

`make x86-flight-check` (`tools/x86_flight_check.py`) builds both modes and the lockstep check `tools/x86_flight_lockstep.c`. That check is GPL, host only, and links yaAGC. It runs each scenario on the program's own step. Every routine and every core slot is checked: yaAGC, with its clock frozen (`AgcFrozenClock`, as under scaled time in docs/FLIGHT-STORE.md), runs from the state the routine started in until it reaches the routine's continuation with the same 2,048 erasable words (Z aside), I/O channels and interrupt state. Q has one exception. Native routines leave Q's 16th bit clear where yaAGC sign-extends bit 15 into it (LM P63's landing displays, LLGE and integration init, CM P11 and P22). Nothing reads that bit back except as a return address, and the host differential compares no central register. So the check accepts that exact difference (native bit 16 clear, yaAGC's bit 16 equal to its bit 15) and still fails any other. The clock steps and interrupt entries are the program's own `rk_timer_phase` and `rk_rupt_take`, which `make test-resident-rupt` checks against yaAGC. So a run with no mismatch is one yaAGC run whose clock ran on the x86 program's schedule. The I/O channels compared include the downlink words (channels 34 and 35), which carry the downtelemetry list under `FLIGHT_DOWNRUPT`. The check fails on any mismatch, any basic point no routine holds, or a run that stops early.

On base 085a9162 (the counts of the older rows are those of fea3e3b8):

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
| LM P00, V69E at 23.5 s (TC trap), 4000 cs | 257,961 / 257,961 | 0 | 0 / 0 |
| CM P00, V69E at 23.5 s (TC trap), 4000 cs | 261,034 / 261,034 | 0 | 0 / 0 |

In the V69 rows the spin at VERB69 is checked like any routine: yaAGC's TC VERB69 must leave the same Q (02040). The trap's restart is the x86 program's own, since yaAGC's clock is frozen and its trap cannot fire, and the run from 04000 on is checked as after `FLIGHT_GOJAM`. With the spin's Q set to 02041 on the x86 side only, all four V69 runs fail.

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

The x86 program runs them with the same settings and at the same points as the host differential (`tools/flight_host_run.c`, which still has its own copy): at each sample point after the pokes (the radar after the SPS), then the channel changes (`FLIGHT_CHAN`), then a restart or a key; with `FLIGHT_ENGINE_LAG` the engine, and always the RCS jets, also after every routine. Keys include the mark buttons (`M`, `J`; LM `x`, `y`, `j`: channel 16 and KEYRUPT2). Once each clock period, at the TIME5 step, the x86 program runs the output drives as the C64 resident and the harness do (`rk_cdu_drive`, `include/resident_cdu.h`): the IMU CDU drive and, in the CM, the optics drive, whose pulses the SPS gimbal follows in TVC mode. `X86_START` takes the drives' state from the header (`SNAP_CDU`). The TC trap is modelled as the harness and the C64 model it (#752, docs/FLIGHT-RESTART.md). V69's VERB69 (43,2037) is a TC to itself, so outside an interrupt the x86 program spins there as the idle loop does, to the end of the clock period, with Q 02040. When the spin held the period's T3 step and no interrupt came in between, the next step is the trap's GOJAM, as yaAGC's trap fires at scaler phase 000 after nothing but TCs since phase 020. `platform/x86/tc_trap.h` makes that decision, and `platform/x86/test_tc_trap.c` checks it in `x86-flight-check`: where the step spins, when it takes the GOJAM, and that an interrupt, another bank or a bank without the superbank bit does neither.

`make x86-flight-scenarios` (`tools/x86_flight_scenarios.py`, not in CI) flies every host differential flight scenario the x86 program can run on its own and compares it with the harness on the work clock (`FLIGHT_SCALED=W`) at the first sample point at or after 1,000 cs before its end. The two programs charge time the same way, so most scenarios end in the same state, and the rest close to it. The yardstick is the harness against itself, the same scenario on its other clock (`FLIGHT_SCALED=1,1,2`, yaAGC's own cycles). A scenario passes when all of these hold:

- the major mode is the same;
- RN and VN are within 4 times the harness's own spread, plus 50 m and 0.15 m/s for the per-routine charges the spread cannot see (CM P40 needed it before #786 closed the TVC loop: VN 0.13 m/s apart against a spread of 0.004; on base fccb0653 every row's RN and VN are within 4 times the spread);
- the erasable image (061-3777) differs in at most 1.5 times as many words as the harness's two clocks differ in, plus 10;
- the alarm codes (FAILREG and the two words after it) are the harness's, wherever its two clocks agree on them.

The night watchman is modelled as in the harness (`platform/x86/night_watchman.h`, docs/FLIGHT-RESTART.md, The night watchman). A step from a basic point outside an interrupt that leaves the erasable image as it found it, with a job active, is stuck: it spins to the end of the clock period, as the TC trap's spin does. An exit(3) from `lmi_goto_any` inside a routine (an erasable CADR cycle) is such a step. On the work clock a table left to the interpreter runs through the core, which stops after each GOTOERS hop at GOTO. At a basic point no table holds (GOTO is one), the program runs the core, as the harness's `core_entry` does, on either clock, but keeps the step only when it is stuck: any other such point stays a gap. Each point a cycle starts at is printed ("erasable CADR cycle at Z ... FB ... LOC ..."). The x86 program links with `--wrap=exit` (`tools/x86_flight_build.py` LINK). When the watchman sounds at a period's end, the next step is its GOJAM, and the program prints "night watchman at N cs". The `lm-p52-watchman` row requires it within the TC trap rows' limit of the harness's.

The V69 rows (`lm-v69`, `cm-v69`: P00, then V69E at 23.5 s) take their spread from the work clock against unscaled time, yaAGC's own clock and its own TC trap, because the harness's 1,1,2 clock takes no trap. They also require the TC trap in the x86 run, within 4 times the harness's two trap times apart plus 10 cs. With the trap's GOJAM left out of the x86 program, both stop spinning at VERB69 and fail.

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

On base 7f7f89fa the tool flies 78 scenarios, and all pass with no envelope of their own. 62 end in the same erasable image as the harness on the work clock, every CM scenario among them; the other 13 are LM (Gaps). The seven that stopped at an uncertified Pinball point before Pinball's certificates (#674) fly to the end. Three more rows fly `lm-rendezvous-tpm`, `lm-stable-orbit` and `lm-stable-orbit-lm` with each V33 a display-driven key (`!V33E`, the `-wait` rows): both programs hold the key until the verb and noun flash, as a crew answers the display rather than the clock. The two stable-orbit `-wait` rows end 8 and 17 words from the harness, so their word limit is capped at 100 (`WORD_CAP`); with the x86 program ignoring `!` (`--drop-keys !`) they end over 500 words apart and fail, as does `lm-rendezvous-tpm-wait` on its LM state block.

The nine restart rows (`fhd.RESTART_FLIGHT`: a `FLIGHT_GOJAM` in each phase, the same rows `tools/flight_restart_check.py` checks against yaAGC) are flown too. Every row with a restart, these, `lm-p63-descent-restart` and the V69 rows, must also count as many restarts as the harness on one of its clocks (REDOCTR: LM 0320, CM 01154), because a restart that resumes the same program can leave the state vector and most words as they were. With `FLIGHT_GOJAM` ignored in the x86 run only, 8 of those 10 `FLIGHT_GOJAM` rows fail on REDOCTR (only 2 failed before the check). The other two can't see it: `lm-p00-doi-restart`'s harness clocks count 0 and 1 at the compare point, and `cm-p00-p40-restart`'s compare point is the GOJAM itself. Like the harness's FLIGHT_SNAPSHOT, the x86 program's samples are taken after a sample point's restart (nothing is pending after a GOJAM); before, it sampled one step late at the GOJAM and `cm-p00-p40-restart` differed in 171 words against a limit of 163.

The NASSP rows start from the harness's state at the core dump's time (`X86_START`), and the `lm-p63-lr-*` rows fly the landing radar. So the LM RCS, the throttle lag, touchdown, the P66 crew, the ascent and the abort stage are now compared with the harness (`lm-p63-nassp`, `lm-p67-nassp`, `lm-landing-ascent-nassp`, `lm-abort-p70-nassp`, `lm-abort-p71-nassp`), as well as checked by `test_vehicle_models.c`.

Compared directly with the harness:

| Scenario | Major mode | RN apart (limit; harness spread) | VN apart (limit; spread) | Words differing (limit; spread) |
|---|---|---|---|---|
| `lm-p63-descent` | 77 / 77 | 0 (646; 149) | 0 (0.73; 0.14) | 0 (281; 181) |
| `lm-p63-descent-thrust` | 77 / 77 | 0 (744; 174) | 0 (1.43; 0.32) | 0 (404; 263) |
| `cm-entry` | 103 / 103 | 0 (690; 160) | 0 (11.11; 2.74) | 0 (448; 292) |
| `cm-p52` | 00 / 00 | 0 (50; 0) | 0 (0.15; 0.00) | 0 (23; 9) |
| `cm-p23` | 00 / 00 | 0 (50; 0) | 0 (0.15; 0.00) | 0 (47; 25) |
| `cm-p40` | 00 / 00 | 0 (68; 4) | 0 (0.39; 0.06) | 0 (163; 102) |
| `cm-p11` | 00 / 00 | 0 (447; 99) | 0 (0.43; 0.07) | 0 (206; 131) |
| `cm-tli` | 00 / 00 | 0 (506; 114) | 0 (0.47; 0.08) | 0 (176; 111) |
| `cm-p22` | 00 / 00 | 0 (130; 20) | 0 (0.22; 0.02) | 0 (356; 231) |
| `cm-tei` | 00 / 00 | 0 (96; 12) | 0 (0.38; 0.06) | 0 (242; 155) |
| `cm-p37` | 00 / 00 | 0 (50; 0) | 0 (0.15; 0.00) | 0 (68; 39) |
| `lm-p57` | 00 / 00 | 0 (50; 0) | 0 (0.15; 0.00) | 0 (41; 21) |
| `lm-csi` | 110 / 110 | 0 (50; 0) | 0 (0.15; 0.00) | 0 (37; 18) |
| `cm-csi` | 40 / 40 | 0 (50; 0) | 0 (0.15; 0.00) | 0 (17; 5) |
| `cm-cdh` | 111 / 111 | 0 (50; 0) | 0 (0.15; 0.00) | 0 (19; 6) |
| `cm-cdh-csm` | 41 / 41 | 0 (50; 0) | 0 (0.15; 0.00) | 0 (19; 6) |
| `lm-p63-lr-dropout` | 100 / 100 | 0 (386; 84) | 0 (34.41; 8.56) | 0 (524; 343) |
| `lm-p63-lr-update` | 77 / 77 | 3 m (775; 181) | 0.02 m/s (3.75; 0.90) | 145 (464; 303) |
| `lm-p63-lr-v58` | 77 / 77 | 0 (474; 106) | 0 (2.25; 0.52) | 0 (425; 277) |
| `lm-p63-nassp` | 100 / 100 | 0 (359; 77) | 0 (41.69; 10.38) | 0 (502; 328) |
| `lm-p52-realign` | 64 / 64 | 0 (50; 0) | 0 (0.15; 0.00) | 0 (19; 6) |
| `lm-p52-imu-fail` | 64 / 64 | 0 (50; 0) | 0 (0.15; 0.00) | 0 (19; 6) |
| `lm-p52-imu-cage` | 64 / 64 | 0 (50; 0) | 0 (0.15; 0.00) | 0 (22; 8) |
| `cm-p52-imu-fail` | 00 / 00 | 0 (50; 0) | 0 (0.15; 0.00) | 0 (23; 9) |
| `cm-p52-imu-cage` | 00 / 00 | 0 (50; 0) | 0 (0.15; 0.00) | 0 (34; 16) |
| `cm-p52-gimbal-lock` | 00 / 00 | 0 (50; 0) | 0 (0.15; 0.00) | 0 (20; 7) |
| `cm-p23-attached` | 00 / 00 | 0 (50; 0) | 0 (0.15; 0.00) | 0 (55; 30) |
| `cm-v49-maneuver` | 77777 / 77777 | 0 (50; 0) | 0 (0.15; 0.00) | 0 (14; 3) |
| `cm-p21` | 25 / 25 | 0 (50; 0) | 0 (0.15; 0.00) | 0 (16; 4) |
| `cm-p52-coarse` | 64 / 64 | 0 (50; 0) | 0 (0.15; 0.00) | 0 (143; 89) |
| `cm-p41` | 00 / 00 | 0 (50; 0) | 0 (0.15; 0.00) | 0 (43; 22) |
| `cm-v48-dap` | 77777 / 77777 | 0 (50; 0) | 0 (0.15; 0.00) | 0 (10; 0) |
| `cm-p40-sps-fail` | 00 / 00 | 0 (50; 0) | 0 (0.15; 0.00) | 0 (28; 12) |
| `lm-landing-ascent-nassp` | 14 / 14 | 2 m (330; 70) | 0.01 m/s (0.46; 0.08) | 117 (611; 401) |
| `lm-p67-nassp` | 104 / 104 | 0 (171; 30) | 0.01 m/s (0.20; 0.01) | 83 (652; 428) |
| `lm-doi-nassp` | 00 / 00 | 0 (58; 2) | 0 (0.20; 0.01) | 0 (268; 172) |
| `lm-p41-nassp` | 00 / 00 | 0 (111; 15) | 0 (0.15; 0.00) | 0 (239; 153) |
| `lm-p42-nassp` | 00 / 00 | 0 (116; 16) | 0 (1.45; 0.33) | 0 (223; 142) |
| `lm-abort-p70-nassp` | 106 / 106 | 8 m (1467; 354) | 0.03 m/s (1.51; 0.34) | 123 (418; 272) |
| `lm-abort-p71-nassp` | 107 / 107 | 3 m (3011; 740) | 0.01 m/s (3.02; 0.72) | 150 (436; 284) |
| `lm-p63-descent-restart` | 77 / 77 | 0 (796; 187) | 0 (1.25; 0.28) | 0 (422; 275) |
| `lm-p00-doi-restart` | 00 / 00 | 0 (58; 2) | 0 (0.20; 0.01) | 0 (361; 234) |
| `lm-doi-burn-restart` | 50 / 50 | 0 (117; 17) | 0 (0.19; 0.01) | 0 (218; 139) |
| `lm-p12-restart` | 14 / 14 | 7 m (220; 42) | 0.01 m/s (0.45; 0.07) | 123 (622; 408) |
| `cm-p00-p40-restart` | 00 / 00 | 0 (68; 4) | 0 (0.39; 0.06) | 0 (163; 102) |
| `cm-p40-burn-restart` | 50 / 50 | 0 (83; 8) | 0 (0.48; 0.08) | 0 (229; 146) |
| `cm-entry-restart` | 103 / 103 | 0 (541; 123) | 0 (5.05; 1.23) | 0 (425; 277) |
| `cm-p22-restart` | 26 / 26 | 0 (50; 0) | 0 (0.23; 0.02) | 0 (118; 72) |
| `lm-coast-verbs` | 77777 / 77777 | 0 (50; 0) | 0 (0.15; 0.00) | 0 (85; 50) |
| `lm-coast-programs` | 06 / 06 | 0 (50; 0) | 0 (0.15; 0.00) | 0 (175; 110) |
| `lm-p51` | 63 / 63 | 0 (50; 0) | 0 (0.15; 0.00) | 0 (17; 5) |
| `lm-p22-surface` | 26 / 26 | 0 (50; 0) | 0 (0.15; 0.00) | 0 (20; 7) |
| `lm-v69` | 77777 / 77777 | 0 (50; 0) | 0 (0.15; 0.00) | 0 (10; 0) |
| `cm-v69` | 77777 / 77777 | 0 (50; 0) | 0 (0.15; 0.00) | 0 (10; 0) |
| `lm-cdh` | 41 / 41 | 0 (50; 0) | 0 (0.15; 0.00) | 0 (32; 15) |
| `lm-cdh-csm` | 111 / 111 | 0 (50; 0) | 0 (0.15; 0.00) | 0 (28; 12) |
| `lm-p47-nassp` | 00 / 00 | 0 (75; 6) | 0 (0.21; 0.01) | 0 (115; 70) |
| `lm-p63-v60-nassp` | 100 / 100 | 0 (611; 140) | 0 (32.83; 8.17) | 0 (560; 367) |
| `lm-p65-autoland-nassp` | 104 / 104 | 2 m (83; 8) | 0.01 m/s (0.24; 0.02) | 139 (625; 410) |
| `lm-coast-v55` | 77777 / 77777 | 0 (50; 0) | 0 (0.15; 0.00) | 0 (11; 1) |
| `lm-coast-r04` | 77777 / 77777 | 0 (50; 0) | 0 (0.15; 0.00) | 0 (16; 4) |

With the SPS gimbal held at zero in the x86 run only (TVC enable ignored), `cm-p40` ends 195 km and 853 m/s from the harness and `cm-tei` 120 km and 976 m/s, each with alarm 1407, so both fail (base fc7ded5a).

With the vehicle models removed, P11 ends 6,563 km off and fails (base f499d7ee).

The rendezvous scenarios whose state vector the marks move, compared with the true orbit. Each one's two state blocks are within their limits too (`build/x86-flight/scenarios/summary.json` lists them):

| Scenario | Major mode | RN from the true orbit (limit; harness's larger error) | VN (limit; harness) | Words differing (limit; harness's clocks) | Alarm codes (harness / x86) |
|---|---|---|---|---|---|
| `cm-p20` | 24 / 24 | 11 m (98; 12) | 0.06 m/s (0.41; 0.07) | 0 (602; 395) | same |
| `lm-rendezvous-restart` | 24 / 24 | 47 m (270; 55) | 0.30 m/s (1.64; 0.37) | 17 (1000; 660) | same |
| `cm-p20-restart` | 24 / 24 | 11 m (98; 12) | 0.06 m/s (0.41; 0.07) | 0 (589; 386) | same |
| `lm-rendezvous` | 42 / 42 | 102 m (456; 102) | 0.16 m/s (1.07; 0.23) | 13 (1004; 663) | same |
| `lm-rendezvous-rr-dropout` | 42 / 42 | 111 m (521; 118) | 0.19 m/s (0.95; 0.20) | 1 (973; 642) | same |
| `lm-rendezvous-tpm` | 114 / 114 | 50 m (525; 119) | 0.25 m/s (1.16; 0.25) | 431 (1024; 676) | same |
| `lm-stable-orbit` | 117 / 117 | 50 m (377; 82) | 0.14 m/s (1.12; 0.24) | 323 (878; 579) | same |
| `lm-stable-orbit-lm` | 47 / 47 | 50 m (335; 71) | 0.14 m/s (1.07; 0.23) | 373 (928; 612) | same |
| `cm-rendezvous-tpm` | 114 / 114 | 23 m (142; 23) | 0.09 m/s (0.51; 0.09) | 0 (938; 619) | same |
| `cm-stable-orbit` | 47 / 47 | 11 m (98; 12) | 0.06 m/s (0.41; 0.07) | 0 (964; 636) | same |
| `cm-stable-orbit-lm` | 117 / 117 | 11 m (98; 12) | 0.06 m/s (0.41; 0.07) | 0 (940; 620) | same |
| `cm-tpi-search` | 115 / 115 | 11 m (98; 12) | 0.06 m/s (0.41; 0.07) | 0 (950; 627) | same |
| `cm-p20-vhf-dropout` | 24 / 24 | 20 m (129; 20) | 0.11 m/s (0.60; 0.11) | 0 (616; 404) | same |
| `cm-p20-crew-verbs` | 24 / 24 | 23 m (142; 23) | 0.09 m/s (0.50; 0.09) | 0 (692; 455) | same |

## Live DSKY

`X86_DSKY=1 build/x86-flight/x86-flight-lm ROPE.bin` flies the LM in real time with a DSKY in the terminal (`platform/x86/terminal_dsky.h`, MIT). The display is the port's own DSKY device adapter (`src/dsky_display.c`, the one the C64 draws with): the relay words the program writes to channel 10 and the lamps of channel 11, drawn with ANSI escapes. Lit lamps show in bold yellow. Channel 13 bit 10 (TEST DSKY, set by the `V35E` lamp test) also lights RESTART and STBY. The LM's PRIO DISP and NO DAP (relay row 12, bits 1 and 2) are on the status line under the DSKY, because the adapter's 40x25 screen has no place for them. RESTART is the hardware restart lamp, as in yaAGC: power-up and every GOJAM (`FLIGHT_GOJAM`, the TC trap) light it, and RSET or a channel 11 bit 10 write puts it out. The x86 program prints "RESTART lamp on/off at N cs" when it changes, the lockstep check compares it with yaAGC's after every routine, and the `v69-rset` rows check it goes out on RSET, comes back with the trap and goes out again. `platform/x86/test_restart_lamp.c` checks each light and clear path on its own, since in flight RSET always takes both clear paths at once. STBY lights only in the lamp test (Gaps).

| Key | DSKY |
|---|---|
| `0`-`9`, `+`, `-` | digits and signs |
| `v`, `n`, `e` or Enter | VERB, NOUN, ENTER |
| `c`, `r`, `k` | CLR, RSET, KEY REL |
| `p` | PRO (channel 32 bit 14 held low for half a second) |
| `m`, `j` | the CM's MARK and MARK REJECT (nothing on the LM) |
| `q` or Ctrl-C | quit |

Typed keys go in at idle sample points at least 20 cs of AGC time apart (scenario keys are 50 cs apart). Keys are read without waiting, from a terminal or a pipe, so the AGC clock never stops for input. Keys piped in (`printf v35e | X86_DSKY=1 ...`) work too; with no CS argument, a run whose pipe has ended stops once its last key has gone in. `X86_TYPED=chars` sends the same characters through the same path from the start without a terminal, and the lockstep check takes it too (the typed rows above). KEYS and the `FLIGHT_*` inputs still apply. The lamp test (`V35E`), `V37E` program changes, octal displays (`V01N01E`, `V05N09E`) and decimal displays (`V16N36E`, `V06N65E`) run on both programs.

## Gaps

On base fccb0653:

- **LM scenarios not yet word for word:** 13 of the 75 scenarios, all LM, end a few words to a few hundred from the harness, all within their limits: the rendezvous rows (`lm-rendezvous` 13 words, `-rr-dropout` 1, `-restart` 17, `-tpm` 431, `lm-stable-orbit` 323, `-lm` 373), the landing radar and powered flight rows (`lm-p63-lr-update` 145, `lm-p65-autoland-nassp` 139, `lm-landing-ascent-nassp` 117, `lm-p67-nassp` 83, `lm-abort-p70-nassp` 123, `lm-abort-p71-nassp` 150) and `lm-p12-restart` (123). The likeliest cause is the `FLIGHT_INTERP_STOP` basic tables, which cost only their weight here (The AGC clock); the radar and engine models running at other routine boundaries is the other candidate. Before the work clock (on base 5094c66f), eleven scenarios needed an envelope of their own for timing (CM P11's liftoff 33 cs late, the LM P52 cage at another sample point, the V25 redisplays in the LM rendezvous rows, the CM VHF marks, the five Pinball rows); none does now.
- **STBY lamp** lights only in the lamp test. The x86 program models no standby.
- **The CM's navigation DSKY RSET** (channel 16, 022) does not put the RESTART lamp out directly as yaAGC's does: x86 channel 16 carries only the mark buttons. Pinball's ERROR would still put it out through channel 11 bit 10.
- **Missing models:** uplink (`FLIGHT_UPLINK`), stolen time (`FLIGHT_STEAL`) and the harness's new output models, gyro torquing turning the platform (`FLIGHT_GYRO`) and the CM sextant following the optics drive (`FLIGHT_OPTICS`; the `lm-p52-cage-torque` and `cm-p52-optics` rows); the tool leaves those scenarios out.
- The uncompiled interpretive code by file is listed in the design note.
