---
id: legacy-085-port-real-makeplay-priority-and-wake-prelude
title: Port real MAKEPLAY priority and wake prelude
seq: 85
---

Native MAKEPLAY follows the actual PRIOCHNG, JOBWAKE, GOSLEEPS, GOPLAY and duplicate ENDOFJOB routes with source XCHTOEND replacement. Original comparisons cover 12288 complete states, matching sanitizer streams and 64 C64 cases; the sleep and resumed lifecycle remains separately qualified. See `docs/P66-MAKEPLAY.md`.

