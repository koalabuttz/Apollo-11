# WAITLIST, native tasks and CIA clock events

The fifth milestone translates the bounded WAITLIST insertion, T3RUPT2 dispatch
and TASKOVER paths. Nine task 2CADRs, eight encoded differential intervals,
TIME3 and RUPTAGN retain the original words. A separate native task trampoline
runs ordinary C callbacks under exclusion. CIA1 Timer B publishes elapsed ticks;
foreground code collects them and dispatches tasks at declared safe points.
This foreground delivery is an explicit adaptation of Apollo's interrupt tasks.

## Original list and counter behavior

`wl_schedule(state, delay, address)` takes a positive 1..16250 centisecond delay
and a complete basic historical 2CADR. WAIT2 compares the new deadline with TIME3
and the ascending LST1 differential intervals. Equal deadlines insert after
existing tasks. WTLST2 splits an interval; WTLST4 shifts the affected lists.
The native routine retains signed-zero and uncorrected accumulator arithmetic,
including CCS's separate -0 branch. It does not substitute a generic task queue.

TIME3 counts toward positive overflow. PINC of 037777 resets it to +0 and requests
T3RUPT; subsequent counter increments measure time already overdue. WAIT2 has
an octal 0200 window for recognizing that overflow. At exactly TIME3=0200 its
initial CCS sees -0 and takes a different path. The raw synthetic boundary tests
reproduce this behavior. The native runtime rejects an unserviced overflow at
128 centiseconds or later, before changing TIME3, rather than admitting tasks
with an ambiguous timer phase. The 128cs check is a phase limit, not a deadline.

`wl_advance` supplies controlled centisecond PINCs and reports overflow. The
runtime owns a separate pending latch. Counter progress during assembly tests
comes from Virtual AGC's real CounterPINC routine; it is not inferred from the
native implementation or the host time spent executing assembly.

`wl_begin` executes T3RUPT2's list shift and timer reload, returns the first task
pair, and sets RUPTAGN to +1 on positive arithmetic overflow or -0 otherwise.
`wl_taskover` dispatches another already-due task when RUPTAGN=+1; otherwise it
returns idle. The oracle checks the actual physical task transfer and the original
lead-in's saved bank and TASKOVER's restored BBANK/SUPERBNK. The native callback
ABI and platform restore replace hardware interrupt entry/RESUME mechanisms.

## Initialization and alarm boundaries

`wl_init` sets TIME3=037777, LST1 words=057777, and all nine LST2 pairs to the
positive ENDTASK pair {05240,04063}, as in fresh-start resource initialization.
RUPTAGN=-0 and the alarm marker=0 are deterministic native initialization.
ENDTASK points to SVCT3, which can schedule IMU housekeeping in Apollo. The
native trampoline treats its pair as a built-in no-op task followed by TASKOVER;
SVCT3's drift/IMU body is outside this milestone. Empty native lists therefore
still execute the original dummy timer/list maintenance when due.

The original capacity check distinguishes ENDTASK by its location alone. The
native mapping API reserves that location to prevent a user task from masquerading
as a free slot. A tenth task reaches the 1203 boundary. An insertion before or
inside a full list can already have displaced the last task when it fails; an
append after all nine can fail before list mutation. Both forms match assembly.
Zero/negative delay reaches 1204, the original POODOO1 boundary. Positive delay
above 16250 or malformed native address/bank is rejected before mutation.
Alarm bodies, display changes, abort routing and recovery remain untranslated.

## Native task API

`include/native_tasks.h` exposes an immutable full-pair mapping table, runtime
and small IRQ clock-publication structure. Task and Executive tables are separate.
The clock source never invokes either dispatcher or changes task/Executive state.

- `wl_native_schedule` checks the mapping and inserts under restoring exclusion.
  A task can schedule another task through this helper while its C frame is live.
- `wl_native_advance` applies explicitly counted centiseconds and latches a due
  interrupt. `wl_native_collect` copies/consumes published clock ticks under the
  same exclusion, preserving them on failure.
- `wl_native_start` selects a due task. `wl_native_step` runs one selected task
  callback and performs TASKOVER after the callback returns. Another due task
  can be selected for the next step. Each native call restores the caller's I
  state; IRQs may run between steps. Apollo kept its task chain inside the ISR.
- A callback returns WL_OK to finish. Other results fault the task runtime;
  WL_IDLE is not a callback completion action. A helper's latched fault blocks
  TASKOVER even if the callback returns OK. Step/start/clock re-entry from inside
  a callback is rejected; scheduling is permitted.
- Tasks may use guarded Executive admission/wake helpers. Admission does not
  run a job callback. Jobs still exchange only through their native trampoline.

There is no suspended task stack or hidden persistence. Tables, hooks and user
storage must outlive the runtime. Unmapped pairs, capacity/delay alarms, ambiguous
clock phase and publication overflow fail-stop; no rollback/restart is claimed.
The task state is a separate C structure. Original scratch aliases WAITADR/NEWPRIO
and WAITTEMP/LOCCTR are not shared with the native Executive: only the documented
persistent task words and returned targets form this comparison boundary.

`wl_clock` has one non-nested IRQ producer and one foreground consumer. Its
16-bit count is copied/reset only with that producer masked. This contract is
not C11 thread synchronization. Saturation at 65535 latches overflow and retains
accepted ticks; collection faults before timer mutation. Init/reset requires a
stopped source. Overflow of the counter buffer cannot detect CIA hardware events
that coalesced before publication.

## CIA1 Timer B adapter

The target adapter claims an unused stock CIA1 Timer B: CRB=$08, stopped count
$ffff, stock KERNAL vector $ea31 and no enabled VIC raster source. It uses phi2
counting, enables only the B interrupt mask and restores these ownership markers
on removal. It leaves CIA1 Timer A's KERNAL mask and timer programming intact.
CIA interrupt masks/latches cannot be fully read back, so this is a stock-ROM
ownership contract, not general sharing with arbitrary installed IRQ handlers.
CIA2/NMI and other timer owners remain unsupported.

The gate reads/acknowledges CIA1 ICR once, saves all 32 LLVM-MOS registers,
clears decimal mode and switches to a separate 256-byte software stack. The
producer publishes ticks only. After restoring registers, it chains to stock
$ea31 if Timer A or another unowned CIA bit was present; otherwise it exits
through $ea81. The pinned ROM updates the jiffy clock before its final ICR read,
so acknowledging A in the gate does not prevent that chained service. KERNAL's
entry/exit preserve CPU A/X/Y and P. Both target sources and applications require
`-fno-static-stack -mlto-zp=0`; the helper rejects extra zero-page allocations.

The retained PAL diagnostic uses period=19705 cycles and tick_step=2, approximately
20 ms per event. It publishes two logical centiseconds per underflow. Odd
centisecond deadlines can wait until the next two-centisecond sample, plus
foreground delay. No compensation for long callback/masked intervals is invented.
The 10 ms trial exposed insufficient margin between the longest measured
insertion and IRQ handling; PERFORMANCE.md records the chosen 20 ms samples.

CIA flags coalesce repeated underflows. To preserve elapsed events, the longest
IRQ-masked interval plus handler/entry/other-source delay must stay shorter than
the selected period. Current finite tests do not prove that budget for every task,
state or source phase. Each application must bound callbacks, masks and polling.
A continuous source is supported by budget=0, but the retained evidence uses
finite budgets; it establishes neither a continuous workload guarantee nor flight
deadlines. NTSC requires a new period/configuration and remains untested.

## Independent reference and native target checks

The fixture overlays upstream EXECUTIVE, WAITLIST and INTERRUPT_LEAD_INS
byte-for-byte onto pinned supporting sources. All assemble, and the rope SHA256
remains unchanged. The host-only GPL adapter links the actual Virtual AGC engine
and input-only constructors, with no native task, Executive or arithmetic code.
It executes public WAITLIST, fixed-fixed T3 lead-in, T3RUPT2 and TASKOVER.

| Cases | Coverage |
|---|---|
| 0..89 | Every insertion position, equal/immediately-before deadlines, duplicate pairs, EB/SB flags, capacity before/after shifts, zero/one differential intervals and pending-overflow inputs |
| 90..119 | Six TIME3 boundaries, minimum/maximum delay and negative zero |
| 120..143 | Task dispatch, complete banks, interval zeros/boundaries and overdue reload |
| 144..151 | TASKOVER chain or return; physical interrupted-bank restoration |
| 152..155 | Both zero and negative delay alarm boundaries |
| 156..165 | Ten actual native four-task sequences: equal deadlines, rescheduling inside task B, delayed clock collection, A/B/D/C execution order |
| 166..195 | Zero/minimum/127/16384/65535 controlled PINCs, positive overflow and multiple wraps |

The 196 cases produce 306 rows of 34 fields: case/step/status, all 29 persistent
task words including alarm annotation, and the selected pair. A/L/Q, WAIT scratch,
AGC hardware interrupt save registers and peripheral effects are excluded.
Original task targets are identified by PC and physical BBANK/SUPERBNK, not a
predicted native selection. Insertion stops at LVWTLIST or alarm entry; task
return stops after saved-bank restoration before RELINT/hardware RESUME.
Clock progress is explicit fixture input; the engine scaler is held between steps.
Snapshot SHA256 is
`bd71aad75050b3df0fb7753b9519599bda9ca2b22abcecdbd41984001841182b`.
Frozen fingerprints are generated only after complete word-for-word equality.

Host tests add restoring/already-masked guards, exhaustive positive TIME3 overflow
positions, native capacity/mapping/helper faults, callback re-entry and clock-buffer
saturation. All four host suites pass ASan/UBSan. The fourth C64 diagnostic checks
all 306 fingerprints and ten real task sequences, then six actual CIA B IRQs:
a register/decimal poison probe, four periodic events during native arithmetic,
and one event for a task that admits a job and wakes a VAC-owning sleeper. It
checks unchanged state before collection, callback masking, retained payloads,
final VAC release, KERNAL jiffy advancement and Timer B/vector removal.
The Executive bridge is an integration/payload check, not an additional complete
assembly comparison of shared Apollo scratch or CIA hardware timing.

Retained mailbox `a50000c432010a06963a000500`, 331 marker events, 156 primitive
intervals, IRQ/period/bridge records and stack dumps are in `evidence/`.
TWIDDLE, FIXDELAY/VARDELAY/LONGCALL wrappers, SVCT3/IMU work, arbitrary interrupted
AGC register restoration, restart protection, guidance and physical C64 remain
outside this bounded milestone.
