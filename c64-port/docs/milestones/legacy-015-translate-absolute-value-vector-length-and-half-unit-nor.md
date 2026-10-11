---
id: legacy-015-translate-absolute-value-vector-length-and-half-unit-nor
title: Translate absolute value, vector length and half-unit normalization with signed-zero fidelity
seq: 15
---

ABS, ABVAL and UNIT now preserve raw length, half-unit vector, overflow, and scratch effects. All 50,736 complete 104-word assembly comparisons agree, including sixteen signed-zero regressions found by P66 composition. Host and sanitizer tests and all 118 full-state C64 checks pass. Full application binding and remaining flight routines are still in progress. See `docs/UNIT.md`.

