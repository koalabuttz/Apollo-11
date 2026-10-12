---
id: lm-v37
title: Translate Luminary FRESH_START_AND_RESTART (bank 04: verb 37 and POOH) as transfer-ABI routines
date: 2026-10-12T18:00Z
lane: A
---
Every instruction of Luminary099 FRESH_START_AND_RESTART in bank 04 (GOP00FIX, verb 37 with the major-mode table search and the servicer wait, R00, POOH, the rendezvous checks, NOUVEAU, V37XEQ and SETUP70/71) runs as routines of the control-transfer ABI. There are 32 entries, including the return after every TC out of the bank, and the bank's CADRs, constants and major-mode tables are compiled from the source listing. 4,096 cases from every entry, running on into bank 05 and the interbank routines, equal the original Luminary099 program in the interrupt-enable state, SUPERBNK, channels 5, 6, 11, 12, 13 and 14, the trap latches and all 2,048 erasable words. That is 8,497 strict and sanitizer checks, with all 32 entries and all 21 exits reached, and every 15th case matches in VICE. STATEUP (bank 13) and R00's interpretive pairs are `lm-stateup`. Lane A2 (restart, telemetry and support). See `docs/LM-V37.md`.
