---
id: legacy-160-run-original-comanche-fixed-memory-display-and-checksum
title: Run original Comanche fixed memory display and checksum call prefixes
seq: 160
---

Actual DSPFMEM and COMADRS lead-ins now read the entire 40,960-address original fixed-word domain through native SUPDACAL and return to genuine callers. 81,920 source sequences and 163,840 complete source/native/sanitized checkpoints pass; stock C64 verifies 64 sequences and 128 checkpoints with full-state samples. Passive data is separately exact-word qualified. Later octal display/checksum bodies and whole application acceptance remain pending. See `docs/COMANCHE_FIXED_CALLERS.md`.

