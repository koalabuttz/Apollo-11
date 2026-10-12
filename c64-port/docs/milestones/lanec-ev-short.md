---
id: lanec-ev-short
title: Translate the short EXTENDED_VERBS verb bodies
date: 2026-10-12T21:00Z
lane: C
---
Native C for the bank 43 verb bodies of EXTENDED_VERBS that set flags, start jobs and tasks, or check a condition and hand over (Luminary099 V47-V49, V55-V58, V61, V62, V64-V67, V74-V77, V80-V83, V85, V89-V93, V95 and V96; the matching Comanche055 bodies with its CHKPOOH, EXDAPOFF, STABLISH, GOTOR21/23 and V52 code), plus SHOWSUM, translated block by block. 6,368 complete states are identical to the unchanged original programs in Virtual AGC, in strict and ASan/UBSan builds, and 245 of them (every entry, both missions) match the oracle on a VICE PAL C64. See `docs/LANEC-EV-SHORT.md`.
