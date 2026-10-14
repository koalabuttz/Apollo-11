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

It runs Luminary099 from GOJAM for CS centiseconds of AGC time (default 1000), pressing the DSKY KEYS (for example `V37E12E`) one every half second from 2 s. Environment:

| Variable | Effect |
|---|---|
| `X86_SPEED` | AGC time per wall time. 1 is real time (the default), 0 runs flat out. |
| `FLIGHT_CH30`, `FLIGHT_CH31` | Input channels 30 and 31 at start, in octal (`036377`, `057777`: IMU operate, PGNCS control, AUTO, so the LM DAP runs). |
| `FLIGHT_POKE` | `CS:ADDR=VALUE,...`: erasable words set on AGC time, in octal. |
| `X86_REPORT` | A JSON report: the points the interpreter core ran and why (no table, or the table refused), basic points no routine holds, the tables used with their certification, and the clock. |
| `X86_SAMPLES` | Erasable memory at the first idle pass through ADVAN in each centisecond. |

## What links

The program links what `tools/flight_store_build.py --host-only` compiles: every `src/*.c` on the host, and every routine table in the order the flight store resolves them (`build/flight-store/<m>-host.inc`, resident tables first, then `docs/port-coverage.json` order). Each routine sees its own source's inline operands (`<m>-sites.inc`), as its C64 module does, and reads fixed memory from the rope image (`src/rope_image_host.c`). Self-check reads through its own rope hook. The program is MIT and links no Virtual AGC code.

A table's kind (basic, keyed by Z; or interpretive, keyed by LOC under BANKSET) comes from `interpretive_table()` in `tools/flight_store_build.py`. `tools/flight_table_kinds.py` checks every table's kind against its keys' words in the listing.

## How a step runs

`platform/x86/flight_dispatch.c` runs one step:

1. **Boundary.** It runs the clock periods due (`rk_timer_tick`), then `rk_rupt_take`, as the flight resident does.
2. **Point.** If Z is the interpreter's fetch point, or DANZIG with EDOP set, the point is interpretive at LOC and BANKSET. Otherwise it is basic at Z.
3. **Routine.** Two direct-mapped indexes (basic and interpretive, by physical key) give the first table in resolution order that holds the point. The routine runs with its table's io record (`flight_io_enter`, `flight_io_leave`).
4. **Fallback.** For an interpretive point no table holds, or one whose routine refuses (its effects undone), one interpreter core slot runs from that point. Each such point is counted for the report. A basic point no routine holds stops the program: there is no bridge.

`platform/x86/flight_scenario.h` holds the scenario inputs (keys, pokes, the sample point), which the program and its lockstep check share.

## The AGC clock

Each routine adds its cost to a virtual AGC clock (85,333 machine cycles a second). Every 853 1/3 cycles is one clock period, run at the next boundary, with no cap and nothing dropped. At `X86_SPEED` 1 the program sleeps whenever AGC time gets ahead of `CLOCK_MONOTONIC`. When the host falls behind, AGC time runs late (scaled time, docs/RESIDENT-FLIGHT.md). Keys and pokes arrive on AGC time.

The costs come from `docs/flight-routine-mct.json`: for each routine point, the machine cycles yaAGC took for it over the host differential's scenarios and the LM DAP holding and firing (median, runs, sum, sum of squares, least, most and the table that ran; `make x86-flight-mct`, `tools/x86_flight_mct.py`, using `FLIGHT_MCT_OUT` in `tools/flight_host_run.c`). The x86 clock charges each point its median. Points no scenario reaches cost the median of their kind. One interpreter core slot costs 30 cycles, an estimate. The C64 resident's work clock takes its per-module weights from the same file.

## Acceptance: lockstep against yaAGC

`make x86-flight-check` (`tools/x86_flight_check.py`) builds both modes and the lockstep check `tools/x86_flight_lockstep.c`. That check is GPL, host only, and links yaAGC. It runs each scenario on the program's own step. Every routine and every core slot is checked: yaAGC, with its clock frozen (`AgcFrozenClock`, as under scaled time in docs/FLIGHT-STORE.md), runs from the state the routine started in until it reaches the routine's continuation with the same 2,048 erasable words (Z aside), I/O channels and interrupt state. The clock periods and interrupt entries are the program's own `rk_timer_tick` and `rk_rupt_take`, which `make test-resident-rupt` checks against yaAGC. So a run with no mismatch is one yaAGC run whose clock ran on the x86 program's schedule. The check fails on any mismatch, any basic point no routine holds, or a run that stops early.

On base 7262c653 with this change:

| Scenario | Routines and core slots checked (compiled / core) | yaAGC did not match | Points the core ran (compiled / core) |
|---|---|---|---|
| LM P00, 4000 cs | 258,393 / 258,393 | 0 | 0 / 0 |
| CM P00, 4000 cs | 261,359 / 261,359 | 0 | 0 / 0 |
| LM DAP hold, 3000 cs | 177,143 / 177,143 | 0 | 0 / 0 |
| CM V37E00E, 3000 cs | 193,758 / 195,199 | 0 | 4 / 108 |
| CM V37E06E, 3000 cs | 195,381 / 195,385 | 0 | 2 / 4 |

Flat out, 40 s of AGC time takes about 0.05 s of host time.

CI runs `x86-flight-check` in the host job (`ci-extra-host`) when a PR changes any source, header or x86 tool (`ci-watch-x86-flight-check`).

## Gaps

Points the scenarios reach that no compiled routine holds:

- LM `V37E12E` and `V37E63E` stop at 5.00 s: `lm_integration_init_basic` refuses INTSTALL+1 (13,3415), a basic point with no bridge. That is the Certificate fixes lane's file.
- CM INTEGRATION_INITIALIZATION: INTSTALL and GOBAC+1 are refused, and PTOACSM+1 and DT/2COMP+4 have no table. The core runs them.
- The uncompiled interpretive code by file is listed in the design note. The largest parts are LM and CM P20-P25, LM ASCENT_GUIDANCE and LM STABLE_ORBIT.
