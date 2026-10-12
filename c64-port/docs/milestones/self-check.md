---
id: self-check
title: Translate AGC_BLOCK_TWO_SELF-CHECK (Luminary099 and Comanche055, bank 43) as transfer-ABI routines
date: 2026-10-12T20:30Z
lane: A
---
Every instruction of AGC_BLOCK_TWO_SELF-CHECK in both missions but FXADRS runs as a native routine of the control-transfer ABI, from one shared body (`src/self_check_body.inc`) that each mission builds with its own addresses and constants. That includes the self-check idle loop SMODECHK with its options, -1CHK, ERRORS/PRERRORS with the ERESTORE repair, ERASCHK, CNTRCHK, CYCLSHFT, and ROPECHK/STSHOSUM with ADSUM, ADRSCHK, NXTBNK and BNKCHK. 4,096 cases per mission from all 11 entries equal the original Luminary099 and Comanche055 programs in the interrupt-enable state, SUPERBNK and all 2,048 erasable words. Every instruction of the file is executed but FXADRS and two +0 branch outcomes no state can reach. The C64 rows (77 per mission, chosen by outcome) match in VICE. FXADRS (ROPECHK's read of the fixed-fixed rope) is not translated; a run stops there. Lane A2 (restart, telemetry and support). See `docs/SELF-CHECK.md`.
