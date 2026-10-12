---
id: lanec-ev-dap
title: Translate the V48 DAP data load (LM R03, CM DAPDISP)
date: 2026-10-12T15:40Z
lane: C
---
Native C for the EXTENDED_VERBS V48 DAP data load: Luminary099 R03 (DAPDATA1 in bank 20, STIKLOAD through TRIMDONE in bank 01) and Comanche055 DAPDISP with DAPDISP1 (banks 43 and 42), translated block by block. 2,832 complete states are identical to the unchanged original program in Virtual AGC, in strict and ASan/UBSan builds, and 258 of them (every entry) match the oracle on a VICE PAL C64. See `docs/LANEC-EV-DAP.md`.
