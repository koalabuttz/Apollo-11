# Shared resident scheduler body

`exec_native_step` is an explicitly outlined C ABI function. Arithmetic and
basic bindings call one copy of its lifecycle code under LLVM-MOS LTO. The
source algorithms, guards, return actions and canonical source image are
unchanged. Global inlining remains enabled for the surrounding code.

The qualified live-guidance resident now occupies 25,398 text bytes, 759
read-only data bytes, 26 initialized data bytes and 23 BSS bytes, ending at
`$6e6b`. This leaves 405 bytes before the canonical source image at `$7000`.
The preceding milestone remains retained under its original artifact names.

The new PRG and D81 pass the complete stock PAL C64 disk-loader proof: 192
independent 493-word source fingerprints, 27 actual arithmetic overlays, two
frozen inputs, both SUPER forms, real two-P66 preemption, interrupts enabled
and masked, and restoration of I and `$01`. Mailbox is `a50000c00002`; measured
software stack extent remains 736 of 2,048 bytes. Strict and ASAN/UBSAN mixed
runtime, live guidance and pending-request comparisons retain their existing
source results. Hashes and sizes are in
`evidence/p66-resident-outlined-proof.json`.

Reproduce with `tools/p66_outlined_build.py --cc ... --c1541 ...`, after the
normal guidance module header is built, then `tools/p66_outlined_vice_test.py`
with VICE, ROM and data-directory paths. The build replaces only the resident
program in a copy of the retained D81; arithmetic and proof modules stay
identical. This sizing result applies to the qualified prefix through AFC,
before downstream feedback, display and task additions.
