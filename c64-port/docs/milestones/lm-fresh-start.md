---
id: lm-fresh-start
title: Translate Luminary FRESH_START_AND_RESTART (bank 05) as transfer-ABI routines
date: 2026-10-12T17:44Z
lane: A
---
Every instruction of Luminary099 FRESH_START_AND_RESTART in bank 05 and fixed-fixed (V36 fresh start, hardware and software restart with the phase-table check and group restarts, ENEMA, the KLEAN routines, the STARTSUB family and GOTOPOOH) runs as routines of the control-transfer ABI. There are 11 entries, including the returns after MMDSPLAY, SWCALL and ALARM, and the file's own CADRs and constants are compiled from the source listing. 4,096 cases from every entry equal the original Luminary099 program in the interrupt-enable state, SUPERBNK, channels 5, 6, 11, 12, 13 and 14, the trap latches and all 2,048 erasable words. That is 8,879 strict and sanitizer checks, with all 11 entries and all seven kinds of exit reached, and every 16th case matches in VICE. The V37/POOH code in banks 04 and 13 is the next milestone. Lane A2 (restart, telemetry and support). See `docs/LM-FRESH-START.md`.
