---
id: lm-dap-interface
title: Translate Luminary DAP interface subroutines as transfer-ABI routines
date: 2026-10-12T14:40Z
lane: F
---
All of Luminary099 DAP_INTERFACE_SUBROUTINES (bank 20, 67 words: RESTORDB, SETMAXDB, SETMINDB, CALLACCS, PFLITEDB, ZATTEROR, STOPRATE, ALLCOAST) runs as twelve native routines of the control-transfer ABI, and CALLACCS hands 1/ACCJOB to the native NOVAC. All 7,168 cases, from every entry to the caller's return or the 1202 alarm, equal the original program in all 2,048 erasable words (strict and ASAN/UBSAN), every assembled word maps to its native owner after a fresh yaYUL assembly, and every sixteenth case (448) passes in VICE. See `docs/LM-DAP-INTERFACE.md`.
