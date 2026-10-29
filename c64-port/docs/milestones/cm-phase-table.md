---
id: cm-phase-table
title: Translate Comanche PHASE_TABLE_MAINTENANCE as transfer-ABI routines
date: 2026-10-12T16:20Z
lane: A
---
Comanche055 NEWMODEX, NEWMODEA, MMDSPLAY, CHECKMM, PHASCHNG, 2PHSCHNG, SETUPDSP, PHSCHNG2 and CON2 run as routines of the control-transfer ABI, with every caller's inline operands (162 `TC` sites, 229 words, including each PHASCHNG/2PHSCHNG's variable-length tail decoded as the source decodes it) compiled from the source listing. All 4,096 cases, entered from every caller site plus NEWMODEA and MMDSPLAY from random banks, equal the original Comanche055 program in all 2,048 erasable words, the interrupt-enable state and SETUPDSP's NOVAC request (4,673 strict and sanitizer checks, every registered entry reached), and every sixteenth case matches in VICE. Lane A2 (restart, telemetry and support). See `docs/CM-PHASE-TABLE.md`.
