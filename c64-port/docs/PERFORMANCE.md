# Native C64 resource and timing findings

The retained artifacts use LLVM-MOS SDK v23.3.0, `-Os`, and PAL x64sc 3.7.1 with
ordinary ROMs. `executive.prg` is the original unbound console regression;
`irq.prg` adds the bounded raster harness with `-fno-static-stack -mlto-zp=0`.
These are measured diagnostic samples, not flight-software bounds or AGC cycle
equivalence. Exact binaries, maps, logs, mailbox/stack dumps and SHA256 hashes
are retained under `evidence/`.

## Image and RAM

| Quantity | Console regression | IRQ diagnostic |
|---|---:|---:|
| PRG file, including two-byte load address | 20113 bytes | 21288 bytes |
| Loaded image | 20111 bytes, $0801..$568f | 21286 bytes, $0801..$5b26 |
| BASIC SYS entry | $082b | $080d |
| Native text | 19065 bytes | 20311 bytes |
| Read-only data | 1004 bytes | 963 bytes |
| Linked BSS | 658 bytes, $5690..$5921 | 1665 bytes, $5b27..$61a7 |
| Linked `.noinit` | 658 bytes, $5922..$5bb3 | 1 byte, $61a8 |
| Image + BSS/noinit | 21427 bytes | 22952 bytes |
| Compiler zero page | 91 bytes, $02..$5c | 32 imaginary registers, $02..$21 |
| Executive state | 648 bytes | 648 bytes |
| Admission-request buffer | Unbound | 32 bytes, four requests |
| Reserved ISR software stack | None | 256 bytes inside BSS |
| Software-stack sentinel extent below $d000 | 89 bytes | 1356 bytes |
| ISR-stack sentinel extent | N/A | 16 bytes |
| Hardware-stack sentinel extent below entry baseline | 24 bytes | 37 bytes |

The IRQ BSS includes two Executive copies for unchanged-state checks, queue,
runtime, saved registers and ISR stack. Its dynamic foreground stack includes
the 658-byte local scenario fixture and other diagnostics; it is not all dispatch
overhead. The console build has a 658-byte static fixture frame instead. A single
linked runtime `.noinit` byte remains in the IRQ artifact; the application and
producer are compiled with dynamic frames, and the ISR calls no console helpers.
No core, dispatcher or diagnostic heap allocation is used.

The console build places its 30-byte callback table in initialized zero page
and allocates 29 additional spill bytes. It is unsuitable for the IRQ gate's
32-byte preservation contract. The IRQ map has no additional ZP allocation,
checked by its VICE helper. Both helpers discover SYS from the actual PRG header.

## Guarded operation samples

The IRQ monitor pairs stores $10/$20 at $c030, with group/case at $c031/$c032.
Setup, screen output and state fingerprints are outside the intervals. The first
71 paths mask IRQs around the raw Executive operation; the two four-request
batches use the protected native poll. Each interval includes marker/call
overhead and VIC-II bus effects. Raster production has stopped for these samples;
stock KERNAL IRQs can still affect the two wrapper batches and guard pair.
At approximately 985248 cycles per second:

| Group | Samples | Maximum observed PAL cycles | Time | Case |
|---|---:|---:|---:|---|
| Admission / ENDJOB selection | 26 | 3816 | 3.873 ms | Scenario 13, eighth-core FINDVAC |
| Basic context exchange | 36 | 4570 | 4.638 ms | Dispatch 25 |
| Completion / basic dispatch | 6 | 6137 | 6.229 ms | Dispatch 36 |
| Bounded idle startup | 3 | 4115 | 4.177 ms | Dispatch 52 |
| Four-request safe point | 2 | 18357 | 18.632 ms | Four FINDVACs fill the last four cores |
| Exclusion enter/leave | 1 | 24 | 0.024 ms | Restoring initially enabled I |

The 74 intervals cover all 26 old scenarios, 45 isolated dispatch fixtures,
two full buffered batches and one guard pair. The four-request NOVAC batch is
also retained in the JSON. Five mappings are present. These are maxima over the
tested fixtures, not all valid Executive states or all table sizes. The guard
pair is optimized diagnostic code, not a universal indirect-hook call cost.
The largest batch nearly occupies a PAL frame; capacity alone cannot establish
a source-rate or scheduling guarantee.

## Actual IRQ and safe-point latency

The nine owned IRQ intervals run from marker $81 (after owned-source recognition
and acknowledgement) to $82 (after register restore). Eight normal entries take
2027..2181 PAL cycles (2.057..2.214 ms); the register-poison canary takes 2597
cycles (2.636 ms). These include saving/restoring rc0..31, separate-stack setup,
C request publication, 16-byte ISR-local canaries and marker overhead.

They exclude initial CPU entry/KERNAL prologue and the final KERNAL epilogue.
For the pinned ROM and this stack range, the unmarked instruction budget is
86 cycles: seven hardware entry, 29 KERNAL prologue, 25 source recognition/first
marker and 25 final jump/pop/RTI. This arithmetic is an instruction budget, not
a measured interrupt-response bound: instruction completion, IRQ masking,
another pending handler and VIC bus stalls may add time. The monitor retains
raster coordinates and CPU status at each marker for inspection.

The second two-job run omits state hashing. It retains fixed 512-iteration native
C arithmetic, safety checks and payload touches, with one raster request armed
during the low callback. Its recorded intervals are:

| Boundary | PAL cycles | Time |
|---|---:|---:|
| Low callback start marker to poll-return marker | 163271 | 165.716 ms |
| IRQ start marker to poll-return marker | 152525 | 154.809 ms |
| IRQ restore marker to poll-return marker | 150344 | 152.595 ms |
| Poll-entry marker to poll-return marker | 3748 | 3.804 ms |
| Poll-return marker to higher callback entry marker | 7768 | 7.884 ms |
| Deliberately masked three-arithmetic interval | 463052 | 469.985 ms |
| Pre-unmask marker to completed deferred delivery marker | 2534 | 2.572 ms |

IRQ publication alone leaves Executive state unchanged. Poll admits the higher
job while the low callback still runs; the higher callback begins after yield
and another step. The first, fingerprint-checked sequence is retained too, but
its latency is dominated by diagnostic hashing and is not used for this table.

Latency must include the application's longest remaining callback/poll interval,
masked work, source/ISR interference and guarded admission work. Dispatch adds
the time until yield/completion and the next step. The test source has a finite
budget; a continuous source could overflow during the long arithmetic callback.
No unconditional maximum request latency or flight deadline is claimed.

## Console regression and stack limits

The separately retained `executive.prg` still passes 135 original-derived
fingerprints and eight actual two-job sequences. Its four IRQ-enabled samples
are 1187 cycles for idle NOVAC, 2183 for full-core FINDVAC/1202, 5305 for slot-seven
completion/dispatch, and 3668 for slot-seven CHANG1. Different flags, LTO call
graphs, layout and IRQ phase make these unsuitable for extrapolating IRQ-build
costs. Older milestone measurements describe older binaries only.

Sentinel observations are test-specific lower bounds: coincidental $cd values,
untouched paths or IRQ phase can hide deeper use. Hardware instrumentation spans
$0100..$01f1, protecting the 14-byte BASIC/start chain; the IRQ extent corresponds
to about 51 bytes including that chain. Foreground software instrumentation
spans $c100..$cfff. ISR-local content, interrupted arithmetic and explicit SP /
register canaries complement these observations. Reserve the full hardware stack
page and conservative separate software-stack space; no complete BASIC exit or
future callback stack bound is established.

The SDK exposes RAM under BASIC, retains KERNAL/I/O and sets software-stack top
$d000. Respect $0000/$0001 memory mapping, compiler zero page, hardware stack,
screen RAM, loaded image/BSS and reserved mailboxes ($c000..$c00c console;
$c020..$c026 and $c030..$c032 IRQ). Target `int`/pointers are 16 bits. The native
trampoline retains no suspended C stack; persistent job values belong in MPAC /
VAC. Physical C64, NTSC, interpreter dispatch and guidance feasibility remain
unmeasured.
