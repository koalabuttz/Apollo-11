# Canonical LM admission with raw nonnegative priorities

`lm_admission_raw` translates NOVAC, FINDVAC, SPVAC and SPVACIN to the actual caller return, or the literal BAILOUT1 entry. The frozen `lm_admission_image` and typed Executive admission service remain unchanged.

The additive domain accepts source priorities from +0 through 037000, including unused low bits. Source AD still adds the fake work-area pointer or the allocated VAC pointer. These inputs keep that addition below positive overflow. A +0 incoming priority consequently admits the actual positive packed value; it is not changed to a chosen default priority.

NOVAC/FINDVAC read an erasable inline descriptor from the sole canonical image at Q, including the real TCMPAC Q=0155 caller. Banked erasable addresses use the incoming EBANK. Fixed-memory inline descriptors remain explicit compiled source metadata supplied by the caller. SPVAC/SPVACIN use the actual incoming A/L pair. A free core zero with a genuinely positive pending request follows the original SPECTEST/SETLOC route and compares packed priorities; the request is not cleared to force the idle path.

The source proof compares 16,384 complete 2048-word images plus SUPER and interrupt state. It covers all four entries, success, both resource bailouts, +0/low-bit priorities, Q=0155 and Q=0705, free and occupied cores/VACs, and pending admission into core zero. The native and ASAN/UBSAN streams match the unchanged original assembly byte for byte. A further 16,384 historical inputs produce exactly the same results as the frozen wrapper. Invalid-domain tests check that all canonical words and I/O remain unchanged.

The stock PAL C64 ROM/D81 proof checks 64 frozen original states using 32 input/checker modules. Its mailbox is `a50000400002`; the measured software stack is 752 bytes, hardware stack 30 bytes and additional zero page zero. Every fixture module returns before the next disk load. The canonical image remains at $7000 and the native producer guard restores incoming I/$01.

The domain still requires lawful VAC-use markers, nonzero core priorities and legal NEWJOB offsets. Core-zero-free/NEWJOB=+0 reaches the source CCSHOLE route and is outside this body. Negative priorities, TS/ADS overflow paths and corrupt resource markers are also outside the qualification. Admitted job bodies and BAILOUT1 recovery remain explicit source continuations. This packet does not certify the whole Executive file.

Fresh-checkout targets are `lm-admission-raw-port-test` and `lm-admission-raw-port-sanitize` in `tools/lm_admission_raw_port_integration.mk`. They use retained original states and private build directories; no raw build stream or target rebuild is required.
