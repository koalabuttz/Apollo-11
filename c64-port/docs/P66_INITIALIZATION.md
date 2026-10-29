# Native P66 basic initialization

STOPRATE clears the nine original DAP rate/error words and restores caller EBANK. FASTCHNG writes caller Q to the phase-three restart-location word at01442, retaining the paired BBCON01443. It does not update the phase/complement words at0756/0757. Both preserve original A/L/Q return debris, SUPERBNK and incoming IRQ state.

All 1,024 complete 17-word source checkpoints agree, covering all EBANK values, 256 returns, physical SUPERBNK words and IRQ states; nine unrelated erasable guards remain unchanged. Snapshot SHA256 is `503953025ca3dae1c7bbbb50b3531c7f4c43881ae7393db4cd0c4b6986eb5857`. A private fixture overlays the byte-identical original DAP file and proves its entire assembled rope equals the retained fixture; retained source and historical files are untouched.

Strict and ASan/UBSan 128-state checks pass. The C64 compares all128 full states, mailbox `a500008011`, PRG10,443 bytes, zero additional zero page and hardware/software stack extents10/10. The complete startup, display and DAP actuation continuations have separate coverage requirements.
