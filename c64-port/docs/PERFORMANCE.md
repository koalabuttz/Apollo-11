# Native C64 resource and timing findings

The retained artifacts use LLVM-MOS SDK v23.3.0, `-Os`, and PAL x64sc 3.7.1 with
ordinary ROMs. `executive.prg` is the unbound console regression; `irq.prg` and
`lifecycle.prg` use `-fno-static-stack -mlto-zp=0` and the bounded raster gate.
These are diagnostic samples, not flight-software bounds or AGC cycle equivalence.
Exact binaries, maps, logs, mailbox/stack dumps and hashes are in `evidence/`.
All figures below refer to the retained lifecycle-milestone binaries.

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
