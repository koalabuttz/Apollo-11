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
| `FLIGHT_ENGINE` ... `FLIGHT_SIVB` | The vehicle models (below). |
| `FLIGHT_GOJAM` | `CS[,CS...]`: a hardware restart (yaAGC's GOJAM: Q = Z, Z = 04000, interrupts enabled and cleared, output channels zeroed), then the program's own restart code. |
| `FLIGHT_DOWNRUPT` | `1`: a DOWNRUPT every 20 ms of AGC time, so the downtelemetry program runs. |
| `X86_REPORT` | A JSON report: the points the interpreter core ran and why (no table, or the table refused), basic points no routine holds, the tables used with their certification, and the clock. |
| `X86_SAMPLES` | Erasable memory at the first idle pass through ADVAN in each centisecond. |
| `X86_SAMPLES_FROM` | The first centisecond `X86_SAMPLES` writes (default 1). |
| `X86_DSKY` | `1`: a live DSKY in the terminal (below). With no CS argument the run goes on until `q`. |

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

The costs come from `docs/flight-routine-mct.json`: for each routine point, the machine cycles yaAGC took for it over the host differential's scenarios and the LM DAP holding and firing (median, runs, sum, sum of squares, least, most and the table that ran; `make x86-flight-mct`, `tools/x86_flight_mct.py`, using `FLIGHT_MCT_OUT` in `tools/flight_host_run.c`). The x86 clock charges each point its median. Points no scenario reaches cost the median of their kind. One interpreter core slot costs 30 cycles, an estimate. The C64 resident's work clock takes its per-module weights from the same file.

## Acceptance: lockstep against yaAGC

`make x86-flight-check` (`tools/x86_flight_check.py`) builds both modes and the lockstep check `tools/x86_flight_lockstep.c`. That check is GPL, host only, and links yaAGC. It runs each scenario on the program's own step. Every routine and every core slot is checked: yaAGC, with its clock frozen (`AgcFrozenClock`, as under scaled time in docs/FLIGHT-STORE.md), runs from the state the routine started in until it reaches the routine's continuation with the same 2,048 erasable words (Z aside), I/O channels and interrupt state. Q has one exception. Native routines leave Q's 16th bit clear where yaAGC sign-extends bit 15 into it (LM P63's landing displays, LLGE and integration init, CM P11 and P22). Nothing reads that bit back except as a return address, and the host differential compares no central register. So the check accepts that exact difference (native bit 16 clear, yaAGC's bit 16 equal to its bit 15) and still fails any other. The clock steps and interrupt entries are the program's own `rk_timer_phase` and `rk_rupt_take`, which `make test-resident-rupt` checks against yaAGC. So a run with no mismatch is one yaAGC run whose clock ran on the x86 program's schedule. The I/O channels compared include the downlink words (channels 34 and 35), which carry the downtelemetry list under `FLIGHT_DOWNRUPT`. The check fails on any mismatch, any basic point no routine holds, or a run that stops early.

On base f499d7ee:

| Scenario | Routines and core slots checked (compiled / core) | yaAGC did not match | Points the core ran (compiled / core) |
|---|---|---|---|
| LM P00, 4000 cs | 258,356 / 258,356 | 0 | 0 / 0 |
| CM P00, 4000 cs | 261,326 / 261,326 | 0 | 0 / 0 |
| LM DAP hold, 3000 cs | 177,110 / 177,110 | 0 | 0 / 0 |
| CM V37E00E, 3000 cs | 193,699 / 195,166 | 0 | 0 / 108 |
| CM V37E06E, 3000 cs | 195,334 / 195,337 | 0 | 0 / 4 |
| LM P00 with DOWNRUPT, 4000 cs | 257,741 / 257,741 | 0 | 0 / 0 |
| CM P00 with DOWNRUPT, 4000 cs | 259,481 / 259,481 | 0 | 0 / 0 |
| LM P00, GOJAM at 20 s, DOWNRUPT, 4000 cs | 257,518 / 257,518 | 0 | 0 / 0 |
| CM V37E06E at 25 s, GOJAM at 35 s, DOWNRUPT, 4000 cs | 258,548 / 258,551 | 0 | 0 / 4 |

The same check on the 13 flight scenarios below (compiled; not in CI, a few minutes on 4 cores) shows no miss on the four LM runs. Each of the nine CM runs shows one miss: channel 12 bit 5 (zero ICDU) at 06,2356 in `cm_t4rupt_imu_routines` (Gaps).

Flat out, 40 s of AGC time takes about 0.05 s of host time.

CI runs `x86-flight-check` in the host job (`ci-extra-host`) when a PR changes any source, header or x86 tool (`ci-watch-x86-flight-check`).

## Vehicle models

`platform/x86/vehicle_models.h` (MIT, header only) holds the vehicle the AGC flies: the LM descent and ascent engines with touchdown, abort stage and the P66 crew (`FLIGHT_ENGINE`, `FLIGHT_ENGINE_LAG`, `FLIGHT_TOUCHDOWN`, `FLIGHT_ASCENT`, `FLIGHT_STAGE`, `FLIGHT_PILOT`), the LM RCS jets (`FLIGHT_RCS`), the CSM's SPS (`FLIGHT_SPS`), entry drag and lift (`FLIGHT_AERO`), the IMU gimbals for P01 (`FLIGHT_IMU`), the Saturn V boost (`FLIGHT_SATURN`) and TLI (`FLIGHT_SIVB`). They were written for this port's flight scenarios and contain no Virtual AGC code. `platform/x86/test_vehicle_models.c` checks each one (`make x86-flight-check` builds and runs it first).

The x86 program runs them with the same settings and at the same points as the host differential (`tools/flight_host_run.c`, which still has its own copy): at each sample point after the pokes, then the channel changes (`FLIGHT_CHAN`), then a restart or a key; with `FLIGHT_ENGINE_LAG` the engine, and always the RCS jets, also after every routine. Keys include the mark buttons (`M`, `J`; LM `x`, `y`, `j`: channel 16 and KEYRUPT2).

`make x86-flight-scenarios` (`tools/x86_flight_scenarios.py`, not in CI) flies every host differential flight scenario the x86 program can run on its own and compares it with the harness on the work clock (`FLIGHT_SCALED=W`) at the first sample point at or after 1,000 cs before its end. The two programs charge routine costs differently: the x86 program charges each point's median, and the work clock charges each table's weight. So their states are close, not equal. The yardstick is the harness against itself, the same scenario on its other clock (`FLIGHT_SCALED=1,1,2`, yaAGC's own cycles). A scenario passes when all of these hold:

- the major mode is the same;
- RN and VN are within 4 times the harness's own spread, plus 50 m and 0.15 m/s for the per-routine charges the spread cannot see (the largest such case is CM P40: VN 0.13 m/s against a spread of 0.004);
- the erasable image (061-3777) differs in at most 1.5 times as many words as the harness's two clocks differ in, plus 10.

The word count is what sees programs that don't move the state vector. With the mark buttons left out of the x86 run (`--drop-keys MJxyj`), P52 differs in 210 words against a limit of 19 and P57 in 188 against 37, so both fail. P22 and P23 fail on RN as well.

On base f499d7ee:

| Scenario | Major mode | RN apart (limit; harness spread) | VN apart (limit; spread) | Words differing (limit; spread) |
|---|---|---|---|---|
| LM P63 descent (no engine) | 77 / 77 | 125 m (588; 135) | 0.06 m/s (0.87; 0.18) | 318 (509; 333) |
| LM P63 descent, engine | 77 / 77 | 135 m (751; 175) | 0.58 m/s (0.92; 0.19) | 276 (395; 257) |
| LM P63 descent, restart at 200 s | 77 / 77 | 147 m (746; 174) | 0.57 m/s (0.63; 0.12) | 242 (434; 283) |
| LM P57 | 00 / 00 | 0 | 0 | 14 (37; 18) |
| CM P11 launch to orbit | 00 / 00 | **449 m (371; 80)** | **0.73 m/s (0.37; 0.06)** | 189 (211; 134) |
| CM TLI | 00 / 00 | 153 m (524; 119) | 0.08 m/s (0.48; 0.08) | 114 (178; 112) |
| CM P40 SPS burn | 00 / 00 | 21 m (107; 14) | 0.13 m/s (0.17; 0.004) | 92 (127; 78) |
| CM TEI | 00 / 00 | 16 m (70; 5) | 0.06 m/s (0.38; 0.06) | 145 (221; 141) |
| CM entry (P61-P67) | 103 / 103 | 30 m (329; 70) | 1.84 m/s (9.33; 2.30) | 320 (437; 285) |
| CM P22 | 00 / 00 | 0 | 0 | 17 (43; 22) |
| CM P23 | 00 / 00 | 0 | 0 | 9 (29; 13) |
| CM P37 | 00 / 00 | 0 | 0 | 27 (50; 27) |
| CM P52 | 00 / 00 | 0 | 0 | 6 (19; 6) |

CM P11 is outside its limits (Gaps). The tool lists it as known, with an envelope of its own: RN within 600 m and VN within 1 m/s, reaching the compare point with exit 0, the same major mode and the normal word limit. Inside that envelope it doesn't fail; outside it, it does. With the vehicle models removed, P11 ends 6,563 km off and fails. The other 12 scenarios pass.

The rest need models the x86 program does not have yet: the rendezvous radar and VHF (`FLIGHT_RR`), the landing radar (`FLIGHT_LR`, `FLIGHT_LR_POS2`), the NASSP P63 core (`FLIGHT_CORE`), uplink and stolen time. So no scenario compared here flies the LM RCS (`FLIGHT_RCS`), the throttle lag, touchdown, the P66 crew, the ascent or the abort stage. Those run only in the NASSP scenarios, and the unit checks in `test_vehicle_models.c` are their only x86 test so far.

## Live DSKY

`X86_DSKY=1 build/x86-flight/x86-flight-lm ROPE.bin` flies the LM in real time with a DSKY in the terminal (`platform/x86/terminal_dsky.h`, MIT). The display is the port's own DSKY device adapter (`src/dsky_display.c`, the one the C64 draws with): the relay words the program writes to channel 10 and the lamps of channel 11, drawn with ANSI escapes. Lit lamps show in bold yellow.

| Key | DSKY |
|---|---|
| `0`-`9`, `+`, `-` | digits and signs |
| `v`, `n`, `e` or Enter | VERB, NOUN, ENTER |
| `c`, `r`, `k` | CLR, RSET, KEY REL |
| `p` | PRO (channel 32 bit 14 held low for half a second) |
| `m`, `j` | the CM's MARK and MARK REJECT |
| `q` or Ctrl-C | quit |

Typed keys go in at idle sample points at least 0.2 s of AGC time apart, as scenario keys do, so keys piped in (`printf v35e | X86_DSKY=1 X86_SPEED=0 ...`) work too. KEYS and the `FLIGHT_*` inputs still apply. The lamp test (`V35E`), `V37E` program changes and octal displays (`V01N01E`, `V05N09E`) run on both programs. Decimal noun displays don't yet (Gaps).

## Gaps

On base f499d7ee:

- **CM P11 boost:** at 89,001 cs the x86 run is 449 m and 0.73 m/s from the harness, against a spread of 80 m and 0.06 m/s between the harness's own two clocks. The cause is when the AGC sees liftoff. The gyrocompass loop polls the liftoff discrete (channel 30 bit 5), and P11 zeroes its clock and navigates from the time it saw it. With `FLIGHT_SATURN` lifting off at 10,000 cs, TLIFTOFF is 10,005 on the harness's work clock, 10,022 on its other clock, and 10,038 on the x86 program's clock. So the state vector carries that offset from liftoff on (149 m at 10,051 cs against a spread of 74 m), and it grows to orbit. This comes from the clock schedule, not from a routine computing differently. Comparing at equal time since liftoff, or making the x86 schedule closer to yaAGC's, would bring P11 inside the limits.
- **CM channels 11-14:** every CM table runs with `FLIGHT_IO_CM_NATIVE`. Its copy back into the hardware channels undoes the channel writes that routines make straight to the hardware: `src/cm_t4rupt_imu.c` (zero ICDU, cage, DSALMOUT) and `src/keyrupt_uprupt.c`. The x86 lockstep shows this as the one channel 12 miss above. The scenario comparison can't see it, because the harness behaves the same way and compares no channels. The cause is in shared code (`src/flight_io.c`), and D1 is fixing it, together with a channel compare in the harness lockstep.
- **Decimal noun displays** (`V06N36E`, `V16N36E`, `V16N65E`) stop at Pinball's basic point 42,3230 (LM) or 42,3143 (CM) in the live DSKY. The not yet certified `lanec_pin_lm_routines` and `lanec_pin_cm_routines` refuse them. Lane C's Pinball LM and CM certificate covers them.
- **Missing models:** the radar, NASSP core, uplink and stolen-time scenarios above.
- The uncompiled interpretive code by file is listed in the design note.
