# Native LM and CM matrix multiplication

`matrix_native_phase` translates original INTERPRETER MXV and VXM for
Luminary099 and Comanche055. It accepts an ordinary row-order, 18-word
DP matrix and the canonical erasable image. MXV selects rows; VXM selects
columns. Each unrounded three-component dot product follows DOTSUB,
DMPSUB and OVERFLOW, retaining both word signs, signed zeros, the third
product word and actual accumulator overflow debris. The private direct
DOTSUB path retains vector MODE and the pre-DANZIG accumulator; the
existing public DOT kernel remains unchanged.

Four explicit compiled phases cover the prologue and each dot/exchange
group. They use source return addresses and preserve the complete CPU,
MPAC, BUF, BUF2, VBUF, ADDRWD, MATINC, DOTINC, DOTRET, MPTEMP and OVFIND
state. The final phase reaches the entry of DANZIG. It does not execute its
scheduler poll or advance the source instruction stream. No AGC instruction
fetch, decoder, emulated CPU, heap or floating point exists in the native
kernel.

| Phase | CM final PC | LM final PC | Final Q |
| --- | --- | --- | --- |
| 0, prologue | 07313 | 07344 | 3 |
| 1, first dot and exchanges | 07330 | 07361 | 0140 |
| 2, second dot and exchanges | 07341 | 07372 | 0140 |
| 3, final dot and exchanges | 06030 | 06061 | 0140 |

The supported domain is source vector MODE 077776, legal aligned banks,
ordinary caller RAM and a logical matrix base 01000 through 07757. Raw
16-bit A/L/Q patterns are accepted. The caller supplies the actual selected
ordinary matrix, keeps it stable during a phase and outside overwritten
scratch, and holds producer exclusion. Phased ADDRWD and MATINC/DOTINC
must match the selected source stage. External SUPER, IRQ and device state
remain unchanged. Unsupported contexts preserve the entire image and matrix.
Matrix operand loading and subsequent DANZIG dispatch belong to separate
compiled caller stages.

The independent original assembly oracle compares every 2,048 erasable
word plus SUPER and IRQ at all four phases. For each mission it runs 4,096
inputs in both directions: 32,768 complete states, 65,536 total. Inputs cover
all EB/FB and SUPER/IRQ combinations, raw accumulator/register patterns,
positive and negative zero, maximum products, mixed-sign DP words and
random matrices/vectors with independent per-case numerical seeds. Strict
C and the complete ASAN/UBSAN streams match original assembly exactly:

- CM: `479604a96cff10daf5b0ba5febfd0c9c1b6bf2abb88b9eecd250e041a89daaa2`
- LM: `9f5b3f2e23199eb38e9f1a6e708f60eb92ec62617dcd83133132d7b86c120bd6`

The fixtures retain unchanged source arithmetic; the differential runner
checks normalized source identity for all arithmetic and matrix bodies.
The CM original-declaration rope is
`6e3150ae9b43535237e10f565205d1f65e6d60cafbe8935be598e03e9810b061`.
The GPL host engine serves only as an independent oracle. Native arithmetic
is MIT C using the existing integer word/product helpers.

The frozen regression corpus contains 128 complete source checkpoints,
64 per mission, and 66 unchanged invalid-domain checks. Strict and sanitized
host tests pass. The actual PAL C64 VICE diagnostic passes every frozen
state, mailbox `a500008000`, PRG 24,364 bytes. Measured stack extent is
29 hardware bytes (excluding 14 ROM bytes) and 287 software bytes; no
additional zero-page allocation occurs. This establishes the bounded matrix
kernel, not whole INTERPRETER coverage or complete DYNDISP execution.

Reproduce with `tools/matrix_differential.py`, `tools/freeze_matrix.py`,
`tools/matrix.mk` and `tools/matrix_vice_test.py`. Compressed source streams,
source PC traces, hashes and target artifacts are enumerated in
`evidence/matrix-proof.json`. Historical exploratory correlated-input
artifacts are excluded from the qualified packet.
