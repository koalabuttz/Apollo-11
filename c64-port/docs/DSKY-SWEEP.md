# DSKY sweep on the C64

Acceptance item 6: DSKY key sequences give the same displays and loads on
the C64 as on the host. `tools/dsky_sweep_vice.py` runs them.

## What a case is

A case is a mission and a key string in `tools/flight_host_run.c` syntax:
`V`, `N`, digits, `E`, `C` (CLR), `K` (KEY REL), `R` (RSET). Every case
starts from one host snapshot of a fresh start at 1000 cs. The keys go in
from 1100 cs, and the case ends 150 cs after its last key.

- The host runs on from the snapshot and writes its input rows and its end
  state.
- One VICE run takes a batch of cases (`DP_SWEEP` in
  `platform/c64/dap_pass_check.c`). It loads the snapshot, replays a case's
  rows, stashes the end state in the REU, reloads the snapshot and goes on
  to the next case.
- A case passes when the C64 ends at the host's sample point in the host's
  state: erasable words, timers and output channels.
- A case whose host run reaches a point with no native routine (a refused
  or unregistered point) fails before VICE, naming the point. The C64
  would stop there.

## Running it

    make dsky-sweep-vice                       # the CI sample, against the known list
    python3 tools/dsky_sweep_vice.py --cases ci
    python3 tools/dsky_sweep_vice.py --keys V16N36E,V06N43E
    python3 tools/dsky_sweep_vice.py --cases all   # Lane C's full list (tools/dsky_sweep_cases.py)

The results go to `build/flight-store/dsky-sweep/dsky-sweep.json`.

## The CI gate

`dsky-sweep-vice` runs in the c64 CI jobs when a Pinball, display, keyboard
or sweep source changes (`ci-watch-dsky-sweep-vice` in
`tools/mk/ci-extra.mk`). It runs the 27-case sample (`CI_CASES`) on both
missions.

`tools/dsky_sweep_known.json` lists the cases that do not match yet, each
with its reason. The target fails on a failing case that is not listed. It
also fails on a listed case that now matches, so the list only shrinks.

`tools/cert_precheck.py` fails a complete Pinball certificate while any of
its mission's cases is still on the list. Cases whose reason names
`owner RK` are an exception: they get a warning. Those are the clock class
(the C64 clock or a timer one tick apart, and words that follow from it),
which belongs to the resident clock (`flight-scenarios/replay-mismatches.md`),
not to Pinball.
