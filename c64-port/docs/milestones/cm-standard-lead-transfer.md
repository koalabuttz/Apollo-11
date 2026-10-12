---
id: cm-standard-lead-transfer
title: Run Comanche standard lead-ins through the native trampoline
date: 2026-10-11T23:23Z
lane: A
---
The Comanche055 SYSTEM_TEST_STANDARD_LEAD_INS entries and the INTPRET forms they reach are registered native routines. 3,072 stage cases match the certified stage functions, and 512 E/BKCALL and 384 E/CALL round trips go through the trampoline into a native callee and back (10,241 strict and sanitizer checks). 896 sampled cases match the host digests in VICE. See `docs/CM-STANDARD-LEAD-TRANSFER.md`.
