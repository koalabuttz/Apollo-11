# Original noun and component caller checks

`dsky_noun_check` and `cm_noun_check` execute COMPTEST, COMPTST1,
DCOMPTST, DECTEST, DCTSTCYC, NOUNTEST, and GETCOMP at their actual bank
41 entries 2424/2426/2437/2443/2452/2460/2515. Both missions use their
own pinned source assemblies and exceptional entry addresses. The native
body reproduces GETCOMP's selected normal/mixed type field, LEFT5's CYL
read/write debris, component-count comparison, decimal-only test, and
no-load test. It preserves the source Q/L/MPAC+2 return conventions.

These reusable caller checks support operator loading without reusing the
bounded display-verb private helpers. Existing output and keyboard bytes
remain unchanged. A failed component test reaches genuine GODSPALM;
DCTSTCYC reaches ALMCYCLE; positive zero at the component comparison
reaches actual CCSHOLE. No source exception is converted into job end.

The canonical image and caller producer exclusion remain authoritative.
The contract accepts MIXBR 1/2 and ordinary type words, all EBANKs,
selecting SUPER 100/120/140/160, IRQ 0/1, and raw unused registers.
Component arguments, only when consumed, are minus two, minus one, or
either zero. Ordinary caller Q 0100..7777 is required, or caller L for
direct COMPTST1. Channels/events/restart and Executive metadata are
unused and remain opaque. Rejection preserves all input state and route.

8,192 independent unchanged-original checkpoints exercise all 32 type
codes, normal/mixed selection, the four component arguments, raw unused
registers, caller returns, and real alarm/recycle/hole outcomes. All
2,048 words, SUPER/IRQ, channels 11/12/13, event mask, restart flip-flop,
and route agree between original, native, and full ASan/UBSan streams
(2,056 fields), SHA-256:

`56340cd8883e67c7bb1df65196fe3c8df848fc8bface8aab80817e80b544d385`.

Frozen 64 states and 16 atomic guards pass host/sanitizer. Actual stock-ROM
PAL C64 runs compare 32 full states per mission: LM `a500002000`, PRG
5,431 bytes (5,429 loaded), stack extents 10/21; CM `a500002001`, PRG
5,451 bytes (5,449 loaded), stack extents 10/22. No additional zero-page
storage is allocated and both builds remain below C000. The sampled
caller addresses avoid colliding with exceptional checkpoints; this is
not exhaustive enumeration of every caller/input product. Fresh source
fixture ropes remain byte-identical to their pinned originals.

`tools/dsky_noun_check.mk` supplies isolated host/test/sanitizer/source
comparison and C64 targets. These seven local algorithm entries close
under the stated caller contract; selected alarm/recovery bodies, actual
load request/store loops, and input scale routines remain separate source
continuations to compose. No emulator or AGC decoder is linked into native
code, and no extra scheduler poll is added.
