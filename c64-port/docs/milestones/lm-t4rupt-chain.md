---
id: lm-t4rupt-chain
title: Run whole LM T4RUPT interrupts through the trampoline
date: 2026-10-13T00:15Z
lane: A
---
Whole Luminary099 T4RUPT interrupts run through `agc_transfer_run` over the three routine tables of T4RUPT_PROGRAM.agc, from T4RUPT to the first continuation outside the file. All 4,096 interrupts equal the original program in yaAGC in all 2,048 erasable words, SUPERBNK, the interrupt-enable bit, InIsr and channels 10, 11, 12, 14, 30 and 33, and 265 of them, which run every branch outcome the host interrupts run, match in VICE. The whole-file source map shows every instruction of the file executed by a native routine's oracle run. See `docs/LM-T4RUPT-CHAIN.md`.
