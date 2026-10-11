---
id: lm-interbank-transfer
title: Run Luminary INTER-BANK entries as native routines
date: 2026-10-11T23:23Z
lane: A
---
The 13 Luminary099 INTER-BANK_COMMUNICATION entries are native routines in the control-transfer ABI registry, and the CADR after every genuine `TC BANKCALL/IBNKCALL/POSTJUMP/USPRCADR` (448 sites) is a compiled constant. BANKCALL and IBNKCALL round trips run through the trampoline into a native callee and back via SWRETURN/ISWRETRN. 1,792 cases and 15,826 strict and sanitizer checks match `lm_interbank` fed the original rope words, and every fourth case matches the host digests in VICE. See `docs/LM-INTERBANK-TRANSFER.md`.
