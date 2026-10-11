# P66 desired radial acceleration and gravity

`p66_acceleration` translates BDSU VDGVERT through DDV TAUROD, PDVL GDT/2,
ABVAL, DDV GSCALE, SR2, STORE20D and the implicit-pop DAD. It stops at
DANZIG LOC 03422 with EDOP zero before PDVL UNITX and the CDU rotation call.
The VAC retains desired acceleration at 0/1 and gravity acceleration at 20/21;
MPAC holds their sum. GSCALE is the source fixed pair {01440,0}. TAUROD,
desired vertical velocity and the gravity vector are stable raw source inputs.

The caller supplies ordinary words, excludes producers, and owns the legal VAC
with PUSHLOC at its base. Unsupported MODE/base/pointer rejects unchanged.
The kernel retains signed zeros, source division overflow continuation, scratch,
and A/L effects. A square-root failure reports the source 1302 boundary before
alarm dispatch; it does not invent an Executive transition.

All 16,384 complete 112-word original-assembly states agree, using the pinned
hardware-DV oracle qualification documented in GENERAL_DIVIDE.md. Snapshot SHA:
`185c0485b8277ffede8d4e5b7c3b7fe9fa9e1168d03c8028aa8a5559ba8d4627`.
The independently composed live prefix through acceleration also agrees for
2,467 inputs and 7,428 complete 136-word checkpoints; SHA:
`34df7eea4c6ec3fb768bda8e779792b0fe7e5abda02718608e86c127fe65a9dc`.

Strict host and all-state ASan/UBSan checks pass. PAL C64 execution checks
96 complete original stage states; mailbox `a500006000`. The PRG is 42,502
bytes, RAM ends at $aee7, and extra compiler zero page is zero. Measured hardware
and software stack extents are 30/301 bytes; the hardware observation excludes
the 14-byte ROM prefix. These measurements qualify a finite stage diagnostic.
The target loads source stage inputs; full-prefix and full application C64
execution still require the ongoing overlay and continuation integration.

`tools/acceleration.mk` provides the host, sanitizer and C64 targets.
`tools/acceleration_differential.py` compares the stage, and
`tools/acceleration_prefix_differential.py` compares the composed prefix.
The lossless snapshots, maps, target mailboxes and logs are retained in evidence.
