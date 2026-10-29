# Canonical antenna vector left-shift continuation

`sband_shift_left()` executes the original LM/CM SHORTV left branch and VSSL loop for counts 1 through 8. The antenna uses counts 1 and 2. The entry is PC 02121, fixed bank 0, vector MODE -1, and CYR 020000 + count - 1. Ordinary 15-bit image words/Q, raw 16-bit A/L, coherent EB/BB, and ZERO +0 are required. The caller owns the canonical 2048-word image and excludes producers across the continuation. Invalid inputs preserve the complete image.

The adapter reuses frozen `p66_vector_shift_left` arithmetic, gathers and commits original MPAC/scratch words from the sole authoritative image, applies the original CYR edit, and retains Q unless a genuine overflow-helper call returns through TC Q, leaving Q 3. It stops before the actual mission DANZIG entry. External SUPER/IRQ stay untouched; no instruction decoder or scheduler executes.

The fresh original-source oracle links input constructors and GPL engine code only. Native strict and ASAN/UBSAN streams each match all 524,288 complete original 2048-word states plus SUPER/IRQ/status. Each mission/count group sweeps every ordinary MPAC component and Q marginal; raw A/L, banks, scratch, and latch combinations vary. This is not an exhaustive Cartesian context or instruction-edge claim. Exact source words and unchanged original rope identities are retained in the source map. Right shifts and generic shift processing are outside this adapter.

The stock-ROM PAL C64 diagnostic compares every original word individually for 384 frozen states. It records mailbox `a5800100`, PRG 19,978 bytes, loaded end $5609, and BSS end $7609. Standalone diagnostic memory includes two complete images and fixtures and does not establish full-application residency. Unique integration targets are `test-sband-shift` and `sanitize-sband-shift`.

Both containing antenna files and both INTERPRETER files remain partial. This dependency completes a native vector-shift boundary needed by the antenna frame transformation; it does not certify an entire antenna algorithm.
