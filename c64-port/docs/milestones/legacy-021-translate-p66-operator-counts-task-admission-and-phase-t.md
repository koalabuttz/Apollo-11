---
id: legacy-021-translate-p66-operator-counts-task-admission-and-phase-t
title: Translate P66 operator counts, task admission and phase-timer entry paths
seq: 21
---

DESCBITS, RODTASK and P66VERTA now follow original operator arithmetic, priority-22 admission, phase termination, delayed task scheduling and immediate RODCOMP entry with exact shared scratch. All 73,728 original-entry checkpoints agree; host, sanitizer and 128 full-state C64 checks pass. STRTP66 bias initialization, display admission and later continuations are separate ongoing milestones. See `docs/P66_STARTUP.md`.

