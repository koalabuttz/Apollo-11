---
id: lm-update
title: Translate Luminary UPDATE_PROGRAM as transfer-ABI routines
date: 2026-10-12T17:15Z
lane: A
---
Every instruction of Luminary099 UPDATE_PROGRAM (P27: the V70-V73 uplink verbs, data collection and verification, restart-protected stores into erasable memory and the TIME2/TIME1 adjustment) runs as routines of the control-transfer ABI. There are 35 entries: the verbs, the POSTJUMP targets, the job, its post-EXIT code, two restart points and the return after every TC out of the file. The file's own BANKCALL/POSTJUMP CADRs and constants are compiled from the source listing. 4,096 cases from every entry equal the original Luminary099 program in the interrupt-enable state, SUPERBNK, channel 11 and all 2,048 erasable words (12,399 strict and sanitizer checks, all 35 entries and all 13 external exits reached), and every 16th case matches in VICE. Lane A2 (restart, telemetry and support). See `docs/LM-UPDATE.md`.
