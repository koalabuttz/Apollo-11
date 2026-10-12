---
id: cm-interbank-transfer
title: Run Comanche INTER-BANK entries as native routines
date: 2026-10-11T23:23Z
lane: A
---
The 13 Comanche055 INTER-BANK_COMMUNICATION entries are native routines in the control-transfer ABI registry, and the CADR after every genuine `TC BANKCALL/IBNKCALL/POSTJUMP/USPRCADR` (354 sites) is a compiled constant. BANKCALL and IBNKCALL round trips go through the trampoline into a native callee and back. 1,416 cases and 12,826 strict and sanitizer checks match `cm_interbank_complete` fed the original rope words, and every fourth case matches the host digests in VICE. See `docs/CM-INTERBANK-TRANSFER.md`.
