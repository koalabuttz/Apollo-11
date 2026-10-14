# Hardware restarts in flight

Lane A2 (restart and telemetry). A hardware restart (GOJAM) in the middle of
a flight scenario, the AGC's own restart code running natively, and the
resumed program checked against yaAGC. On the host this is the whole run; on
the C64 flight store it is the restart burst so far (see Gaps).

## How the restart is forced

`FLIGHT_GOJAM=CS[,CS...]` (`tools/flight_gojam.h`, read by
`tools/flight_host_run.c`) restarts the AGC at the first sample point at or
after each CS, on both sides. A sample point is a clean point: idle at
ADVAN, no interrupt in progress or pending, no instruction half done. The
harness runs no DOWNRUPT, so no restart lands inside one. The pokes for that
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

`lm-p63-descent-restart` is also registered with
`tools/flight_host_differential.py`.

| Scenario | GOJAM | Groups restarted (phase, octal) | Native routines (yaAGC did not match: points, new) | After |
|---|---|---|---|---|
| `lm-p63-descent-restart` (lm-p63-descent-thrust, scaled x1) | 200 s, braking phase, 54 s after ignition | 2 (phase 21), 5 (phase 3) | 1,276,090 (8, 0 new) | Back in P63. The LLGE flies on. At 300 s TTF/8 equals the run without the restart, and the alarms are the same (01107, 01703). |
| `lm-dap-hold-restart` (DAP holding attitude, overrun schedule) | 20.18 s | none | 4,767 (0) | P00. The DAP restarts through STARTSB1's IDLEADR. |
| `cm-entry-restart` (cm-entry, scaled x1) | 250.69 s, P64 | 1 (phase 3), 5 (phase 3), 6 (phase 11) | 1,431,996 (1, 0 new) | P67 at the end (320 s), as without the restart. |
| `lm-p00-restart` (unscaled) | 25 s | none | 256,955 (0) | 3,872 of 4,000 samples equal (4,000 without the restart). |
| `cm-p00-restart` (unscaled) | 25 s | none | 259,971 (0) | 3,998 of 4,000 samples equal, the same as without the restart. |

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

`platform/c64/restart_check.c` boots the flight resident on
`build/flight-store/lm.reu`. It loads the native state from just after the
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
| `lm-p63-descent-restart` | 23 resident steps: GOPROG through the group 2 and 5 restarts and ENDRSTRT, to DSPMMJB (40,3534), EXEC_UNMAPPED | 2,048 / 2,048 words equal (yaAGC: 1,372 instructions) | 23 steps in 42 cs, then stops at DSPMMJB |
| `lm-dap-hold-restart` | 11 resident steps, to DSPMMJB, EXEC_UNMAPPED | 2,048 / 2,048 words equal (yaAGC: 859 instructions) | 11 steps in 19 cs, then stops at DSPMMJB |

## Gaps

- **DSPMMJB is not in the C64 store.** GOPROG3 always calls MMDSPLAY, so
  after any restart the first job the Executive runs is Pinball's DSPMMJB.
  The store has no record for it, because `lanec_pin_routines` is too big
  to split (`report.json`: an overflow of 236,641 bytes). The C64 resume
  stops there, as would any major-mode display. The LLGE (`lm_llge`) and
  P12 (`lm_p12`) do not fit either. So P63 and P12 cannot resume on the C64
  until those modules fit.
- **No GOJAM source on the C64.** The resident has no TC trap, night
  watchman or rupt lock. A C64 restart needs these:
  - a resident entry that makes the same change as `gojam_native`;
  - `apollo_ov2_restart` for the job seams of the killed jobs, keeping the
    pinned slots;
  - a trigger for each alarm, for example `TC VERB69` to itself (V69),
    ADVAN or NEWJOB not reached for 0.64 s (night watchman), or an
    interrupt held for too long (rupt lock).

  That is the Resident-kernel lane's code. The check program here applies
  the GOJAM on the host before it takes the snapshot.
- **The x86 program** (docs/X86-FLIGHT.md) does not take `FLIGHT_GOJAM` yet.
  `platform/x86/flight_scenario.h` is Lane B's.

## Telemetry next

The harness runs no DOWNRUPT, so DOWNTELEMETRY's word stream is not yet
compared. In this scenario, the restart sets the stream back.

- STARTSUB points DNTMGOTO at LDNPHAS1, so the next DOWNRUPT starts the
  current downlist from its first word.
- SETINFL copies AGSWORD into DNLSTCOD.
- The hardware GOJAM zeroes channels 34 and 35.

To compare the stream, a DOWNRUPT request would be raised every 20 ms of
AGC time on both sides (`rk_timer_tick` schedule natively,
`InterruptRequests[8]` in yaAGC). Each channel 34 and 35 pair written
would be logged with its time. The two logs would then be compared
(yaAGC's `DecodeDigitalDownlink` names the lists). The lockstep compares
erasable memory, not output channels, so this log is the check.
