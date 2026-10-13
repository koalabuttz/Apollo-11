---
id: interbank-oracle
title: INTER-BANK_COMMUNICATION of both missions against yaAGC
date: 2026-10-13T04:03Z
lane: A
---
Every genuine inline-CADR call site (448 LM, 354 CM) and every one of the 14 registered entries of INTER-BANK_COMMUNICATION in each mission now runs through `agc_transfer_run` against the original program in yaAGC. 2,146 cases match on all 2048 erasable words, the interrupt inhibit and SUPERBNK; 2,145 reach a continuation. The 2,150 strict and sanitizer checks and 723 VICE checks pass, and a source map shows the oracle executed every instruction of the file in both missions. This is re-certification evidence for the INTER-BANK items of #129. See `docs/INTERBANK-ORACLE.md`.
