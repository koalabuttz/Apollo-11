---
id: lm-alarm
title: Translate Luminary ALARM_AND_ABORT as transfer-ABI routines
date: 2026-10-12T18:30Z
lane: A
---
Every instruction of Luminary099 ALARM_AND_ABORT runs as routines of the control-transfer ABI: ALARM through VARALARM in fixed-fixed memory, PRIOLARM in bank 10, FLAGS in bank 13, the points control comes back to (MULTEXIT targets, BANKCALL and SUPERSW returns) and the erasable ALMCADR continuation. The alarm codes after all 73 genuine caller TCs, the file's own BANKCALL/POSTJUMP CADRs and its constants are compiled from the source listing. 4,096 cases from every caller site and entry, with each FAILREG state, equal the original Luminary099 program in the interrupt-enable, SUPERBNK and in-interrupt states and all 2,048 erasable words (9,049 strict and sanitizer checks, all 26 entries reached), and every 16th case matches in VICE. Lane A2 (restart, telemetry and support). See `docs/LM-ALARM.md`.
