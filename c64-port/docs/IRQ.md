# Bounded C64 interrupt request delivery

The third milestone adds platform exclusion and a four-entry admission-request
buffer. A real VIC-II raster IRQ can interrupt native C, publish a request, and
return without changing Executive state. The foreground admits requests at
guarded safe points. Basic jobs still change through explicit callback returns;
an IRQ does not suspend a C call stack or dispatch another job.

This is a bounded platform adaptation, not a translation of Apollo interrupt
lead-ins, WAITLIST, timer tasks or INHINT/RELINT instruction timing. Historical
NOVAC/FINDVAC and basic dispatch behavior remains independently verified against
unchanged original assembly. See DISPATCH.md and VERIFICATION.md.

## API and ownership

`include/irq_delivery.h` defines `exec_request`, `exec_irq_queue` and
`exec_exclusion`. Initialize the Executive/runtime/queue, then bind with
`exec_native_bind_irq` before starting the source. Queue, mapping table, guard
hooks and their user storage must outlive the runtime. Reset/rebind only with
the source stopped or excluded and no callback running.

There is exactly one non-nested IRQ producer and one foreground consumer. The
IRQ calls only `exec_irq_post`; it never calls Executive admission, native poll,
start or step. The consumer uses platform hooks to mask that producer. This is
a volatile, interrupt-excluded queue, not a C11 thread synchronization API.
Multiple producers, nested application IRQs and NMI producers are unsupported.

Each entry holds priority, complete historical `{location, bank}`, and a VAC
flag. The producer validates positive upper-field priority, basic location and
canonical bank/flag values. It writes the whole volatile payload before
publishing the byte write ticket. The consumer copies it while the IRQ is masked
and advances the byte read ticket only after successful admission. Unsigned
ticket subtraction and four-slot indexing handle byte wrap. FIFO admission
preserves the original packed-priority selection; the buffer is not a ready-job
queue. Queued addresses must match both words in the immutable callback table.

Foreground entry points:

- `exec_native_start` drains requests before bounded idle startup.
- `exec_native_step` drains before callback entry and after callback return,
  then performs the returned yield/sleep/priority/completion action under the same guard. A request can start work
  from idle on the next step. Before-entry admission alone does not switch an
  already running callback.
- `exec_native_poll` drains at an explicit callback safe point, without yielding
  or invoking a callback.
- `exec_native_admit` drains earlier IRQ requests, then admits the foreground
  request. Callbacks use this helper for their own admission when IRQs are bound.

`exec_native_wake` also drains earlier IRQ admissions before its guarded
foreground wake. `exec_native_request_priority` stages callback metadata under
the guard; the matching returned action performs the mutation after frame return.
The producer remains admission-only. LIFECYCLE.md documents these APIs and the
real-IRQ integration check that admits a waker while a VAC-owning job sleeps.

All guarded paths restore the caller's previous IRQ-enable state. An enabled
caller runs callbacks enabled; an already masked caller stays masked. Nested
guards restore the outer mask. Unbound runtimes retain the cooperative API;
raw `exec_*` primitives still require caller-owned exclusion. Do not call a
second native step from inside a callback or mutate the runtime from an IRQ.

## Fault boundaries

The fifth pending request returns `EXEC_QUEUE_FULL`, latches overflow and keeps
all four accepted payloads. Further posts fail; the rejected counter saturates
at 255. At the next safe point overflow faults the runtime before any admission.
This is a native delivery fault, separate from historical octal 1201/1202.

Invalid producer input is rejected without publication; the source must inspect
the return status. Unmapped queued addresses fault before core mutation. Pending
bailout blocks delivery. A failed resource admission retains its exact original
boundary: 1202 can keep the VAC reserved before the full-core scan. The failed
entry stays queued, earlier successful entries stay admitted, and subsequent
native operations return the latched fault. A fault raised by a helper inside a
callback also blocks its returned action. No transaction rollback,
retry is implemented by IRQ delivery. Bounded caller-controlled recovery is
separate (RESTART.md). Stop the source before explicit reset.

## C64 raster harness and ABI

`platform/c64/irq_platform.c` supplies PHP/SEI guards and restores I from the
saved token. It arms raster line 250 with a finite budget of 1..5 PAL frames;
five events are used deliberately for overflow. It requires stock KERNAL vector
$ea31 at $0314 and no existing raster owner. Arm only while the previous budget
is exhausted, or while excluded with no pending producer execution. The
installed source copies its request, so later changes to the caller's request
do not change publications. CIA timers and interrupt masks are untouched.

The assembly gate is entered through stock KERNAL $ff48, which already saved
A/X/Y. Non-owned IRQs chain to the original vector. For the owned raster source
it acknowledges only VIC bit zero, clears decimal mode for C, saves all 32
LLVM-MOS imaginary-register bytes (including the software-stack pointer), and
switches to an isolated 256-byte software stack. It calls the C producer, then
restores all 32 bytes and jumps to KERNAL $ea81. The KERNAL pops Y/X/A and RTI
restores the interrupted P/PC. A simultaneous CIA IRQ remains asserted and is
handled by the normal chain on the next entry. Jiffy advancement is checked
during the diagnostic, as is restoration of the original vector on removal.

The IRQ target **must** use `-fno-static-stack -mlto-zp=0`. The first prevents
compiler automatic frames from overlapping an interrupted foreground frame;
the second prevents unpreserved extra zero-page allocations beyond rc0..31.
The VICE helper rejects extra ZP sections in the linked map. The pinned compiler
also needs the diagnostic's observer selector to remain a byte: narrowing that
global to LLVM i1 triggers a register-allocation error. It is volatile telemetry,
not a synchronization mechanism. Re-audit the ABI and map when changing SDKs,
flags, producer code or libraries. The ISR calls no console/heap routines.

Removal restores the vector, VIC enable mask and display bits. The previous
disabled raster comparator is unreadable and is not restored; the contract
excludes an existing raster owner. The diagnostic owns $c020..$c026 and
$c030..$c032 for mailbox/markers. These marker writes, register poisoning and
stack canaries are test instrumentation. The harness is not a general IRQ
installation framework. Custom KERNALs, application NMIs/RESTORE, raster owners,
RS-232 IRQ integration and physical C64 operation have not been validated.

## Executed evidence and latency limits

Host tests cover FIFO publication over 600 starting-ticket cases, nested and
already-disabled guards, idle delivery, automatic post-callback admission,
priority exchange/payload restoration, overflow, invalid/unmapped requests,
pending bailout, exact 1201/1202 retention, and callback-local failure.

`make c64-irq` and `tools/irq_vice_test.py` execute nine real raster IRQs in VICE.
An assembly canary seeds rc2..31 and A/X/Y, enables decimal mode, and deliberately
poisons all 32 imaginary registers after the C ISR returns. The canary checks
registers, original stack pointer, restored D/I and binary ISR arithmetic.
The ISR writes and checks 16 volatile local bytes on its separate stack. A live
foreground callback runs independently checked 32-bit arithmetic with a local
32-byte array while interrupted. Eight saved-state checkpoints match the
original-derived two-job sequence; a second run omits hashing for latency.
Five actual events then overflow the queue without Executive mutation. A final
masked interval defers publication until the caller restores IRQ permission.

The monitor records 74 operation intervals and all 179 marker events in
`evidence/irq-timing.json`. PERFORMANCE.md reports the maximum observed samples
and exact measurement boundaries. IRQ marker intervals exclude initial hardware
entry/KERNAL prologue and final KERNAL epilogue. Finite loop lengths, queue
capacity and table size bound this harness's work, but these samples are not
exhaustive worst-case timing proofs.

There is no unconditional delivery deadline: latency includes masked foreground
work, any residual callback work until poll/return, ISR work, and the guarded
admission batch. Dispatch adds the wait until a callback action return and the next step.
An application must bound each interval, its mapping-table size and source rate.
Our arithmetic callback intentionally lasts several PAL frames; its source is
armed for one event, so it cannot overwhelm the buffer. Continuous raster input
with the same callback could overflow. Four requests alone do not provide a
real-time guarantee. CIA WAITLIST/task scheduling and guidance deadlines remain
separate work.

## Task clock source

WAITLIST.md defines the separate CIA1 Timer B tick-publication contract. It uses
a dedicated gate, stock Timer A chaining, foreground clock collection and native
task callbacks under exclusion. Timer IRQs never dispatch jobs/tasks. The raster
admission and CIA timer diagnostics own separate sources and vectors; simultaneous
installation, multi-source integration and NMI remain unsupported.
