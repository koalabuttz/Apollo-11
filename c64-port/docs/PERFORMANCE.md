# Native C64 resource and timing findings

The retained second-milestone diagnostic uses LLVM-MOS SDK v23.3.0, `-Os`, and
PAL x64sc 3.7.1 with ordinary C64 ROMs. It contains the native basic dispatcher,
actual mapped demonstration jobs and reference-fingerprint diagnostics. These
are measured samples, not flight-software bounds or AGC cycle equivalence.

| Quantity | Result | Evidence / limit |
|---|---|---|
| PRG file size | **19043 bytes** | Includes two-byte load address; `evidence/executive.prg` |
| Loaded image | 19041 bytes, $0801..$5261 | BASIC header, initialized zero-page table, text and read-only data |
| Native text | 17995 bytes ($464b) | Includes Executive, dispatcher, callbacks, diagnostics and runtime helpers |
| Read-only data | 1004 bytes ($3ec) | Includes 135 state fingerprints and diagnostic strings |
| Initialized zero-page table | 30 bytes | Five historical-address/function-pointer mappings; copied from the image |
| Linked BSS | 658 bytes, $5262..$54f3 | Static diagnostic scenario |
| Linked `.noinit` static stack | 658 bytes, $54f4..$5785 | LLVM-MOS's static allocation for fixture preparation; must count as RAM |
| Image plus BSS/noinit | 20357 bytes, $0801..$5785 | Excludes zero page, dynamic stacks, ROMs and mailbox |
| Executive state | **648 bytes** | Host and C64 target mailbox agree |
| Zero-page allocation | 89 bytes, $02..$5a | 32 imaginary-register bytes, 30 `.zp.data`, 27 `.zp.noinit` |
| Heap allocation | None | Core, dispatcher and diagnostic do not allocate |
| Hardware stack sentinel extent | 24 bytes below entry baseline | About 38 bytes with the 14-byte BASIC/start chain; lower-bound observation |
| Software stack sentinel extent | 81 bytes below $d000 | Instrumented $c100..$cfff; separate from the 658-byte static fixture stack |
| Diagnostic mailbox | 13 bytes, $c000..$c00c | Completion, failures, sizes/counts and timing markers |

The BASIC SYS entry is $082b. Initialized zero-page data precedes text, so the
previous $080d entry is no longer executable code. `vice_test.py` reads the SYS
address from the PRG itself. Startup, code and data all fit stock C64 RAM; the
map is diagnostic-specific, not a complete flight-software memory plan.

## Isolated timing samples

The VICE monitor stops at paired stores to $c008 and measures the intervening
PAL cycles. Setup, fingerprinting and screen output stay outside the intervals.
The intervals include function/call and end-marker overhead, VIC-II bus effects
and any stock IRQ service. IRQs remain enabled. At approximately 985248 cycles
per second, the retained run records:

| Path | PAL cycles | Approximate time | Fixture |
|---|---:|---:|---|
| First idle NOVAC | 1187 | 1.205 ms | Original scenario 0 |
| FINDVAC with all eight cores full, retained VAC, 1202 boundary | 1957 | 1.986 ms | Original scenario 25 |
| ENDJOB1/EJSCAN and slot-seven basic dispatch | 5387 | 5.468 ms | Dispatch case 38; includes native interpreter preflight scan |
| Basic CHANG1 context exchange to slot seven | 3893 | 3.951 ms | Dispatch case 6 |

The first milestone's 657-cycle admission applied to its smaller diagnostic
and different LTO call graph. It does not describe this artifact. An intermediate
build in this milestone also varied with code layout/IRQ phase; only the retained
binary and run above are current evidence. The largest of these four samples is
not a proved worst case. Case 38 scans all waiting positions but is not a search
of every valid priority/state combination. The full diagnostic is dominated by
FNV hashing; its completion time is not Executive scheduling cost.

## Stack, IRQ and memory limits

Sentinel observations are test-specific lower bounds: coincidental $cd values,
untouched paths or IRQ phase can hide deeper use. The protected entry chain was
not overwritten. The final store stops the test before returning to BASIC, so
there is no complete BASIC exit-stack bound. Reserve the full hardware stack
page and conservative software-stack space for future IRQ tests.

The native trampoline does not preserve suspended C stacks. Every callback
returns before another callback is dispatched; persistent job values reside in
MPAC/VAC. The measured dynamic software stack includes callback/trampoline and
observer calls. No recursion, host threads, generic ready queue or AGC instruction
emulator is linked into the PRG.

$0000/$0001 control the 6510 map, zero page also holds SDK/compiler state, $0100
is the hardware stack, screen RAM is $0400 and the BASIC SYS image begins $0801.
The SDK exposes RAM under BASIC, retains KERNAL/I/O for console calls, and sets
software-stack top $d000. The mailbox is explicitly reserved from normal free
$c000 RAM. Overlays must respect these regions and the added static fixture RAM.

Target `int` and pointers are 16 bits; host sizes differ. The implementation uses
fixed-width unsigned words. Compiler arithmetic helpers are included; 32-bit FNV
multiplication is diagnostic overhead. Stock KERNAL IRQs do not request Executive
jobs. Safe application IRQ delivery, LLVM-MOS register preservation, maximum
request latency and safe-point bounds are the next milestone. No physical C64,
NTSC timing, guidance deadline or mission feasibility measurement is claimed.
