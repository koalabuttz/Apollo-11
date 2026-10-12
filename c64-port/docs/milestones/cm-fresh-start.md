---
id: cm-fresh-start
title: Translate Comanche FRESH_START_AND_RESTART (bank 05) as transfer-ABI routines
date: 2026-10-12T19:10Z
lane: A
---
Every instruction of Comanche055 FRESH_START_AND_RESTART in bank 05 and fixed-fixed (V36 fresh start, hardware and software restart with the phase-table check and group restarts, the optics, coarse-alignment and TVC restarts, ENEMA, the KLEAN routines, the STARTSUB family, T5IDLOC and GOTOPOOH) runs as routines of the control-transfer ABI. There are 17 entries, including the return after every TC to code outside the file, and the file's constants and T5IDLER table are compiled from the source listing. 4,096 cases from every entry equal the original Comanche055 program in the interrupt-enable state, SUPERBNK, channels 5, 6, 11, 12, 13, 14 and 33, the trap latches and all 2,048 erasable words. That is 8,963 strict and sanitizer checks, with all 17 entries and all 13 kinds of exit reached; together the cases execute every instruction but STARTSIM, and 257 outcome-chosen rows match in VICE. V37/POOH (bank 04), VAC5STOR (bank 20) and bank 13 are later milestones. Lane A2 (restart, telemetry and support). See `docs/CM-FRESH-START.md`.
