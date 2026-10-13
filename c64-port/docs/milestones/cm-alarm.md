---
id: cm-alarm
title: Translate Comanche ALARM_AND_ABORT as transfer-ABI routines
date: 2026-10-13T21:30Z
lane: A
---
Every instruction of Comanche055 ALARM_AND_ABORT runs as routines of the control-transfer ABI: ALARM through VARALARM and BAILOUT/WHIMPER in fixed-fixed memory, PRIOLARM in bank 10, the points control comes back to (MULTEXIT targets, BANKCALL and DOWNFLAG returns) and the erasable ALMCADR continuation. The flight image reached ALARM 02,3650 with no routine table before. The alarm codes after all 62 genuine caller TCs, the file's own BANKCALL/POSTJUMP CADRs and its constants are compiled from the source listing. 4,096 cases from every caller site and entry, with each FAILREG state, equal the original Comanche055 program in the interrupt-enable, SUPERBNK and in-interrupt states and all 2,048 erasable words (all 21 entries reached), and every 16th case, which takes every host branch arc, matches in VICE. See `docs/CM-ALARM.md`.
