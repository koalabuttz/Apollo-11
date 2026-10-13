---
id: cm-stateup
title: Translate Comanche FRESH_START_AND_RESTART (bank 13: STATEUP and THISVINT; R00's CALL/EXIT) as interpretive transfer-ABI routines
date: 2026-10-12T19:50Z
lane: A
---
Every order pair of Comanche055 FRESH_START_AND_RESTART in bank 13 (STATEUP, which extrapolates the CSM and LM state vectors for P00, STATEND and THISVINT), and R00's CALL INTSTALL; EXIT in bank 04, runs as an interpretive routine of the control-transfer ABI, keyed by LOC at NEWOPS, through the Comanche055 builds of the compiled interpreter paths. 4,096 cases from all 12 pairs equal the original Comanche055 program in the interrupt-enable state, SUPERBNK and all 2,048 erasable words. That is 11,480 strict and sanitizer checks, with every pair and all eight exits reached, and every 17th case matches in VICE. With `cm-fresh-start` and `cm-v37` this covers every instruction of the file but the patch-only STARTSIM. Lane A2 (restart, telemetry and support). See `docs/CM-STATEUP.md`.
