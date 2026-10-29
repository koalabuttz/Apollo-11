---
id: lm-stateup
title: Translate Luminary FRESH_START_AND_RESTART (bank 13: STATEUP and THISVINT) as interpretive transfer-ABI routines
date: 2026-10-12T18:26Z
lane: A
---
Every order pair of Luminary099 FRESH_START_AND_RESTART in bank 13 (STATEUP, which extrapolates the CSM and LM state vectors for P00, and THISVINT), and R00's CALL INTSTALL; EXIT in bank 04, runs as an interpretive routine of the control-transfer ABI, keyed by LOC at NEWOPS, through the existing compiled interpreter paths. 4,096 cases from all 13 pairs equal the original Luminary099 program in the interrupt-enable state, SUPERBNK and all 2,048 erasable words. That is 11,766 strict and sanitizer checks, with every pair and all eight exits reached, and 256 C64 rows chosen by native branch outcome (`tools/a2_c64_rows.py`: every 16th case plus a greedy cover of the gcov branch arcs of every linked source; the freeze fails unless the rows take all 214 arcs the host cases take) match in VICE. With `lm-fresh-start` and `lm-v37` this covers every instruction of the file but the patch-only STARTSIM. Lane A2 (restart, telemetry and support). See `docs/LM-STATEUP.md`.
