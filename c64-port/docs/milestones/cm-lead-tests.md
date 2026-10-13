---
id: cm-lead-tests
title: Comanche055 system-test entry words COMPVER, GTSCPSS1 and REDO in the transfer ABI
date: 2026-10-12T23:55Z
lane: A
---
The three fixed entry words at the start of Comanche055's SYSTEM_TEST_STANDARD_LEAD_INS (COMPVER 33,2000, GTSCPSS1 33,2001 and REDO 33,2002) are now registered in `cm_standard_lead_routines`. RESTART_TABLES, FRESH_START_AND_RESTART and EXTENDED_VERBS reach them by 2CADR or FCADR. Each is one TC: Q is left at the next word (REDO's `MM 07` operand, which NEWMODEX reads) and Z at GCOMPVER, GTSCPSS or NEWMODEX. 384 cases through `agc_transfer_run` match yaAGC on all 2048 erasable words. The 390 strict, sanitizer and C64 checks pass. This addresses the CM STANDARD_LEAD_INS item of #129.
