---
id: lm-standard-lead-transfer
title: Run Luminary standard lead-ins through the native trampoline
date: 2026-10-11T23:23Z
lane: A
---
The Luminary099 SYSTEM_TEST_STANDARD_LEAD_INS entries (E/BKCALL, its three returns, E/CALL, the resume point, E/JOBWAK) and the INTPRET/INTPRET+2 and EXIT forms they reach are registered native routines. 3,072 stage cases match the certified stage functions, and 511 E/BKCALL and 384 E/CALL round trips go through the trampoline into a native callee and back (10,241 strict and sanitizer checks). 896 sampled cases match the host digests in VICE. See `docs/LM-STANDARD-LEAD-TRANSFER.md`.
