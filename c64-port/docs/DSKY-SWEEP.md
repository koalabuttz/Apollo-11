# DSKY sweep on the C64

Acceptance item 6: DSKY key sequences give the same displays and loads on
the C64 as on the host, and the host gives what yaAGC gives.
`tools/dsky_sweep_vice.py` runs them.

## What item 6 needs

Item 6 is met only by **`--cases all --replay-exact` on both missions with
no failure at all**, the clock class included, committed as hashed evidence:

    python3 tools/dsky_sweep_vice.py --cases all --replay-exact --evidence evidence/dsky-sweep
    python3 tools/evidence.py add dsky-sweep <run>/dsky-sweep.json
    python3 tools/evidence.py add dsky-sweep <run>/dsky-sweep-summary.json --tree
    python3 tools/evidence.py push dsky-sweep

A full sweep's `dsky-sweep.json` is about 1 MB, past the 256 KiB tree
limit, so it is stored in the release, and CI does not fetch it.
`dsky-sweep-summary.json` stays in the tree: the run's mode, missions and
hashes, the counts per mission and every failed case in full, with the
full report's hash in `full_sha256`. `tools/cert_precheck.py` checks the
full report when it is on disk (and that the summary is its summary), and
the summary otherwise.

`evidence/dsky-sweep/dsky-sweep.json` records the run's mode and missions,
the store, rope and manifest hashes per mission, each VICE program's hash,
and every case's result. Its `item6` field is true only for such a run.
The CI sample and its known list (below) are a ratchet for pull requests,
not item 6.

`--cases all` is 2,161 cases a mission (`tools/dsky_sweep_cases.py`): every
display, monitor and load verb on every noun, V07, the spare and illegal
verbs below 40, the extended verbs, V30/V31 with noun 26 loaded, octal and
signed decimal loads, CLR, KEY REL, RSET and PRO. On one 4-job machine the
`--replay-exact` run took 80 min (LM) and 62 min (CM) on 2026-10-24. Loading
noun 36 with V25 is left out: it moves the AGC clock past the sweep's
sample point.

The V30/V31 cases end at MPAC+3 (erasable 0157), where REQUESTC stores
TC ENDOFJOB; `src/lanec_pin_request_routines.c` runs that word, and its
resolver record sits under bank 41 (`ERASABLE_RECORDS` in
`tools/flight_store_build.py`). All six match the host with
`--replay-exact` on the C64 (2026-10-25) and are in the CI sample.

The clock class (below) blocks item 6 until the resident clock (RK) clears
it. It does not block the Pinball file certificate.

CM results carry the TEMPORARY `SCHEDULE_SKIP` of
`tools/flight_scenario_vice.py` (DSPTAB sent bits, DSPCNT, DSRUPTSW). Each
result lists what it skipped in `skipped`, and the item 6 evidence must
say so while the skip exists.

## What a case is

A case is a mission and a key string in `tools/flight_host_run.c` syntax:
`V`, `N`, digits, `E`, `C` (CLR), `K` (KEY REL), `R` (RSET), plus `Q` for
PRO. PRO is no key code: it is channel 32 bit 14, so the sweep gives the
host a `FLIGHT_CHAN` press (the bit down for 30 cs) in that key's slot, and
the C64 gets it with the host's input rows. A case can also carry its own
channel inputs after a `|` (`KEYS|CS:CH=VALUE,...`, Lane C's `pro_cases`,
timed as a run whose keys start at 200 cs), moved to the sweep's key time. Every case starts from one host
snapshot of a fresh start at 1000 cs. The keys go in from 1100 cs, and the
case ends 150 cs after its last key.

- The host runs on from the snapshot and writes its input rows and its end
  state.
- One VICE run takes a batch of cases (`DP_SWEEP` in
  `platform/c64/dap_pass_check.c`). It loads the snapshot, replays a case's
  rows, stashes the end state in the REU, reloads the snapshot and goes on
  to the next case. Every case's snapshot header must be byte-equal to the
  first's (but its comment line), since the C64 loads the first's.
- A case passes when the C64 ends at the host's sample point in the host's
  state: erasable words, timers and output channels 5, 6, 10, 11-14, 34 and
  35.
- A case fails before VICE when its host run, after the snapshot, reaches
  a point with no native routine (a refused or unregistered point: the C64
  would stop there), or runs a native routine yaAGC did not match (the host
  itself is wrong). It names the point or the routine.

## Classes

The tool gives each failure a class and a signature (the class and what
differs or where it stops). Nobody writes them by hand.

| Class | Meaning |
|---|---|
| clock | Only the centiseconds and TIME1-TIME6 differ, each by one count and all the same way (TIME2 only with TIME1). |
| difference | Any other erasable word, timer or channel differs. |
| stop | The host run bridges a point after the snapshot. |
| host-miss | The host run has a native routine yaAGC did not match after the snapshot. |
| run | The C64 stopped before the window's end, or the batch gave no result. |

## Running it

    make dsky-sweep-vice                       # the CI sample, against the known list
    python3 tools/dsky_sweep_vice.py --cases ci
    python3 tools/dsky_sweep_vice.py --keys V16N36E,V06N43E
    python3 tools/dsky_sweep_vice.py --keys V16N36E --keys 'V37E00E|1200:032=17777,1300:032=37777'
    python3 tools/dsky_sweep_vice.py --cases all   # Lane C's full list (tools/dsky_sweep_cases.py)

The results go to `build/flight-store/dsky-sweep/dsky-sweep.json`.

## The CI gate

`dsky-sweep-vice` runs in the c64 CI jobs when a Pinball, display, keyboard
or sweep source changes (`ci-watch-dsky-sweep-vice` in
`tools/mk/ci-extra.mk`). It runs the CI sample (`CI_CASES`) on both
missions.

`tools/dsky_sweep_known.json` lists the sample's failures not fixed yet,
each with the tool's signature and a reason. The target fails on:

- a failure that is not listed;
- a listed case that now matches (so the list only shrinks);
- a listed case that fails with a different signature;
- a listed case of a run mission that the run did not exercise.

`--write-known FILE` writes the list from a run, keeping the reasons.

## The precheck

`tools/cert_precheck.py` checks a Pinball certificate, draft or complete,
for its mission. It FAILs a draft (and WARNs on a certificate already
accepted complete) when:

- `tools/dsky_sweep_known.json` is missing, or lists a case of the mission
  that is not the clock class;
- `evidence/dsky-sweep/dsky-sweep.json` is missing or is not a `--cases all`
  run of the mission;
- that run has a failure of the mission that is not the clock class.
- its rope hash, or its store hash when the store is built, is not the
  current one (the evidence is of another build).

Clock-class cases get a WARN. They belong to the resident clock
(`flight-scenarios/replay-mismatches.md`), not to Pinball.
