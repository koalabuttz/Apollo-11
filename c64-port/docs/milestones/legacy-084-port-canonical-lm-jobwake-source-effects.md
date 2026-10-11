---
id: legacy-084-port-canonical-lm-jobwake-source-effects
title: Port canonical LM JOBWAKE source effects
seq: 84
---

The original JOBWAKE service preserves raw L, bank and scheduling effects in 8192 complete source states, including first wake, no match and both signed zeros. Strict host, sanitizer and 64 C64 comparisons pass. See `docs/LM-BASIC-WAKE.md`.

