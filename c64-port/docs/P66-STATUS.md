# Full-width P66 callback status

`p66_native_binding.status` now stores the complete `uint16_t` result returned
by a compiled arithmetic entry. LLVM-MOS represents the small native-status
enum in one byte; storing AGC alarm `01302` in that enum lost its high byte.
The numerical entry, canonical source image and scheduling behavior are
unchanged. Internal UNIT-terminal results remain normalized by the invoker.

The dedicated transport check returns `01302`, `01202`, `0x8003`, `0xffff` and
native status 4 from a guarded callback. It verifies complete status retention,
committed MPAC and source PC/Q, no successful observation or context exchange,
and no second callback after the fault. This fixture tests resident transport;
it does not claim new numerical or flight-domain coverage.

Strict host and ASAN/UBSAN tests pass. A stock PAL C64 runs all five cases with
interrupts enabled and masked, restores caller I and `$01`, and reports mailbox
`a500000a02`: ten checks, zero failures, two-byte status. Its measured software
stack extent is 99 bytes; hardware stack extent is 30 bytes. Evidence is in
`evidence/p66-status-proof.json` and the matching retained PRG/map/mailbox.

The existing 160 prefix fingerprints, 2,944 live guidance checkpoints, 400
two-job checkpoints and 704 pending-request checkpoints retain their original
results and snapshot hashes. The published live-guidance PRG/D81 identities
remain retained as historical artifacts of that milestone.

Reproduce with `make -f Makefile -f tools/p66_status.mk test-p66-status
sanitize-p66-status c64-p66-status`, supplying `MOS_CC` for the target compiler;
then run `tools/p66_status_vice_test.py` with VICE, ROM and data-directory paths.
