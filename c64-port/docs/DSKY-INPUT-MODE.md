# Original operator input mode checks

`dsky_input_mode` and `cm_input_mode` translate ALLDC/OC (both 41,3011),
ALL3DEC (LM 42,3573 / CM 42,3506), and FORCEV25 (LM 42,3603 / CM 42,3516).
These local algorithms were absent from the native operator load path.
They stop at the genuine ordinary caller or original ALMCYCLE entry.
No scheduler poll or alarm recovery is substituted.

ALLDC/OC preserves TS DECOUNT, the three actual SR edits, both CS reads,
CCS and the exact one's-complement mode comparison. It accepts raw A,
including TS overflow skipping CS DECBRNCH. The normal PUTXYZ/PUTXY
callers supply minus six/minus five. ALL3DEC checks all three decimal
component bits; failure executes FORCEV25, storing the original negative
verb 25 into VERBSAVE before ALMCYCLE. Both signed zeros and all ordinary
DECBRNCH values are modeled.

The caller owns a canonical source image and excludes producers. Ordinary
Q 0100..7777, raw A/L, all EBANKs, selecting SUPER 100/120/140/160 and IRQ
0/1 are supported. Source bank is 41 for ALLDC/OC or 42 for ALL3DEC and
FORCEV25. Executive words and all channel/event/restart state are unused
and opaque. Rejection leaves the full image, IO, event mask, restart
flip-flop, and route unchanged. The native body executes compiled source
control, with no AGC opcode decoder or rope instruction fetch.

The independent unchanged-original oracle compares 8,192 complete states:
all 2,048 source words, SUPER/IRQ, channels 11/12/13, event mask, restart
flip-flop, and caller/recycle route (2,056 fields). This corpus samples
ordinary DECBRNCH values and explicitly covers decimal bit combinations,
both zeros/extremes, raw-A sign/overflow cases, actual minus-six/minus-five
callers, retained EB/SUPER/IRQ and raw L. It is not an exhaustive enumeration
of every 16-bit input product. Native and full ASan/UBSan outputs match
original byte for byte, SHA-256:

`7f130db099ff15cea4b3138ee39e2bcfa09e79763fb9910995d80b39d83ee0f2`.

Frozen 64 states and 16 atomic rejection cases pass host/sanitizer. Actual
stock-ROM PAL C64 runs compare 32 full states per mission: LM
`a500002000`, PRG 5,099 bytes (5,097 loaded), stack extents 10/19; CM
`a500002001`, PRG 5,096 bytes (5,094 loaded), stack extents 10/19. Both
stay below C000 and allocate no extra zero page. Fresh source fixtures
assemble to byte-identical pinned ropes. The isolated make fragment is
`tools/dsky_input_mode.mk` with host-only `all`, host/test/sanitizer,
differential and target builds.

This closes these three local algorithms under their stated caller
contract. Actual load-verb request and store loops, noun/component checks,
each selected input scale, ALMCYCLE composition, and canonical caller/job
adoption remain follow-on work. This packet is not a whole Pinball or
operator-loading certificate.
