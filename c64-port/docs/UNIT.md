# Absolute value, vector length and half-unit vector

`guidance_unit.h` translates ABS, ABVAL and UNIT from Luminary 099's unchanged
INTERPRETER. ABS requires double-precision MODE; the vector operations require
vector MODE and an owned VAC. Unsupported mode or VAC inputs return without
mutation. Callers exclude producers while operating on the shared scratch.

ABVAL computes the raw squared length at VAC words 34/35 and returns the length.
UNIT retains that square and the length at words 36/37, returning the historical
half-unit vector. The implementation preserves both zeros, normalization shifts,
source DV overflow paths, and A/L and scratch effects. A negative square-root
boundary reports alarm 1302 before SQRTABRT; alarm execution remains separate.
UNIT overflow returns through the source interpreter path, so a composed caller
must continue rather than invent an abort. Norm BUF-third aliases DOT scratch
when bound to original erasable storage.

The original assembly oracle checks 50,736 complete 104-word states: 16,912 each
ABS, ABVAL and UNIT. UNIT has 14,044 successful and 2,868 overflow outcomes.
The corpus covers normalization branches, mixed signs, both zeros, and controlled
DVSIGN/OVFIND inputs. Sixteen regressions came from composing UNIT into P66:
a nonzero component can shift to zero before GENDDV, which requires preserving
its original signed-zero and L debris rather than using a generic zero shortcut.
The independently composed radial group now agrees at all 3,640 checkpoints.

Division uses the pinned hardware-DV oracle described in GENERAL_DIVIDE.md,
not the pinned engine's fast DV path. The complete standalone snapshot SHA256 is
`fdeebfdb5b7d2d1aeb707df1beb1bc5a538a2f7af1ef2ff0bf865db3e7f32e28`.
Strict host and ASan/UBSan verification passes, including unchanged-state
rejection checks. PAL C64 execution passes 118 losslessly frozen complete states;
mailbox `a500007600`. The PRG is 44,655 bytes; modeled state ends at $b740,
with no extra zero page. Measured dynamic hardware/software stack extents are
30/293 bytes; the hardware observation excludes the 14-byte ROM prefix.
These are finite diagnostic observations, not a complete application or WCET proof.

`tools/unit.mk` supplies `test-unit`, `sanitize-unit`, and `c64-unit`.
`tools/unit_differential.py` reproduces all original-assembly comparisons;
`tools/unit_vice_test.py` runs the target with the ROM and VICE data paths used
by the other diagnostics. Retained evidence is indexed in evidence/SHA256.json.
