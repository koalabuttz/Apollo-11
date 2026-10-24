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

The same meter runs in two places:
- on the host, `tools/flight_host_run.c` with `FLIGHT_HOLD=1`, over the replay window;
- on the C64, `tools/flight_scenario_vice.py --replay --hold`, which builds with `-DRK_HOLD_METER`, a measurement build only. The result is in mailbox bytes 0x36-0x39.

Where a window matches the host, both sides report the same hold.

## Results

`tools/rupt_hold_report.py` runs the host over every replay window in the program maps' `replay_windows`. It writes `Luminary099-interrupt-hold.json` and `Comanche055-interrupt-hold.json`. The run below was at b56219b3, with the #838 meter.

| Program | Windows | Median hold | Longest hold | Where |
|---|---|---|---|---|
| Luminary099 | 62 | 50 MCT (0.6 ms) | 1,186 MCT (13.9 ms) | lm-coast-verbs 2500+1000, TIME1 05624 |
| Comanche055 | 79 | 325 MCT (3.8 ms) | 1,425 MCT (16.7 ms) | cm-p20 9210+300, TIME1 22401 |

The C64 meter was checked against the host (C64 runs at 1cfd6bc5, the base before #674; the host values are the same at b56219b3):

| Window | Host | C64 | Note |
|---|---|---|---|
| cm-p20 9210+300 | 1,425 MCT | 1,425 MCT | ends in the host's state |
| lm-landing-ascent-nassp 178000+200 | 428 MCT | 428 MCT | ends in the host's state |
| lm-coast-verbs 2500+1000 | 1,186 MCT | 1,204 MCT | the window differs from the host at 0020 on this base (Interpreter's row in replay-mismatches.md) |

## Bound

**A request waits at most 1,706 MCT (20 ms) for a native boundary.** That is one DOWNRUPT period (20 ms), the shortest fixed interrupt period in flight, so a held request can't let a second DOWNRUPT arrive and merge with it.

The longest measured hold is 1,425 MCT, 84% of the bound. `tools/rupt_hold_report.py --bound 1706` fails a window that holds a request longer.

The bound is a limit on the port, not a claim that the real AGC behaves this way. A held T6RUPT ends an RCS jet firing up to that much later than the AGC would. The host and C64 agree on it, so replay matches. Flight effect is a model question for the lanes that own the DAP (D1) and powered flight (F).
