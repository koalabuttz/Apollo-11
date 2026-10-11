# P66 feedback overlays

`guidance_throttle_phased.h` exposes six native phases without an instruction
interpreter or scheduler call. The owner must retain the same VAC and exclude
all producers across the sequence. Ordinary 15-bit inputs and source DP/vector
MODE are required. The canonical driver owns pending interpreter control.

| Phase | Completed LOC | Pending EDOP | PUSH offset |
|---|---|---|---|
| vector | 03436 | 0 | 0 |
| length | 03437 | 00052 (PDDL) | 0 |
| elapsed | 03446 | 0 | 0 |
| correction | 03464 | 0 | 2 |
| limits | 03472 | 0 | 6 |
| clamp | 03507 | 0 | 0 |

All 98,304 checkpoints (16,384 inputs, six checkpoints) match all 124 modeled
raw words and clamp paths from original assembly. SHA256:
`2681746eff8644b4b4ff08531cc7cc9d4d1eddd11d8cc943ce30d98a7c76bd76`.
The host oracle uses the pinned hardware-DV branch; this qualification does not
change native arithmetic. Strict checks cover 192 original phase entry/exit
states and 196,602 invalid MODE states, preserving every owned object. Full
source states pass ASAN/UBSAN. Each of six separate C64 images passes its 32
original complete states, including input 8.

The isolated final clamp originally returned MAXIMUM rather than NORMAL for
input 8 on the C64: its branch argument was {-0,31370,+0}, whose first nonzero
limb is negative. The host returned NORMAL. Replacing the signed integer
conversion in the branch predicate with a direct ordinary-word sign test
(skip both zero encodings, then inspect AGC_SIGN) preserves source semantics
and fixes the observed target result. Equivalent correction was applied to the
monolithic helper. The existing monolithic frozen corpus already included
input 8 and passed before and after the correction; the discrepancy was
observed in the new isolated compilation context. This evidence does not by
itself establish a general compiler defect.

The original split source, C64 PRG and complete mismatch dump are retained in
`build/throttle-phased-original-sign.c`, `build/feedback-original-sign.prg`,
and `build/feedback-original-sign-mismatches.bin`. Corrected target mailbox is
`a500002005`; original target mailbox was `a504002005`. No prior published
numerical corpus changed.

`guidance_control_observer.h` independently accounts for actual TC return-Q
debris at four acceleration boundaries. It matches 65,536 original Q states
(SHA256 `217608c6f09909f94b43efa34d7f1c4eb63f743176680cb0b2d00c8c3ed9ba99`),
all sanitizer states and 65 original C64 fixtures. It preserves no hidden
execution state. Source root unnormalization uses Q02314 when ADDRWD is zero
and MPTEMP is not -0; the all-zero root instead leaves Q3.
