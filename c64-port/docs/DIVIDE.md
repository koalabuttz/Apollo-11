# P66 DDV 4SEC(28)

`p66_divide_four_seconds` translates DDV immediately after STORE 30D. It divides
the raw elapsed pair by the original constant `{0,00620}` (decimal 400), leaving
the quotient in MPAC and stopping at DANZIG before pending PDVL loads GDT/2.
LUNAR_LANDING_GUIDANCE_EQUATIONS:1007..1009 places DDV/PDVL at bank 32,3362,
the divisor address at 32,3363 and the later load address at 32,3364.

## Fixed-divisor contract and ownership

The caller owns distinct preparation, scaling, elapsed and division objects,
excludes their producers, and supplies DP MODE=+0 with ordinary 15-bit memory
words. Independent word signs and both zeros are supported. A/L remain 16-bit
AGC registers. Unsupported MODE returns OUT_OF_SCOPE without mutation, an
explicit native scope adaptation rather than a historical alarm.

The new scratch object owns BUF +1, DVSIGN, DVNORMCT and MAXDVSW: four words /
eight bytes. BUF high continues to alias `p66_elapsed_state.buf`; OVFIND and
MPTEMP remain in scaling scratch. Total modeled context is 93 words / 186 bytes.
The saved elapsed pair at VAC words 30/31, all 43 VAC words, FIXLOC/PUSHLOC,
PIPTIME, MPTEMP and unrelated preparation/vector state remain unchanged.
MPAC 0/+1/+2, BUF, the four new words, A/L and possibly OVFIND change.

This is a specialization of the source sequence for the fixed four-second
constant, not a general DDV API. `SERVICER.agc:1095` declares `2DEC 400 B-28`.
Preparation verifies that unchanged declaration and its source hash, while the
adapter independently checks the actual assembled pair at 33,3152/+1. The
fixture retains its twelve byte-identical overlays and existing corrected
support; SERVICER is verified separately, not added as a full overlay. Its
original source SHA256 is
`4e3798d5dabcc7ecca813d762824f7dfc41ad352a9869a7526565fc3a05f4415`.
The rope remains
`1f5326e038de5b741b2f27b01ec949dbd688cf1906994e997402587c8628f40e`.

## Arithmetic and retained debris

DDV (7577) loads BUF then switches FBANK to zero. DDV/BDDV (00,2353) initializes
DVSIGN, DVNORMCT and MAXDVSW to -1. Since the divisor high word is zero,
BUFZERO clears MPAC +2 and calls TPAGREE, preserving its exact raw word stores
and TS skip effects. Its original return address 02365 remains in L until a
later exchange/divide replaces it. Mixed-sign pairs such as `{1,040000}` become
the small positive count they represent rather than a spurious large dividend.

A nonzero sign-agreed major word takes the original overflow exit immediately.
Otherwise BUFZERO shifts the operand pairs by a word, making the divisor
`{400,0}` and the dividend the original minor count. Zero exits with canonical
+0 MPAC words, A=+0, L=02365 and OVFIND retained. Negative nonzero inputs set
DVSIGN=-0; positive inputs retain -1.

Magnitude >=400 uses DVOVF: both quotient words become same-sign maxima,
OVFIND=1 and A=1. L retains 02365 for a positive overflow, or +POSMAX after the
negative result exchange. MPAC +2 retains the sign-agreement zero. Overflow
before the word shift leaves BUF `{0,400}`; overflow afterward leaves `{400,0}`.
Exactly 400 also sets MAXDVSW=+0, while larger magnitudes retain -1.

For magnitudes 1..399, five left shifts normalize BUF to `{12800,0}`, with
DVNORMCT=-6. The low divisor word remains zero, so GENDDV's two integer DV
stages yield an exact truncated 28-bit fraction for this constant. Native code
uses two 14-bit quotient stages and 32-bit arithmetic, with no 64-bit arithmetic,
floating point, heap or instruction decoder. It adds no rounding: the generic
low-divisor approximation and a normal MAXDV quotient path are unnecessary here.
Final A and MPAC +2 are +0. For a positive quotient L is the final remainder;
for a negative quotient the source DCS/DXCH leaves the old positive low quotient
in L. OVFIND retains its earlier contents on zero and ordinary quotient paths.
The result enum observes zero/quotient/overflow; it is not historical memory.

## Hardware-DV oracle and pinned fast-path discrepancy

This milestone's oracle executes unchanged original assembly using the pinned
engine's existing `SimulateDV`, its separate model of the hardware register
steps, for every DV. A retained one-line host-only branch change selects that
path. `tools/prepare_hardware_dv.py` checks the pinned engine source hash, emits
the reviewable patch and compiles a generated copy under build/. The pinned
checkout, all non-DV instructions, rope and earlier oracles remain unchanged.
No native numerical code is linked into either oracle; the generated engine
and adapters remain GPL host tools and are never linked into a C64 PRG.

The pinned fast mathematical DV path passes sign-extended L=-0 as `int16_t -1`
to SpToDecent, whose zero comparison expects the 15-bit value 32767. It treats
that zero as a mixed-sign nonzero and decrements the high dividend. For elapsed
-1, the correct magnitude stages are 40/15728: the hardware path produces raw
MPAC 32727/17039 and L=15728. The fast path instead gives 32728/21626 and L=11141.
This is an engine conversion discrepancy, not source rounding to reproduce.

`tools/audit_divide_models.py` compares both engine variants without native
code over all 8098 checkpoints and records every differing case. Exactly 798
rows differ, only in L, MPAC and MPAC +1; all path observations and other words
agree. The audit is part of the division differential command. Its JSON, log,
source identities and patch are retained. Verification claims here use the
hardware DV model and do not claim equality with the pinned fast-DV shortcut
or physical Apollo hardware testing.

## Coverage and boundaries

| Cases | Coverage |
|---|---|
| 0..15 | Both zeros, +/-1, +/-399/400/401, mixed signs and major overflow |
| 16..143 | All 128 integrated preparation skip inputs in both sign families |
| 144..146 | Three four-step retained full-prefix histories |
| 147..402 | 256 raw source/destination corner inputs |
| 403..592 | All 190 VAC/push positions, including overlapping STORE slots |
| 593..2640 | 2048 direct deterministic random inputs |
| 2641..3664 | 1024 integrated deterministic random inputs |
| 3665..5712 | Every count -1024..1023, covering all possible ordinary quotients |
| 5713..6734 | 1022 mixed-sign representations of +/-1..511 across a major-word boundary |

6735 cases produce 8098 complete 93-word/result checkpoints: 5390 direct final
states and 1354 integrated calls with both post-STORE and final DDV boundaries.
Final paths count 69 zero, 1614 quotient and 5061 overflow. Matched snapshot:
`ae9ad539cb822f1f36ab85ac4bc12ae3dc1e7bc5eae786b63c6ac65202a9bb70`.

The oracle asserts the fixed operand/banks, final MODE, pointer, interrupt
permission, eleven adjacent guards, VACUSE/upper bound and ZERO register 7.
DDV entry has ADDRWD=03152, FBANK=066000 and EBANK=03400. Final BBANK=7,
FBANK=0, EBANK=03400, LOC=03363 and ADDRWD=03152. Final Q is 2 on zero, 3 on
ordinary quotient and 02413 on overflow. LOC/BANKSET/ADDRWD/EDOP/Q and physical
interpreter dispatch remain outside the native resume contract. Hardware counter
progress is held and watchdog alarms suppressed for bounded oracle calls.

All twelve host and ASan/UBSan suites pass (managed-environment leak scanning
disabled). All unsupported 15-bit modes reject without mutation. Frozen target
expectations contain 156 full 93-word/result states, all 128 integrated skip
inputs and three retained histories. The twelfth standalone C64 PRG passes all
expectations, enabled/already-masked restoring I and ZP/RAM/stack checks, with
mailbox `a500009c0003ba5d24`. All 195 earlier evidence identities and historical
sources remain unchanged; the manifest now holds 218 identities.

## Target measurements and continuation

The PRG is 46492 file bytes / 46490 loaded at $0801..$bd9a, SYS $080d. Text is
16907 bytes, rodata 29571, BSS 186 and noinit one byte. Expected words/paths use
29172 bytes. Image plus BSS/noinit is 46677 bytes, ending at $be55, below the
reserved mailbox/stack RAM. There are 32 imaginary ZP registers and no extra ZP.

| Guarded sample | PAL cycles | At 985248 Hz |
|---|---:|---:|
| Zero DDV | 447 | 0.454 ms |
| Positive quotient | 6236 | 6.329 ms |
| Negative quotient | 6423 | 6.519 ms |
| Division overflow | 1222 | 1.240 ms |
| Full RODCOMP through DDV, history step 0 | 27940 | 28.358 ms |

Marker/call and VIC bus effects are included; setup/full comparisons are outside
the intervals. Full-prefix masking exceeds a PAL frame. These finite observations
do not establish WCET, a live producer-rate budget or a flight deadline. Hardware
stack extent is 44 below entry (58 with ROM frame); software extent 324 includes
the complete checker. These separate diagnostics are not a combined flight image.

```sh
make test
ASAN_OPTIONS=detect_leaks=0 make sanitize
python3 tools/prepare_reference.py
make differential REFERENCE="$PWD/deps/fixture"
python3 tools/freeze_divide.py
make c64-divide MOS_CC=/path/to/mos-c64-clang
python3 tools/divide_vice_test.py --vice /path/to/x64sc \
  --rom-dir /path/to/C64 --data-dir /path/to/VICE/data
```

Next: translate pending `PDVL GDT/2`, preserving the quotient pushdown, vector
load, MODE/PUSHLOC, exchange debris and scratch. Verify the boundary before VSU
and retain the elapsed pair at VAC 30/31. Live PIPA/PIPTIME publication and P66
scheduling still need explicit ownership and timing budgets.
