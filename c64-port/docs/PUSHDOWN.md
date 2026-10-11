# P66 PDDL THISTPIP native pushdown and timestamp load

The tenth milestone translates the pending `PDDL THISTPIP` operation following
VXSC KPIP1. `p66_push_timestamp` saves the scaled six-word vector into an owned
VAC payload, loads THISTPIP into DP MPAC, and preserves the original exchange
debris. The boundary is DANZIG (6061) after PDDL, before DSU PIPTIME. All code
is native C; no interpreter decoder or emulator is linked into the C64.

## State and ownership

```c
enum p66_push_status p66_push_timestamp(p66_preparation *state,
                                       p66_pushdown *push);
```

`p66_pushdown` owns 43 payload words plus two raw AGC words: `base` represents
FIXLOC and `pushloc` represents PUSHLOC. The five supported VAC bases are octal
0401, 0455, 0531, 0605 and 0661. The preceding VACUSE word is outside the object.
The native object is 90 bytes. Together with the prior 78-byte preparation and
four-byte scaling scratch, a complete diagnostic state is 172 bytes / 86 words.
Each word has one representation: MODE remains MPAC +7 and A/L remain the
preparation registers. Scaling scratch is retained, not duplicated.

The caller owns distinct preparation and pushdown objects and excludes count,
PIPA/time and temporary producers. There is no allocation, implicit Executive
VAC copy, context switch or mapped guidance callback. A later job adapter must
bind this ownership to its selected Executive context. These explicit calls
compose within one owned interval:

```c
p66_prepare(state);
p66_scale_velocity(state, scale_scratch);
status = p66_push_timestamp(state, push);
```

A vector entry requires MODE=-1 (`077776`) and six free words starting at
PUSHLOC. Any offset 0..37 within the selected 43-word payload is supported,
including odd offsets. Invalid mode, VAC base or pointer returns
`P66_PUSH_OUT_OF_SCOPE` before either object changes. This native scope check
adds no historical alarm or generic overflow handling. Historical PDDL does not
perform this bounds check; writing outside a selected VAC is excluded from the
translated domain. Successful offset 37 leaves PUSHLOC at the one-past address;
a subsequent vector push without released space is rejected.

## Original exchanges and final scratch

Let P be the initial PUSHLOC, Vx/Vy/Vz the three input DP pairs, and O0..O5 the
old words at P..P+5. PDDL exchanges THISTPIP with MPAC +0/+1, exchanges Vx into
the first pushdown pair, then advances PUSHLOC by six. CCS MODE selects the
vector branch and leaves A=+0, which clears MODE and MPAC +2. The remaining
DXCH operations exchange Vy/Vz into the other pushdown pairs. They leave:

| State | Final value |
|---|---|
| VAC[P..P+5] | Vx.high, Vx.low, Vy.high, Vy.low, Vz.high, Vz.low |
| PUSHLOC | P+6 |
| MPAC +0/+1 | THISTPIP.high / THISTPIP.low |
| MPAC +2/+3 / MODE (+7) | +0 / +0 / +0 (DP mode) |
| MPAC +4/+5/+6 | O1 / O2 / O3 |
| A / L | Sign-extended O4 / O5 |

O0 is discarded by the CCS path. Both signed zeros and independent high/low
signs remain raw words. The remaining 37 VAC words, FIXLOC, timestamp input,
all unrelated preparation words, MPTEMP and OVFIND are preserved. No low-word
normalization, sensor conversion or timestamp subtraction is added.

## Original-assembly verification

The adapter links only input constructors and genuine pinned Virtual AGC
objects. Direct cases enter PDDL at 6523 with ADDRWD=1761, E-bank 7 and bank 32.
Integrated cases execute unchanged RODCOMP from 32,3275, stop at DANZIG after
VXSC, then continue through the real pending PDDL dispatch. At PDDL entry they
assert ADDRWD, E/F-bank, MODE, PUSHLOC, LOC=3356, BANKSET and RELINT permission.
After PDDL they assert DP MODE, pointer advance, bank/interrupt permission,
five adjacent preparation guards, the preceding VACUSE and the word after the
selected VAC. All 86 modeled words are read from original physical state,
including FIXLOC and every payload word. Hardware counter progress is held and
hardware watchdog alarms suppressed during these bounded oracle calls.

| Cases | Coverage |
|---|---|
| 0..59 | Timestamp edge pairs, varied vector/scratch words and VAC contents |
| 60..187 | All 128 full-prefix six-TS skip inputs in both sign families |
| 188..190 | Three four-push histories, retaining all previously saved vectors |
| 191..274 | Remaining timestamp pairs, completing the 12 by 12 word-edge matrix |
| 275..1298 | 1024 direct random vectors/timestamps and full random VAC contents |
| 1299..2322 | 1024 full-prefix random inputs and random VAC contents |

All five VACs at every one of the 38 supported offsets are exercised and
asserted by the oracle. 2323 cases produce 3496 checkpoints: 1168 direct final
states, and 1164 integrated calls with both scaling and PDDL boundaries. Frozen
C64 expectations select 200 complete final states from the first 191 cases;
intermediate scaling boundaries remain in host evidence. Snapshot SHA256:
`33fdd2b7febd6827521fc95b72f27732ab1653786a27b80fd020e567bf594f3c`.

All nine host and AddressSanitizer/UndefinedBehaviorSanitizer suites pass
(LeakSanitizer disabled for this managed process environment). Host rejection
tests assert unchanged objects for invalid mode/base/pointer, including a second
push at capacity. The target also checks an out-of-capacity rejection. Earlier
snapshots, all 138 earlier evidence hashes, the 11 source identities and the
assembled rope remain unchanged.

LOC/BANKSET/ADDRWD/CYR/EDOP/Q and physical bank-control state are outside the
native operation's contract; FIXLOC/PUSHLOC now have explicit native fields.
Original RELINT and machine dispatch are replaced by an explicit call with a
restoring platform I guard, tested with initially enabled and already masked
callers. This is a bounded native adaptation, not a complete interpreter resume
context or scheduling equivalence claim.

## C64 measurements and continuation

The ninth standalone PRG passes 200 complete final-state checks and all three
four-push histories in PAL VICE, with mailbox `a50000c80003ac5624`. It is 46543
file bytes / 46541 loaded bytes at $0801..$bdcd, SYS $080d. Text is 11925 bytes,
read-only data 34604 (34400 are expected words), BSS 172 and noinit one byte.
Image plus BSS/noinit is 46714 bytes, ending at $be7a. It fits stock C64 RAM;
these standalone diagnostics are not combined into one flight image. There are
32 imaginary ZP registers and no extra compiler ZP, heap or floating point.

Four measured PDDL-only intervals take 912, 1028, 1051 and 1033 PAL cycles
(0.926..1.067 ms). The sampled complete prefix through PDDL takes 20790 cycles
(21.101 ms), exceeding one PAL frame while masked in this diagnostic. Marker,
call and VIC bus effects are included, with setup/checks outside the intervals.
These are finite observations, not WCET or a live guidance/source-rate deadline
proof. Hardware-stack extent is 32 bytes below entry (46 with the 14-byte ROM
frame); software-stack extent is 472 bytes, including full-state checker and
unchanged-on-rejection fixtures. Logs, binary/map, dumps and timing JSON are
retained under `evidence/push-*`.

`DSU PIPTIME` is now implemented and independently verified in ELAPSED.md,
including overflow realignment and retained vector/VAC state. The next operation
is `STORE 30D`; this PDDL API retains its original pre-DSU boundary. A mapped job still needs defined producer ownership and scheduling
budgets before integration.
