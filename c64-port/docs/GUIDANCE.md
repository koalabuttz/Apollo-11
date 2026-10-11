# P66 rate-of-descent update and multiword arithmetic

The seventh milestone translated the first six instructions of `RODCOMP` in
unchanged `Luminary099/LUNAR_LANDING_GUIDANCE_EQUATIONS.agc`, lines 948..953:
`INHINT`, `CAF ZERO`, `XCH RODCOUNT`, `EXTEND`, `MP RODSCAL1`, `DAS VDGVERT`.
Entry is physical bank 32,3275; the oracle stops at 32,3303 before the following
PIPA load. This is a real landing-guidance calculation with a small boundary.
That API remains bounded to the rate update. The pre-interpreter extension is
now documented in PREPARATION.md; a complete P66 guidance job remains future work.

## Numerical contract

`p66_rate_state` retains six words: RODCOUNT, RODSCAL1, the two VDGVERT words,
and raw 16-bit A/L results. It occupies 12 target bytes. All memory inputs are
valid 15-bit one's-complement words, including both zeros; A may have any
16-bit pattern. The standalone DAS primitive requires sign-extended L and
ordinary erasable destination words, with independent signs permitted.
Register-address aliases, edit registers and overlapping pointers are excluded.

`agc_multiply` corrects accumulator overflow before multiplying two 14-bit
magnitudes with a 32-bit unsigned intermediate. The product is split into
14-bit high and low magnitudes, complemented for negative results, then sign
extended into A/L. A zero memory multiplier always gives two +0s. Otherwise a
zero accumulator retains the sign determined by the operand signs. A nonzero
negative product with zero low magnitude therefore has -0 in its low word.
No floating point, host signed overflow or saturation is used.

`agc_double_add` adds the low words as sign-extended SP values. Low positive or
negative overflow adds +1 or -1 to the high sum before correction. Both stored
words are corrected separately; their signs are not normalized. Final high
overflow returns sign-extended +1/-1 in A, otherwise +0; L becomes +0.
Overflow wraps as in the original, with no alarm or saturation added.

`p66_rate_update` consumes RODCOUNT and clears it to +0, multiplies the consumed
count by RODSCAL1, adds that product to VDGVERT, and retains the final A/L.
RODSCAL1 is unchanged. Native scratch replaces the hardware registers and bank
addresses; the Executive's MPAC/VAC state is not implicitly updated.

Scales remain raw historical words. `STARTP66` copies the existing altitude rate
to VDGVERT and its interpreter setup loads RODSCALE into RODSCAL1. RODSCALE is
an erasable click-scale input, not a fixed ROM constant in this slice. DESCBITS
adds +1 or -1 for switch events. For a canonical pair, its integer numerator is
`high * 16384 + low`; this update adds the integer product of count and scale
in that representation. No physical-unit conversion or assumed uplink value is
introduced. Initialization and the switch trap are traced, not translated here.

## Ownership and interrupt adaptation

The caller owns this state and excludes any count producer for the whole update.
The API itself neither enables nor disables interrupts. Original INHINT leaves
AGC interrupts inhibited into later RODCOMP work. The standalone C64 diagnostic
uses restoring guards around measured calls and verifies enabled entry permission is restored and an initially masked
caller remains masked. Restoring permission at this native boundary is an
explicit adaptation; no original RELINT/RESUME or live click IRQ is claimed.

The native functions contain direct arithmetic and calls, with no opcode decoder,
rope image or AGC engine. PIPA snapshot/delta preparation is now translated
(PREPARATION.md). Interpreter vector operations, thrust calculation, throttle
control, task scheduling of RODCOMP,
guidance restart protection and physical switch input remain future work.

## Independent verification

`guidance_reference.c` links Virtual AGC and input-only constructors, never native
arithmetic/guidance. It executes actual MP/DAS instructions in a host-only
erasable test fixture; rate cases execute the original assembled RODCOMP bytes.
All nine selected source files in the full reference fixture are byte-identical
to the repository. The rope remains SHA256
`1f5326e038de5b741b2f27b01ec949dbd688cf1906994e997402587c8628f40e`.
Listing checks pin entry/data addresses. Counter progress is held during bounded
calls; physical BBANK, adjacent destination words and original INHINT are checked.

12,835 cases produce 12,856 checkpoints:

| Cases | Workload |
|---|---|
| 0..143 | MP over twelve signed-zero/sign/magnitude edges per operand |
| 144..399 | All 256 four-word DAS combinations of +0, -0, +maximum, -maximum |
| 400..543 | Rate updates over twelve count and scale edges, mixed initial word signs |
| 544..546 | Three eight-step histories; +1/+1/-1/+0/-0/+3/-3/+0 clicks, retained rate |
| 547..4642 | 4096 deterministic MP cases, including uncorrected 16-bit A |
| 4643..8738 | 4096 deterministic DAS cases with independent signs and 16-bit A |
| 8739..12834 | 4096 deterministic count/scale/initial-rate cases |

Every row compares all six modeled words, including full 16-bit A/L, exactly.
Native/reference snapshot SHA256 is
`919925e42e327b9a0d426444be5f96af636bdf69812723f946db07ce086aed55`.
`freeze_guidance.py` requires complete equality before freezing the first 568
checkpoints as full six-word expectations, with no fingerprint collision risk.
Host tests also check all 65,536 SP/memory-zero products and explicit positive/
negative carry and overflow examples. All six suites pass ASan/UBSan with leak
inspection disabled for this executor. Earlier snapshots remain unchanged.

The sixth C64 PRG runs all 568 complete-word checks and histories in PAL VICE.
Mailbox at $c080..$c088 is `a500003802030c0d25`, with zero failures. Six measured
calls, restoring-I checks, compiler-ZP checks and stack sentinels pass.
PERFORMANCE.md records image, memory and timing observations. These finite
samples establish neither worst-case execution time nor a flight deadline.

## Reproduce

```sh
make test
ASAN_OPTIONS=detect_leaks=0 make sanitize
python3 tools/prepare_reference.py
make differential REFERENCE="$PWD/deps/fixture"
python3 tools/freeze_guidance.py
make c64-guidance MOS_CC=/path/to/mos-c64-clang
python3 tools/guidance_vice_test.py --vice /path/to/x64sc \
  --rom-dir /path/to/C64 --data-dir /path/to/VICE/data
```

Evidence includes matched snapshots, source/fixture hashes, traces, test logs,
the PRG/map, VICE mailbox/stack dumps and measured intervals. Earlier evidence is
retained unchanged. No hardware C64 or full landing trajectory has been tested.

The later VXSC KPIP1 operation is now implemented independently; SCALING.md
defines its numeric and interpreter-entry boundaries. This rate API is unchanged.
