---
id: legacy-082-preserve-pending-newjob-during-real-regodspr-admission
title: Preserve pending NEWJOB during real REGODSPR admission
seq: 82
---

Actual REGODSPR NOVAC admission preserves or replaces the original pending priority request across 8192 complete source states, matching sanitizers and 64 C64 cases. No synthetic polling point or job-completion placeholder is introduced. See `docs/P66-RUNTIME-REGODSPR.md`.

