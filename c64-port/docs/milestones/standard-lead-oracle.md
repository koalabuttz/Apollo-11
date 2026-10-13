---
id: standard-lead-oracle
title: SYSTEM_TEST_STANDARD_LEAD_INS of both missions against yaAGC, with its CALL and EXIT words
date: 2026-10-13T04:03Z
lane: A
---
Every registered entry of SYSTEM_TEST_STANDARD_LEAD_INS in both missions, the interpreter entries they reach, and the file's interpretive CALL and EXIT words now run through `agc_transfer_run` against the original program in yaAGC. The CALL word is a new registered interpretive key (`standard_lead_interp_call_order`), and the EXIT key is registered at its word. 1,408 cases match yaAGC on all 2048 erasable words, the interrupt inhibit and SUPERBNK; 1,344 reach a continuation. The 1,414 strict and sanitizer checks and 475 VICE checks pass, and a source map shows every instruction executed by the oracle (COMPVER, GTSCPSS1 and REDO in cm-lead-tests). This is re-certification evidence for the STANDARD_LEAD_INS items of #129. See `docs/STANDARD-LEAD-ORACLE.md`.
