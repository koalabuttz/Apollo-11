---
id: legacy-159-qualify-exact-passive-original-fixed-word-storage-for-bo
title: Qualify exact passive original fixed word storage for both applications
seq: 159
---

Both original fixed-word datasets now have data-only native readers: 36 populated banks and four zero banks per mission. Strict and sanitizer audits exhaust all 65,536 reader arguments per link group. Stock C64 reads match all 81,920 individual words per mission across both IRQ states while preserving the canonical image and ROM mapping. The LM allocation-only disk retains every existing file; executable registry allocation and source caller controls remain separate work. See `docs/PASSIVE_FIXED_STORAGE.md`.

