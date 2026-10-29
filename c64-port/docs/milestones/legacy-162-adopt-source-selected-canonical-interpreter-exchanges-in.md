---
id: legacy-162-adopt-source-selected-canonical-interpreter-exchanges-in
title: Adopt source-selected canonical interpreter exchanges in the native scheduler
seq: 162
---

Native scheduler adoption now preserves the exact source-selected CHANG2 context without repeating exchange or admitting pending IRQs. All 16,384 complete source transitions and 64 real three-overlay C64 cases are qualified; historical scheduler and complete P66 regressions pass. Remaining application bodies and live scheduling paths retain separate completion gates. See `docs/NATIVE-CANONICAL-EXCHANGE.md`.

