---
id: cm-downtelemetry
title: Translate Comanche DOWN-TELEMETRY_PROGRAM as transfer-ABI routines
date: 2026-10-12T17:10Z
lane: A
---
Comanche055 DODOWNTM, the DNTMGOTO continuation it executes from erasable memory, and the downlist and erasable-dump phases (DNPHASE1/2, DNDUMPI, DNDUMP, DNDUMP1 and SENDID's return) run as routines of the control-transfer ABI, from the DOWNRUPT lead-in state to TCF RESUME. The five downlists, their sublists and snapshots and the program's constants are compiled from the source listing. 48 sequences of 18,500 downrupts in all (every downlist sent past its end and restarted; 1, 2 and 4 complete erasable dumps returning to the downlist) equal the original Comanche055 program in channels 13, 34 and 35 and Q after every downrupt, the channel-13 traps and all 2,048 erasable words (18,815 strict and sanitizer checks, every control word of all five downlists sent), and every second sequence matches in VICE. Lane A2 (restart, telemetry and support). See `docs/CM-DOWNTELEMETRY.md`.
