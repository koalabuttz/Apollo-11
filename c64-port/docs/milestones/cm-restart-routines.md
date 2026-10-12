---
id: cm-restart-routines
title: Translate Comanche RESTARTS, RESTART_TABLES and NEWPHASE as transfer-ABI routines
date: 2026-10-12T16:00Z
lane: A
---
Comanche055 RESTARTS, GETPART2 and PHSPART2 select variable, table and X.1 display restarts natively from RESTART_TABLES words compiled from the source, and run as routines of the control-transfer ABI from RESTARTS to SWRETURN, handing each selected service its 2CADR. NEWPHASE, the executable part of RESTART_TABLES, is a routine entered from its nine `TC NEWPHASE` sites with its group operand compiled from the source. All 7,168 selection cases (including targeted signed-zero and overflow cases), 5,120 RESTARTS-to-SWRETURN compositions and 1,024 NEWPHASE calls equal the original Comanche055 program in all 2,048 erasable words, covering all 79 table entries and all nine NEWPHASE sites (15,907 strict and sanitizer checks), and every sixteenth case matches in VICE. Lane A2 (restart, telemetry and support). See `docs/CM-RESTART-ROUTINES.md`.
