# Basic context exchange and native execution

The second milestone adds bounded basic-job CHANG1/CHANJOB, completion through
basic dispatch, and native callable continuations. Two demonstration jobs are
admitted, run, yielded, resumed and completed on host and stock PAL C64 VICE.
The same saved state and selected historical 2CADRs match controlled executions
of the unchanged original Executive at 109 checkpoints.

This executes small native C demonstration jobs. Apollo guidance jobs,
interpreter continuations, interrupt-origin requests, idle self-check, activity
lamp hardware, WAITLIST and restart/recovery remain outside this milestone.

## Context exchange

`exec_change_basic(state, resume, next)` enters at the native equivalent of
CHANG1. `resume` supplies the basic caller's Q return location and complete
BBCON (BBANK plus channel-7 SUPERBNK bits). NEWJOB must name an active basic
context in slots 1..7. The exchange preserves:

- Outgoing resume LOC/BANKSET in the selected saved context.
- All seven MPAC words and MODE, including raw +0 and -0.
- PUSHLOC and PRIORITY, retaining the historical working-area base.
- Outgoing nonzero OVFIND encoded by complementing PUSHLOC; either zero in
  OVFIND leaves PUSHLOC unchanged.
- Incoming negative PUSHLOC complemented back, with OVFIND=1; incoming -0
  becomes +0 with OVFIND=1. Positive/+0 PUSHLOC yields OVFIND=0.
- FIXLOC from the incoming priority's low nine bits and NEWJOB=+0.

The comparison includes the **dispatch epilogue**, not just the swap. At
ENDPRCHG, `DXCH LOC` takes the selected pair into A/L. It writes A=+0 and the
old current BANKSET debris back into core-zero LOC/BANKSET. Thus the live
running address must be held separately in `exec_native.current`; core-zero
LOC is not a native program counter. `exec_address next` preserves the selected
pair. The oracle executes DTCB and independently checks the resulting Z,
BBANK (mask 076007), and SUPERBNK (mask 0160).

`exec_complete_basic` runs the original ENDJOB1 release and ascending EJSCAN.
The first largest **entire packed positive priority** wins, including low VAC
bits; dormant and free contexts are skipped. Before exchange, the outgoing
BANKSET debris is ORed with the current SUPERBNK value, exactly as CHANJOB-1's
ROR. The freed context keeps its stale work words and location debris. No
work means the bounded DUMMYJOB NEWJOB=-0 store and EXEC_IDLE, rather than an
idle/self-check implementation.

`exec_start_basic` provides bounded startup from the idle dispatch branches:
NUDIRECT when NEWJOB=+0, or NUCHANG2/CHANJOB+4 for a positive candidate. Entry
scratch A=2 and L=0 are explicit fixture/platform conventions. It preserves
their different LOC/BANKSET debris; it does not translate the activity-lamp I/O
or polling/self-check loop surrounding those branches.

Malformed candidate displacements, non-basic locations, invalid supplied
superbank bits and a pending bailout are rejected. Interpreter selection is
preflighted **before** completion releases the running resources. Rejections
leave the Executive and output address unchanged. The supported domain retains
the admission invariants in ARCHITECTURE.md; it does not validate every possible
corrupted core/VAC word.

## Native continuation contract

`include/native_dispatch.h` exposes an allocation-free trampoline:

1. Define an immutable `exec_mapping` table of complete historical
   `{location, bank}` pairs and native C function pointers.
2. Initialize the runtime, admit a basic job, then call `exec_native_start`.
3. Call `exec_native_step` to run one callback and perform its returned action.
4. A callback returns EXEC_YIELD with an explicit resume pair, or EXEC_FINISH.
   EXEC_IDLE reports that all active jobs completed.

The five mappings in `tests/dispatch_cases.c` form a runnable example. The low
job admits a higher-priority job, then yields. When it wins the full packed-word
comparison, execution is low entry, high entry, high resume/completion, low
resume, low finish/completion. Otherwise the low job continues until completion
and then the high job runs. Both jobs use yield continuations. A yield with no
pending higher-priority candidate follows the basic CCS NEWJOB +0 polling path
and resumes directly; it does **not** force a lower-priority job to run.

Each callback returns through the normal C ABI. No suspended C call stack is
retained, and no setjmp/longjmp, fiber, emulator, native bank switch or generic
ready queue is introduced. Job-local values that must survive a yield belong in
MPAC/VAC. The example checks their restored values inside the resumed callbacks,
in addition to the state fingerprints at every event. The mapping and `user`
storage must outlive the runtime. C pointers are never encoded into AGC words.

Duplicate mappings, null callbacks and addresses outside positive 12-bit basic
locations are rejected. Lookup includes **both** words. Unmapped continuations
and invalid callback actions latch a runtime fault and block further calls.
A callback may already have changed state, or a valid exchange may already have
occurred before an unmapped target is detected; faults are fail-stop boundaries,
not transactional rollback or Apollo restart. Keep all admitted and resume
pairs in the table. Successful initialization is required before other runtime
calls. Native faults are separate from the historical 1201/1202 alarm marker.

## Sequential reference and target checks

`tools/dispatch_reference.c` links only the pinned Virtual AGC engine and fixture
input constructors. It does not link the native Executive, dispatcher or word
implementation. `DISPATCH_INPUT_ONLY` omits all native execution code from the
shared fixture translation unit. Assembly runs in the same machine instance
across the eight steps of each two-job sequence; scratch registers and stale
saved context/VAC state survive. Only specified job payload inputs and routine
entry registers are set between bounded calls.

| Cases | Coverage |
|---|---|
| 0..27 | Every candidate position 1..7, with both outgoing/incoming overflow states |
| 28..35 | Incoming PUSHLOC +0/-0; outgoing OVFIND +0/-0/nonzero |
| 36..41 | Completion/VAC release, exact priority ties, differing low bits, dormant contexts, slot-seven winner and idle |
| 42..49 | Eight sequential admission/start/yield/completion sequences; higher/equal/lower/max priorities and NOVAC/VAC combinations |
| 50..52 | Direct idle start and two queued idle exchanges |

All 53 cases yield 109 checkpoints. Each row records case, step, status, all
96 core words, all 220 VAC words, seven Executive globals, alarm marker and the
two selected address words. A/L/Q are ABI/return scratch rather than persisted
native job registers; their storage effects in LOC/BANKSET/NEWLOC are compared.
EXECTEM/BUF remain scan scratch. The oracle checks actual transfer Z/BBANK/
SUPERBNK on each non-idle dispatch; no basic bank transition is excused from
verification. Idle stops after NEWJOB=-0, before RELINT/activity-light/self-check.

The reference's channel-7 output stub retains Virtual AGC SocketAPI.c's real
channel-7 storage behavior; omitting it would retain the previous superbank and
invalidate the reference. Peripheral callbacks otherwise remain disabled,
interrupt requests are absent and hardware watchdog alarms are inhibited.
The fixture contains a three-instruction erasable basic caller for the +0
CCS NEWJOB poll. The historical Executive and assembled rope stay unchanged.

The host comparison is word-for-word. `tools/freeze_vectors.py` produces 109
original-derived fingerprints, checked on both host and C64. These FNV checks
are compact regressions with possible hash collisions, not replacements for
host comparisons. The C64 diagnostic executes the actual mapped C callbacks
for all eight sequences; it also repeats three representative timed paths.

## Interrupt and timing boundary

This milestone is cooperative and single-threaded. The platform caller owns
exclusion around Executive mutations; no application IRQ handler may call or
mutate these APIs. Stock C64 KERNAL IRQs remain active for the console diagnostic
and do not enqueue jobs. AGC INHINT/RELINT are not modeled as global C64 SEI/CLI.
Callbacks execute ordinary C code and yield only at their declared returns.

The next milestone must establish platform exclusion wrappers, preservation of
LLVM-MOS imaginary registers, request delivery safe points and measured bounds
before accepting interrupt-origin jobs. Four native timing samples and stack
sentinels are recorded in PERFORMANCE.md. They cover long scans and an exchange,
but do not prove a worst-case execution time or interrupt-latency bound.
