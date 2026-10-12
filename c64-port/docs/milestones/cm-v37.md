---
id: cm-v37
title: Translate Comanche FRESH_START_AND_RESTART (bank 04 V37/P00, VAC5STOR) as transfer-ABI routines
date: 2026-10-12T20:00Z
lane: A
---
Every instruction of Comanche055 FRESH_START_AND_RESTART in bank 04 runs as routines of the control-transfer ABI. That covers verb 37 with its IMU, R00 and mode-table checks, CANV37, POOH, SEUDOP00, GROUPKIL, the rendezvous restart-group checks, NOUVEAU, V37XEQ, GOP00FIX and INITSUB/RAKE. VAC5STOR in bank 20 is included too. There are 36 entries, including CANV37 (a POSTJUMP from SERVICER207) and the return after every TC out of the file. The file's constants and its PREMM1, FCADRMM1, DNLADMM1 and FLAGTABL tables are compiled from the source listing. 4,096 cases from every entry equal the original Comanche055 program in the interrupt-enable state, SUPERBNK, channels 5, 6, 11, 12, 13, 14 and 33, the trap latches and all 2,048 erasable words. Together the cases execute every instruction of both banks. That is 12,957 strict and sanitizer checks, with all 36 entries and all 29 exits reached. 160 C64 rows, chosen by outcome to cover every word the host cases execute, match in VICE. R00's two interpretive words and bank 13 are `cm-stateup`. Lane A2 (restart, telemetry and support). See `docs/CM-V37.md`.
