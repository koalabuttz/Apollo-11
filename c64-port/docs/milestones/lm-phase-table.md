---
id: lm-phase-table
title: Translate Luminary PHASE_TABLE_MAINTENANCE as transfer-ABI routines
date: 2026-10-12T16:45Z
lane: A
---
Luminary099 NEWMODEX, NEWMODEA, MMDSPLAY, CHECKMM, PHASCHNG, 2PHSCHNG, PHSCHNGA, SETUPDSP, PHSCHNG2 and CON2 run as routines of the control-transfer ABI, with every caller's inline operands (142 `TC` sites, 181 words, each PHASCHNG/2PHSCHNG tail decoded as the source decodes it) compiled from the source listing. All 4,096 cases, entered from every caller site (the PHSCHNGA site with each phase word its producer computes) plus NEWMODEA and MMDSPLAY from random banks, equal the original Luminary099 program in all 2,048 erasable words, the interrupt-enable state and SETUPDSP's NOVAC request (4,586 strict and sanitizer checks, every registered entry reached), and every sixteenth case matches in VICE. Lane A2 (restart, telemetry and support). See `docs/LM-PHASE-TABLE.md`.
