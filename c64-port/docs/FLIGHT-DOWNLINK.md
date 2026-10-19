# The downlink in flight

Lane A2 (restart and telemetry). DOWNTELEMETRY (Luminary099 and Comanche055
DOWN-TELEMETRY_PROGRAM: the DOWNRUPT lead-in, DODOWNTM, the DNTMGOTO
continuation and the list phases; `src/lm_downtelemetry.c`,
`src/cm_downtelemetry.c`) run in flight scenarios. The channel 34 and 35
word stream it sends is checked against yaAGC in the same scenario, the
restart scenarios included. The milestone checks of the routines themselves
are in docs/LM-DOWNTELEMETRY.md and docs/CM-DOWNTELEMETRY.md.

## How DOWNRUPT is raised

The flight harness (`tools/flight_host_run.c`) raises no DOWNRUPT unless it
is asked. `FLIGHT_DOWNRUPT=1` (`tools/flight_downlink.h`) turns it on.

A DOWNRUPT is requested whenever SCALER1 (channel 4, 3,200 counts a second)
passes a multiple of 64 counts, every 20 ms of AGC time. The same rule holds
under both clock models, so the request lands at the same AGC time on both
sides.

- **Unscaled (yaAGC's clock).** Each `agc_engine` call that moves channel 4
  over such a multiple sets `InterruptRequests[8]` (DOWNRUPT in
  `agc_engine.c`). That covers the free-running reference, the lockstep
  spans (whose requests become the native side's, as for every other
  interrupt) and bridged code. yaAGC's own DOWNRUPT model (20 ms after both
  channels have been written: `Downlink`, `DownruptTime`) is switched off.
- **Scaled time (frozen clock).** Each `rk_timer_tick` period adds 32
  counts, so every second period requests `RK_RUPT_DOWN`. That holds
  natively and in the periods bridged code gets.

The header wraps `agc_engine()` and `rk_timer_tick()` as macros, so the
harness keeps its calls. Its other hooks in `flight_host_run.c` are added
lines only. With the knob unset the wrappers do nothing more.

## The word streams

Every word written to channel 34 or 35 is logged with its channel, its AGC
time (TIME2:TIME1), its 20 ms frame (channels 3:4 over 64) and the
word-order bit (channel 13 bit 7). `FLIGHT_DOWNLINK=PATH` writes the logs to
PATH.

- **NAT** is the native side. It holds the pair DNTMEXIT writes when a
  downtelemetry routine ends at TCF RESUME, any other change a native
  routine makes to channels 34 and 35, and bridged code's writes.
- **SPAN** is yaAGC in lockstep. It holds the writes of every span that
  matched a native routine, and bridged code's writes.
- **REF** is the free-running reference, in unscaled runs only.

The writes are seen through yaAGC's `CpuWriteIO` `Downlink` bits, which are
cleared before each instruction. The lockstep compares erasable memory, so
these logs are what checks the output channels. Each start from 04000 (the
cold start, then each GOJAM) is marked in NAT.

## Scenarios (`make flight-downlink-check`)

`tools/flight_downlink_check.py` runs every scenario of
`tools/flight_restart_check.py`, the GOJAM included, with
`FLIGHT_DOWNRUPT=1`. It also runs each scenario once without DOWNRUPT. A
scenario fails in any of these cases:

- The run stops early.
- NAT and SPAN differ in any word, word-order bit, frame or length.
- Words fall in unmatched spans.
- (Unscaled) NAT and REF differ.
- The run adds a routine yaAGC does not match to the run without DOWNRUPT.
- A list is not whole (100 pairs, from its ID word and the 77340 sync word
  to the next ID word), except the last one and the one the GOJAM cut.
- The first words after the GOJAM are not a list's ID and sync words.

The list IDs are 77777 - DNLSTCOD. They are named after yaAGC's
`ddd-<ID>-Luminary099.tsv` and the Colossus 2 lists for Comanche055.

Results on base 4bde5ef8 with this change (native words are channel 34 and
35 writes, two a DOWNRUPT; none of them came from bridged code):

| Scenario | Native words (= yaAGC in lockstep) | Differ | Reference (unscaled) | Lists sent (whole) | GOJAM: list cut, then | Routines yaAGC did not match (new) |
|---|---|---|---|---|---|---|
| `lm-p63-descent-restart` (scaled x1) | 29,964 | 0 | - | 77777 coast and align x13 (13), from 2 cs; 77773 descent and ascent x137 (135), from 26.02 s | 200.01 s: 77773 cut; at 200.03 s 77773 77340, the descent and ascent list again, 200 words | 8 (0) |
| `lm-dap-hold-restart` (overrun) | 1,144 | 0 | - | 77777 x7 (5), from 0.21 s | 20.30 s: 77777 cut; at 20.59 s 77777 77340 | 0 (0) |
| `cm-entry-restart` (scaled x1) | 23,930 | 0 | - | 77777 coast and align x13 (13); 77774 powered x17 (17), from 27.30 s; 77776 entry and update x91 (89), from 64.56 s | 250.75 s: 77776 cut; at 250.79 s 77776 77340 | 2 (0) |
| `lm-p00-restart` (unscaled) | 3,998 | 0 | 3,998, 0 differ (frames included) | 77777 x21 (19) | 25.00 s: 77777 cut; at 25.01 s 77777 77340 | 0 (0) |
| `cm-p00-restart` (unscaled) | 3,998 | 0 | 3,998, 0 differ (frames included) | 77777 x21 (19) | 25.00 s: 77777 cut; at 25.02 s 77777 77340 | 0 (0) |

The lists not counted whole are the one each GOJAM cut and the last one,
which the run's end cuts. P63 (selected at 22 s) switches the LM to the
descent and ascent list, which starts at 26.02 s. Under the overrun schedule
(four times slower than the AGC, each span's cost jittered), ticks are
dropped and a DOWNRUPT request waits, so the DAP hold sends 1,144 words in
30 s, not 3,000. The 8 LM and 2 CM unmatched routines are the same ones as
without DOWNRUPT (docs/FLIGHT-RESTART.md).

In each restart, STARTSUB points DNTMGOTO at LDNPHAS1. The first DOWNRUPT
after the GOJAM sends the ID and sync words of the list DNLSTCOD names, and
the list then runs whole. The list the GOJAM cut is left short, as the hardware does
(GOJAM zeroes channels 34 and 35).

With `FLIGHT_DOWNRUPT=1` set for the whole of `tools/flight_restart_check.py`,
every restart check still passes. The LM P00 restart has 342 sample
differences against 336 without the restart, inside its slack. The sample
differences in unscaled LM P00 runs grow with DOWNRUPT. This is native
interrupt latency (docs/FLIGHT-RESTART.md), not the downlink: natively a
request waits for the end of the routine it lands in. A DOWNRUPT due inside
AOSTASK's long GETAOSUV+6 routine is taken about 15 ms late. That shifts
T4RUPT a centisecond against the free-running reference, and one sample in
each 120 ms cycle differs in DSRUPTSW (the TIME4 count along both paths
matches). Even so, the native stream equals the reference's word for word,
frame for frame.

## Bug found and fixed

WOZERO's `TC Q` (the RETURN at 05,3610 in Luminary and 05,3623 in
Comanche) set Q to the next address natively. On the AGC, as in yaAGC, TC Q
leaves Q alone and runs the word in Q, here a TC back to the caller, as an
instruction. That leaves Q = 3. Every DOWNRUPT that sent a list's first
words (SENDID, through WOZERO) or a word-order change (SETWO) then
mismatched yaAGC in Q at TCF RESUME. In flight that is 1 DNPHASE1 and 9
DNPHASE2 mismatches in the first 10 s of LM P00. Both files now leave Q =
3, as the other compiled sources do ("a TC leaves Q = 3"). The milestone
cases missed this because they compared Q only in the final image, after a
DOWNRUPT that does not pass through WOZERO. They now also compare Q after
every DOWNRUPT, and the old code fails them. The vectors are refrozen.

## On the C64 store (`make flight-downlink-vice`)

`platform/c64/downlink_check.c` boots the flight resident on
`build/flight-store/lm.reu` (or `cm.reu`) and loads a native snapshot taken at an idle
sample point of a scenario run with `FLIGHT_DOWNRUPT`. With CIA1 interrupts
off (no clock period and no other interrupt), it takes 100 DOWNRUPTs one
after another. Each is set pending, and resident steps run until it has been
taken and RESUME has returned. Channels 34 and 35 and the word-order bit
after each one must equal yaAGC's from the same state, with its clock
frozen and the same DOWNRUPTs (`tools/flight_downlink_reference.c`).

Results (100 DOWNRUPTs each, 300 resident steps: lead-in, DODOWNTM and
DNTMGOTO, the list phase):

| Window | Snapshot | C64 against yaAGC | Lists started |
|---|---|---|---|
| `lm-p63-descent` | `lm-p63-descent-restart` at 199.50 s, braking phase, before the GOJAM | 100 / 100 DOWNRUPTs equal | 77773 descent and ascent at DOWNRUPT 30 |
| `lm-p63-descent-after-gojam` | the same run at 200.10 s, after the GOJAM, the restarted list 5 pairs in | 100 / 100 equal | 77773 at DOWNRUPT 95 (the restarted list ran whole) |
| `lm-p00` | `lm-p00-restart` at 24.00 s, P00 | 100 / 100 equal | 77777 coast and align at DOWNRUPT 1 |
| `lm-rendezvous` | `lm-rendezvous-restart` at 199.90 s, P20 radar tracking, before the GOJAM | 100 / 100 equal | 77775 rendezvous/prethrust at DOWNRUPT 6 |
| `cm-p00` | `cm-p00-p40-restart` at 729.90 s, P00 after the LOI-1 burn | 100 / 100 equal | 77777 coast and align at DOWNRUPT 6 |
| `cm-p40-burn` | `cm-p40-burn-restart` at 499.90 s, P40 SPS burn with the TVC DAP | 100 / 100 equal | 77774 powered at DOWNRUPT 5 |
| `cm-entry` | `cm-entry-restart` at 249.90 s, P64 entry | 100 / 100 equal | 77776 entry and update at DOWNRUPT 79 |

The last four are the whole-program acceptance's downlink windows: each
starts mid-phase from its restart row's own scenario, without the GOJAM, so
the lists carry that phase's live state (state vectors, burn and entry
quantities). The CM windows run on `build/flight-store/cm.reu`. The
`--replay` windows of tools/flight_scenario_vice.py run with DOWNRUPT off,
since a DOWNRUPT every 20 ms would change the replayed scenarios' timing and
inputs; channels 34 and 35 are checked here instead, and on the host by
`make flight-downlink-check` over the 11 restart rows that can take
DOWNRUPT (the 3 that start from a NASSP core run on scaled time, where
tools/flight_host_run.c has none).

The downtelemetry module is in the store (slot 1, as in the host run), and
so are the DOWNRUPT lead-in and RESUME. P63's guidance and displays do not
fit (docs/FLIGHT-RESTART.md), so with the clock frozen no job runs between
DOWNRUPTs. The descent list's words are those of the snapshot's state.

The C64 cannot restart by itself yet (docs/FLIGHT-RESTART.md, Gaps), and
its restart burst stops at DSPMMJB, which the store does not hold. So the
window after the GOJAM starts from a host snapshot taken after the restart,
when the AGC is idle again.

## The C64 flight program's own downlink (`RK_DOWNRUPT`)

With `RK_DOWNRUPT=1` the flight resident's timers request DOWNRUPT every
20 ms of AGC time, whenever SCALER1 (channel 4) passes a multiple of 64
counts: `rk_timer_phase` in `src/resident_rupt.c`, the rule
`tools/flight_downlink.h` applies on the host. Each step adds at most 16
counts, so a pass is a change in channel 4's bit 6, one byte test. The
flight program on its disk (`tools/flight_disk.py`) and the resident size
report (`tools/resident_flight_build.py`) build with it; it costs 24 bytes
on each program (LM 23,891 resident bytes, 2,476 free; CM 23,634, 2,733
free; the floor is 2,048). Every other build leaves it off.

`tools/flight_scenario_vice.py --replay --downrupt` runs a window with
DOWNRUPT on both sides (`FLIGHT_DOWNRUPT=1` on the host, `RK_DOWNRUPT=1` on
the C64). Each checkpoint compares channels 34 and 35 and the downlink's
erasable state (DNTMGOTO, LDATALST and the list words) with the rest; with
`--replay-every 2` there is a checkpoint at every DOWNRUPT.

| Window | Result |
|---|---|
| `cm-p22` 14000 + 40, every 2 cs | matches at all 20 checkpoints, one per DOWNRUPT: the P22 list (77773) from its ID and sync words on |
| `cm-p22` 14000 + 800, every 50 cs | matches at all 16 checkpoints and ends in the host's state |
| `lm-csi` 2100 + 800, every 50 cs | matches at 50-400 cs; at 450 cs LDATALST is one word ahead on the C64 (02215 against 02214). Without DOWNRUPT the same window leaves the host at 51 cs (centiseconds 62 against 63): the LM clock drift on the replay-mismatch list, not the downlink |

## Gaps

- The C64 windows above run with a frozen clock and set DOWNRUPT pending
  themselves; the live flight program is checked below.
- The x86 program (docs/X86-FLIGHT.md) does not take `FLIGHT_DOWNRUPT` yet
  (`platform/x86/flight_scenario.h` is Lane B's).
- Nothing reads the stream as telemetry yet (yaAGC's
  `DecodeDigitalDownlink` decodes a list's fields). The check names the
  lists by their ID words.
