# Original hours/minutes/seconds input scaling

This additive native service translates HMSIN, RND/TST, MPACTST and SIZETST
from both original Pinball sources. HMSIN composes unchanged ALL3DEC and
DMPSUB arithmetic, executes the original SHORTMP multiply/carry exchanges,
combines the actual hours/minutes/seconds contributions, performs TPAGREE
and writes the real final indexed double-precision destination.

| Entry | LM bank42 PC | CM bank42 PC |
|---|---:|---:|
| HMSIN | 03452 | 03365 |
| RND/TST | 03543 | 03456 |
| MPACTST | 03554 | 03467 |
| SIZETST | 03561 | 03474 |

`dsky_input_time` accepts the canonical image and `dsky_lamp_io`;
`cm_input_time` accepts the canonical image, `cm_native_io`, retained channel
write events and restart light. Callers retain restoring producer exclusion.
The source image alone owns persistent RAM. Entries require FB064000,
SUPER100/120/140/160, aligned retained EBANK0..7 with coherent BBANK, ZERO+0,
IRQ0/1 and restart0/1. Raw A/L overflow encodings and all ordinary independent
MPAC limb signs and either zero are supported. Unused fields, including
MIXBR, DECOUNT, MODE and overflow flag, are retained opaquely.

HMSIN requires NOUNADD logical0000..1776 selecting an ordinary physical
RAM1100..3776 bank-local pair. Register/editor/scratch alias stores are outside
this caller contract. This destination restriction applies even if an early
source alarm prevents the final store. Incoming Q is unused by HMSIN. The
three standalone helpers instead require their ordinary incoming caller
Q0100..7777; NOUNADD is unused and imposes no restriction there. Unsupported
inputs preserve the complete image, IO, events, restart and route atomically.
This source-bounded destination/caller contract is not a whole Pinball
arbitrary-address certificate.

HMSIN checks actual DECBRNCH bits034 through ALL3DEC; failure executes the
real FORCEV25 store of -25 in VERBSAVE and reaches ALMCYCLE. All three DMP
calls use the literal WHOLECON00006/03240 and record the mission-specific
ADDRWD. RND/TST preserves both conditional TS overflow skips, the major-word
carry and MPACTST signed-zero CCS behavior. Both nonzero major signs take the
real ALMCYCLE with exact source Q and diminished-magnitude A. Standalone
SIZETST also preserves a raw overflowing initial TS, including its skipped
CCS instruction; the range comparison accepts both zeros and negative values.

The hours contribution uses HRCON00025/37100 (360000 centiseconds) and the
actual SHORTMP MP/DAS sequence. Minutes and seconds use the original59 and
5999 range constants, with MINCON13560 (6000 centiseconds). HITEMIN0123/0124,
MPTEMP, MPAC limbs, YREGLP/ZREGLP exchanges, source Q/L and every DAS overflow
indicator are retained. Each real overflow reaches ALMCYCLE rather than a
fabricated successful return. The final clear/TPAGREE carry stores preserve
signed results, actual return-in-L and Q2. Both DXCH exchanges preserve the
displaced source/destination words, then actual POSTJUMP reaches LOADLV41,2775,
FB062000 and Q0775. The LOADED route ends there; LOADLV/deferred recall is
separate subsequent source work. The helper CALLER route returns via ordinary
source TCQ/Q3. RECYCLE ends at actual ALMCYCLE04145/04161. These bodies write
no device channels and retain SUPER, IRQ, events and restart light.

The independent oracle freshly assembles both original programs and confirms
each rope against the pinned fixture. It links no tested native routine.
All8192 original LM+CM complete states match strict native C and the full
ASan/UBSan stream: all2048 image words plus SUPER, IRQ, channels11/12/13,
events, restart and route. Snapshot SHA256:
`83f2d8a06a91fe1f3213f624edf0a9cfc9d6ac1d9c6b6ed03a6821373678feaf`.
The corpus covers all four entries, actual completed stores, decimal-bit
failure, hour/minute/second range and contribution overflow, independent
signed limbs and both zeros, raw register encodings, all retained EBANKs and
selecting SUPER forms. It samples combinations, not their Cartesian product.
Frozen64 whole states and24 meaningful atomic guards pass strict host and
sanitizer tests, including unsupported destination aliases/bank edge and
standalone caller rejection.

Actual C64 PAL VICE diagnostics pass32 full source states per mission. LM
mailboxa500002000, PRG11631 bytes/loaded11629; CM mailboxa500002001,
PRG11648 bytes/loaded11646. Both hardware/software stack extents are19/124,
with no added zero-page storage and below-C000 diagnostic image budget.
These linked component checks do not establish application disk-overlay
integration. Exact dependencies, source hashes, fixture audits/listings,
full compressed original/native/sanitizer streams, logs, PRG/maps and raw
mailbox/stack evidence are frozen. `tools/dsky_input_time.mk` supplies host-only
all/test/sanitize aliases and explicit C64/differential/freeze commands.
Final load-verb caller stores, channel output, LOADLV/deferred request lifecycle
and uninterrupted keyboard/application composition remain tracked work.
