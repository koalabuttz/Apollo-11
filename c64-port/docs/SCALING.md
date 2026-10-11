# P66 VXSC KPIP1 native scaling

The ninth milestone translates P66's first interpretive arithmetic operation,
`VXSC KPIP1` at ITRPNT1 (bank 32,3354). `p66_scale_velocity` directly computes
that operation in native C. The original assembly adapter stops at DANZIG
(6061), before the pending PDDL executes. No instruction decoder is linked into
the C64. This is one bounded operation, with explicit native scratch ownership;
interpreter dispatch and a complete scheduled P66 job remain future work.

## Representation and arithmetic

KPIP1 is the unchanged constant at bank 33,2021: `{00321,26706}` in octal,
assembled from `2DEC .0128`. The original P66 comment describes the output as
m/centisecond times 2^-7. Input words remain the raw AGC vector assembled by
RODCOMP; the native API adds no SI conversion or sensor calibration.

MODE aliases MPAC +7 and must contain -1 (`077776`). Components occupy
MPAC +0/+1, +3/+4 and +5/+6. Each word uses its own one's-complement sign.
Independent signs and both zero encodings remain meaningful; no host signed
integer or floating-point vector conversion is introduced.

DMPSUB forms four partial products with the existing verified normal-memory
MP primitive. It discards the low word of the minor/minor product, accumulates
the cross products with DAS, then adds the major/major product. That ordering
and intermediate word correction are preserved, including mixed-sign pairs.
Replacing it with a mathematical full product followed by a generic round
would not describe the historical word operations.

VROUND exchanges MPAC +2 with the final DAS accumulator, doubles the old third
word, and uses TS L's overflow skip to select a signed rounding adjustment.
It rounds at exactly half as well as above half: positive and negative half
produce +1 and -1 adjustments respectively. A low-word overflow propagates
into the high word. DXCH on the vector exit corrects and sign-extends surviving
A/L, including the doubled-third debris in L. MPAC +2 becomes +0. MPTEMP becomes
KPIP1's high word; OVFIND is retained in every tested case. The translated
high-overflow branch follows the source, but KPIP1's small magnitude does not
exercise an OVFIND event; no coverage of that branch is claimed.

## API, state and boundary

```c
void p66_scale_velocity(p66_preparation *state, p66_scale_scratch *scratch);
```

The 78-byte preparation state is reused. Four additional bytes represent
MPTEMP and OVFIND, for 41 modeled words / 82 target bytes together. MPAC, MODE
and A/L have one owner and one representation. All other preparation words
are preserved by scaling. Both objects must be distinct and owned by the
caller, with count/PIPA/time/temporary producers excluded across a composed
call. To compose, call `p66_prepare` then `p66_scale_velocity` within that owned
interval. There is no implicit copy to an Executive context or native callback.

The genuine interpreter entry executes RELINT, initializes LOC/BANKSET and
decodes the first operand into ADDRWD, switching FBANK to 33. Integrated oracle
cases execute these instructions from RODCOMP, assert vector mode, operand
address 2021, FBANK, LOC=3355, BANKSET and enabled interrupt permission, then
run VXSC to DANZIG. Direct cases enter VXSC with that operand bank/address set.
Five adjacent memory guards and all 64 actual preparation TS-skip masks in
both sign families are checked. Hardware counters are held stable and
hardware watchdog alarms suppressed during these bounded oracle executions.

Native code uses an explicit call and a fixed scalar, rather than retaining
LOC, BANKSET, ADDRWD, CYR, EDOP, Q or bank-selector machinery. These machine
control/scratch fields are outside the 41-word contract. The target diagnostic
uses a restoring I guard around each owned operation; it does not reproduce
RELINT or invoke scheduling partway through the arithmetic. Initially enabled
and already masked callers are checked. This boundary is an explicit native
adaptation, not a complete interpreter-context equivalence claim.

## Verification

The host adapter links only input constructors and genuine Virtual AGC engine
objects; no native numerical or guidance code is linked. The fixture overlays
11 unchanged historical source files, adding INTERPRETER and CONTROLLED_CONSTANTS
to the prior nine. The assembled rope retains SHA256
`1f5326e038de5b741b2f27b01ec949dbd688cf1906994e997402587c8628f40e`.

| Cases | Coverage |
|---|---|
| 0..143 | All 12 by 12 high/low edge pairs, with different component permutations |
| 144..151 | Below/exactly/above half and low-word rounding carry, both signs |
| 152..279 | Full RODCOMP/interpreter entry from every six-TS skip input in both signs |
| 280..282 | Three four-step histories, retaining time/PIPA/rate/DELV/MPAC/scratch |
| 283..2330 | 2048 deterministic direct random vectors with independent word signs |
| 2331..3354 | 1024 deterministic full-prefix random inputs |

All 3355 cases / 3364 complete 41-word checkpoints match. The original engine
observes third-word magnitudes below/equal/above half: 2826/3/2249 positive and
2780/3/2231 negative, 4486 low adjustments and six high-word propagations.
Expected states are frozen only after the complete independent comparison.
Snapshot SHA256:
`11993e66747e7685e964c555e08e72c73d9782d2674a46d3ec0b2d4b00c36f2a`.

All eight host and AddressSanitizer/UndefinedBehaviorSanitizer suites pass
(LeakSanitizer disabled for the managed process environment). Every earlier
snapshot, historical file, rope and all 119 earlier evidence hashes remain
unchanged. The eighth standalone C64 PRG checks 292 complete states and retained
histories in PAL VICE, with zero failures and mailbox `a50000240103522924`.
`tools/scale_vice_test.py` verifies mailbox completion, restoring I, compiler ZP
allocation, five timing intervals and instrumented stack extents.

## Target measurements and continuation

The PRG is 32979 file bytes / 32977 loaded bytes at $0801..$88d1, SYS $080d.
Text is 8818 bytes, read-only data 24147 (23944 bytes are full expected words),
BSS 82 and noinit one byte. Image plus BSS/noinit is 33060 bytes. The compiler
uses its 32 imaginary ZP registers with no additional ZP allocation. Native
arithmetic uses integer operations, with no heap or floating point.

Measured intervals include marker/call overhead and VIC bus effects. Below
half, exact half, positive carry and negative carry take 23415, 24297, 24799 and
25737 PAL cycles. The sampled complete prefix plus scaling takes 19838 cycles
(20.135 ms). The largest sample is 26.122 ms, exceeding one PAL frame and
held with interrupts masked in this diagnostic. These are finite fixture
observations, not WCET or a guidance/source-rate deadline proof. A live job
needs an explicit producer exclusion and scheduling budget before integration.
Hardware-stack sentinel extent is 26 bytes below entry (40 with the 14-byte ROM
frame); the diagnostic's software-stack extent is 187 bytes, including fixtures.

PDDL THISTPIP is implemented in the following milestone; PUSHDOWN.md defines
its VAC ownership and timestamp/exchange effects. This scaling API still stops
before PDDL. DSU PIPTIME is now verified in ELAPSED.md; STORE 30D is verified in STORE.md; DDV is next.
