# Hardware restarts in flight

Lane A2 (restart and telemetry). A hardware restart (GOJAM) in the middle of
a flight scenario, the AGC's own restart code running natively, and the
resumed program checked against yaAGC. On the host this is the whole run; on
the C64 flight store it is the restart burst so far (see Gaps).

## How the restart is forced

`FLIGHT_GOJAM=CS[,CS...]` (`tools/flight_gojam.h`, read by
`tools/flight_host_run.c`) restarts the AGC at the first sample point at or
after each CS, on both sides. A sample point is a clean point: idle at
ADVAN, no interrupt in progress or pending, no instruction half done. With
`FLIGHT_DOWNRUPT` (docs/FLIGHT-DOWNLINK.md) no DOWNRUPT is pending there either, so no restart
lands inside one. The pokes for that
point go in first, and the key for that point waits for the next sample
point.

The restart is the hardware one. It is what yaAGC does when a TC trap
(V69), the night watchman, a rupt lock or a parity alarm fires
(`agc_engine.c`, the GOJAM and TC 4000 sequences):

- Q takes Z, and Z becomes 04000.
- Interrupts are enabled and none is in progress. Every request and every
  HANDRUPT trap is cleared.
- Output channels 5, 6, 10-14, 34 and 35 are zeroed, and channel 33 bit 11
  is set.
- The superbank and every erasable word but Z and Q stay as they were.

The native side and yaAGC take the same change. From 04000 the rope's own
code runs. In Luminary that is GOJAM, GOPROG (REDOCTR, RSBBQ),
DORSTART/STARTSUB, SETINFL, the phase-table check (GOPROG3, PCLOOP),
MMDSPLAY, then NXTRST and RESTARTS for each active group (the restart
tables, PHSPART2 and GETPART2, and the FINDVAC, NOVAC and WAITLIST calls
they make), and finally ENDRSTRT. Every native routine on that path is
checked against yaAGC in lockstep, as everywhere else in the run.

yaAGC alone cannot be made to restart reproducibly from outside. Its alarms
run on its own clock, and `InhibitAlarms` (which the harness needs) turns
them off. A TC to 04000 (a poke of Z) would skip the hardware part. So both
sides apply the GOJAM at the same clean point.

## The TC trap (V69)

V69 (VERB69, 43,2037 in both ropes) is `TC VERB69`, a TC to itself. On the
AGC the hardware TC trap watches each 5 ms from scaler phase 020 to phase
000 of the 10 ms cycle: if only TCs (or TCFs) ran in it, the trap restarts
the computer at phase 000. Any other instruction, such as an interrupt
routine's, clears the watch. yaAGC models it (`agc_engine.c` TCTrap and
NoTC); with `InhibitAlarms` on here it sets channel 77 bit 3
(`CH77_TC_TRAP`) without the GOJAM.

Unscaled (`tools/flight_host_run.c` `yaagc_trap`), the host lets yaAGC's
own trap decide, on both sides:

- The free-running yaAGC reference takes the GOJAM (`gojam_yaagc`) after
  the instruction that set the bit.
- On the native side VERB69 is never a native step: yaAGC bridges it
  (whatever table holds the point), interrupts go to their native routines
  as usual, and when yaAGC's trap fires in the bridge the native side takes
  the GOJAM (`gojam_native`). So the restart comes at yaAGC's time on both
  sides, and the sample compare checks the restart against yaAGC's.
- The trap anywhere but VERB69, or the no-TC alarm the same bit reports,
  stops the run (exit 3): the C64 does not model it.

The C64 has no instruction stream to watch. Its resident step
(`rk_resident_step`, `platform/c64/resident_flight.c`) does the same on its
work clock: at VERB69 outside an interrupt it sets Q (TC VERB69's) and spins
as the idle loop does, to the end of the 10 ms period (`rk_spin`). If the
spin was there for the period's T3 step (TIME1/TIME3, scaler phase 020) and
no interrupt came before its T5 step (TIME5, phase 000), it takes the GOJAM
there (`rk_gojam`); an interrupt ends the spin, and its routine runs. Under
scaled time the host's native side spins the same way (`spin`), and its
bridge stops at VERB69 for it, so a C64 replay checks the C64's restart
against the host's. A TC or TCF to itself anywhere else stops the run on
both: the host's bridge under scaled time, and on the C64 the compiled
routines refuse one (`resident_flight_main.c`).

`lm-v69` and `cm-v69` (`V69_FLIGHT` in `tools/flight_host_differential.py`)
key V69E at 23.5 s in P00. The restart runs GOPROG with no group active,
back to P00:

- Host: `lm-v69-restart` and `cm-v69-restart` in `make flight-restart-check`
  (table below). yaAGC's trap fires at 25.01 s on both missions, and the
  native side restarts with it: LM 4,000 of 4,000 samples equal and CM
  3,998 of 4,000, each the same as its run without the restart.
- C64, from the trap's snapshots: both rows are in `make flight-restart-vice`
  (the burst from just after the GOJAM against yaAGC) and `make
  flight-gojam-vice` (the GOJAM module from just before it).
- C64, the spin itself: `tools/flight_scenario_vice.py --scenario cm-v69
  --snap-cs 2300 --cs 400 --replay --replay-every 50` matches the host at
  all 8 checkpoints: the trap at 203 cs into the window (the host's work
  clock) and the restart, to the end of the window. `lm-v69` matches at
  51-200 cs, up to its trap at 201 cs. At 250 cs it differs at TIME4 (host
  37776, C64 37777) and DSRUPTSW, the LM's T4RUPT phase after a restart
  (the `lm-p00-restart` class below), logged in
  `/mnt/project-files/flight-scenarios/replay-mismatches.md`.

The x86 program takes the TC trap the same way (`platform/x86/tc_trap.h`,
#806): `make x86-flight-scenarios` flies `V69_FLIGHT` against the harness,
and `make x86-flight-check` runs V69 in lockstep against yaAGC
(docs/X86-FLIGHT.md).

## The night watchman

On the AGC (yaAGC `agc_engine.c`) the night watchman arms every 1.28 s, at
scaler phase 04000. Any access to NEWJOB (067) disarms it. If it is still
armed 0.64 s later, at phase 00000, it sounds and the AGC restarts (GOJAM).
The Executive reads NEWJOB all the time (DANZIG before each interpretive
instruction, ADVAN in the idle loop, CCSNEWJOB, NOVAC), so only a job that
loops without the Executive trips it. The one such loop found in the port
is the interpreter's GOTO on an erasable CADR cycle: GOTOERS fetches
erasable CADRs until it finds a fixed one, and a cycle never ends.

The host harness and the x86 program model it the same way
(`platform/x86/night_watchman.h`, under scaled time):

- **The clock.** The scaler is the clock period count: the alarm arms 64
  periods into each 128 and sounds at the period that ends them.
- **A stuck step.** A step from a basic point outside an interrupt, with a
  job active (NEWJOB not -0), that leaves every erasable word as it found
  it, would run again and again from the same state, as the AGC would. An
  interpretive step is never stuck: it ends at DANZIG or the next fetch, so
  a loop of them reads NEWJOB (CM P20 waits that way). It spins to the end of
  the clock period, interrupts are taken, and the job is hung at that point
  until a step there is not stuck. The interpreter core stops after each
  GOTOERS hop at GOTO with the same state. `lmi_goto_any`
  (`src/lm_interp_orders.c`) still stops a host build with exit(3) on the
  cycle; the x86 program links with `--wrap=exit` and turns that exit
  inside a routine into a stuck step.
- **NEWJOB accesses.** The Executive runs natively, so an access is read
  from the steps (#912 item 5). These count:
  - A step from one of the Executive's points that read NEWJOB, in or out of
    an interrupt: NOVAC, FINDVAC, SPVACIN, SPVAC, CHANG1, CHANG2, JOBSLEEP,
    JOBWAKE, PRIOCHNG, ENDOFJOB, DUMMYJOB and its restart entry, ADVAN,
    NUCHANG2 and DANZIG.
  - An interpretive step outside an interrupt, while no job is hung, that
    ends at NEWOPS 1 to 3 words on. It went round DANZIG. A CALL or GOTO
    also ends at NEWOPS, but at a new LOC and without DANZIG.
  - Any step that changes NEWJOB or a core set's PRIORITY.

  No other step counts, even outside an interrupt. A job's basic steps away
  from the Executive do not read NEWJOB. A hung job's steps are stuck.
- **Checked against yaAGC.** Each lockstep and bridge span sets yaAGC's own
  NightWatchman flag and reads it back, so the harness also tracks the
  watchman by yaAGC's own NEWJOB accesses. Its summary line counts the
  checks where the two disagree. `tools/flight_host_differential.py` fails
  a scaled row with any disagreement, on every schedule. So the alarm
  sounds exactly where yaAGC's accesses would sound it.
- **Boot starvation on the wall-clock schedules.** At boot, lm V37E12E on
  x1 and overrun starves the job (and cm V37E06E on x1 can, by where the
  store's chunk lists put the swaps). Every boundary takes
  an interrupt (T4, T6, the DAP idler, the P and Q/R axes), each with a
  module swap. In 0.64 s the job gets only a few basic steps, and none
  reaches the Executive. A real AGC this slow would sound the night watchman
  and restart. The harness does the same, at the same checks as yaAGC's
  accesses. On lm x1 and overrun the restart starves again, so the AGC
  restarts every 1.28 s and V37 never runs. A scaled run that never idles
  takes no sample point, so it also ends 10 s of AGC time past its end. The
  unscaled run never restarts, because yaAGC at AGC speed is not starved.
  The `-dap` boot rows (`FLIGHT_SWAP_MCT=0`) and the flight rows on x1
  never sound it. The work clock, which the C64 flight build and the x86
  program run, does not starve the job: its rows never sound the alarm.
  Before #912 item 5, the native rule counted every step outside an
  interrupt as an access. That missed these alarms, and the doc blamed
  yaAGC reads outside the spans.
- **The GOJAM.** When the alarm sounds, the next step is the GOJAM, with Q
  at the hung point. The harness prints "night watchman at N cs".
- **Lockstep at GOTO.** A native continuation at GOTO (06646 LM, 06615 CM)
  with A already POLISH matches yaAGC at GOTO +1. GOTOERS loops back there
  (`TCF GOTO +1`), and GOTO's `CA POLISH` sets no other word.

`lm-p52-watchman` (`WATCHMAN_FLIGHT` in `tools/flight_host_differential.py`)
is lm-p52-realign with SAVQR52 (QTEMP1, 03665) poked at 24 s, during R52's
V01N70, to point to itself. After the R60 maneuver (ENTER at 34 s), R52's
GOTO SAVQR52 loops in GOTOERS:

- **The harness.** The watchman restarts the AGC at 35.84 s on both scaled
  clocks (171 stuck steps on the work clock). R52 restarts at V01N70 and
  P52 goes on to its end. No native routine misses yaAGC, and yaAGC's
  NEWJOB accesses agree with the rule at every check.
- **The x86 program.** `make x86-flight-scenarios` flies the row against
  the harness. The watchman comes at 35.85 s, within the TC trap rows'
  limit of the harness's 35.84 s. The x86 run ends with the same major
  mode, REDOCTR and state vector, and 11 words differ (limit 34).
- **The C64.** The resident does not model the watchman yet. Its boundary
  words hold no GOTO, so no interrupt is taken at the stuck point either:
  the C64 hangs there instead of restarting. This is listed as a known gap
  in the restart acceptance docs. Nominal flight never trips it.

## The rupt lock

On the AGC (yaAGC `agc_engine.c`) the rupt lock alarm arms every 160 ms, at
scaler phase 0400 of 0777, and checks 140 ms later, at 0300. It is armed
twice over. RUPT LOCK is cleared by any instruction that ends outside an
interrupt, so by each RESUME; still set, the AGC was held in interrupts.
NO RUPT is cleared by any instruction that ends inside one; still set, no
interrupt came for 140 ms. Either one sounds the alarm, sets channel 77 bit
4 and restarts the AGC.

`platform/x86/night_watchman.h` (`rupt_lock`, #989) takes it on the clock
period count, as the watchman does: arm 8 periods into each 16, check at 6.
A step ran inside an interrupt if it started or ended in one, and outside
one if it started or ended outside. An interrupt step that ends at RESUME
does both. Clock periods are taken at the boundary after the steps that ran
in them, and a long step runs over several. So each period counts every
step since the last period, or the last step, still running, when none ran.

- **Checked against yaAGC.** The harness sets yaAGC's two flags at the
  start of every lockstep and bridge span and reads them back. A bridged
  span is yaAGC's code, so it counts for the native rule as it counts for
  yaAGC. At every check, on every scaled row, the native rule must sound
  exactly where yaAGC's flags would, and no span may leave channel 77 bit 4
  set. `tools/flight_host_differential.py` fails the row otherwise. Each
  row's summary records the alarms and the longest runs of clock periods
  with nothing inside an interrupt and nothing outside one.
- **Results.** On the 81 flight rows the alarm never sounds. The longest run
  without an interrupt step is 12 periods (`cm-p52-scaled-x1`, T4RUPT's
  period), and the longest without a step outside one is 8. With
  `FLIGHT_DOWNRUPT` (on the work clock), `cm-p11`, `cm-cdh-csm` and
  `cm-stable-orbit-lm` go at most 2 periods without an interrupt; without
  it, 11. The alarm sounds only on the boot rows under the C64 tick
  schedules, where yaAGC's flags agree at every check. There the LM boot is
  starved by interrupts, as for the watchman, and the CM P06 boot holds
  INHINT past 140 ms with interrupts pending. On the work clock neither
  happens.
- **No restart yet.** The harness and the x86 program count and print where
  it would sound ("rupt lock would sound at"), but restart nothing. As a
  restart, the CM boot rows on x1 and overrun restart over 200 times, and
  the watchman's check then disagrees on one of them. The C64 waits for
  #990.

## Scenarios (`make flight-restart-check`)

`tools/flight_restart_check.py` runs each scenario twice, once without the
GOJAM and once with it. It also takes native snapshots (`FLIGHT_SNAPSHOT`)
just before the GOJAM, just after it and at the end. A scenario fails in
any of these cases:

- The restart run stops early.
- The restart run adds a routine that yaAGC does not match.
- A point in `FRESH_START_AND_RESTART`, `RESTARTS_ROUTINE` or
  `PHASE_TABLE_MAINTENANCE` runs outside a routine table.
- REDOCTR does not go up by one.
- RSBBQ+1 does not hold the interrupted Z.
- The major mode or the alarm registers at the end differ from the run
  without the restart.
- In an unscaled run, more samples differ than the slack allows.

Results on base b3a56a5a with this change. `lm-p63-descent-restart` is also registered with
`tools/flight_host_differential.py`.

| Scenario | GOJAM | Groups restarted (phase, octal) | Native routines (yaAGC did not match: points, new) | After |
|---|---|---|---|---|
| `lm-p63-descent-restart` (lm-p63-descent-thrust, scaled x1) | 200 s, braking phase, 54 s after ignition | 2 (phase 21), 5 (phase 3) | 1,276,090 (8, 0 new) | Back in P63. The LLGE flies on. At 300 s TTF/8 equals the run without the restart, and the alarms are the same (01107, 01703). |
| `lm-dap-hold-restart` (DAP holding attitude, overrun schedule) | 20.18 s | none | 4,767 (0) | P00. The DAP restarts through STARTSB1's IDLEADR. |
| `cm-entry-restart` (cm-entry, scaled x1) | 250.69 s, P64 | 1 (phase 3), 5 (phase 3), 6 (phase 11) | 1,431,996 (1, 0 new) | P67 at the end (320 s), as without the restart. |
| `lm-p00-restart` (unscaled) | 25 s | none | 256,955 (0) | 3,872 of 4,000 samples equal (4,000 without the restart). |
| `cm-p00-restart` (unscaled) | 25 s | none | 259,971 (0) | 3,998 of 4,000 samples equal, the same as without the restart. |
| `lm-v69-restart` (unscaled, V69E at 23.5 s) | yaAGC's TC trap at 25.01 s, Z at VERB69 | none | 256,682 (0) | P00. 4,000 of 4,000 samples equal, the same as without the restart. |
| `cm-v69-restart` (unscaled, V69E at 23.5 s) | yaAGC's TC trap at 25.01 s, Z at VERB69 | none | 259,716 (0) | P00. 3,998 of 4,000 samples equal, the same as without the restart. |

The 8 LM and 1 CM unmatched points are the same ones the scenarios have
without the restart. They are the compiled interpretive Q sign extension
(docs/FLIGHT-STORE.md). No restart point is bridged.

In the LM P63 run, a later path reaches BUFNEG+27 (INTERPRETER, 00,2560)
once. yaAGC bridges it.

The 128 LM P00 samples are a matter of native interrupt latency, not of
the restart path. STARTSB1 makes DAPIDLER call 1/ACCS. Natively, AOSTASK's
routine from GETAOSUV+6 runs for about 1.2 cs as one call, while yaAGC takes
the T4RUPT due in that time at INVERT. After that, T4RUPT runs a centisecond
late natively, so one sample in each 120 ms differs in DSRUPTSW. Scaled time
has no such difference, because there every routine runs under the same
tick schedule.

## On the C64 store (`make flight-restart-vice`)

`platform/c64/restart_check.c` boots the flight resident on the mission's
store (`build/flight-store/lm.reu` or `cm.reu`). Idle is ADVAN (Luminary
01,3214, Comanche 01,3231) with NEWJOB -0, as DUMMYJOB leaves it. It loads the native state from just after the
GOJAM (the `-gojam` snapshot: Z 04000) and runs resident steps from there.

1. **Burst.** With CIA1 interrupts off, the resident runs to the next idle
   pass through ADVAN, or to the first point the store does not hold. Then
   the erasable image at $7000 is compared with yaAGC run from the same
   state, with its clock frozen and no interrupt taken
   (`tools/flight_restart_reference.c`), up to the same point.
2. **Resume.** With CIA1 interrupts on, the resident runs for 200 cs of AGC
   time. This step is reported, not checked.

| Scenario | Burst | Image against yaAGC | Resume |
|---|---|---|---|
| `lm-p63-descent-restart` | 24 resident steps: GOPROG through the group 2 and 5 restarts, ENDRSTRT and the display job, to idle at ADVAN | 2,048 / 2,048 words equal (yaAGC: 1,734 instructions) | 1,441 steps over 200 cs, no stop |
| `lm-dap-hold-restart` | 12 resident steps, to idle at ADVAN | 2,048 / 2,048 words equal (yaAGC: 938 instructions) | 911 steps over 200 cs, no stop |
| `cm-entry-restart` | 37 resident steps: GOPROG, the group 1, 5 and 6 restarts and the jobs they start, to idle at ADVAN (01,3231) | 2,048 / 2,048 words equal (yaAGC: 2,647 instructions) | 1,351 steps over 200 cs, no stop |
| `cm-p52-restart` | 81 resident steps: GOPROG, the group 4 restart (phase 00502, N93 gyro torquing) and its jobs, to idle at ADVAN | 2,048 / 2,048 words equal (yaAGC: 5,657 instructions) | 676 steps over 200 cs, no stop |

## The GOJAM on the C64 (`make flight-gojam-vice`)

The C64 side of a hardware restart is split with the Resident-kernel lane
(RK). This lane provides the restart itself: a module and a runtime call.
RK provides the resident hook that calls them and the triggers (V69, the
night watchman, a rupt lock). Neither piece adds a resident byte. The
resident needs only the module id.

### The GOJAM module, 0x01E5 (`RK_MODULE_GOJAM`)

`platform/c64/resident_gojam_module.c` is registered with the resident's
own modules (`tools/resident_flight_build.py` `RESIDENT_MODULES`, LM and
CM), so every flight store (`tools/flight_store_build.py`) and the resident
flight check stores hold it. It is a task-slot module (class 1), linked for
the job slot in a store with a class-4 module, as the other resident modules
are. It has 1 selector, no resolver record and no services. Its main makes
the same change as `gojam_native` (`tools/flight_gojam.h`), then returns
`DONE`:

- Q (word 2) takes Z (word 5), and Z becomes 04000.
- `rk_device.interrupt_enabled` = 1 (and `rk_flight`'s copy), and
  `rk_hardware.in_interrupt` = 0.
- The HANDRUPT traps in `rk_flight` (`trap31a`, `trap31b`, `trap32`) are
  cleared. On the CM, `rk_device` (`cm_native_io`) holds copies of the
  traps and of channels 11-14, and those are cleared too.
- Channels 5, 6 and 10-14 in `rk_hardware`, and 34 and 35 in `rk_flight`,
  are zeroed.
- Channel 33 bit 11 (`02000`) is set in `rk_hardware`.
- The superbank, every other erasable word and every other field stay.

It reaches `rk_hardware` through `rk_env.program->hardware`, `rk_device`
through `rk_env.io`, and `rk_flight` at `RK_FLIGHT_IO`. It cannot reach
`rk_pending`, which has no fixed address, so the caller clears it.

### `apollo_ov2_restart_keep_pinned`

```c
void apollo_ov2_restart_keep_pinned(apollo_ov2_runtime *);   /* include/overlay_v2.h */
```

It does what `apollo_ov2_restart` does for the killed jobs:

- Every seam of all nine contexts is dropped.
- Both run-slot caches (job and task: `loaded`, `selectors`) are cleared.
- `busy` and the fault latch are cleared.

The boot-loaded slot classes stay loaded: the fixed extension at $D000
(class 2), the pinned module at $C000 (class 3), whose table list
`rk_pinned` keeps naming, and the pinned task region at $E000 (class 4). It
writes no slot, header buffer or image byte. With a class-4 module, the
task-slot cache stays empty, as boot left it.

The runtime has no record of the class 2-4 modules, so on the runtime's own
fields the variant has the same effect as `apollo_ov2_restart`. The two
share one body. What the variant adds is the contract: the resident must not
reload classes 2-4 after it, and cannot, because their loader ran in the
job slot at boot. `apollo_ov2_restart` and its callers are unchanged.
`tests/test_overlay_v2.c` (`make -f tools/overlay_v2.mk overlay-v2-test`)
checks the variant on the class-4 store image. It sets seams in job and
task contexts, a cached job module, a latched fault and `busy`, then
checks the following:

- After the variant, the runtime is byte-for-byte what `apollo_ov2_restart`
  leaves.
- The image, the header buffer, $C000-$CBFF and $E000-$FFF9 are unchanged.
- A job run reloads its module and leaves the class-4 region in place.
- A fault a run latched clears the same way.

### Entry contract (RK's resident trigger)

Before the call:

1. Call at a resident step boundary, never from the IRQ handler or from
   inside a module run. `rk_runtime.busy` must be 0. CIA1 interrupts may
   stay on: the call loads a module by DMA, as any step does.
2. If `rk_runtime.fault` is latched, call
   `apollo_ov2_restart_keep_pinned(&rk_runtime)` first. Otherwise
   `apollo_ov2_run` refuses the module with `APOLLO_OV2_FAULTED`. Calling
   it on both sides of the module is harmless.

The call:

3. `rk_run_module((apollo_ov2_cont){RK_MODULE_GOJAM, 0})`. It must return
   `APOLLO_OV2_OK` with `rk_env.status` still `EXEC_OK`. It runs in
   context 0, or in context 8 inside an interrupt, and either is fine.

After the call:

4. Set `rk_pending = 0`: every interrupt request is dropped, as the
   hardware does.
5. Call `apollo_ov2_restart_keep_pinned(&rk_runtime)`. The killed jobs'
   seams go, and so does the GOJAM module's cache entry.
6. Do not reboot, reload classes 2-4, or touch `rk_pinned`, `rk_ticks`,
   `rk_dilation` or the superbank. The AGC clock runs on.
7. Return from the step (EXEC_OK). The next `rk_resident_step` runs the
   rope's GOJAM code from 04000: GOPROG, the phase-table check,
   STARTSUB/STARTSB2 and the group restarts.

### The check

`--vice --gojam-module` (`make flight-gojam-vice`, in `ci-extra-c64`)
compiles `platform/c64/restart_check.c` with `RS_GOJAM_MODULE`. It loads the
`-pre` snapshot: the native state at the restart point, before the GOJAM.
The program links `resident_flight.c` into itself to call its static
`rk_run_module`. Then it does what the contract above says (steps 3 to 5),
with no resident trigger. Two VICE runs follow for each scenario:

1. **The GOJAM** (`RS_GOJAM_ONLY`): the run stops after the module. The
   image at $7000 must equal the post-GOJAM snapshot (the `-gojam` one,
   from `gojam_native` on the host) in all 2,048 words. The `rk_hardware`,
   `rk_flight` and `rk_device` fields must equal that snapshot's
   `SNAP_HW`, `SNAP_FLIGHT` and `SNAP_IO`.
2. **The burst**: the module, `apollo_ov2_restart_keep_pinned`, then the
   restart burst as in `flight-restart-vice`. Its image must equal yaAGC's
   from the post-GOJAM state, and the runtime must end with no cached run
   slot and no fault.

Results on base 1b20b3a8 with this change. The module is 486 bytes in
the LM store (job slot, since the LM store has a class-4 module) and 730
bytes in the CM store (task slot). Both stores keep their image size
(1,499,392 and 1,434,880 bytes), with no new overflow or link failure in
`report.json`. The resident sizes (`build/resident-flight/sizes.json`) are
unchanged.

| Scenario | GOJAM module on the C64 against the post-GOJAM snapshot | Burst after `apollo_ov2_restart_keep_pinned` | Image against yaAGC |
|---|---|---|---|
| `lm-p63-descent-restart` | `APOLLO_OV2_OK`, `EXEC_OK`. Q 03214, Z 04000. 2,048 / 2,048 words, 28 / 28 device fields | 24 resident steps, to idle at ADVAN. No cached run slot, no fault | 2,048 / 2,048 words equal (yaAGC: 1,734 instructions) |
| `lm-dap-hold-restart` | `APOLLO_OV2_OK`, `EXEC_OK`. Q 03214, Z 04000. 2,048 / 2,048 words, 28 / 28 device fields | 12 resident steps, to idle at ADVAN. No cached run slot, no fault | 2,048 / 2,048 words equal (yaAGC: 938 instructions) |
| `cm-entry-restart` | `APOLLO_OV2_OK`, `EXEC_OK`. Q 03231, Z 04000. 2,048 / 2,048 words, 28 / 28 device fields | 37 resident steps, to idle at ADVAN. No cached run slot, no fault | 2,048 / 2,048 words equal (yaAGC: 2,647 instructions) |
| `cm-p52-restart` | `APOLLO_OV2_OK`, `EXEC_OK`. Q 03231, Z 04000. 2,048 / 2,048 words, 28 / 28 device fields | 81 resident steps, to idle at ADVAN. No cached run slot, no fault | 2,048 / 2,048 words equal (yaAGC: 5,657 instructions) |

The `-pre` snapshot is taken with `FLIGHT_SNAPSHOT_PENDING`, at the first
sample point at the GOJAM time even with an interrupt pending, because that
is where `FLIGHT_GOJAM` restarts. Without it, the snapshot waits for the
interrupt to run, so in `cm-p52-restart` it came one interrupt later than the
GOJAM and 20 words differed.

These are the same bursts as in `flight-restart-vice`, which starts from
the host's post-GOJAM state. So applying the GOJAM on the C64 gives the same
restart.

## Gaps

- **Every resume runs its 200 cs.** Since Lane C's Pinball split (#425),
  DSPMMJB and the display jobs are in the store, so every burst above runs
  to idle. Since the LM store fit (#625), P20-P25 (`lm_p20_p25`) is in the
  store too, so the P63 resume runs on past R12LITES, where it used to stop
  after 409 steps. The step counts above are from base 3384c02b; the whole
  LM P63 restart window (`--scenario lm-p63-descent-restart --snap-cs 19950
  --cs 1050`) runs to its end there with no stop (20.7x real time).
- **The TC trap is the C64's one GOJAM source.** The resident step spins
  at VERB69 until the trap fires and then follows the entry contract
  (`rk_gojam`, `platform/c64/resident_flight.c`; see The TC trap (V69)
  above). The resident's main (`platform/c64/resident_flight_main.c`) used
  to take a GOJAM after any step that left Z and FB as they were with Q =
  Z + 1; it now just steps, since no other step spins. A refusal
  (`EXEC_INVALID_ACTION`) stops the run instead (#607). Compiled routines
  refuse for points they do not cover, and that must stop, not restart.
  The compiled TCs and TCFs to themselves refuse: LM T6JOBCHK +3 (17,2032)
  with TIME6 negative (`src/lm_t6rupt.c`), and Comanche055 TVCEXECUTIVE
  16,2763 (`tools/basic_compile.py` TCF +0, marked NOT USED) from its
  routine's entry, with Z not at the trap and the routine's earlier writes
  kept. If a reachable one appears, the compiler should leave Z at the trap
  and the resident should spin there as at VERB69. The night watchman (NEWJOB not
  tested for 0.64 s) is modelled on the host and the x86 program, not on
  the C64 (The night watchman, above). The rupt lock (an interrupt held too
  long, or none for 140 ms) is counted and checked against yaAGC on the
  host and the x86 program, but restarts nothing yet (The rupt lock, above). The stack canary now stops the run with `EXEC_OUT_OF_SCOPE`,
  so a stack overflow is never taken for a TC trap. `rk_gojam` also lights
  the DSKY RESTART lamp (`rk_restart_lamp` at `RK_RESTART_LAMP`, $6FF5),
  the hardware flip-flop ERROR RESET's `WRITE 77` clears. Pinball's
  `restart_light` reads and writes it there (Lane C).
- **The x86 program** (docs/X86-FLIGHT.md) takes `FLIGHT_GOJAM`, the TC
  trap and the night watchman through `fd_gojam`, which also lights its RESTART lamp; RSET and
  channel 11 bit 10 put the lamp out, as in yaAGC.

## Telemetry

With `FLIGHT_DOWNRUPT=1` the harness raises DOWNRUPT every 20 ms of AGC time
on both sides. The downlink word stream is then checked against yaAGC
through each of these restarts (docs/FLIGHT-DOWNLINK.md,
`make flight-downlink-check`). After the GOJAM the stream starts the current
list again from its ID word: STARTSUB points DNTMGOTO at LDNPHAS1, SETINFL
copies AGSWORD into DNLSTCOD, and the hardware GOJAM zeroes channels 34 and
35. Every check here still passes with the knob on.
