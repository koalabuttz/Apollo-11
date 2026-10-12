---
id: lm-restart-select
title: Translate Luminary RESTARTS and run its restart table entries through the trampoline
date: 2026-10-12T02:02Z
lane: A
---
Luminary099 RESTARTS, GETPART2 and PHSPART2 select variable, table and X.1 display restarts natively from RESTART_TABLES words compiled from the source, and run as routines of the control-transfer ABI from RESTARTS to SWRETURN, handing each selected service its 2CADR. All 6,144 selection cases and 4,096 RESTARTS-to-SWRETURN compositions equal the original Luminary099 program in all 2,048 erasable words, covering all 40 table entries (12,703 strict and sanitizer checks), and every sixteenth case matches in VICE. See `docs/LM-RESTART-SELECT.md`.
