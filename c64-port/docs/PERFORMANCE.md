# Native C64 resource and timing findings

The retained artifacts use LLVM-MOS SDK v23.3.0, `-Os`, and PAL x64sc 3.7.1 with
ordinary ROMs. `executive.prg` is the unbound console regression; `irq.prg` and
`lifecycle.prg` use `-fno-static-stack -mlto-zp=0` and the bounded raster gate.
These are diagnostic samples, not flight-software bounds or AGC cycle equivalence.
Exact binaries, maps, logs, mailbox/stack dumps and hashes are in `evidence/`.
The three existing artifacts are unchanged from the lifecycle milestone. The
additional WAITLIST artifact and CIA measurements are recorded separately below.

## Image and RAM

| Quantity | Console regression | IRQ diagnostic | Lifecycle diagnostic |
|---|---:|---:|---:|
| PRG file, including two-byte load address | 21440 bytes | 23692 bytes | 28499 bytes |
| Loaded image | 21438 bytes, $0801..$5bbe | 23690 bytes, $0801..$648a | 28497 bytes, $0801..$7751 |
| BASIC SYS entry | $082b | $080d | $080d |
| Native text | 20392 bytes | 22715 bytes | 27802 bytes |
| Read-only data | 1004 bytes | 963 bytes | 683 bytes |
| Linked BSS | 658 bytes, $5bbf..$5e50 | 1668 bytes, $648b..$6b0e | 1663 bytes, $7752..$7dd0 |
| Linked `.noinit` | 658 bytes, $5e51..$60e2 | 1 byte, $6b0f | 1 byte, $7dd1 |
| Image + BSS/noinit | 22754 bytes | 25359 bytes | 30161 bytes |
| Compiler zero page | 91 bytes, $02..$5c | 32 registers, $02..$21 | 32 registers, $02..$21 |
| Executive state | 648 bytes | 648 bytes | 648 bytes |
| Admission-request buffer | Unbound | 32 bytes, four requests | 32 bytes, four requests |
| Reserved ISR software stack | None | 256 bytes inside BSS | 256 bytes inside BSS |
| Foreground software-stack sentinel extent below $d000 | 98 bytes | 1356 bytes | 762 bytes |
| ISR-stack sentinel extent | N/A | 16 bytes | 16 bytes |
| Hardware-stack sentinel extent below entry baseline | 25 bytes | 34 bytes | 35 bytes |

The guarded BSS includes two Executive copies for unchanged-state checks, queue,
runtime, saved registers and ISR stack. The target native runtime is 21 bytes,
including staged priority and callback-active metadata. The foreground stack
includes local fixtures and diagnostics; it is not all dispatch overhead. The
console build has a 658-byte static fixture frame instead. A single linked
runtime `.noinit` byte remains in each guarded artifact; application/producer
frames are dynamic and the ISR calls no console helpers. No core, dispatcher
or diagnostic heap allocation is used. All three images fit stock C64 RAM.

The console build places a 30-byte callback table in initialized zero page and
allocates 29 additional spill bytes. It is unsuitable for the IRQ gate's 32-byte
preservation contract. Guarded maps have no additional ZP allocation, checked
by their VICE helpers. The helpers discover SYS from the actual PRG header.

## Guarded IRQ operation samples

The IRQ monitor pairs stores $10/$20 at $c030, with group/case at $c031/$c032.
Setup, screen output and fingerprints are outside these intervals. The first
71 paths mask IRQs around the raw operation; two four-request batches use the
protected native poll. Intervals include marker/call overhead and VIC bus effects.
Raster production has stopped; stock KERNAL IRQs can still affect wrapper batches
and the guard pair. PAL rate is approximately 985248 cycles per second.

| Group | Samples | Maximum PAL cycles | Time | Case |
|---|---:|---:|---:|---|
| Admission / ENDJOB selection | 26 | 3820 | 3.877 ms | Scenario 13, eighth-core FINDVAC |
| Basic context exchange | 36 | 4535 | 4.603 ms | Dispatch 24 |
| Completion / basic dispatch | 6 | 6179 | 6.272 ms | Dispatch 37 |
| Bounded idle startup | 3 | 3771 | 3.827 ms | Dispatch 52 |
| Four-request safe point | 2 | 18450 | 18.726 ms | Case 8, four FINDVACs |
| Exclusion enter/leave | 1 | 24 | 0.024 ms | Restoring initially enabled I |

The 74 intervals cover all 26 old scenarios, 45 isolated dispatch fixtures,
two full buffered batches and one guard pair. The NOVAC batch is retained in JSON.
These are maxima over tested fixtures with five mappings, not all valid states
or table sizes. The guard pair is optimized diagnostic code, not a universal
indirect-hook cost. The largest batch nearly occupies a PAL frame; capacity alone
cannot establish a source-rate or scheduling guarantee.

## Actual IRQ and safe-point latency

Nine owned IRQ intervals run from marker $81, after source recognition and
acknowledgement, to $82, after register restore. Eight ordinary entries take
2060..2214 PAL cycles (2.091..2.247 ms); the register-poison canary takes 2630
cycles (2.669 ms). They include rc0..31 save/restore, separate-stack setup,
C publication, 16-byte ISR-local canaries and marker overhead.

Initial CPU entry/KERNAL prologue and final KERNAL epilogue are excluded. For the
pinned ROM and stack range, the unmarked instruction budget is 86 cycles: seven
hardware entry, 29 KERNAL prologue, 25 source recognition/first marker, and 25 final
jump/pop/RTI. This is instruction arithmetic, not a measured response bound:
instruction completion, masking, another handler and VIC stalls may add time.
Raster coordinates and CPU status at each marker remain in the monitor record.

The second two-job run omits hashing. It retains fixed 512-iteration native C
arithmetic, safety checks and payload touches, with a raster request armed in
the low callback. Recorded intervals are:

| Boundary | PAL cycles | Time |
|---|---:|---:|
| Low callback start to poll return | 163309 | 165.754 ms |
| IRQ start to poll return | 152788 | 155.076 ms |
| IRQ restore to poll return | 150574 | 152.829 ms |
| Poll entry to poll return | 3789 | 3.846 ms |
| Poll return to higher callback entry | 7811 | 7.928 ms |
| Deliberately masked three-arithmetic interval | 463075 | 470.009 ms |
| Pre-unmask to completed deferred delivery | 2567 | 2.605 ms |

IRQ publication leaves Executive state unchanged. Poll admits while the low
callback still runs; the higher callback begins after yield and another step.
The fingerprint-checked sequence is also retained, but hashing dominates its
latency and it is not used for this table. Application latency includes remaining
callback/poll work, masked work, source/ISR interference and guarded admission.
Dispatch adds the wait until an action return and the next step. The test source
has a finite budget; a continuous source could overflow during the long callback.
No unconditional request latency or flight deadline is established.

## Lifecycle operation and IRQ bridge samples

The lifecycle monitor pairs $10/$20 stores at $c048, with the case at $c04d.
Its 76 isolated fixtures run under exclusion; setup and fingerprinting remain
outside the measured operation. Intervals include marker/call overhead and VIC
bus effects. Five mappings are present.

| Group | Samples | Maximum PAL cycles | Time | Case |
|---|---:|---:|---:|---|
| JOBSLEEP / exchange or idle | 22 | 5280 | 5.359 ms | 11 |
| JOBWAKE / first dormant match or no match | 29 | 4111 | 4.173 ms | 37 |
| PRIOCHNG / direct return or exchange | 25 | 5537 | 5.620 ms | 57 |

After the full fingerprint suite, a VAC-owning job sleeps. A real raster IRQ
publishes an admission for the waker, preserving Executive state. Foreground
admission/start runs that job; it wakes the sleeper and lowers its own priority.
The sleeper resumes with retained payload/overflow/VAC, finishes and releases
its VAC, then the waker completes. The additional bridge intervals are:

| Boundary | PAL cycles | Time |
|---|---:|---:|
| Enable marker to observed publication | 2585 | 2.624 ms |
| Observed publication to awakened dispatch | 64076 | 65.035 ms |
| Awakened completion | 10280 | 10.434 ms |
| Waker completion | 6234 | 6.327 ms |

The 64076-cycle interval includes a full 648-byte unchanged-state comparison,
safety checks, and native admission/start/wake/priority actions. It is not a
scheduler-only latency measurement. Bridge intervals omit hashing but retain
integration checks; the single finite IRQ event is not a continuous-source bound.
The timing JSON retains all intervals, markers and stack observations.

## Console regression and stack limits

The console diagnostic still passes 135 original-derived fingerprints and eight
native two-job sequences. Four IRQ-enabled samples are 1187 cycles for idle NOVAC,
2129 for full-core FINDVAC/1202, 5395 for slot-seven completion/dispatch and 4011
for slot-seven CHANG1. Compiler flags, LTO layout and IRQ phase differ, so these
cannot establish guarded-build costs. Older measurements refer to older binaries.

Sentinels give test-specific lower bounds: coincidental $cd values, untouched
paths or IRQ phase can hide deeper use. Hardware instrumentation spans
$0100..$01f1, protecting the 14-byte BASIC/start chain. Guarded extents correspond
to about 48/49 bytes including that chain. Foreground software instrumentation
spans $c100..$cfff. ISR locals, interrupted arithmetic and explicit SP/register
canaries complement these observations. Reserve the full hardware-stack page
and conservative separate software-stack space; no complete BASIC exit or future
callback stack bound is established.

The SDK exposes RAM under BASIC, retains KERNAL/I/O and sets software-stack top
$d000. Respect $0000/$0001, compiler zero page, hardware stack, screen RAM, image
and BSS, plus mailboxes $c000..$c00c (console), $c020..$c026 and $c030..$c032 (IRQ),
and $c040..$c04d (lifecycle). Target `int`/pointers are 16 bits. The trampoline
retains no suspended C stack; persistent job values belong in MPAC/VAC. Physical
C64, NTSC, interpreter dispatch and guidance feasibility remain unmeasured.

## WAITLIST diagnostic resources and timings

The fourth artifact uses the same guarded compiler flags. Its retained file is
31804 bytes; the loaded image is 31802 bytes at $0801..$843a, with SYS $080d.
Text is 30315 bytes ($080d..$7e77), read-only data 1475 bytes ($7e78..$843a),
BSS 1793 bytes ($843b..$8b3b), and `.noinit` one byte at $8b3c. Image plus BSS /
noinit totals 33596 bytes, with heap start $8b3d; no heap is used. Compiler ZP is
only the preserved 32 registers. This diagnostic fits stock C64 RAM.

Task state is 58 bytes, task runtime 18 bytes and tick publication three bytes.
The diagnostic includes two task states, two 648-byte Executive states, a 32-byte
admission buffer for bound Executive helpers, saved IRQ registers and a 256-byte
ISR stack. Foreground sentinel extent is 105 bytes, ISR extent 16, and hardware
extent 31 below the protected 14-byte entry chain (about 45 total observed bytes).
These are lower-bound observations with the same sentinel limits described above.

156 isolated operations pair $10/$20 stores at $c058, with case at $c05d.
Source is not armed during this suite. Setup and fingerprinting are outside each
measured primitive interval; marker/call/VIC bus effects remain included.

| Group | Samples | Maximum PAL cycles | Time | Case |
|---|---:|---:|---:|---|
| WAITLIST insertion, including capacity failures | 120 | 8660 | 8.790 ms | 3, full list displaced before 1203 |
| T3RUPT2 dispatch/reload | 24 | 4018 | 4.078 ms | 125 |
| TASKOVER chain/return | 8 | 4187 | 4.250 ms | 145 |
| Invalid-delay alarm boundary | 4 | 658 | 0.668 ms | 154 |

The complete fingerprint suite also checks synthetic multi-wrap PINC loops.
Those large elapsed-time tests mask IRQs with the CIA B source stopped; they
are correctness fixtures, not admissible continuous-timer service budgets.

Six CIA IRQ marker intervals ($81/$82 at $c060) are 2377, 1853, 1853, 1854,
1876 and 1790 PAL cycles. The first is a poison-register canary (2.413 ms);
the five normal entries span 1790..1876 cycles (1.817..1.904 ms), including
register/stack preservation, ISR locals, two tick publications and marker stores.
CPU/KERNAL entry, ICR read/selection and final exit/chaining are excluded.
Coincident Timer A service adds the stock KERNAL clock/keyboard routine after
register restore. No universal interrupt-response bound is inferred.

A 10 ms trial left insufficient margin between worst observed insertion and IRQ
handling. The retained source uses 19705 cycles per event, approximately 20 ms,
publishing two logical centiseconds. Four finite periodic events occur during
unmasked checked native arithmetic. Their three start-marker periods are 19706,
19705 and 19704 cycles; small entry-phase variation does not indicate tick loss.
No elapsed tick is inferred for a coalesced CIA flag. The longest mask plus
handler/entry/other-source delay must stay below the configured period.

The separate one-event task/job bridge records:

| Boundary | PAL cycles | Time |
|---|---:|---:|
| Pre-unmask marker to observed publication ($92..$93) | 21560 | 21.883 ms |
| Observed publication to selected task ($93..$94) | 52547 | 53.334 ms |
| Task callback admission/wake and TASKOVER ($94..$95) | 8441 | 8.567 ms |
| Resumed sleeper and admitted-job completion ($95..$96) | 13584 | 13.787 ms |

The 52547-cycle interval includes both unchanged-state comparisons (648-byte
Executive and 58-byte task state), safety checks, clock collection and dispatch.
It is not a scheduler-only latency measurement. Callback time includes guarded
Executive helpers and assertions. The final interval covers two separate job
steps with restoring I state, not one continuous mask. Source has a finite budget
and stops after publication, so these bridge intervals do not prove continuous
source capacity or application deadlines. Odd-centisecond deadlines additionally
wait for two-centisecond sampling before foreground collection.

The helper retains all events/intervals, verifies periodic spacing and rejects
extra compiler ZP or exhaustion of instrumented stacks. Reserve $c050..$c05d and
$c060 for mailbox/timing stores in addition to the earlier diagnostic regions.
CIA ownership, stock-ROM restoration, callback/mask constraints, and unsupported
NTSC/NMI/multi-source behavior are defined in WAITLIST.md.
