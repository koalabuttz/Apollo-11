# Comanche 055 AX*SR*T

The MIT `cm_axis_rotate` translates original CM basic `AX*SR*T` at `23,3650`, using source SINCDU/COSCDU arrays and VBUF. Both direction words, +3 and -3, rotate through the original three axes and retain source overflow behavior, raw A/L, MODE, all MPAC and VAC words, scratch and return-address debris. The native return occurs before the source saved Q return is executed. Invalid direction or return-address bits reject the call without mutation.

CM `TERM1TMP` aliases BUF2 at `0133/0134`; the original LM helper aliases MPAC+3/+4. Independent original CM execution exposed this difference, so CM has its own translation instead of calling the LM routine. Generic word multiplication and context types are shared after source qualification. No source decoder, floating point, heap or native mission-program dispatch is introduced.

All 32768 cases compare every 131 modeled word plus status, covering both directions, all VAC addresses, signed zeros, corners, randomized values and raw overflow registers. The independent original oracle also verifies physical bank registers, FIXLOC, interpreter-location guards, adjacent erasable guards and unchanged VAC boundaries. Snapshot SHA256 is `7e5179f3fe6fe5ce4b36f06678e78fcb947bf1d7ce94dc3602ba00a9da1c5d68`. Raw values outside the historical below-unity vector guarantee still follow the original arithmetic; this test does not extend that geometric guarantee.

The original POWERED_FLIGHT_SUBROUTINES overlay, its one indentation-only pseudo-label repair and complete-rope reconciliation are audited in `COMANCHE_NUMERICS.md` and `evidence/cm-numeric-*`. The fixture is pinned at Virtual AGC `1b5d6474841a7ab53237ead070f76f35f162f68d`; rope SHA256 is `2ba31de9291cd10fb351a64d261bae8514a1cb75b4651bfa6a135dfa821a2d79`. The GPL oracle is host-only, and historical originals are untouched.

Host and ASan/UBSan pass 128 complete frozen 132-value states, encoded losslessly in 29662 bytes plus 256 index bytes, and all unsupported direction/return words remain unchanged. Stock PAL C64 passes those same 128 states: mailbox `a500008000`, PRG 45659 bytes, text 15467, rodata 30178, BSS 262, image end `$bb60`, hardware/software stack extents 23/570 bytes, no extra zero-page sections. This standalone diagnostic fits below `$c000`; it does not demonstrate whole-application overlay fit or WCET. Application callers must gather and commit views to the sole canonical image and keep producers excluded through the source group.

Reproduce with `make cm-rotation-differential`, `python3 tools/cm_freeze_rotation.py`, `make test-cm-rotation sanitize-cm-rotation c64-cm-rotation`, then `python3 tools/cm_rotation_vice_test.py --vice /path/to/x64sc --rom-dir /path/to/C64 --data-dir /path/to/vice`. Before shared Makefile integration, add `-f Makefile -f tools/cm_rotation.mk`. Differential preparation independently builds the original CM numerical fixture and host hardware-DV variant. Evidence hashes are `evidence/cm-rotation-evidence.sha256`.

The exact target binaries are retained as `evidence/cm-norm.prg`,
`evidence/cm-dp-divide.prg`, `evidence/cm-unit.prg` and
`evidence/cm-rotation.prg`; the complete evidence manifest records their
SHA256 identities alongside the original maps, mailboxes and stack dumps.
