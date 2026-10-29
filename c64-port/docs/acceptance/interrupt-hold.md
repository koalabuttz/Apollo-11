# Interrupt hold: how long a request waits for a native boundary

The real AGC takes an interrupt request between any two instructions, unless interrupts are inhibited, it is already in an interrupt, A holds overflow, or it is inside EXTEND or INDEX. The port takes a request only at a native routine boundary. Since #825, a job's split-file part chain also counts as one routine, as the host runs it. So a long routine holds a pending request until it ends (#838 item 1).

Frozen-clock lockstep with yaAGC cannot see this, because the harness takes the interrupt at the same boundary. This page measures the hold and gives it a bound.

## What is measured

`include/rupt_hold.h` meters each request, on the replay window's work clock, in machine cycles (MCT; 1 MCT = 11.72 µs). A request's hold runs:

- **from** the later of:
  - the work clock step that raised it, or the first boundary that saw it pending;
  - the last boundary where the AGC itself would have held it (in an interrupt, interrupts inhibited, or overflow in A).
- **to** the boundary that takes it.

The second "from" case is an upper bound: the AGC would take the request somewhere inside the next routine, at its RELINT or RESUME.

Cycles stolen while a request waits (`FLIGHT_STEAL`, the counter cycles Apollo 11's RR CDU counts took) stay in the elapsed hold, since the AGC takes a waiting request at its next instruction and a steal delays only the port's take. In a replay window `FLIGHT_STEAL` spreads each period's stolen MCT over the period's three steps by their length, and charges each step's share as the clock passes the step's end, also when the end falls inside the routine holding the request (issue #912 item 3). So a routine of W MCT of work under a steal of N MCT a period takes about W × 853 / (853 − N) MCT of clock, as the AGC's would. Before, the steal of the steps a routine spanned went in only after the take, so a hold showed at most the steal of the step that raised it. The host and the C64 replay harness (`RK_STEAL`, platform/c64/resident_flight.c) charge it the same way. Under `FLIGHT_STEAL` the host also prints the MCT stolen during the hold and the longest hold without them, the hold's work:

```
interrupt hold: max 4149 MCT at TIME1 11450 (1149 without 3000 stolen)
interrupt hold work: max 1241 MCT at TIME1 11021 (2700 stolen)
```

The same meter runs in two places:
- on the host, `tools/flight_host_run.c` with `FLIGHT_HOLD=1`, over the replay window;
- on the C64, `tools/flight_scenario_vice.py --replay --hold`, which builds with `-DRK_HOLD_METER`, a measurement build only. The result is in mailbox bytes 0x36-0x39. That build leaves out the idle memo (`RK_IDLE_MEMO`, docs/RESIDENT-FLIGHT.md) to make room for the meter in the overload replay harness: the memo writes the result and charges the work the idle step itself would (tests/test_idle_memo.c), so the work clock and the meter read the same without it, only slower.

Where a window matches the host, both sides report the same hold.

## Results

`tools/rupt_hold_report.py` runs the host over every replay window in the program maps' `replay_windows`. It writes `Luminary099-interrupt-hold.json` and `Comanche055-interrupt-hold.json`. The run below was at b56219b3, with the #838 meter.

| Program | Windows | Median hold | Longest hold | Where |
|---|---|---|---|---|
| Luminary099 | 62 | 50 MCT (0.6 ms) | 1,186 MCT (13.9 ms) | lm-coast-verbs 2500+1000, TIME1 05624 |
| Comanche055 | 79 | 325 MCT (3.8 ms) | 1,425 MCT (16.7 ms) | cm-p20 9210+300, TIME1 22401 |

The C64 meter was checked against the host (C64 runs at 1cfd6bc5, the base before #674; the host values are the same at b56219b3). cm-p20 9210+300 was rerun at c8fc9622, after #674 and #849: host 1,425 MCT, C64 1,425 MCT, and the C64 ends the window in the host's state.

| Window | Host | C64 | Note |
|---|---|---|---|
| cm-p20 9210+300 | 1,425 MCT | 1,425 MCT | ends in the host's state |
| lm-landing-ascent-nassp 178000+200 | 428 MCT | 428 MCT | ends in the host's state |
| lm-coast-verbs 2500+1000 | 1,186 MCT | 1,204 MCT | the window differs from the host at 0020 on this base (Interpreter's row in replay-mismatches.md) |

## Bound

**A request waits at most 1,706 MCT (20 ms) for a native boundary.** That is one DOWNRUPT period (20 ms), the shortest fixed interrupt period in flight, so a held request can't let a second DOWNRUPT arrive and merge with it.

The longest measured hold is 1,425 MCT, 84% of the bound. The bound is `HOLD_BOUND_MCT` in `tools/flight_scenario_vice.py`. `tools/rupt_hold_report.py` checks it over every program map window, and every `--replay` window checks it, so each replay window CI runs is gated:

- **Nominal windows** gate the elapsed hold: the host's, and with `--hold` the C64's too.
- **Flight-steal windows** (`FLIGHT_STEAL` at or under the flight's share, `fhd.FLIGHT_STEAL_MCT`, below) gate the elapsed hold, stolen MCT and all: the host's, and with `--hold` the C64's too.
- **Overload windows** (`FLIGHT_STEAL` above the flight's share) gate the host's hold work, the longest hold without its stolen MCT. Every such window still prints its stolen MCT. The C64 meter has no stolen share, so with `--hold` its elapsed hold is held to the host's instead: it fails when longer than the host's elapsed hold plus `HOLD_C64_SLACK_MCT` (0, since the holds agree exactly where a window matches).

### Known limit: no take inside a basic math body

The port takes no interrupt inside a basic math body. UNIT, SQRT and the trig routines are about 1,100 MCT each. Under overload, cycles stolen inside such a body add to the elapsed hold without adding any work, and with the steal charged as the clock runs they stretch it by 853 / (853 − N). Three overload windows hold a request longer than the bound, on the host and the C64 alike (73054cae with this charge, host and C64 matching):

| Window | Steal (MCT a period) | Elapsed hold | Work | Stolen |
| --- | --- | --- | --- | --- |
| cm-entry-overload 20900+600 | 600 | 4,149 MCT at TIME1 11450 | 1,149 | 3,000 |
| cm-entry-overload 22000+400 | 600 | 3,635 MCT at TIME1 13461 | 1,085 | 2,550 |
| lm-p63-overload-nomon 19400+750 | 420 | 2,242 MCT at TIME1 06651 | 982 | 1,260 |

The CM holds are T5RUPT inside the interpreter (`cm_body_attitude_interpretive` at 03,2037, NEWOPS). The program map's overload windows (cm-entry-overload 20301+300 and 20370+300) end before the steal starts, so they hold 1,013 and 850 MCT, unchanged. This is a known limit of the port at base: the overload gate counts work, every window's work is under the nominal windows' 1,425 MCT maximum, and the bound stays 1,706 MCT. The risk the bound covers is open in these windows: an elapsed hold longer than one DOWNRUPT period lets a second DOWNRUPT arrive and merge with the first, and a downlink word is lost. The host and the C64 agree on that, and frozen-clock lockstep cannot see it, so it is not checked against the AGC anywhere. Most of these holds are in the interpreter's dispatch, across the CM's and the LM's interpretive tables, and a single math op stretched by a 600 MCT steal is already over the bound. Meeting it there would take a take point between interpretive ops and inside the math bodies.

### The flight's steal

The overload rows steal far more than the flight did, to force the 1201/1202 alarms. Apollo 11's RR CDU counts took about 13% of the LGC's duty cycle at the landing: "An uncorrected problem in the rendezvous radar interface stole approximately 13% of the computer's duty cycle" (Don Eyles, [Tales from the Lunar Module Guidance Computer](https://klabs.org/history/apollo_11_alarms/eyles_2004/eyles_2004_z.htm), AAS 04-064, 2004, abstract). That is 111 of a period's 853 MCT (`fhd.FLIGHT_STEAL_MCT`). `cm-entry-steal13` and `lm-p63-steal13-nomon` (`fhd.STEAL_FLIGHT`) are the overload rows with the steal set to that share, over the same windows. They gate the elapsed hold against the bound on both sides (`make hold-flight-steal`). At 9797e2c9:

| Window | Elapsed hold, host and C64 | Work | Stolen |
| --- | --- | --- | --- |
| cm-entry-steal13 20900+600 | 1,141 MCT at TIME1 11013 | 1,002 | 139 |
| cm-entry-steal13 22000+400 | 1,063 MCT at TIME1 13304 | 925 | 138 |
| lm-p63-steal13-nomon 19400+750 | 993 MCT at TIME1 06024 | 826 | 167 |

Each window's C64 replay matches the host's state at every checkpoint. The holds are under the bound with 565 MCT or more to spare, so at the steal that flew, a DOWNRUPT is not merged.

The bound is a limit on the port, not a claim that the real AGC behaves this way. The host and C64 agree on every hold, so replay matches.

## T6RUPT: late jet-off

A T6RUPT that comes due inside a routine is taken at the next native boundary, so the DAP's jet-off write to channels 5 and 6 goes out that much later than the AGC's would. The same meter, counting T6RUPT requests only (a local change of `RUPT_HOLD_PENDING` to `02u`, the T6RUPT bit, on the host, over every program map window, at 36103941):

| Program | Windows with a held T6RUPT | Longest | Where |
|---|---|---|---|
| Luminary099 | 2 of 62 | 259 MCT (3.0 ms) | lm-abort-p71-nassp 115000+600, TIME1 27006; next lm-p42-nassp 48800+1000, 11 MCT |
| Comanche055 | 2 of 79 | 3 MCT (0.04 ms) | cm-tei 51100+300 and cm-v48-dap 7450 |

3.0 ms is about 5 TIME6 ticks and about a fifth of the DAP's 14 ms minimum jet pulse, in one window. The host's RCS model (`FLIGHT_RCS`) has no attitude dynamics, and the extra translation is about 1e-4 m/s a jet. The only cure would be taking T6RUPT inside a routine, which the native design does not do, so this stays a documented limit (#838 part b). Results: /mnt/project-files/cert-fixes/t6-hold/.
