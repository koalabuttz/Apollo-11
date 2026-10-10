# Native C64 resource and timing findings

These measurements cover the retained diagnostic build using LLVM-MOS SDK
v23.3.0, `-Os`, normal PAL C64 emulation in x64sc 3.7.1. They are not worst-case
flight-software bounds. The native core is aggressively inlined by LTO.

| Quantity | Result | Evidence / limit |
|---|---|---|
| PRG file size | **5829 bytes** | Includes two-byte load address; `evidence/executive.prg` |
| Loaded image | 5827 bytes, $0801..$1ec3 | BASIC SYS header, text, read-only data; VICE load log |
| Native text | 5316 bytes | `.text`=$14c4, including runtime helpers |
| Read-only data | 499 bytes | `.rodata`=$1f3; includes diagnostics and reference digests |
| Linked BSS | **658 bytes** | `.bss`=$292, $1ec4..$2155; static diagnostic scenario |
| Executive state alone | **648 bytes** | Target mailbox and host size agree |
| Total image plus BSS | 6485 bytes | $0801..$2155; excludes zero page, stacks, ROMs, mailbox |
| Zero-page use | 62 bytes, $02..$3f | 32 imaginary register bytes plus 30 `.zp.noinit` bytes in map |
| Heap allocation | None | Native core/diagnostic has no allocator or heap calls |
| Hardware stack observation | 19-byte sentinel extent below entry baseline; about 33 bytes including existing 14-byte BASIC/start chain | Instrumented $0100..$01f1; lowest changed sentinel=$01df; earlier run observed 21/35 bytes; varies with IRQ phase |
| Software stack observation | 0 changed sentinel bytes in $c100..$cfff | SDK initializes downward stack at $d000; this LTO diagnostic did not touch instrumented region |
| Diagnostic mailbox | 9 bytes at $c000..$c008 | Explicit platform test reservation, outside loaded image |
| First idle NOVAC admission | **657 PAL cycles** (~0.667 ms at ~985248 Hz) | Monitor stopwatch between $c008 start/end stores; includes end-marker store/branch overhead |
| Full scenario/fingerprint diagnostic | Roughly 33 million PAL cycles after first timing marker (~33.5 seconds of emulated time) | Monitor done-store stopwatch; dominated by FNV hashing, not Executive work |

An earlier build measured 655 cycles; the final retained build is 657. The
diagnostic's full-state hashing is intentionally expensive. Warp mode changes
host execution speed, not the emulated target cycle count. The stopwatch includes
VIC-II bus effects and any IRQ servicing in its interval. Only this one admission
path is isolated and measured; no other native routine cost is asserted.

Stack sentinel observations are test-specific lower bounds, not a proved stack
requirement. Sentinel collisions and untouched paths can hide deeper use. The
initial 14 bytes of hardware stack are protected rather than overwritten. Normal
C64 IRQs can increase the observed extent. Code ends the test before returning
from main; no end-to-end BASIC return/exit stack bound is claimed. Keep the full
hardware stack page available and reserve a conservative software stack until
dispatch/interrupt tests establish stronger bounds.

C64 constraints: $0000/$0001 control the 6510 memory map; zero page also contains
SDK/compiler state; $0100 is the hardware stack; screen RAM is $0400; the BASIC
SYS image starts at $0801. The SDK exposes RAM under BASIC during execution,
retains KERNAL/I/O for console calls, and sets software-stack top $d000. The
mailbox consumes part of normally free $c000 RAM and must be removed or explicitly
reserved in a production mapping. Future overlays cannot assume KERNAL, I/O,
screen, or stack memory is freely available.

Compiler considerations: target `int` and pointers are 16 bits, unlike the host;
numerical data uses fixed-width unsigned types and wider explicit intermediates.
The target includes compiler division/modulo and multiplication helpers. The
32-bit multiplication helper in this diagnostic is for fingerprinting, not AGC
guidance arithmetic. No MOS-specific assembly has been required yet. LLVM-MOS
imaginary-register protection in IRQ/native continuation boundaries remains an
open integration question. Stock hardware testing and NTSC timing are unavailable.
