# Original input arithmetic, both missions

This additive native C service translates TPSL1, TPLEFTN/LEFTNCOM,
2ROUND/+2/2RNDEND, and TESTOFUF from the unchanged original
Luminary099 and Comanche055 sources. It operates directly on the canonical
2048-word image. It does not execute or decode instructions at runtime.

`dsky_input_arithmetic` uses `dsky_lamp_io`; `cm_input_arithmetic` uses
`cm_native_io` plus retained channel-event and restart-light words. Producers
must remain excluded throughout the call. Both entry points return the actual
source caller, alarm/recycle entry, or negative-zero fallthrough, as reported
by the route. All channel, IRQ, SUPER and restart metadata are retained.

| Entry | LM logical PC | CM logical PC |
|---|---:|---:|
| TPSL1 | 04404 | 04420 |
| TPLEFTN | 03153 | 03060 |
| LEFTNCOM | 03156 | 03063 |
| 2ROUND | 03163 | 03070 |
| 2ROUND+2 | 03165 | 03072 |
| 2RNDEND | 03173 | 03100 |
| TESTOFUF | 03174 | 03101 |

TPSL1 is in fixed-fixed memory and accepts all 32 aligned retained FBANKs
and all eight legal SUPER latch values. Other entries select physical bank40:
FB060000 and SUPER100/120/140/160. All eight retained EBANKs are supported,
with coherent BBANK, ZERO=+0, and IRQ0/1. A/L may contain raw 16-bit register
values; TP limbs and flag are ordinary 15-bit words with independent signs.
The selected caller is ordinary logical 0100..7777; LEFTNCOM takes it from
SFTEMP2, while other entries take Q. TPLEFTN/LEFTNCOM accept N-1=0..14,
performing one through fifteen original shifts. Unsupported contexts preserve
all image, IO, event, restart and route values atomically.

TPSL1 preserves original DAS/ADS overflow correction and the A/L debris.
An overflow stores +1 or -1 in MPAC+6; a non-overflow retains its prior value.
The shift loop preserves each source CCS decrement and final SFTEMP1,
SFTEMP2, Q and caller effects. 2ROUND preserves both conditional TS skips,
including the second overflow flag store. Direct 2ROUND+2 starts with the raw
incoming A; 2RNDEND preserves it through its source TC Q.

TESTOFUF accepts every ordinary MPAC+6 value. +0 returns to the source caller.
Nonzero positive and negative values reach actual ALMCYCLE (LM04145/CM04161)
with the original diminished magnitude and return address. -0 reaches the
literal fallthrough LM03200/CM03105, retaining Q and setting A=+0. The
neighboring source continuation is an explicit frontier, not a synthesized
return. Normal input-scale producers initialize this flag to +0 and these
arithmetic helpers subsequently store only +/-1 on overflow; the standalone
service nevertheless proves the actual -0 path. ALMCYCLE execution, caller
continuations, data stores and input scaling are separate services.

The independent oracle assembles both original programs freshly and checks
that each rope equals its pinned fixture. It links only the original AGC
reference engine and constructor, never this native service. The native target
links only C word arithmetic and compiled source bodies, never that engine.
The reference engine is used only as a build-time source oracle.

All 8192 cases compare every one of 2048 image words plus SUPER, IRQ, three
channels, write-event mask, restart lamp and route. Corpus covers all entries,
shift counts, retained banks/latches, both zeros, independent TP signs, raw
register/overflow values and flag classes; it samples their combinations rather
than asserting their Cartesian product. Full snapshot SHA256:
`189276a6c2f01efa0d621f28c1516a2ecdb22ca364e12a4c3ebf7196c71d972a`.
Strict host and complete ASan/UBSan streams match the original byte for byte.
Frozen64 whole states plus16 meaningful atomic guards are independently checked
on host. Separate native LM/CM C64 diagnostics run32 frozen whole states each.
They validate actual PAL VICE mailbox, memory layout, no added zero-page
storage and hardware/software stack sentinels. These linked diagnostics are
component checks; production disk-overlay integration is a separate milestone.

Reproduce via `tools/dsky_input_arithmetic.mk`: host-only `all`, `test` and
`sanitize` aliases; explicit `c64-dsky-input-arithmetic`, and
`differential-dsky-input-arithmetic` for fresh source assembly/full comparison.
The frozen packet records exact dependency hashes, source fixture audits,
complete compressed proof streams, source listings, host logs, target PRG/maps
and actual mailbox/stack bytes.
