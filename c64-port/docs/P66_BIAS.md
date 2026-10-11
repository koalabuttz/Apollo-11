# Canonical P66 bias initialization

`p66_bias_start` executes STRTP66A through VRTSTART on the sole authoritative 2,048-word image. It constructs the bias vector, runs native VXSC, writes VBIAS/TEMX and source flags, clears old PIPA vectors and acceleration history, and retains all original scratch, register and bank aliases. No opcode decoder is used.

All1,024 original source-entry comparisons match every2,048 physical erasable words plus SUPERBNK and IRQ. Snapshot SHA256 is `fc4994a1de243a04382aadb76ed17784c0d60847a9d7de4c9e7c68c6154c2c4f`. A private source fixture proves byte-identical whole-rope assembly after restoring the original flag file. The caller owns a running core/VAC, excludes producers, has NEWJOB0 and source bank31/EB7; unsupported ownership rejects unchanged.

Strict and ASan/UBSan64 whole-image cases pass. The C64 compares all64 full2,050-word states, mailbox `a500004002`, PRG22,247bytes, text9,770, rodata12,463, BSS8,197, static78bytes, additional zero page0, hardware/software stack19/154bytes. Frozen expectations are lossless changed-word lists, and every erasable word is checked. VRTSTART and display/startup continuations are separate modules.
